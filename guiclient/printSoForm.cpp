/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printSoForm.h"

#include <metasql.h>

#include "errorReporter.h"

printSoForm::printSoForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());
  _so->setAllowedTypes(OrderLineEdit::Sales);

  setDoctype("SO");
  setReportKey("sohead_id");

  _docinfoQueryString = "SELECT cohead_id     AS docid, cohead_id,"
                        "       cohead_number AS docnumber,"
                        "       false         AS printed,"
                        "<? if exists('reportname') ?>"
                        "       (SELECT form_report_name"
                        "          FROM form"
                        "         WHERE ((form_name=<? value('reportname') ?>)"
                        "            AND (form_key='SO')))"
                        "<? else ?>"
                        "       COALESCE((SELECT MIN(form_report_name)"
                        "                   FROM form WHERE (form_key='SO')),"
                        "                'SalesOrderAcknowledgement')"
                        "<? endif ?> AS reportname"
                        "  FROM cohead"
                        " WHERE (cohead_id=<? value('docid') ?>);" ;

  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SO') "
                     "ORDER BY form_name;" );

  connect(_so,    SIGNAL(newId(int, QString)), this, SLOT(sHandleNewOrderId()));
  connect(_report,         SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(this,     SIGNAL(finishedWithAll()), this, SLOT(sFinishedWithAll()));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printSoForm::~printSoForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printSoForm::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printSoForm::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cohead_id", &valid);
  if (valid)
    setId(param.toInt());

  return printSinglecopyDocument::set(pParams);
}

ParameterList printSoForm::getParams(XSqlQuery *docq)
{
  ParameterList params = printSinglecopyDocument::getParams(docq);

  params.append("includeFormatted", true);
  params.append("excludeCancelled", true);
  params.append("reportname",       _report->code());

  return params;
}

ParameterList printSoForm::getParamsDocList()
{
  ParameterList params = printSinglecopyDocument::getParamsDocList();
  if (_report->isValid())
    params.append("reportname", _report->code());

  return params;
}

void printSoForm::sFinishedWithAll()
{
  _so->setId(-1);
}

void printSoForm::sHandleButtons()
{
  setPrintEnabled(_so->isValid() && _report->isValid());
}

void printSoForm::sHandleNewOrderId()
{
  setId(_so->id());
  if (_so->isValid())
  {
    ParameterList params;
    params.append("docid", _so->id());
    MetaSQLQuery mql(_docinfoQueryString);
    XSqlQuery    qry = mql.toQuery(params);
    if (qry.first())
      _report->setCode(qry.value("reportname").toString());
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Getting Sales Order Form"),
                                  qry, __FILE__, __LINE__))
      return;
  }
  sHandleButtons();
}

void printSoForm::sPopulate(XSqlQuery *docq)
{
  if (docq)
    _so->setId(docq->value("docid").toInt());
}
