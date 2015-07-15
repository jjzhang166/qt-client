/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPoForm.h"

#include <metasql.h>

#include "errorReporter.h"

printPoForm::printPoForm(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("PO");
  setReportKey("pohead_id");

  _docinfoQueryString = "SELECT pohead_id      AS docid, pohead_id,"
                        "       pohead_number  AS docnumber,"
                        "       pohead_printed AS printed,"
                        "<? if exists('reportname') ?>"
                        "       (SELECT form_report_name"
                        "          FROM form"
                        "         WHERE ((form_name=<? value('reportname') ?>)"
                        "            AND (form_key='PO')))"
                        "<? else ?>"
                        "       COALESCE((SELECT MIN(form_report_name)"
                        "                   FROM form WHERE (form_key='PO')),"
                        "                'PurchaseOrder')"
                        "<? endif ?> AS reportname"
                        "  FROM pohead"
                        " WHERE (pohead_id=<? value('docid') ?>);" ;

  _report->populate( "SELECT form_id, form_name, form_name "
                     "FROM form "
                     "WHERE (form_key='PO') "
                     "ORDER BY form_name;" );

  connect(_po,    SIGNAL(newId(int, QString)), this, SLOT(sHandleNewOrderId()));
  connect(_report,         SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(this,     SIGNAL(finishedWithAll()), this, SLOT(sFinishedWithAll()));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printPoForm::~printPoForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printPoForm::languageChange()
{
  retranslateUi(this);
}

ParameterList printPoForm::getParamsDocList()
{
  ParameterList params = printSinglecopyDocument::getParamsDocList();
  if (_report->isValid())
    params.append("reportname", _report->code());

  return params;
}

void printPoForm::sFinishedWithAll()
{
  _po->setId(-1);
}

void printPoForm::sHandleButtons()
{
  setPrintEnabled(_po->isValid() && _report->isValid());
}

void printPoForm::sHandleNewOrderId()
{
  setId(_po->id());
  if (_po->isValid())
  {
    ParameterList params;
    params.append("docid", _po->id());
    MetaSQLQuery mql(_docinfoQueryString);
    XSqlQuery    qry = mql.toQuery(params);
    if (qry.first())
      _report->setCode(qry.value("reportname").toString());
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Getting Purchase Order Form"),
                                  qry, __FILE__, __LINE__))
      return;
  }
  sHandleButtons();
}

void printPoForm::sPopulate(XSqlQuery *docq)
{
  _po->setId(docq->value("docid").toInt());
  _report->setCode(docq->value("reportname").toString());
}
