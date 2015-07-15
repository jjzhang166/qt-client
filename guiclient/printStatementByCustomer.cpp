/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementByCustomer.h"

#include <QMessageBox>
#include <QVariant>

#include <metasql.h>

#include "errorReporter.h"

printStatementByCustomer::printStatementByCustomer(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());
  _asOf->setDate(omfgThis->dbDate(), true);
  _cust->setType(CLineEdit::ActiveCustomers);

  setDoctype("AR");
  setReportKey("cust_id");

  _docinfoQueryString = "SELECT cust_id     AS docid, cust_id,"
                        "       cust_number AS docnumber,"
                        "       false       AS printed,"
                        "       findCustomerForm(cust_id, 'S') AS reportname"
                        "  FROM custinfo"
                        " WHERE (cust_id=<? value('docid') ?>);" ;

  connect(_cust,          SIGNAL(valid(bool)), this, SLOT(setPrintEnabled(bool)));
  connect(_cust,           SIGNAL(newId(int)), this, SLOT(setId(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
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
  QVariant param;
  bool     valid;

  param = pParams.value("asofDate", &valid);
  if (valid)
    _asOf->setDate(param.toDate());

  return printSinglecopyDocument::set(pParams); // this does XDialog::set()
}

void printStatementByCustomer::clear()
{
  setId(-1);
  _cust->setId(-1);
  _cust->setFocus();
}

ParameterList printStatementByCustomer::getParams(XSqlQuery *docq)
{
  return getParams();
}

ParameterList printStatementByCustomer::getParams()
{
  ParameterList params;

  params.append("docid",    _cust->id());
  params.append("cust_id",  _cust->id());
  params.append("invoice",  tr("Invoice"));
  params.append("return",   tr("Return"));
  params.append("debit",    tr("Debit Memo"));
  params.append("credit",   tr("Credit Memo"));
  params.append("deposit",  tr("Deposit"));
  params.append("asofdate", _asOf->date());

  return params;
}

bool printStatementByCustomer::isOkToPrint()
{
  if (!_cust->isValid())
  {
    QMessageBox::warning(this, tr("Enter a Valid Customer Number"),
                         tr("<p>You must enter a valid Customer Number for "
                            "this Statement.") );
    _cust->setFocus();
    return false;
  }

  ParameterList params = getParams();
  if (params.value("checkParamsReturn") == false)
    return false;

  MetaSQLQuery agem("SELECT * FROM araging (<? value('asofDate') ?>, true)"
                    " WHERE (araging_cust_id = <? value('cust_id') ?>);"); 
  XSqlQuery ageq = agem.toQuery(params);
  if (ageq.first())
    ; // fall through - cust is OK
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                ageq, __FILE__, __LINE__))
    return false;
  else
  {
    QMessageBox::warning(this, tr("No Statement to Print"),
                         tr("<p>No statement is available for the specified "
                            "Customer and Asof Date.") );
    return false;
  }

  return true;
}

void printStatementByCustomer::sPopulate(XSqlQuery *docq)
{
  _cust->setId(docq->value("docid").toInt());
}
