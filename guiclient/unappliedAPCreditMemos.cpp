/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unappliedAPCreditMemos.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "applyAPCreditMemo.h"
#include "apOpenItem.h"
#include "mqlutil.h"
#include "errorReporter.h"

unappliedAPCreditMemos::unappliedAPCreditMemos(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainAPMemos"));

  connect(_apopen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _apopen->addColumn( tr("Doc. #"),       _itemColumn,     Qt::AlignLeft,   true,  "apopen_docnumber" );
  _apopen->addColumn( tr("Vendor"),       -1,              Qt::AlignLeft,   true,  "vendor"   );
  _apopen->addColumn( tr("Amount"),       _moneyColumn,    Qt::AlignRight,  true,  "apopen_amount"  );
  _apopen->addColumn( tr("Amount (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_amount"  );
  _apopen->addColumn( tr("Applied"),      _moneyColumn,    Qt::AlignRight,  true,  "apopen_paid"  );
  _apopen->addColumn( tr("Applied (%1)").arg(CurrDisplay::baseCurrAbbr()), _moneyColumn, Qt::AlignRight, false, "base_applied"  );
  _apopen->addColumn( tr("Balance"),      _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _apopen->addColumn( tr("Currency"),     _currencyColumn, Qt::AlignCenter, !omfgThis->singleCurrency(),  "currAbbr" );
  _apopen->addColumn( tr("Balance (%1)").arg(CurrDisplay::baseCurrAbbr()), _bigMoneyColumn, Qt::AlignRight, true, "basebalance");

  if (_privileges->check("ApplyAPMemos"))
    connect(_apopen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  connect(_vendorgroup, SIGNAL(updated()), this, SLOT(sFillList()));

  sFillList();
}

unappliedAPCreditMemos::~unappliedAPCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void unappliedAPCreditMemos::languageChange()
{
  retranslateUi(this);
}

bool unappliedAPCreditMemos::setParams(ParameterList &params)
{
  _vendorgroup->appendValue(params);
  return true;
}

void unappliedAPCreditMemos::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("UnappliedAPCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void unappliedAPCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode",    "new");
  params.append("docType", "creditMemo");
  if (_vendorgroup->isSelectedVend())
    params.append("vend_id", _vendorgroup->vendId());

  apOpenItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedAPCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apopen->id());

  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedAPCreditMemos::sFillList()
{
  XSqlQuery unappliedFillList;
  MetaSQLQuery mql = mqlLoad("unappliedAPMemos", "list");
  ParameterList params;
  if (! setParams(params))
    return;
  unappliedFillList = mql.toQuery(params);
  _apopen->populate(unappliedFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Unapplied AP Memo Information"),
                                unappliedFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void unappliedAPCreditMemos::sApply()
{
  ParameterList params;
  params.append("apopen_id", _apopen->id());

  applyAPCreditMemo newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}
