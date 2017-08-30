/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemo.h"

#include <QDebug>
#include <QSqlRecord>
#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"

printCreditMemo::printCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("CreditMemoCopies",     "CreditMemoWatermark",
                             "CreditMemoShowPrices", "PostARDocuments",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("CM");
  setReportKey("cmhead_id");
  _distributeInventory = true;

  _docinfoQueryString =
             "SELECT cmhead_id      AS docid,"
             "       cmhead_number  AS docnumber,"
             "       cmhead_printed AS printed,"
             "       cmhead_posted  AS posted,"
             "       cmhead_cust_id,"
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname "
             "FROM cmhead "
             "WHERE (cmhead_id=<? value('docid') ?>);" ;

  _markOnePrintedQry = "UPDATE cmhead "
                       "   SET cmhead_printed=true "
                       " WHERE (cmhead_id=<? value('docid') ?>);" ;
  _postFunction = "postCreditMemo";
  _postQuery    = "SELECT postCreditMemo(<? value('docid') ?>, fetchJournalNumber('AR-CM'), <? value('itemlocSeries') ?>, true) AS result;";

  connect(this, SIGNAL(docUpdated(int)),       this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sHandlePopulated(XSqlQuery*)));
}

printCreditMemo::~printCreditMemo()
{
}

void printCreditMemo::languageChange()
{
  retranslateUi(this);
}

void printCreditMemo::sHandlePopulated(XSqlQuery *qry)
{
  if (qry)
  {
    _number->setText(qry->value("docnumber").toString());
    _cust->setId(qry->value("cmhead_cust_id").toInt());
  }
}

void printCreditMemo::sHandleDocUpdated(int docid)
{
  Q_UNUSED(docid);
  omfgThis->sCreditMemosUpdated();
}

int printCreditMemo::distributeInventory(XSqlQuery *qry)
{
  int creditMemoId = qry->value("docid").toInt();
  int itemlocSeries = distributeInventory::SeriesCreate(0, 0, QString(), QString());
  if (itemlocSeries < 0)
    return -1;

  XSqlQuery cleanup; // Stage cleanup function to be called on error
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  cleanup.bindValue(":itemlocSeries", itemlocSeries);
  
  // Cycle through credit memo items that are controlled and have qty returned, create an itemlocdist record for each
  bool hasControlledItems = false;
  XSqlQuery cmitems;
  cmitems.prepare("SELECT itemsite_id, item_number, "
                    " SUM(cmitem_qtyreturned * cmitem_qty_invuomratio) AS qty "
                    "FROM cmhead JOIN cmitem ON cmitem_cmhead_id=cmhead_id "
                    " JOIN itemsite ON itemsite_id=cmitem_itemsite_id "
                    " JOIN item ON item_id=itemsite_item_id "
                    " JOIN costcat ON costcat_id=itemsite_costcat_id "
                    "WHERE cmhead_id=:cmheadId "
                    " AND cmitem_qtyreturned <> 0 "
                    " AND cmitem_updateinv "
                    " AND isControlledItemsite(itemsite_id) "
                    " AND itemsite_costmethod != 'J' "
                    "GROUP BY itemsite_id, item_number "
                    "ORDER BY itemsite_id;");
  cmitems.bindValue(":cmheadId", creditMemoId);
  cmitems.exec();
  while (cmitems.next())
  {
    if (distributeInventory::SeriesCreate(cmitems.value("itemsite_id").toInt(), 
      cmitems.value("qty").toDouble(), "CM", "RS", cmitems.value("cmitem_id").toInt(), itemlocSeries) < 0)
    {
      cleanup.exec();
      return -1;
    }
    
    hasControlledItems = true;
  }

  // Distribute detail for the records created above
  if (hasControlledItems && distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
    QDate(), true) == XDialog::Rejected)
  {
    cleanup.exec();
    QMessageBox::information( this, tr("Print Credit Memo"), tr("Detail Distribution was Cancelled") );
    return -1;
  }

  return itemlocSeries;
}

