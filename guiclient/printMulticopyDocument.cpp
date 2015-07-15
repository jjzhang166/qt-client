/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printMulticopyDocument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "distributeInventory.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

class printMulticopyDocumentPrivate : public Ui::printMulticopyDocument
{
  public:
    printMulticopyDocumentPrivate(::printMulticopyDocument *parent,
                                  QString postPrivilege = QString()) :
      _alert(true),
      _captive(false),
      _docid(-1),
      _parent(parent),
      _postPrivilege(postPrivilege),
      _printer(0),
      _mpIsInitialized(false)
    {
      setupUi(_parent);

      _printer = new QPrinter(QPrinter::HighResolution);

      _print->setFocus();
    }

    ~printMulticopyDocumentPrivate()
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
    ::printMulticopyDocument *_parent;
    QString                   _postPrivilege;
    QPrinter                 *_printer;
    bool                      _mpIsInitialized;
    QList<QVariant>           _printed;
    QString                   _reportKey;
};

printMulticopyDocument::printMulticopyDocument(QWidget    *parent,
                                               const char *name,
                                               bool        modal,
                                               Qt::WindowFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new printMulticopyDocumentPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  // This indirection allows scripts to replace core behavior - 14285
  connect(this, SIGNAL(timeToPrintOneDoc(XSqlQuery*)),
          this, SLOT(sPrintOneDoc(XSqlQuery*)));
  connect(this, SIGNAL(timeToMarkOnePrinted(XSqlQuery*)),
          this, SLOT(sMarkOnePrinted(XSqlQuery*)));
  connect(this, SIGNAL(timeToPostOneDoc(XSqlQuery*)),
          this, SLOT(sPostOneDoc(XSqlQuery*)));

  _distributeInventory = false;
}

printMulticopyDocument::printMulticopyDocument(QString numCopiesMetric,
                                               QString watermarkMetric,
                                               QString showPriceMetric,
                                               QString postPrivilege,
                                               QWidget    *parent,
                                               const char *name,
                                               bool        modal,
                                               Qt::WindowFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new printMulticopyDocumentPrivate(this, postPrivilege);

  _data->_copies->setWatermarkMetric(watermarkMetric);
  _data->_copies->setShowPriceMetric(showPriceMetric);
  _data->_copies->setNumCopiesMetric(numCopiesMetric);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  // This indirection allows scripts to replace core behavior - 14285
  connect(this, SIGNAL(timeToPrintOneDoc(XSqlQuery*)),
          this, SLOT(sPrintOneDoc(XSqlQuery*)));
  connect(this, SIGNAL(timeToMarkOnePrinted(XSqlQuery*)),
          this, SLOT(sMarkOnePrinted(XSqlQuery*)));
  connect(this, SIGNAL(timeToPostOneDoc(XSqlQuery*)),
          this, SLOT(sPostOneDoc(XSqlQuery*)));

  _distributeInventory = false;

  setPostPrivilege(postPrivilege);
}

printMulticopyDocument::~printMulticopyDocument()
{
  if (_data->_captive)
  {
    orReport::endMultiPrint(_data->_printer);
  }

  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void printMulticopyDocument::languageChange()
{
  _data->retranslateUi(this);
}

enum SetResponse printMulticopyDocument::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _data->_captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value(reportKey(), &valid);
  if (valid)
    setId(param.toInt());

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

    if (_data->_mpIsInitialized)
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printMulticopyDocument::sAddToPrintedList(XSqlQuery *docq)
{
  QVariant docid = docq->value("docid");

  if (! _data->_printed.contains(docid))
    _data->_printed.append(docid);
}

bool printMulticopyDocument::sMarkOnePrinted(XSqlQuery *docq)
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

bool printMulticopyDocument::sPostOneDoc(XSqlQuery *docq)
{
  if (! _postQuery.isEmpty()    &&
      _data->_post->isChecked() &&
      isOnPrintedList(docq->value("docid").toInt()) &&
      ! docq->value("posted").toBool())
  {
    QString docnumber = docq->value("docnumber").toString();
    message(tr("Posting %1 #%2").arg(_data->_doctypefull, docnumber));

    ParameterList postp;
    postp.append("docid",     docq->value("docid"));
    postp.append("docnumber", docq->value("docnumber"));

    if (! _askBeforePostingQry.isEmpty())
    {
      MetaSQLQuery askm(_askBeforePostingQry);
      XSqlQuery askq = askm.toQuery(postp);
      if (ErrorReporter::error(QtCriticalMsg, this,
                               tr("Cannot Post %1").arg(docnumber),
                               askq, __FILE__, __LINE__))
        return false;
      else if (askq.value("ask").toBool() &&
               QMessageBox::question(this, tr("Post Anyway?"),
                                     _askBeforePostingMsg.arg(docnumber),
                                      QMessageBox::Yes,
                                      QMessageBox::No | QMessageBox::Default)
                  == QMessageBox::No)
        return false;
    }

    if (! _errCheckBeforePostingQry.isEmpty())
    {
      MetaSQLQuery errm(_errCheckBeforePostingQry);
      XSqlQuery errq = errm.toQuery(postp);
      if (ErrorReporter::error(QtCriticalMsg, this,
                               tr("Cannot Post %1").arg(docnumber),
                               errq, __FILE__, __LINE__))
        return false;
      else if (! errq.first() || ! errq.value("ok").toBool())
      {
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Cannot Post %1").arg(docnumber),
                             _errCheckBeforePostingMsg, __FILE__, __LINE__);
        return false;
      }
    }

    //TODO: find a way to do this without holding locks during user input
    XSqlQuery("BEGIN;");

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    MetaSQLQuery postm(_postQuery);
    XSqlQuery postq = postm.toQuery(postp);
    if (postq.first())
    {
      int result = postq.value("result").toInt();
      if (result < 0)
      {
        rollback.exec();
        ErrorReporter::error(QtCriticalMsg, this,
                             tr("Cannot Post %1").arg(docnumber),
                             storedProcErrorLookup(_postFunction, result),
                             __FILE__, __LINE__);
        return false;
      }
      if (_distributeInventory &&
          (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected))
      {
        rollback.exec();
        QMessageBox::information(this, tr("Posting Canceled"),
                                 tr("Transaction Canceled") );
        return false;
      }
    }
    else if (postq.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this,
                           tr("Cannot Post %1").arg(docnumber),
                           postq, __FILE__, __LINE__);
      return false;
    }

    XSqlQuery("COMMIT;");

    emit posted(_data->_docid);
  }

  return true;
}

