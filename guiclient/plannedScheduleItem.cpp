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

#include "plannedScheduleItem.h"

#include <QVariant>
#include <QMessageBox>

plannedScheduleItem::plannedScheduleItem(QWidget * parent, const char * name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pschheadid = -1;
  _pschitemid = -1;
  _warehousid = -1;

  _item->setType(ItemLineEdit::cActive | ItemLineEdit::cPlanningMPS);

  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

plannedScheduleItem::~plannedScheduleItem()
{
}

void plannedScheduleItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannedScheduleItem::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if(valid)
  {
    _pschheadid = param.toInt();

    // TODO: find and set the appropriate warehous_id value.
    q.prepare("SELECT pschhead_warehous_id "
              "  FROM pschhead "
              " WHERE (pschhead_id=:pschhead_id);");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
    if(q.first())
    {
      _warehousid = q.value("pschhead_warehous_id").toInt();
      _item->clearExtraClauseList();
      _item->addExtraClause(QString("(itemsite_warehous_id=%1)")
          .arg(_warehousid));
    }
  }

  param = pParams.value("pschitem_id", &valid);
  if(valid)
  {
    _pschitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      _mode = cNew;
    else if("copy" == mode)
    {
      _mode = cNew;
      _pschitemid=-1;
    }
    else if("edit" == mode)
      _mode = cEdit;
    else if("view" == mode)
    {
      _mode = cView;

      _item->setEnabled(false);
      _date->setEnabled(false);
      _qty->setEnabled(false);

      _save->hide();
    }
  }

  return NoError;
}

void plannedScheduleItem::sSave()
{
  if(cNew == _mode)
    q.prepare("INSERT INTO pschitem"
              "      (pschitem_pschhead_id,"
              "       pschitem_linenumber,"
              "       pschitem_itemsite_id,"
              "       pschitem_scheddate, pschitem_qty) "
              "VALUES(:pschhead_id,"
              "       COALESCE((SELECT MAX(pschitem_linenumber) FROM pschitem WHERE pschitem_pschhead_id=:pschhead_id),0)+10,"
              "       (SELECT itemsite_id FROM itemsite WHERE itemsite_item_id=:item_id AND itemsite_warehous_id=:warehous_id),"
              "       :scheddate, :qty);");
  else
    q.prepare("UPDATE pschitem"
              "   SET pschitem_itemsite_id=(SELECT itemsite_id FROM itemsite WHERE itemsite_item_id=:item_id AND itemsite_warehous_id=:warehous_id),"
              "       pschitem_scheddate=:scheddate,"
              "       pschitem_qty=:qty "
              " WHERE (pschitem_id=:pschitem_id); ");

  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":pschitem_id", _pschitemid);
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":scheddate", _date->date());
  q.bindValue(":qty", _qty->text().toDouble());

  if(!q.exec())
  {
    QMessageBox::warning( this, tr("Error Saving"),
      tr("An Error was encountered while trying to save the record.") );
    return;
  }

  accept();
}


void plannedScheduleItem::populate()
{
  q.prepare("SELECT pschitem_pschhead_id,"
            "       pschitem_itemsite_id,"
            "       pschitem_scheddate,"
            "       pschitem_qty,"
            "       pschhead_warehous_id "
            "  FROM pschhead JOIN pschitem ON (pschitem_pschhead_id=pschhead_id) "
            " WHERE (pschitem_id=:pschitem_id);");
  q.bindValue(":pschitem_id", _pschitemid);
  q.exec();
  if(q.first())
  {
    _pschheadid = q.value("pschitem_pschhead_id").toInt();
    _warehousid = q.value("pschhead_warehous_id").toInt();
    _item->clearExtraClauseList();
    _item->addExtraClause(QString("(itemsite_warehous_id=%1)")
        .arg(_warehousid));

    _item->setItemsiteid(q.value("pschitem_itemsite_id").toInt());
    _date->setDate(q.value("pschitem_scheddate").toDate());
    _qty->setText(q.value("pschitem_qty").toString());
  }
}

