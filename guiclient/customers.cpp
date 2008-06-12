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

#include "customers.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "customer.h"
#include "customerTypeList.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a customers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
customers::customers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_cust, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainCustomerMasters"))
  {
    connect(_cust, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_cust, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_cust, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_cust, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _cust->addColumn(tr("Type"),    _itemColumn,  Qt::AlignCenter );
  _cust->addColumn(tr("Number"),  _orderColumn, Qt::AlignCenter );
  _cust->addColumn(tr("Active"),  _ynColumn,    Qt::AlignCenter );
  _cust->addColumn(tr("Name"),    -1,           Qt::AlignLeft   );
  _cust->addColumn(tr("Address"), 175,          Qt::AlignLeft   );
  _cust->addColumn(tr("Phone #"), 100,          Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), SLOT(sFillList(int, bool)));

  sFillList(-1, FALSE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
customers::~customers()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void customers::languageChange()
{
  retranslateUi(this);
}

void customers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sEdit()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "edit");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sView()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "view");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void customers::sReassignCustomerType()
{
  ParameterList params;
  params.append("custtype_id", _cust->altId());

  customerTypeList *newdlg = new customerTypeList(this, "", TRUE);
  newdlg->set(params);
  int custtypeid = newdlg->exec();
  if ( (custtypeid != -1) && (custtypeid != XDialog::Rejected) )
  {
    q.prepare( "UPDATE custinfo "
               "SET cust_custtype_id=:custtype_id "
               "WHERE (cust_id=:cust_id);" );
    q.bindValue(":cust_id", _cust->id());
    q.bindValue(":custtype_id", custtypeid);
    q.exec();
    omfgThis->sCustomersUpdated(_cust->id(), TRUE);
  }
}

void customers::sDelete()
{
  q.prepare("SELECT deleteCustomer(:cust_id) AS result;");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
      QMessageBox::critical(this, tr("Cannot Delete Customer"),
			    storedProcErrorLookup("deleteCustomer", returnVal));
      return;
    }
    omfgThis->sCustomersUpdated(-1, TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customers::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Reassign Customer Type", this, SLOT(sReassignCustomerType()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void customers::sFillList(int pCustid, bool pLocal)
{
  q.prepare( "SELECT cust_id, cust_custtype_id,"
             "       COALESCE(custtype_code, :error), cust_number,"
             "       formatBoolYN(cust_active), cust_name, cust_address1, cust_phone "
             "FROM cust LEFT OUTER JOIN custtype ON (cust_custtype_id=custtype_id) "
             "ORDER BY cust_number;" );
  q.bindValue(":error", tr("Error"));
  q.exec();
 
  if (pLocal)
    _cust->populate(q, pCustid, TRUE);
  else
    _cust->populate(q, TRUE);
}

