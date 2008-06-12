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

#include "dspCountSlipEditList.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "countTagList.h"
#include "countSlip.h"

/*
 *  Constructs a dspCountSlipEditList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCountSlipEditList::dspCountSlipEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_cntslip, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_countTagList, SIGNAL(clicked()), this, SLOT(sCountTagList()));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_postAll, SIGNAL(clicked()), this, SLOT(sPostAll()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

#ifndef Q_WS_MAC
  _countTagList->setMaximumWidth(25);
#endif

  _item->setReadOnly(TRUE);

  _cntslip->addColumn(tr("User"),         _dateColumn, Qt::AlignCenter );
  _cntslip->addColumn(tr("#"),            _itemColumn, Qt::AlignLeft   );
  _cntslip->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft   );
  _cntslip->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft   );
  _cntslip->addColumn(tr("Posted"),       _ynColumn,   Qt::AlignCenter );
  _cntslip->addColumn(tr("Entered"),      _itemColumn, Qt::AlignCenter );
  _cntslip->addColumn(tr("Slip Qty."),    _qtyColumn,  Qt::AlignRight  );

  if (_privileges->check("EnterCountSlips"))
  {
    connect(_cntslip, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(valid(bool)), _new, SLOT(setEnabled(bool)));
    connect(_cntslip, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }

  if (_privileges->check("DeleteCountSlips"))
    connect(_cntslip, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  if (_privileges->check("PostCountSlips"))
  {
    connect(_cntslip, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    _postAll->setEnabled(TRUE);
  }
    
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCountSlipEditList::~dspCountSlipEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCountSlipEditList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCountSlipEditList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
  {
    _cnttagid = param.toInt();
    populate();
  }

  return NoError;
}

void dspCountSlipEditList::sPrint()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);

  orReport report("CountSlipEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountSlipEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  menuItem = pMenu->insertItem("Edit Count Slip...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("EnterCountSlips"))
    pMenu->setItemEnabled(menuItem, FALSE);

  if (((XTreeWidgetItem *)pSelected)->altId() == 0)
  {
    menuItem = pMenu->insertItem("Post Count Slip...", this, SLOT(sPost()), 0);
    if (!_privileges->check("PostCountSlips"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspCountSlipEditList::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cnttag_id", _cnttagid);

  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountSlipEditList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntslip_id", _cntslip->id());

  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountSlipEditList::sDelete()
{
  if (QMessageBox::warning( this, tr("Delete Count Slip?"),
                            tr("Are you sure that you want to delete the selected Count Slip?"),
                            tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare( "DELETE FROM cntslip "
               " WHERE(cntslip_id=:cntslip_id);" );
    q.bindValue(":cntslip_id", _cntslip->id());
    q.exec();

    sFillList();
  }
}

void dspCountSlipEditList::sPost()
{
  q.prepare("SELECT postCountSlip(:cntslip_id);");
  q.bindValue(":cntslip_id", _cntslip->id());
  q.exec();

  sFillList();
}

void dspCountSlipEditList::sPostAll()
{
  q.prepare( "SELECT postCountSlip(cntslip_id) "
             "FROM cntslip "
             "WHERE ( (NOT cntslip_posted)"
             " AND (cntslip_cnttag_id=:cnttag_id) );" );
  q.bindValue(":cnttag_id", _cnttagid);
  q.exec();

  sFillList();
}

void dspCountSlipEditList::sCountTagList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);
  params.append("tagType", cUnpostedCounts);

  countTagList newdlg(this, "", TRUE);
  newdlg.set(params);
  _cnttagid = newdlg.exec();

  populate();
}

void dspCountSlipEditList::populate()
{
  q.prepare( "SELECT invcnt_tagnumber, invcnt_itemsite_id "
             "FROM invcnt "
             "WHERE (invcnt_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttagid);
  q.exec();
  if (q.first())
  {
    _countTagNumber->setText(q.value("invcnt_tagnumber").toString());
    _item->setItemsiteid(q.value("invcnt_itemsite_id").toInt());
  }

  sFillList();
}

void dspCountSlipEditList::sFillList()
{
  q.prepare( "SELECT cntslip_id,"
             "       CASE WHEN (cntslip_posted) THEN 1"
             "            ELSE 0"
             "       END,"
             "       getUsername(cntslip_user_id), cntslip_number,"
             "       CASE WHEN (cntslip_location_id=-1) THEN ''"
             "            ELSE formatLocationName(cntslip_location_id)"
             "       END,"
             "       cntslip_lotserial, formatBoolYN(cntslip_posted),"
             "       formatDateTime(cntslip_entered), formatQty(cntslip_qty) "
             "FROM cntslip, invcnt "
             "WHERE ( (cntslip_cnttag_id=invcnt_id)"
             " AND (NOT invcnt_posted)"
             " AND (invcnt_id=:cnttag_id) ) "
             "ORDER BY cntslip_number;" );
  q.bindValue(":cnttag_id", _cnttagid);
  q.exec();
  _cntslip->populate(q, TRUE);
}

