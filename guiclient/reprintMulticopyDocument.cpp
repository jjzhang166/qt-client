/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintMulticopyDocument.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "errorReporter.h"
#include "storedProcErrorLookup.h"

class reprintMulticopyDocumentPrivate : public Ui::reprintMulticopyDocument
{
  public:
    reprintMulticopyDocumentPrivate(::reprintMulticopyDocument *parent) :
      _mpIsInitialized(false),
      _parent(parent),
      _printer(0)
    {
      setupUi(_parent);
      _printer = new QPrinter(QPrinter::HighResolution);

      _print->setFocus();
    }

    ~reprintMulticopyDocumentPrivate()
    {
      if (_printer)
      {
        delete _printer;
        _printer = 0;
      }
    }

    QString                     _doctype;
    QString                     _doctypefull;
    bool                        _mpIsInitialized;
    ::reprintMulticopyDocument *_parent;
    QList<QVariant>             _printed;
    QPrinter                   *_printer;
    QString                     _reportKey;
};

reprintMulticopyDocument::reprintMulticopyDocument(QWidget    *parent,
                                                   const char *name,
                                                   bool        modal,
                                                   Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new reprintMulticopyDocumentPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sPopulate()));

  // this indirection allows scripts to replace core behavior - 14285
  connect(this, SIGNAL(timeToPrintOneDoc(XTreeWidgetItem*)),
          this, SLOT(sPrintOneDoc(XTreeWidgetItem*)));
}

reprintMulticopyDocument::reprintMulticopyDocument(QString numCopiesMetric,
                                                   QString watermarkMetric,
                                                   QString showPriceMetric,
                                                   QWidget    *parent,
                                                   const char *name,
                                                   bool        modal,
                                                   Qt::WFlags  fl)
  : XDialog(parent, name, modal, fl)
{
  _data = new reprintMulticopyDocumentPrivate(this);

  _data->_copies->setWatermarkMetric(watermarkMetric);
  _data->_copies->setShowPriceMetric(showPriceMetric);
  _data->_copies->setNumCopiesMetric(numCopiesMetric);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sPopulate()));

  // this indirection allows scripts to replace core behavior - 14285
  connect(this, SIGNAL(timeToPrintOneDoc(XTreeWidgetItem*)),
          this, SLOT(sPrintOneDoc(XTreeWidgetItem*)));
}

reprintMulticopyDocument::~reprintMulticopyDocument()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void reprintMulticopyDocument::languageChange()
{
  _data->retranslateUi(this);
}

enum SetResponse reprintMulticopyDocument::set(const ParameterList &pParams)
{
  XDialog::set(pParams);

  return NoError;
}

void reprintMulticopyDocument::sAddToPrintedList(XTreeWidgetItem *item)
{
  QVariant docid = item->id();
  if (!_data->_printed.contains(docid))
    _data->_printed.append(docid);
}

void reprintMulticopyDocument::sPrint()
{
  _data->_printed.clear();

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    message(tr("Printing %1 #%2")
            .arg(_data->_doctypefull, item->text("docnumber")));

    emit aboutToStart(item);
    emit timeToPrintOneDoc(item);

    message("");
  }

  orReport::endMultiPrint(_data->_printer);
  _data->_mpIsInitialized = false;

  if (_data->_printed.size() == 0)
    QMessageBox::information(this, tr("No Documents to Print"),
                             tr("There aren't any documents to print."));

  _data->_printed.clear();
  emit finishedWithAll();

  list()->clearSelection();
}

bool reprintMulticopyDocument::sPrintOneDoc(XTreeWidgetItem *item)
{
  QString  reportname = item->text("reportname");
  bool     printedOk  = false;

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
                          tr("<p>Cannot find form '%1' for %2 %3."
                             "It cannot be printed until the Form"
                             "Assignment is updated to remove references "
                             "to this Form or the Form is created.")
                           .arg(reportname, _data->_doctypefull,
                                item->text("docnumber")));
  else
  {
    for (int i = 0; i < _data->_copies->numCopies(); i++)
    {
      report.setParamList(getParamsOneCopy(i, item));
      if (! report.isValid())
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Invalid Parameters"),
                             tr("<p>Report '%1' cannot be run. Parameters "
                                 "are missing.").arg(reportname),
                             __FILE__, __LINE__);
        break;
      }
      else if (report.print(_data->_printer, ! _data->_mpIsInitialized))
      {
        _data->_mpIsInitialized = true;
        printedOk = true;
      }
      else
      {
        report.reportError(this);
        break;
      }
    }
  }

  if (printedOk)
  {
    emit finishedPrinting(item->id());
    sAddToPrintedList(item);
  }

  return printedOk;
}

void reprintMulticopyDocument::sPopulate()
{
  ParameterList getp = getParamsDocList();

  MetaSQLQuery getm(_docListQueryString);
  XSqlQuery    getq = getm.toQuery(getp);
  list()->populate(getq);
  ErrorReporter::error(QtCriticalMsg, this, tr("Getting Documents"),
                       getq, __FILE__, __LINE__);
}

XDocCopySetter *reprintMulticopyDocument::copies()
{
  return _data->_copies;
}

QString reprintMulticopyDocument::doctype()
{
  return _data->_doctype;
}

ParameterList reprintMulticopyDocument::getParamsDocList()
{
  ParameterList params;

  params.append(_data->_reportKey, list()->id());

  return params;
}

ParameterList reprintMulticopyDocument::getParamsOneCopy(int row,
                                                         XTreeWidgetItem *item)
{
  ParameterList params;
  params.append(_data->_reportKey,  item->id());
  params.append("showcosts", (_data->_copies->showCosts(row) ? "TRUE" : "FALSE"));
  params.append("watermark", _data->_copies->watermark(row));

  return params;
}

bool reprintMulticopyDocument::isOnPrintedList(const int docid)
{
  return _data->_printed.contains(docid);
}

XTreeWidget *reprintMulticopyDocument::list()
{
  return _data->_list;
}

QWidget *reprintMulticopyDocument::optionsWidget()
{
  return _data->_optionsFrame;
}

QString reprintMulticopyDocument::reportKey()
{
  return _data->_reportKey;
}

void reprintMulticopyDocument::setDoctype(QString doctype)
{
  _data->_doctype = doctype;
  if (doctype == "AR")
    _data->_doctypefull = tr("A/R Statement");
  else if (doctype == "CM")
    _data->_doctypefull = tr("Credit Memo");
  else if (doctype == "IN")
    _data->_doctypefull = tr("Invoice");
  else if (doctype == "L")
    _data->_doctypefull = tr("Pick List");
  else if (doctype == "P")
    _data->_doctypefull = tr("Packing List");
  else if (doctype == "PO")
    _data->_doctypefull = tr("Purchase Order");
  else if (doctype == "QT)")
    _data->_doctypefull = tr("Quote");
  else if (doctype == "SO")
    _data->_doctypefull = tr("Sales Order");
  else
    qWarning("printMulticopyDocument could not figure out doctypefull for %s",
             qPrintable(doctype));
}

void reprintMulticopyDocument::setNumCopiesMetric(QString metric)
{
  _data->_copies->setNumCopiesMetric(metric);
}

void reprintMulticopyDocument::setReportKey(QString key)
{
  _data->_reportKey = key;
}

void reprintMulticopyDocument::setShowPriceMetric(QString metric)
{
  _data->_copies->setShowPriceMetric(metric);
}

void reprintMulticopyDocument::setWatermarkMetric(QString metric)
{
  _data->_copies->setWatermarkMetric(metric);
}
