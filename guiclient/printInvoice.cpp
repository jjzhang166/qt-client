/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printInvoice.h"

#include "distributeInventory.h"
#include <QSqlRecord>
#include <QVariant>
#include <QMessageBox>

printInvoice::printInvoice(QWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("InvoiceCopies",     "InvoiceWatermark",
                             "InvoiceShowPrices", "PostMiscInvoices",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("IN");
  setReportKey("invchead_id");
  _distributeInventory = true;

  _docinfoQueryString =
         "SELECT invchead_id         AS docid,"
         "       invchead_invcnumber AS docnumber,"
         "       invchead_printed    AS printed,"
         "       invchead_posted     AS posted,"
         "       invchead_cust_id,"
         "       findCustomerForm(invchead_cust_id, 'I') AS reportname "
         "FROM invchead "
         "WHERE (invchead_id=<? value('docid') ?>);" ;

  _askBeforePostingQry = "SELECT invoiceTotal(<? value('docid') ?>) = 0 AS ask;" ;
  _askBeforePostingMsg = tr("<p>Invoice %1 has a total value of 0.<br/>"
                            "Would you like to post it anyway?</p>");

  _errCheckBeforePostingQry =
         "SELECT EXISTS(SELECT 1"
         "                FROM curr_rate"
         "                JOIN invchead ON (curr_id=invchead_curr_id)"
         "               WHERE ((invchead_invcdate BETWEEN curr_effective AND curr_expires)"
         "                 AND  (invchead_id=<? value('docid') ?>))) AS ok;" ;
  _errCheckBeforePostingMsg =
          tr("Could not post Invoice %1 because of a missing exchange rate.");

  _markOnePrintedQry = "UPDATE invchead"
                       "   SET invchead_printed=true"
                       " WHERE (invchead_id=<? value('docid') ?>);" ;

  _postFunction = "postInvoice";
  _postQuery = "SELECT postInvoice(<? value('docid') ?>, fetchJournalNumber('AR-IN'), <? value('itemlocSeries') ?>, true) AS result;" ;

  connect(this, SIGNAL(docUpdated(int)),       this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sHandlePopulated(XSqlQuery*)));
}

printInvoice::~printInvoice()
{
}

void printInvoice::languageChange()
{
  retranslateUi(this);
}

void printInvoice::sHandlePopulated(XSqlQuery *qry)
{
  if (qry)
  {
    _invoiceNum->setText(qry->value("docnumber").toString());
    _cust->setId(qry->value("invchead_cust_id").toInt());
  }
}

void printInvoice::sHandleDocUpdated(int docid)
{
  omfgThis->sInvoicesUpdated(docid, true);
}

int printInvoice::distributeInventory(XSqlQuery *qry)
{
  int invoiceId = qry->value("docid").toInt();

  // Set the series for this invoice
  int itemlocSeries = distributeInventory::SeriesCreate(0, 0, QString(), QString());
  if (itemlocSeries < 0)
    return -1;

  XSqlQuery cleanup; // Stage cleanup function to be called on error
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);

  bool hasControlledItems = false;

  // Handle the Inventory and G/L Transactions for any billed Inventory where invcitem_updateinv is true
  XSqlQuery items;
  items.prepare("SELECT item_number, itemsite_id, invcitem_id, "
                " (invcitem_billed * invcitem_qty_invuomratio) * -1 AS qty, "
                " invchead_invcnumber "
                "FROM invchead " 
                " JOIN invcitem ON invcitem_invchead_id = invchead_id "
                "   AND invcitem_billed <> 0 " 
                "   AND invcitem_updateinv "
                " JOIN itemsite ON itemsite_item_id = invcitem_item_id " 
                "   AND itemsite_warehous_id = invcitem_warehous_id "
                " JOIN item ON item_id = invcitem_item_id "
                "WHERE invchead_id = :invchead_id "
                " AND itemsite_costmethod != 'J' "
                " AND (itemsite_loccntrl OR itemsite_controlmethod IN ('L', 'S')) "
                " AND itemsite_controlmethod != 'N' "
                "ORDER BY invcitem_id;");
  items.bindValue(":invchead_id", invoiceId);
  items.exec();
  while (items.next())
  {
    int result = distributeInventory::SeriesCreate(items.value("itemsite_id").toInt(), 
      items.value("qty").toDouble(), "IN", "SH", items.value("invcitem_id").toInt(), itemlocSeries);
    if (result < 0)
    {
      cleanup.exec();
      return -1;
    }
    
    hasControlledItems = true;
  }

  // Distribute the items from above
  if (hasControlledItems && distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
    == XDialog::Rejected)
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Print Invoice"), tr("Detail Distribution was Cancelled") );
    return -1;
  }

  return itemlocSeries;
}
