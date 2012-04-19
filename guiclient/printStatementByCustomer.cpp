/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementByCustomer.h"

#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "errorReporter.h"
#include "../scriptapi/parameterlistsetup.h"

printStatementByCustomer::printStatementByCustomer(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _asOf->setDate(omfgThis->dbDate(), true);

  connect(_print,  SIGNAL(clicked()), this,   SLOT(sPrint()));
  connect(_close,  SIGNAL(clicked()), this,   SLOT(reject()));
  connect(_cust, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _captive = FALSE;

  _cust->setType(CLineEdit::ActiveCustomers);
  _cust->setFocus();
}

printStatementByCustomer::~printStatementByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementByCustomer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printStatementByCustomer::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

bool printStatementByCustomer::setParams(ParameterList &params)
{
  if (!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Customer Number"),
                         tr("<p>You must enter a valid Customer Number for "
                            "this Statement.") );
    _cust->setFocus();
    return false;
  }

  params.append("invoice",  tr("Invoice"));
  params.append("debit",    tr("Debit Memo"));
  params.append("credit",   tr("Credit Memo"));
  params.append("deposit",  tr("Deposit"));
  params.append("cust_id",  _cust->id());
  params.append("asofdate", _asOf->date());

  return true;
}

ParameterList printStatementByCustomer::getParams()
{
  ParameterList params;
  bool ret = setParams(params);
  params.append("checkParamsReturn", ret);

  return params;
}

void printStatementByCustomer::sPrint()
{
  ParameterList params;
  if(! setParams(params))
    return;

  MetaSQLQuery agem("SELECT * FROM araging (<? value('asofDate') ?>, true)"
                    " WHERE (araging_cust_id = <? value('cust_id') ?>);"); 
  XSqlQuery ageq = agem.toQuery(params);
  if(ageq.first())
  {
    MetaSQLQuery formm("SELECT findCustomerForm(<? value('cust_id') ?>, 'S')"
                       "    AS _reportname;");
    XSqlQuery formq = formm.toQuery(params);
    if (formq.first())
    {
      orReport report(formq.value("_reportname").toString(), params);
      if (report.isValid())
      {
        if (report.print())
          emit finishedPrinting(_cust->id());
      }
      else
      {
        report.reportError(this);
        reject();
      }

      if (_captive)
        accept();
      else
      {
        _close->setText(tr("&Close"));
        _cust->setId(-1);
        _cust->setFocus();
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Form"),
                                  formq, __FILE__, __LINE__))
      return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                ageq, __FILE__, __LINE__))
    return;
  else
    QMessageBox::warning( this, tr("No Statement to Print"),
                          tr("No statement is available for the specified "   
                                        "Customer and Asof Date.") );
}
