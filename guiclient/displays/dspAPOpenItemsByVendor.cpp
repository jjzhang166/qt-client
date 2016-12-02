/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspAPOpenItemsByVendor.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "errorReporter.h"
#include "apOpenItem.h"
#include "applyAPCreditMemo.h"
#include "printApOpenItem.h"
#include "miscVoucher.h"
#include "voucher.h"

dspAPOpenItemsByVendor::dspAPOpenItemsByVendor(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspAPOpenItemsByVendor", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Open Payables"));
  setListLabel(tr("Payables"));
  setReportName("APOpenItemsByVendor");
  setMetaSQLOptions("apOpenItems", "detail");
  setUseAltId(true);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  list()->addColumn(tr("Doc. Type"),    _orderColumn,    Qt::AlignLeft,   true,  "f_doctype" );
  list()->addColumn(tr("Doc. #"),       _orderColumn,    Qt::AlignLeft,   true,  "apopen_docnumber"  );
  list()->addColumn(tr("Vendor#"),      _orderColumn,    Qt::AlignLeft,   true,  "vend_number"  );
  list()->addColumn(tr("Name"),         -1,              Qt::AlignLeft,   true,  "vend_name"  );
  list()->addColumn(tr("P/O #"),        _orderColumn,    Qt::AlignLeft,   true,  "apopen_ponumber"  );
  list()->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignLeft,   true,  "invoicenumber"  );
  list()->addColumn(tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter, false, "apopen_docdate" );
  list()->addColumn(tr("Due Date"),     _dateColumn,     Qt::AlignCenter, true,  "apopen_duedate" );
  list()->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "apopen_amount"  );
  list()->addColumn(tr("Paid"),         _moneyColumn,    Qt::AlignRight,  true,  "paid"  );
  list()->addColumn(tr("Balance"),      -1,              Qt::AlignRight,  !omfgThis->singleCurrency(),  "balance"  );
  list()->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignLeft,   !omfgThis->singleCurrency(),  "currAbbr"   );
  list()->addColumn(tr("Base Balance"), _moneyColumn,    Qt::AlignRight,  true,  "base_balance"  );
  list()->addColumn(tr("Status"),       _moneyColumn,    Qt::AlignCenter, false, "apopen_status"  );

  if (!omfgThis->singleCurrency())
  {
    XSqlQuery qq;
    qq.prepare("SELECT currConcat(baseCurrId()) AS currConcat;");
    qq.exec();
    QString currConcat;
    if (qq.first())
      currConcat = qq.value("currConcat").toString();
    else
      currConcat = tr("?????");
    list()->headerItem()->setText(12, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  if(_preferences->value("APAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

void dspAPOpenItemsByVendor::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspAPOpenItemsByVendor::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendorGroup->setVendId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("asofDate", &valid);
  if (valid)
  {
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(false);
  }

  param = pParams.value("useDocDate", &valid);
  if (valid)
    _useDocDate->setChecked(param.toBool());

  param = pParams.value("useDistDate", &valid);
  if (valid)
    _useDistDate->setChecked(param.toBool());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspAPOpenItemsByVendor::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  QString status(selected->text(1));
   
  QAction *menuItem;

  if (list()->currentItem()->text("f_doctype") == tr("Voucher"))
  {
    menuItem = pMenu->addAction(tr("View Voucher..."), this, SLOT(sViewVoucher()));
    menuItem->setEnabled(_privileges->check("ViewVouchers") || _privileges->check("MaintainVouchers"));
  }
  
  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("EditAPOpenItem"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  XSqlQuery menu;
  menu.prepare("SELECT apopen_status FROM apopen WHERE apopen_id=:apopen_id;");
  menu.bindValue(":apopen_id", list()->id());
  menu.exec();
  if (menu.first())
  {
    if(menu.value("apopen_status").toString() == "O")
    {
      menuItem = pMenu->addAction(tr("On Hold"), this, SLOT(sOnHold()));
      menuItem->setEnabled(_privileges->check("EditAPOpenItem"));
    }
    if(menu.value("apopen_status").toString() == "H") 
    {
      menuItem = pMenu->addAction(tr("Open"), this, SLOT(sOpen()));
      menuItem->setEnabled(_privileges->check("EditAPOpenItem"));
    }	
  }

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrintItem()));
  if (!_privileges->check("EditAPOpenItem"))
    menuItem->setEnabled(false);

  if (list()->currentItem()->text("f_doctype") == tr("Credit Memo"))
  {
    pMenu->addSeparator();
    menuItem = pMenu->addAction(tr("Apply Credit Memo..."), this, SLOT(sApplyAPOpenCM()));
    menuItem->setEnabled(_privileges->check("ApplyAPMemos"));
  }
}

void dspAPOpenItemsByVendor::sViewVoucher()
{
  int voheadid;
  int poheadid;
  XSqlQuery open;
  open.prepare("SELECT vohead_id, COALESCE(pohead_id, -1) AS pohead_id "
               "FROM vohead LEFT OUTER JOIN pohead ON (pohead_id=vohead_pohead_id) "
               "WHERE vohead_number=:vohead_number;");
  open.bindValue(":vohead_number", list()->currentItem()->text("apopen_docnumber"));
  open.exec();
  if (open.first())
  {
    voheadid = open.value("vohead_id").toInt();
    poheadid = open.value("pohead_id").toInt();
  }
  else
    return;
  
  if (!checkSitePrivs(voheadid))
    return;
  
  ParameterList params;
  params.append("mode", "view");
  params.append("vohead_id", voheadid);
  
  if (poheadid == -1)
  {
    miscVoucher *newdlg = new miscVoucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    voucher *newdlg = new voucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAPOpenItemsByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", list()->id());
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", list()->id());
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sApplyAPOpenCM()
{
  ParameterList params;
  params.append("apopen_id", list()->id());

  applyAPCreditMemo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAPOpenItemsByVendor::sPrintItem()
{
  ParameterList params;
  params.append("apopen_id", list()->id());

  printApOpenItem newdlg(this, "", true);
  if (newdlg.set(params) == NoError)
    newdlg.exec();
}

bool dspAPOpenItemsByVendor::setParams(ParameterList & params)
{
  _vendorGroup->appendValue(params);
  if (_docDate->isChecked())
    _dates->appendValue(params);
  else
  {
    params.append("startDueDate", _dates->startDate());
    params.append("endDueDate", _dates->endDate());
  }
  params.append("asofDate", _asOf->date());

  // have both in case we add a third option
  params.append("useDocDate",  QVariant(_useDocDate->isChecked()));
  params.append("useDistDate", QVariant(_useDistDate->isChecked()));

  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("voucher", tr("Voucher"));

  return true;
}

void dspAPOpenItemsByVendor::sOpen()
{
  XSqlQuery open;
  open.prepare("UPDATE apopen SET apopen_status = 'O' WHERE apopen_id=:apopen_id;");
  open.bindValue(":apopen_id", list()->id());
  open.exec();
  sFillList();
}

void dspAPOpenItemsByVendor::sOnHold()
{
  XSqlQuery selectpayment;
  selectpayment.prepare("SELECT * FROM apselect WHERE apselect_apopen_id = :apopen_id;");
  selectpayment.bindValue(":apopen_id", list()->id());
  selectpayment.exec();
  if (selectpayment.first())
  {
    QMessageBox::critical( this, tr("Can not change Status"), tr( "You cannot set this item as On Hold.\nThis Item is already selected for payment." ) );
    return;
  }
  
  XSqlQuery onhold;
  onhold.prepare("UPDATE apopen SET apopen_status = 'H' WHERE apopen_id=:apopen_id;");
  onhold.bindValue(":apopen_id", list()->id());
  onhold.exec();
  sFillList();
}

bool dspAPOpenItemsByVendor::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkVoucherSitePrivs(:voheadid) AS result;");
    check.bindValue(":voheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Voucher as "
                                 "it references a Site for which you have not "
                                 "been granted privileges.")) ;
        return false;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Privileges"),
                                  check, __FILE__, __LINE__))
      return false;
  }
  return true;
}
