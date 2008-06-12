/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspCostedSummarizedBOM.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

dspCostedSummarizedBOM::dspCostedSummarizedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Item Number"),    _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),    -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),            _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext'd Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Unit Cost"),      _costColumn,  Qt::AlignRight  );
  _bomitem->addColumn(tr("Ext'd Cost"),     _priceColumn, Qt::AlignRight  );
  _bomitem->setIndentation(10);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspCostedSummarizedBOM::~dspCostedSummarizedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCostedSummarizedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCostedSummarizedBOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Print;
  }

  return NoError;
}

void dspCostedSummarizedBOM::sPrint()
{
    ParameterList params;
    params.append("item_id", _item->id());
    params.append("revision_id", _revision->id());

    if(_showExpired->isChecked())
      params.append("expiredDays", _expiredDays->value());
	else
      params.append("expiredDays", 0);

    if(_showFuture->isChecked())
      params.append("futureDays", _effectiveDays->value());
	else
      params.append("futureDays", 0);

    if (_useStandardCosts->isChecked())
      params.append("useStandardCosts");

    if (_useActualCosts->isChecked())
      params.append("useActualCosts");

    orReport report("CostedSummarizedBOM", params);

    if (report.isValid())
      report.print();
    else
      report.reportError(this);
}

void dspCostedSummarizedBOM::sFillList()
{
  _bomitem->clear();

  if (_item->isValid())
  {
      QString sql( "SELECT -1, bomdata_item_number,"
                   "       bomdata_itemdescription,"
                   "       bomdata_uom_name,"
                   "       bomdata_qtyper," );

      if (_useActualCosts->isChecked())
        sql += " formatCost(bomdata_actunitcost) AS f_cost,"
               " formatCost(bomdata_actextendedcost) AS f_extcost ";
      else if (_useStandardCosts->isChecked())
        sql += " formatCost(bomdata_stdunitcost) AS f_cost,"
               " formatCost(bomdata_stdextendedcost) AS f_extcost ";

	  sql += "FROM summarizedBOM(:item_id, :revision_id, :expired, :effective) ";

      q.prepare(sql);
      q.bindValue(":item_id", _item->id());
	  q.bindValue(":revision_id", _revision->id());
      q.bindValue(":expired", _expiredDays->value());
      q.bindValue(":effective", _effectiveDays->value());
      q.exec();
      _bomitem->populate(q);
      for (int i = 0; i < _bomitem->topLevelItemCount(); i++)
	_bomitem->collapseItem(_bomitem->topLevelItem(i));
  }
}

void dspCostedSummarizedBOM::sFillList(int, bool)
{
}
