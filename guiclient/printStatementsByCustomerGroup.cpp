/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printStatementsByCustomerGroup.h"
#include <metasql.h>

#include <QMessageBox>
#include <QVariant>

#include "mqlutil.h"
#include "errorReporter.h"

printStatementsByCustomerGroup::printStatementsByCustomerGroup(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
                               : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());
  _asat->setDate(omfgThis->dbDate(), true);

  setDoctype("AR");
  setReportKey("custgrp_id");

  _docinfoQueryString = "SELECT custgrp_id       AS docid, custgrp_id,"
                        "       custgrp_name     AS docnumber,"
                        "       false            AS printed,"
                        "       'StatementbyCustomerGroup' AS reportname"
                        "  FROM custgrp"
                        " WHERE (custgrp_id=<? value('docid') ?>);";

  connect(_custgrp,        SIGNAL(newID(int)), this, SLOT(setId(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));

  setPrintEnabled(true); // or_custgrp->allowNull(true) and connect isValid to setPrintEnabled
}

printStatementsByCustomerGroup::~printStatementsByCustomerGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void printStatementsByCustomerGroup::languageChange()
{
  retranslateUi(this);
}

void printStatementsByCustomerGroup::clear()
{
  setId(-1);
  _custgrp->setId(-1);
  _custgrp->setFocus();
}

ParameterList printStatementsByCustomerGroup::getParams(XSqlQuery *docq)
{
  Q_UNUSED(docq);
  return getParams();
}

ParameterList printStatementsByCustomerGroup::getParams()
{
  ParameterList params;
  params.append("docid",      _custgrp->id());
  params.append("custgrp",    _custgrp->id());
  params.append("invoice",    tr("Invoice"));
  params.append("return",     tr("Sales Credit"));
  params.append("debitMemo",  tr("Debit Memo"));
  params.append("creditMemo", tr("Credit Memo"));
  params.append("deposit",    tr("Deposit"));
  params.append("asofdate",   _asat->date());

  return params;
}

void printStatementsByCustomerGroup::sPopulate(XSqlQuery *docq)
{
  _custgrp->setId(docq->value("docid").toInt());
}
