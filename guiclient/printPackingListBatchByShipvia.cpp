/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPackingListBatchByShipvia.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "mqlutil.h"
#include "errorReporter.h"

printPackingListBatchByShipvia::printPackingListBatchByShipvia(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulateShipVia()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);

  sPopulateShipVia();
}

printPackingListBatchByShipvia::~printPackingListBatchByShipvia()
{
    // no need to delete child widgets, Qt does it all for us
}

void printPackingListBatchByShipvia::languageChange()
{
    retranslateUi(this);
}

void printPackingListBatchByShipvia::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool     setupPrinter = true;

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start and End Date"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  XSqlQuery prtd;
  QString prts("UPDATE pack SET pack_printed=true"
               " WHERE ((pack_head_id=<? value('head_id') ?>) "
	       "   AND  (pack_head_type=<? value('head_type') ?>)"
	       "<? if exists('shiphead_id') ?>"
	       "   AND  (pack_shiphead_id=<? value('shiphead_id') ?>)"
	       "<? else ?>"
	       "   AND (pack_shiphead_id IS NULL)"
	       "<? endif ?>"
	       ");" );


  XSqlQuery packq;
  ParameterList params;
  _dates->appendValue(params);
  _warehouse->appendValue(params);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  if (_shipvia->isValid())
    params.append("shipvia", _shipvia->currentText());
  MetaSQLQuery packm = mqlLoad("packingListBatchByShipVia", "print");
  packq = packm.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List Batch"),
                                packq, __FILE__, __LINE__))
  {
    return;
  }

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Could not initialize printing system "
                            "for multiple reports. ").arg(windowTitle()),__FILE__,__LINE__);
    return;
  }
  while (packq.next())
  {
    ParameterList params;
    params.append("head_id",   packq.value("pack_head_id").toInt());
    params.append("head_type", packq.value("pack_head_type").toString());
    if (packq.value("pack_head_type").toString() == "SO")
      params.append("sohead_id", packq.value("pack_head_id").toInt());
    else if (packq.value("pack_head_type").toString() == "TO")
      params.append("tohead_id", packq.value("pack_head_id").toInt());
    if (! packq.value("pack_shiphead_id").isNull())
    {
      params.append("shiphead_id",  packq.value("pack_shiphead_id").toInt());
    }
    _warehouse->appendValue(params);
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");

    if (packq.value("orderhead_status").toString() != "C")
    {
      bool usePickForm;
      if (_auto->isChecked())
        usePickForm = packq.value("pack_shiphead_id").isNull();
      else
        usePickForm = _pick->isChecked();
        
      orReport report(packq.value( usePickForm ? "pickform" : "packform").toString(), params);

      if (report.isValid())
      {
        if (report.print(&printer, setupPrinter))
        {
          setupPrinter = false;
          emit finishedPrinting(packq.value("pack_head_id").toInt(),
                                packq.value("pack_head_type").toString(),
                                packq.value("pack_shiphead_id").toInt());
        }
        else
        {
          orReport::endMultiPrint(&printer);
          return;
        }
      }
      else
      {
        report.reportError(this);
        orReport::endMultiPrint(&printer);
        return;
      }
    }

    MetaSQLQuery mql(prts);
    prtd = mql.toQuery(params);
    if (prtd.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Printing Packing List Batch"),
                           prtd, __FILE__, __LINE__);
      orReport::endMultiPrint(&printer);
      return;
    }

  }
  orReport::endMultiPrint(&printer);
  sPopulateShipVia();
}

void printPackingListBatchByShipvia::sPopulateShipVia()
{
  XSqlQuery printPopulateShipVia;
  ParameterList params;
  _warehouse->appendValue(params);
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");
  MetaSQLQuery mql = mqlLoad("packingListBatchByShipVia", "shipVia");
  printPopulateShipVia = mql.toQuery(params);
  _shipvia->populate(printPopulateShipVia);

  if (printPopulateShipVia.lastError().type() != QSqlError::NoError)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Packing List Batch Information"),
                       printPopulateShipVia, __FILE__, __LINE__);
}
