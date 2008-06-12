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

#include "dspBillingSelections.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include <openreports.h>
#include "selectOrderForBilling.h"
#include "printInvoices.h"
#include "postBillingSelections.h"

/*
 *  Constructs a dspBillingSelections as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBillingSelections::dspBillingSelections(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_postAll, SIGNAL(clicked()), this, SLOT(sPostAll()));
  connect(_cobill, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(valid(bool)), _cancel, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  statusBar()->hide();
  
  _cobill->addColumn(tr("Document #"), _orderColumn,  Qt::AlignLeft   );
  _cobill->addColumn(tr("Order #"),    _orderColumn,  Qt::AlignLeft   );
  _cobill->addColumn(tr("Cust. #"),    _itemColumn,   Qt::AlignLeft   );
  _cobill->addColumn(tr("Name"),        -1,           Qt::AlignLeft   );
  _cobill->addColumn(tr("Subtotal"),   _priceColumn,  Qt::AlignLeft, false );
  _cobill->addColumn(tr("Misc."),      _priceColumn,  Qt::AlignLeft, false ); 
  _cobill->addColumn(tr("Freight"),    _priceColumn,  Qt::AlignLeft, false );
  _cobill->addColumn(tr("Tax"),        _priceColumn,  Qt::AlignLeft, false );
  _cobill->addColumn(tr("Total"),      _priceColumn,  Qt::AlignLeft, false );
  _cobill->addColumn(tr("Payment rec'd"), _priceColumn, Qt::AlignLeft, false );

  if (_privileges->check("PostARDocuments"))
    connect(_cobill, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBillingSelections::~dspBillingSelections()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBillingSelections::languageChange()
{
    retranslateUi(this);
}

void dspBillingSelections::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  pMenu->insertItem("Cancel...", this, SLOT(sCancel()), 0);

  menuItem = pMenu->insertItem("Create Invoice", this, SLOT(sPost()), 0);
  if (!_privileges->check("PostARDocuments"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspBillingSelections::sFillList()
{
  q.exec( "SELECT cobmisc_id, cohead_id,"
          "       COALESCE(TEXT(cobmisc_invcnumber), '?') AS docnumber,"
          "       cohead_number, cust_number, cust_name,"
          "       formatmoney(sum(round(coitem_price*coitem_qtyord,2))),"
          "       formatmoney(cobmisc_misc),formatmoney(cobmisc_freight),"
          "       formatmoney(cobmisc_tax),"
          "       formatmoney(sum(round(coitem_price*coitem_qtyord,2))+cobmisc_misc+cobmisc_freight+cobmisc_tax),"
          "       formatmoney(cobmisc_payment)"
          "  FROM cobmisc, cohead, cust, coitem "
          " WHERE((cobmisc_cohead_id=cohead_id)"
          " AND (cohead_cust_id=cust_id)"
          "   AND (coitem_cohead_id=cohead_id)"
          "   AND (NOT cobmisc_posted)) "
          " GROUP BY cobmisc_id, cohead_id, cobmisc_invcnumber, cohead_number, cust_number,"
          "          cust_name, cobmisc_misc,cobmisc_freight,cobmisc_tax,cobmisc_payment "
          " ORDER BY docnumber, cohead_number;" );
  _cobill->populate(q, TRUE);
}

void dspBillingSelections::sPostAll()
{
  postBillingSelections newdlg(this, "", TRUE);
  newdlg.exec();
}

void dspBillingSelections::sPost()
{
  int soheadid = _cobill->altId();

  q.prepare("SELECT postBillingSelection(:cobmisc_id) AS result;");
  q.bindValue(":cobmisc_id", _cobill->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
      QMessageBox::critical( this, tr("Cannot Post one or more Invoices"),
                             tr( "The G/L Account Assignments for the selected Invoice are not configured correctly.\n"
                                 "Because of this, G/L Transactions cannot be posted for this Invoices.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may post this Invoice." ) );
    else if (result < 0)
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );

    omfgThis->sInvoicesUpdated(result, TRUE);
    omfgThis->sSalesOrdersUpdated(soheadid);
    omfgThis->sBillingSelectionUpdated(soheadid, TRUE);

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspBillingSelections::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspBillingSelections::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _cobill->altId());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspBillingSelections::sCancel()
{
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _cobill->altId());
    q.exec();

    sFillList();
  }
}


void dspBillingSelections::sPrint()
{
  orReport report("BillingSelections");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

