/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printToForm.h"

#include <metasql.h>

#include "errorReporter.h"

printToForm::printToForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("TO");
  setReportKey("tohead_id");

  _docinfoQueryString = "SELECT tohead_id      AS docid, tohead_id,"
                        "       tohead_number  AS docnumber,"
                        "<? if exists('reportname') ?>"
                        "       (SELECT form_report_name"
                        "          FROM form"
                        "         WHERE ((form_name=<? value('reportname') ?>)"
                        "            AND (form_key='TO')))"
                        "<? else ?>"
                        "       COALESCE((SELECT MIN(form_report_name)"
                        "                   FROM form WHERE (form_key='TO')),"
                        "                'TransferOrder')"
                        "<? endif ?> AS reportname"
                        "  FROM tohead"
                        " WHERE (tohead_id=<? value('docid') ?>);" ;

  _report->populate( "SELECT form_id, form_name, form_name "
                     "FROM form "
                     "WHERE (form_key='TO') "
                     "ORDER BY form_name;" );

  connect(_to,    SIGNAL(newId(int, QString)), this, SLOT(sHandleNewOrderId()));
  connect(_report,         SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(this,     SIGNAL(finishedWithAll()), this, SLOT(sFinishedWithAll()));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printToForm::~printToForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printToForm::languageChange()
{
  retranslateUi(this);
}

ParameterList printToForm::getParamsDocList()
{
  ParameterList params = printSinglecopyDocument::getParamsDocList();
  if (_report->isValid())
    params.append("reportname", _report->code());

  return params;
}

void printToForm::sFinishedWithAll()
{
  _to->setId(-1);
}

void printToForm::sHandleButtons()
{
  setPrintEnabled(_to->isValid() && _report->isValid());
}

void printToForm::sHandleNewOrderId()
{
  setId(_to->id());
  if (_to->isValid())
  {
    ParameterList params;
    params.append("docid", _to->id());
    MetaSQLQuery mql(_docinfoQueryString);
    XSqlQuery    qry = mql.toQuery(params);
    if (qry.first())
      _report->setCode(qry.value("reportname").toString());
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Getting Transfer Order Form"),
                                  qry, __FILE__, __LINE__))
      return;
  }
  sHandleButtons();
}

void printToForm::sPopulate(XSqlQuery *docq)
{
  _to->setId(docq->value("docid").toInt());
  _report->setCode(docq->value("reportname").toString());
}
