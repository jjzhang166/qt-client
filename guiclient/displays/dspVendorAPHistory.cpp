/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspVendorAPHistory.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <xdateinputdialog.h>

#include "apOpenItem.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "dspGLSeries.h"
#include "errorReporter.h"

dspVendorAPHistory::dspVendorAPHistory(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspVendorAPHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Vendor History"));
  setReportName("VendorAPHistory");
  setMetaSQLOptions("vendorAPHistory", "detail");
  setUseAltId(true);

  connect(_searchInvoiceNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchInvoiceNum()));

  list()->setRootIsDecorated(true);
  list()->addColumn(tr("Open"),         _dateColumn,     Qt::AlignCenter, true,  "f_open" );
  list()->addColumn(tr("Doc. Type"),    _itemColumn,     Qt::AlignCenter, true,  "documenttype" );
  list()->addColumn(tr("Doc. #"),       _orderColumn,    Qt::AlignRight,  true,  "docnumber"  );
  list()->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignRight,  true,  "invoicenumber"  );
  list()->addColumn(tr("P/O #"),        _orderColumn,    Qt::AlignRight,  true,  "ponumber"  );
  list()->addColumn(tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter, true,  "docdate" );
  list()->addColumn(tr("Due Date"),     _dateColumn,     Qt::AlignCenter, true,  "duedate" );
  list()->addColumn(tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  list()->addColumn(tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, false, "base_amount"  );
  list()->addColumn(tr("Applied (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, false, "base_applied"  );
  list()->addColumn(tr("Balance"),      _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  list()->addColumn(tr("Currency"),     _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  list()->addColumn(tr("Base Balance"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_balance"  );
  list()->setPopulateLinear();

}

void dspVendorAPHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspVendorAPHistory::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspVendorAPHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  QAction *menuItem;

  XTreeWidgetItem * item = (XTreeWidgetItem*)pSelected;
  if (item->id() != -1)
  {
    menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
    menuItem->setEnabled(_privileges->check("EditAPOpenItem"));

    pMenu->addAction(tr("View A/P Open..."), this, SLOT(sView()));

    menuItem = pMenu->addAction(tr("View G/L Series..."), this, SLOT(sViewGLSeries()));
    menuItem->setEnabled(_privileges->check("ViewGLTransactions"));

    if(item->altId() == -1 && item->text(1)==tr("Voucher"))
    {
      menuItem = pMenu->addAction(tr("View Voucher..."), this, SLOT(sViewVoucher()));
      menuItem->setEnabled(_privileges->check("ViewVouchers") || _privileges->check("MaintainVouchers"));

      if(item->rawValue("amount")==item->rawValue("balance"))
      {
        pMenu->addSeparator();
  
        menuItem = pMenu->addAction(tr("Void"), this, SLOT(sVoidVoucher()));
        menuItem->setEnabled(_privileges->check("VoidPostedVouchers"));
      }
    } 
  }
}

void dspVendorAPHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  if (list()->altId() == -1)
    params.append("apopen_id", list()->id());
  else
    params.append("apopen_id", list()->altId());

  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

void dspVendorAPHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");
  if (list()->altId() == -1)
    params.append("apopen_id", list()->id());
  else
    params.append("apopen_id", list()->altId());

  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspVendorAPHistory::sFillList()
{
  display::sFillList();

  sSearchInvoiceNum();
}

bool dspVendorAPHistory::setParams(ParameterList &params)
{
  if (isVisible())
  {
    if (!_vend->isValid())
    {
      QMessageBox::warning( this, tr("Select Vendor"),
                            tr("Please select a valid Vendor.") );
      _vend->setFocus();
      return false;
    }

    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return false;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return false;
    }
  }

  params.append("creditMemo", tr("Credit Memo"));
  params.append("debitMemo", tr("Debit Memo"));
  params.append("check", tr("Check"));
  params.append("voucher", tr("Voucher"));
  params.append("other", tr("Other"));
  params.append("vend_id", _vend->id());
  _dates->appendValue(params);

  return true;
}

void dspVendorAPHistory::sSearchInvoiceNum()
{
  QString sub = _searchInvoiceNum->text().trimmed();
  if(sub.isEmpty())
    return;

  XTreeWidgetItem *item     = 0;
  XTreeWidgetItem *foundSub = 0;
  for (int i = 0; i < list()->topLevelItemCount(); i++)
  {
    item = list()->topLevelItem(i);
    if (item->text(3) == sub)
    {
      foundSub = item;
      break;
    }
    else if (foundSub == 0 && item->text(3).startsWith(sub))
      foundSub = item;
  }
  if (foundSub)
  {
    list()->setCurrentItem(foundSub);
    list()->scrollToItem(foundSub);
  }
}

void dspVendorAPHistory::sVoidVoucher()
{
  XSqlQuery dspVoidVoucher;
  dspVoidVoucher.prepare("SELECT voidApopenVoucher(:apopen_id, :voidDate) AS result;");
  XDateInputDialog newdlg(this, "", true);
  ParameterList params;
  params.append("label", tr("On what date did you void the Voucher?"));
  params.append("default", list()->rawValue("docdate"));
  newdlg.set(params);
  int returnVal = newdlg.exec();
  if (returnVal == XDialog::Accepted)
  {
    QDate voidDate = newdlg.getDate();
    dspVoidVoucher.bindValue(":apopen_id", list()->id());
    dspVoidVoucher.bindValue(":voidDate", voidDate);
    dspVoidVoucher.exec();

    if(dspVoidVoucher.first())
    {
      if(dspVoidVoucher.value("result").toInt() < 0)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Voucher Information"),
                             dspVoidVoucher, __FILE__, __LINE__);
      else
        sFillList();
    }
    else
      ErrorReporter::error(QtCriticalMsg, this, tr("Voiding Voucher"),
                           dspVoidVoucher, __FILE__, __LINE__);
  }

}

void dspVendorAPHistory::sViewVoucher()
{
  XSqlQuery dspViewVoucher;
  dspViewVoucher.prepare("SELECT vohead_id, COALESCE(pohead_id, -1) AS pohead_id"
            "  FROM apopen, vohead LEFT OUTER JOIN pohead ON (vohead_pohead_id=pohead_id)"
            " WHERE((vohead_number=apopen_docnumber)"
            "   AND (apopen_id=:apopen_id));");
  dspViewVoucher.bindValue(":apopen_id", list()->id());
  dspViewVoucher.exec();
  if(dspViewVoucher.first())
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vohead_id", dspViewVoucher.value("vohead_id").toInt());
  
    if (dspViewVoucher.value("pohead_id").toInt() == -1)
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
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Voucher Information"),
                       dspViewVoucher, __FILE__, __LINE__);
}

void dspVendorAPHistory::sViewGLSeries()
{
  XSqlQuery dspViewGLSeries;
  dspViewGLSeries.prepare("SELECT apopen_docdate, apopen_journalnumber"
            "  FROM apopen"
            " WHERE(apopen_id=:apopen_id);");
  dspViewGLSeries.bindValue(":apopen_id", list()->id());
  dspViewGLSeries.exec();
  if(dspViewGLSeries.first())
  {
    ParameterList params;
    params.append("startDate", dspViewGLSeries.value("apopen_docdate").toDate());
    params.append("endDate", dspViewGLSeries.value("apopen_docdate").toDate());
    params.append("journalnumber", dspViewGLSeries.value("apopen_journalnumber").toInt());
  
    dspGLSeries *newdlg = new dspGLSeries();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving GL Series Information"),
                       dspViewGLSeries, __FILE__, __LINE__);
}

