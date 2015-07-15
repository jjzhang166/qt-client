/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printSinglecopyDocument.h"

#include <QMessageBox>
#include <QSqlRecord>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "errorReporter.h"

class printSinglecopyDocumentPrivate : public Ui::printSinglecopyDocument
{
  public:
    printSinglecopyDocumentPrivate(::printSinglecopyDocument *parent) :
      _alert(true),
      _captive(false),
      _docid(-1),
      _parent(parent),
      _printer(0),
      _mpIsInitialized(false)
    {
      setupUi(_parent);

      _printer = new QPrinter(QPrinter::HighResolution);

      _print->setFocus();
    }

    ~printSinglecopyDocumentPrivate()
    {
      if (_printer)
      {
        delete _printer;
        _printer = 0;
      }
    }

    bool                      _alert;
    bool                      _captive;
    int                       _docid;
    QString                   _doctype;
    QString                   _doctypefull;
    ::printSinglecopyDocument *_parent;
    QPrinter                 *_printer;
    bool                      _mpIsInitialized;
    QList<QVariant>           _printed;
    QString                   _reportKey;
};

printSinglecopyDocument::printSinglecopyDocument(QWidget    *parent,
                                               const char *name,
                                               bool        modal,
                                               Qt::WindowFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new printSinglecopyDocumentPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  // This indirection allows scripts to replace core behavior - 14285
  connect(this, SIGNAL(timeToPrintOneDoc(XSqlQuery*)),
          this, SLOT(sPrintOneDoc(XSqlQuery*)));
  connect(this, SIGNAL(timeToMarkOnePrinted(XSqlQuery*)),
          this, SLOT(sMarkOnePrinted(XSqlQuery*)));
}

printSinglecopyDocument::~printSinglecopyDocument()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void printSinglecopyDocument::languageChange()
{
  _data->retranslateUi(this);
}

enum SetResponse printSinglecopyDocument::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _data->_captive = true;

  QVariant param;
  bool     valid;

  if (! reportKey().isEmpty())
  {
    param = pParams.value(reportKey(), &valid);
    if (valid)
      setId(param.toInt());
  }

  param = pParams.value("docid", &valid);
  if (valid)
    setId(param.toInt());

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (pParams.inList("persistentPrint"))
  {
    _data->_alert = false;

    if (isSetup())
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printSinglecopyDocument::sAddToPrintedList(XSqlQuery *docq)
{
  QVariant docid = docq->value("docid");

  if (! _data->_printed.contains(docid))
    _data->_printed.append(docid);
}

void printSinglecopyDocument::sClearPrintedList()
{
  _data->_printed.clear();
}

bool printSinglecopyDocument::sMarkOnePrinted(XSqlQuery *docq)
{
  bool wasMarked = false;

  if (_markOnePrintedQry.isEmpty())
    wasMarked = true; // not really but a reflection of best effort
  else
  {
    ParameterList markp;
    markp.append("docid",     docq->value("docid"));
    markp.append("docnumber", docq->value("docnumber"));

    MetaSQLQuery markm(_markOnePrintedQry);
    XSqlQuery markq = markm.toQuery(markp);
    if (! ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                               markq, __FILE__, __LINE__))
      wasMarked = true;

    if (wasMarked && _data->_alert)
      emit docUpdated(docq->value("docid").toInt());
  }

  if (wasMarked)
    sAddToPrintedList(docq);

  return wasMarked;
}

void printSinglecopyDocument::sPrint()
{
  if (! isOkToPrint())
    return;

  bool mpStartedInitialized = isSetup();

  sClearPrintedList();

  MetaSQLQuery  docinfom(_docinfoQueryString);
  ParameterList alldocsp = getParamsDocList();
  XSqlQuery     docinfoq = docinfom.toQuery(alldocsp);
  while (docinfoq.next())
  {
    message(tr("Processing %1 #%2")
              .arg(_data->_doctypefull, docinfoq.value("docnumber").toString()));

    // This indirection allows scripts to replace core behavior - 14285
    emit aboutToStart(&docinfoq);
    emit timeToPrintOneDoc(&docinfoq);
    emit timeToMarkOnePrinted(&docinfoq);

    message("");
  }

  if (! mpStartedInitialized)
  {
    orReport::endMultiPrint(_data->_printer);
    setSetup(false);
  }

  if (_data->_printed.size() == 0)
    QMessageBox::information(this, tr("No Documents to Print"),
                             tr("There aren't any documents to print."));
  else if (! _markAllPrintedQry.isEmpty() &&
           QMessageBox::question(this, tr("Mark Documents as Printed?"),
                                 tr("<p>Did all of the documents print correctly?"),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
  {
    ParameterList allp;
    allp.append("printedDocs", QVariant(_data->_printed));
    MetaSQLQuery markAllPrintedm(_markAllPrintedQry);
    XSqlQuery markPrintedq = markAllPrintedm.toQuery(allp);
    ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                         markPrintedq, __FILE__, __LINE__); // don't return

    if (_data->_alert)
      emit docUpdated(-1);
  }

  sClearPrintedList();
  emit finishedWithAll();

  if (_data->_captive)
    accept();
  else
    clear();

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Cannot Print"),
                           docinfoq, __FILE__, __LINE__))
    return;
}