void printMulticopyDocument::sPrint()
{
  if (! isOkToPrint())
    return;

  bool mpStartedInitialized = _data->_mpIsInitialized;

  _data->_printed.clear();

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
    emit timeToPostOneDoc(&docinfoq);

    message("");
  }

//  if (! mpStartedInitialized)
  if (!_data->_captive)
  {
    orReport::endMultiPrint(_data->_printer);
    _data->_mpIsInitialized = false;
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

  _data->_printed.clear();
  emit finishedWithAll();

  if (_data->_captive)
    accept();
  else
    clear();

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Cannot Print"),
                           docinfoq, __FILE__, __LINE__))
    return;
}

bool printMulticopyDocument::sPrintOneDoc(XSqlQuery *docq)
{
  QString reportname = docq->value("reportname").toString();
  QString docnumber  = docq->value("docnumber").toString();
  bool    printedOk  = false;

  if (! _data->_mpIsInitialized)
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
    for (int i = 0; i < _data->_copies->numCopies(); i++)
    {
      report.setParamList(getParamsOneCopy(i, docq));
      if (! report.isValid())
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Invalid Parameters"),
                             tr("<p>Report '%1' cannot be run. Parameters "
                                 "are missing.").arg(reportname),
                             __FILE__, __LINE__);
        printedOk = false;
        continue;
      }
      else if (report.print(_data->_printer, ! _data->_mpIsInitialized))
      {
        _data->_mpIsInitialized = true;
        printedOk = true;
      }
      else
      {
        report.reportError(this);
        printedOk = false;
        continue;
      }
    }
  }

  if (printedOk)
    emit finishedPrinting(docq->value("docid").toInt());

  return printedOk;
}

void printMulticopyDocument::populate()
{
  ParameterList getp = getParamsDocList();

  MetaSQLQuery getm(_docinfoQueryString);
  XSqlQuery getq = getm.toQuery(getp);
  if (getq.first())
  {
    if (getq.value("posted").toBool())
    {
      _data->_post->setChecked(false);
      _data->_post->hide();
    }
    else
      _data->_post->setVisible(! _data->_postPrivilege.isEmpty());
    emit populated(&getq);
  }
}

void printMulticopyDocument::clear()
{
}

XDocCopySetter *printMulticopyDocument::copies()
{
  return _data->_copies;
}

QString printMulticopyDocument::doctype()
{
  return _data->_doctype;
}

ParameterList printMulticopyDocument::getParamsDocList()
{
  ParameterList params;

  if (_data->_docid > 0)
  {
    params.append(_data->_reportKey, _data->_docid);
    params.append("docid",           _data->_docid);
  }

  return params;
}

ParameterList printMulticopyDocument::getParamsOneCopy(int row, XSqlQuery *qry)
{
  ParameterList params;
  params.append(_data->_reportKey, qry->value("docid"));
  params.append("showcosts", (_data->_copies->showCosts(row) ? "true" : "false"));
  params.append("watermark", _data->_copies->watermark(row));

  return params;
}

int printMulticopyDocument::id()
{
  return _data->_docid;
}

bool printMulticopyDocument::isOkToPrint()
{
  return true;
}

bool printMulticopyDocument::isOnPrintedList(const int docid)
{
  return _data->_printed.contains(docid);
}

bool printMulticopyDocument::isSetup()
{
  return _data->_mpIsInitialized;
}

QWidget *printMulticopyDocument::optionsWidget()
{
  return _data->_optionsFrame;
}

QString printMulticopyDocument::reportKey()
{
  return _data->_reportKey;
}

void printMulticopyDocument::setDoctype(QString doctype)
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
    qWarning("printMulticopyDocument could not figure out doctypefull for %s",
             qPrintable(doctype));
}

void printMulticopyDocument::setId(int docid)
{
  _data->_docid = docid;
  populate();
  emit newId(_data->_docid);
}

void printMulticopyDocument::setNumCopiesMetric(QString metric)
{
  _data->_copies->setNumCopiesMetric(metric);
}

void printMulticopyDocument::setPostPrivilege(QString priv)
{
  _data->_postPrivilege = priv;

  _data->_post->setVisible(! priv.isEmpty());
  _data->_post->setChecked(_privileges->check(priv));
  _data->_post->setEnabled(_privileges->check(priv));
}

void printMulticopyDocument::setReportKey(QString key)
{
  _data->_reportKey = key;
}

void printMulticopyDocument::setSetup(bool pSetup)
{
  _data->_mpIsInitialized = pSetup;
}

void printMulticopyDocument::setShowPriceMetric(QString metric)
{
  _data->_copies->setShowPriceMetric(metric);
}

void printMulticopyDocument::setWatermarkMetric(QString metric)
{
  _data->_copies->setWatermarkMetric(metric);
}
