/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unappliedARCreditMemos.h"

#include <QMenu>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include "applyARCreditMemo.h"
#include "arOpenItem.h"
#include "errorReporter.h"
#include "mqlutil.h"

unappliedARCreditMemos::unappliedARCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_aropen, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));

  _new->setEnabled(_privileges->check("MaintainARMemos"));

  connect(_aropen, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  _aropen->addColumn( tr("Doc. #"),   _itemColumn,     Qt::AlignCenter, true,  "aropen_docnumber" );
  _aropen->addColumn( tr("Cust. #"),  _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  _aropen->addColumn( tr("Customer"), -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _aropen->addColumn( tr("Amount"),   _moneyColumn,    Qt::AlignRight,  true,  "aropen_amount"  );
  _aropen->addColumn( tr("Applied"),  _moneyColumn,    Qt::AlignRight,  true,  "applied"  );
  _aropen->addColumn( tr("Balance"),  _moneyColumn,    Qt::AlignRight,  true,  "balance"  );
  _aropen->addColumn( tr("Currency"), _currencyColumn, Qt::AlignLeft,   true,  "currAbbr" );

  if (omfgThis->singleCurrency())
    _aropen->hideColumn(5);

  if (_privileges->check("ApplyARMemos"))
    connect(_aropen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
unappliedARCreditMemos::~unappliedARCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void unappliedARCreditMemos::languageChange()
{
  retranslateUi(this);
}

void unappliedARCreditMemos::sPrint()
{
  ParameterList params;
  params.append("isReport", true);

  orReport report("UnappliedARCreditMemos", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


void unappliedARCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("docType", "creditMemo");

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void unappliedARCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());

  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void unappliedARCreditMemos::sPopulateMenu( QMenu * )
{
}

void unappliedARCreditMemos::sFillList()
{
  MetaSQLQuery mql = mqlLoad("arCreditMemos", "unapplied");
  
  ParameterList params;
  
  XSqlQuery qry = mql.toQuery(params);
  _aropen->populate(qry);
  if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void unappliedARCreditMemos::sApply()
{
  ParameterList params;
  params.append("aropen_id", _aropen->id());

  applyARCreditMemo newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