bool printSinglecopyDocument::sPrintOneDoc(XSqlQuery *docq)
{
  QString reportname = docq->value("reportname").toString();
  QString docnumber  = docq->value("docnumber").toString();
  bool    printedOk  = false;

  if (! isSetup())
  {
    bool userCanceled = false;
    if (orReport::beginMultiPrint(_data->_printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return false;
    }
  }

  orReport report(reportname);
  if (! report.isValid())
    QMessageBox::critical(this, tr("Cannot Find Form"),
                          tr("<p>Cannot find form '%1' for %2 %3. "
                             "It cannot be printed until the Form "
                             "Assignment is updated to remove references "
                             "to this Form or the Form is created.")
                           .arg(reportname, _data->_doctypefull, docnumber));
  else
  {
    report.setParamList(getParams(docq));
    if (! report.isValid())
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Invalid Parameters"),
                           tr("<p>Report '%1' cannot be run. Parameters "
                               "are missing.").arg(reportname),
                           __FILE__, __LINE__);
      printedOk = false;
    }
    else if (report.print(_data->_printer, ! isSetup()))
    {
      setSetup(true);
      printedOk = true;
    }
    else
    {
      report.reportError(this);
      printedOk = false;
    }
  }

  if (printedOk)
    emit finishedPrinting(docq->value("docid").toInt());

  return printedOk;
}

void printSinglecopyDocument::sPopulate()
{
  ParameterList getp = getParamsDocList();

  MetaSQLQuery getm(_docinfoQueryString);
  XSqlQuery getq = getm.toQuery(getp);
  if (getq.first())
    emit populated(&getq);
}

void printSinglecopyDocument::clear()
{
}

QString printSinglecopyDocument::doctype()
{
  return _data->_doctype;
}

ParameterList printSinglecopyDocument::getParamsDocList()
{
  ParameterList params;

  if (_data->_docid > 0)
  {
    params.append(reportKey(), _data->_docid);
    params.append("docid",     _data->_docid);
  }

  return params;
}

ParameterList printSinglecopyDocument::getParams(XSqlQuery *docq)
{
  ParameterList params;
  if (! reportKey().isEmpty() && docq)
  {
    QVariant docid = docq->record().contains("docid") ? docq->value("docid")
                                                      : docq->value(reportKey());

    params.append(reportKey(), docid);
    params.append("docid",     docid);
  }

  return params;
}

int printSinglecopyDocument::id()
{
  return _data->_docid;
}

bool printSinglecopyDocument::isOkToPrint()
{
  return true;
}

bool printSinglecopyDocument::isOnPrintedList(const int docid)
{
  return _data->_printed.contains(docid);
}

bool printSinglecopyDocument::isSetup()
{
  return _data->_mpIsInitialized;
}

QWidget *printSinglecopyDocument::optionsWidget()
{
  return _data->_optionsFrame;
}

QString printSinglecopyDocument::reportKey()
{
  return _data->_reportKey;
}

void printSinglecopyDocument::setDoctype(QString doctype)
{
  _data->_doctype = doctype;
  if (doctype == "AR")
    _data->_doctypefull = tr("A/R Statement");
  else if (doctype == "CM")
    _data->_doctypefull = tr("Return");
  else if (doctype == "IN")
    _data->_doctypefull = tr("Invoice");
  else if (doctype == "L")
    _data->_doctypefull = tr("Pick List");
  else if (doctype == "P")
    _data->_doctypefull = tr("Packing List");
  else if (doctype == "PO")
    _data->_doctypefull = tr("Purchase Order");
  else if (doctype == "QT")
    _data->_doctypefull = tr("Quote");
  else if (doctype == "SO")
    _data->_doctypefull = tr("Sales Order");
  else
    qWarning("printSinglecopyDocument could not figure out doctypefull for %s",
             qPrintable(doctype));
}

void printSinglecopyDocument::setId(int docid)
{
  if (_data->_docid != docid)
  {
    _data->_docid = docid;
    sPopulate();
    emit newId(_data->_docid);
  }
}

void printSinglecopyDocument::setPrintEnabled(bool enabled)
{
  _data->_print->setEnabled(enabled);
  _data->_print->setFocus();
}

void printSinglecopyDocument::setReportKey(QString key)
{
  _data->_reportKey = key;
}

void printSinglecopyDocument::setSetup(bool pSetup)
{
  _data->_mpIsInitialized = pSetup;
}
