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

#include "itemPricingSchedules.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include "itemPricingSchedule.h"

/*
 *  Constructs a itemPricingSchedules as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemPricingSchedules::itemPricingSchedules(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_ipshead, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));

  statusBar()->hide();
  
  _ipshead->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft   );
  _ipshead->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _ipshead->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter );
  _ipshead->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter );

  if (_privileges->check("MaintainPricingSchedules"))
  {
    connect(_ipshead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_ipshead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();

  _searchFor->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemPricingSchedules::~itemPricingSchedules()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemPricingSchedules::languageChange()
{
    retranslateUi(this);
}

void itemPricingSchedules::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemPricingSchedule newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    sFillList(result);
}

void itemPricingSchedules::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipshead_id", _ipshead->id());

  itemPricingSchedule newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    sFillList(result);
}

void itemPricingSchedules::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("ipshead_id", _ipshead->id());

  itemPricingSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

void itemPricingSchedules::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ipshead_id", _ipshead->id());

  itemPricingSchedule newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemPricingSchedules::sDelete()
{
  q.prepare( "SELECT ipsass_id "
             "FROM ipsass "
             "WHERE (ipsass_ipshead_id=:ipshead_id) "
             "LIMIT 1;" );
  q.bindValue(":ipshead_id", _ipshead->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Pricing Schedule"),
                           tr( "The selected Pricing Schedule cannot be deleted as it has been assign to one or more Customers.\n"
                               "You must delete these assignments before you may delete the selected Pricing Schedule." ) );
    return;
  }

  q.prepare( "SELECT sale_id "
             "FROM sale "
             "WHERE (sale_ipshead_id=:ipshead_id) "
             "LIMIT 1;" );
  q.bindValue(":ipshead_id", _ipshead->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Pricing Schedule"),
                           tr( "The selected Pricing Schedule cannot be deleted as it has been assign to one or more Sales.\n"
                               "You must delete these assignments before you may delete the selected Pricing Schedule." ) );
    return;
  }

  q.prepare( "DELETE FROM ipsitem "
             "WHERE (ipsitem_ipshead_id=:ipshead_id); "
             "DELETE FROM ipsprodcat "
             "WHERE (ipsprodcat_ipshead_id=:ipshead_id); "
             "DELETE FROM ipshead "
             "WHERE (ipshead_id=:ipshead_id);" );
  q.bindValue(":ipshead_id", _ipshead->id());
  q.exec();

  sFillList();
}
void itemPricingSchedules::sFillList()
{
  sFillList(-1);
}

void itemPricingSchedules::sFillList(int pIpsheadid)
{
  QString sql( "SELECT ipshead_id, ipshead_name, ipshead_descrip,"
               "       formatDate(ipshead_effective, :always),"
               "       formatDate(ipshead_expires, :never) "
               "FROM ipshead " );

  if (!_showExpired->isChecked())
  {
    sql += "WHERE ( (ipshead_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (ipshead_effective <= CURRENT_DATE) ) ";
    else
      sql += " ) ";
  }
  else if (!_showFuture->isChecked())
    sql += "WHERE (ipshead_effective <= CURRENT_DATE) ";

  sql += "ORDER BY ipshead_name, ipshead_effective;";

  q.prepare(sql);
  q.bindValue(":always", tr("Always"));
  q.bindValue(":never", tr("Never"));
  q.exec();

  if (pIpsheadid == -1)
    _ipshead->populate(q);
  else
    _ipshead->populate(q, pIpsheadid);
}

void itemPricingSchedules::sSearch( const QString & pTarget)
{
  _ipshead->clearSelection();
  int i;
  for (i = 0; i < _ipshead->topLevelItemCount(); i++)
  {
    if (_ipshead->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _ipshead->topLevelItemCount())
  {
    _ipshead->setCurrentItem(_ipshead->topLevelItem(i));
    _ipshead->scrollToItem(_ipshead->topLevelItem(i));
  }
}
