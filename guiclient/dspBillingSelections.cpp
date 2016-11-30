/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBillingSelections.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include "selectOrderForBilling.h"
#include "printInvoices.h"
#include "createInvoices.h"

#include "errorReporter.h"
#include "storedProcErrorLookup.h"

dspBillingSelections::dspBillingSelections(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

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

  _cobill->addColumn(tr("Order #"),       _orderColumn,  Qt::AlignLeft,   true,  "cohead_number"   );
  _cobill->addColumn(tr("Cust. #"),       _itemColumn,   Qt::AlignLeft,   true,  "cust_number"   );
  _cobill->addColumn(tr("Name"),          -1,            Qt::AlignLeft,   true,  "cust_name"   );
  _cobill->addColumn(tr("Subtotal"),      _moneyColumn,  Qt::AlignLeft,   false, "subtotal" );
  _cobill->addColumn(tr("Misc."),         _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_misc" ); 
  _cobill->addColumn(tr("Freight"),       _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_freight" );
  _cobill->addColumn(tr("Tax"),           _moneyColumn,  Qt::AlignLeft,   true, "cobmisc_tax" );
  _cobill->addColumn(tr("Total"),         _moneyColumn,  Qt::AlignLeft,   true, "total" );
  _cobill->addColumn(tr("Payment Rec'd"), _bigMoneyColumn,  Qt::AlignLeft,   true, "cobmisc_payment" );

  if (_privileges->check("PostARDocuments"))
    connect(_cobill, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

dspBillingSelections::~dspBillingSelections()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspBillingSelections::languageChange()
{
    retranslateUi(this);
}

void dspBillingSelections::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  pMenu->addAction("Edit...", this, SLOT(sEdit()));
  pMenu->addAction("Cancel...", this, SLOT(sCancel()));

  menuItem = pMenu->addAction("Create Invoice", this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostARDocuments"));
}

void dspBillingSelections::sFillList()
{
  XSqlQuery dspFillList;
  MetaSQLQuery mql = mqlLoad("billingSelections", "detail");
  ParameterList params;
  dspFillList = mql.toQuery(params);
  _cobill->populate(dspFillList);
}

void dspBillingSelections::sPostAll()
{
  createInvoices newdlg(this, "", true);
  newdlg.exec();
}

void dspBillingSelections::sPost()
{
  XSqlQuery dspPost;
  int soheadid = -1;
  dspPost.prepare("SELECT cobmisc_cohead_id AS sohead_id "
            "FROM cobmisc "
            "WHERE (cobmisc_id = :cobmisc_id)");
  dspPost.bindValue(":cobmisc_id", _cobill->id());
  dspPost.exec();
  if (dspPost.first())
  {
    soheadid = dspPost.value("sohead_id").toInt();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Billing Selection(s)"),
                                dspPost, __FILE__, __LINE__))
  {
    return;
  }

  dspPost.prepare("SELECT createInvoice(:cobmisc_id) AS result;");
  dspPost.bindValue(":cobmisc_id", _cobill->id());
  dspPost.exec();
  if (dspPost.first())
  {
    int result = dspPost.value("result").toInt();
    if (result < 0) {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Billing Selection(s)"),
                               storedProcErrorLookup("createInvoice", result),
                               __FILE__, __LINE__);
      return;
     }

    omfgThis->sInvoicesUpdated(result, true);
    omfgThis->sSalesOrdersUpdated(soheadid);
    omfgThis->sBillingSelectionUpdated(soheadid, true);

    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Billing Selection(s)"),
                                dspPost, __FILE__, __LINE__))
  {
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
  XSqlQuery dspEdit;
  ParameterList params;
  params.append("mode", "edit");

  dspEdit.prepare("SELECT cobmisc_cohead_id AS sohead_id "
            "FROM cobmisc "
            "WHERE (cobmisc_id = :cobmisc_id)");
  dspEdit.bindValue(":cobmisc_id", _cobill->id());
  dspEdit.exec();
  if (dspEdit.first())
  {
    int sohead_id = dspEdit.value("sohead_id").toInt();
    params.append("sohead_id", sohead_id);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Billing Selection"),
                                dspEdit, __FILE__, __LINE__))
  {
    return;
  }

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspBillingSelections::sCancel()
{
  XSqlQuery dspCancel;
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    dspCancel.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_id=:cobmisc_id);" );
    dspCancel.bindValue(":cobmisc_id", _cobill->id());
    dspCancel.exec();

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

