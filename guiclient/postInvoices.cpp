/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "distributeInventory.h"
#include <openreports.h>
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

postInvoices::postInvoices(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);
}

postInvoices::~postInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void postInvoices::languageChange()
{
  retranslateUi(this);
}

void postInvoices::sPost()
{
  XSqlQuery postPost;
  postPost.exec( "SELECT invchead_printed, COUNT(*) AS number "
          "FROM ( "
          "  SELECT * FROM invchead WHERE NOT (invchead_posted)) AS data "
          "WHERE (checkInvoiceSitePrivs(invchead_id)) "
          "GROUP BY invchead_printed;" );
  if (postPost.first())
  {
    int printed   = 0;
    int unprinted = 0;

    do
    {
      if (postPost.value("invchead_printed").toBool())
        printed = postPost.value("number").toInt();
      else
        unprinted = postPost.value("number").toInt();
    }
    while (postPost.next());

    if ( ( (unprinted) && (!printed) ) && (!_postUnprinted->isChecked()) )
    {
      QMessageBox::warning( this, tr("No Invoices to Post"),
                            tr( "Although there are unposted Invoices, there are no unposted Invoices that have been printed.\n"
                                "You must manually print these Invoices or select 'Post Unprinted Invoices' before these Invoices\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Invoices to Post"),
                          tr("There are no Invoices, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  bool inclZero = false;
  postPost.exec("SELECT COUNT(invchead_id) AS numZeroInvcs "
         "FROM invchead "
         "WHERE ( (NOT invchead_posted) "
         "  AND   (invoiceTotal(invchead_id) <= 0.0) "
         "  AND   (checkInvoiceSitePrivs(invchead_id)) );");
  if (postPost.first() && postPost.value("numZeroInvcs").toInt() > 0)
  {
    int toPost = QMessageBox::question(this, tr("Invoices for 0 Amount"),
				       tr("There are %1 invoices with a total value of 0.\n"
					  "Would you like to post them?")
					 .arg(postPost.value("numZeroInvcs").toString()),
				       tr("Post All"), tr("Post Only Non-0"),
				       tr("Cancel"), 1, 2);
    if (toPost == 2)
      return;
    else if (toPost == 1)
      inclZero = false;
    else
      inclZero = true;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Invoice Information"),
                                postPost, __FILE__, __LINE__))
  {
    return;
  }

  postPost.exec("SELECT fetchJournalNumber('AR-IN') AS journal;");
  if (!postPost.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Invoice Information"),
                         postPost, __FILE__, __LINE__);
    return;
  }
  int journalNumber = postPost.value("journal").toInt();

  // Gather invoices to cycle through (create parent itemlocdist records) - logic from postInvoices(boolean, boolean, integer).sql
  QList<int> invoiceIds;
  XSqlQuery invoices;
  if (inclZero)
  {
    invoices.prepare("SELECT invchead_id "
                     "FROM invchead "
                     "WHERE NOT invchead_posted "
                     "  AND checkInvoiceSitePrivs(invchead_id) "
                     "  AND (:postUnprinted OR invchead_printed);");
  }
  else 
  {
    invoices.prepare("SELECT invchead_id "
                     "FROM invchead LEFT OUTER JOIN invcitem ON invchead_id = invcitem_invchead_id "
                     "  LEFT OUTER JOIN item ON invcitem_item_id = item_id "
                     "WHERE NOT invchead_posted "
                     "  AND checkInvoiceSitePrivs(invchead_id) "
                     "  AND (:postUnprinted OR invchead_printed) "
                     "GROUP BY invchead_id, invchead_freight, invchead_misc_amount "
                     "HAVING (COALESCE(SUM(round((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price / "  
                     "  CASE WHEN (item_id IS NULL) THEN 1 " 
                     "  ELSE invcitem_price_invuomratio END), 2)),0) "
                     "  + invchead_freight + invchead_misc_amount) > 0;"); 
  }
  invoices.bindValue(":postUnprinted", QVariant(_postUnprinted->isChecked()));
  invoices.exec();
  while (invoices.next())
  {
    invoiceIds.append(invoices.value("invchead_id").toInt());
  }

  if (invoiceIds.count() == 0)
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding the List of Invoices to Post"),
                         invoices, __FILE__, __LINE__);
    return;
  }

  int itemlocSeries;
  // Stage distribution cleanup function to be called on error
  XSqlQuery cleanup;
  cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
  
  // Get the parent series id
  XSqlQuery parentSeries;
  parentSeries.prepare("SELECT NEXTVAL('itemloc_series_seq') AS itemlocSeries;");
  parentSeries.exec();
  if (parentSeries.first() && parentSeries.value("itemlocSeries").toInt() > 0)
  {
    itemlocSeries = parentSeries.value("itemlocSeries").toInt();
    cleanup.bindValue(":itemlocSeries", itemlocSeries);
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Failed to Retrieve the Next itemloc_series_seq"),
                            parentSeries, __FILE__, __LINE__);
    return;
  }

  int succeeded = 0;
  QList<QString> failedInvoiceNumbers;
  QList<QString> errors;
  for (int i = 0; i < invoiceIds.size(); i++)
  {
    bool invoiceLineFailed = false;

    QString invoiceNumber;
    int invoiceId = invoiceIds.at(i);
    // Handle the Inventory and G/L Transactions for any billed Inventory where invcitem_updateinv is true
    XSqlQuery items;
    items.prepare("SELECT item_number, itemsite_id, invcitem_id, "
                  " (invcitem_billed * invcitem_qty_invuomratio) AS qty, "
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
      invoiceNumber = items.value("invchead_invcnumber").toString();
      // Create the parent itemlocdist record for each line item requiring distribution, call distributeInventory::seriesAdjust
      XSqlQuery parentItemlocdist;
      parentItemlocdist.prepare("SELECT createitemlocdistparent(:itemsite_id, :qty, 'IN', "
                                " :orderitemId, :itemlocSeries, NULL, NULL, 'SH');");
      parentItemlocdist.bindValue(":itemsite_id", items.value("itemsite_id").toInt());
      parentItemlocdist.bindValue(":qty", items.value("qty").toDouble() * -1);
      parentItemlocdist.bindValue(":orderitemId", items.value("invcitem_id").toInt());
      parentItemlocdist.bindValue(":itemlocSeries", itemlocSeries);
      parentItemlocdist.exec();
      if (!parentItemlocdist.first())
      {
        cleanup.exec();
        failedInvoiceNumbers.append(invoiceNumber);
        errors.append(tr("Error Creating itemlocdist Record for item %1").arg(items.value("item_number").toString()));
        invoiceLineFailed = true;
        break;
      }
    }

    if (invoiceLineFailed)
      continue;

    // Distribute the items from above
    if (items.size() > 0 && distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
      == XDialog::Rejected)
    {
      cleanup.exec();
      if (QMessageBox::question(this,  tr("Post Invoices"),
        tr("Posting distribution detail for invoice number %1 was cancelled but "
           "there other invoices to Post. Continue posting the remaining invoices?")
        .arg(invoiceNumber), 
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
      {
        failedInvoiceNumbers.append(invoiceNumber);
        errors.append(tr("Detail Distribution Cancelled"));
        continue;
      }
      else
      {
        failedInvoiceNumbers.append(invoiceNumber);
        errors.append(tr("Detail Distribution Cancelled"));
        break;
      }
    }

    // TODO - remove this after postInvoice has had the remaining negative error codes replaced with RAISE EXCEPTIONs
    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    // Post invoice
    XSqlQuery post;
    post.exec("BEGIN;");
    post.prepare("SELECT postInvoice(:invchead_id, :journal, :itemlocSeries, true) AS result;");
    post.bindValue(":invchead_id", invoiceIds.at(i));
    post.bindValue(":journal", journalNumber);
    post.bindValue(":itemlocSeries", itemlocSeries);
    post.exec();
    if (post.first())
    {
      int result = post.value("result").toInt();
      if (result < 0 || result != itemlocSeries)
      {
        rollback.exec();
        cleanup.exec();
        failedInvoiceNumbers.append(invoiceNumber);
        if (result < 0)
          errors.append(tr("Error Posting Invoice. %1").arg(storedProcErrorLookup("postInvoice", result)));
        if (result > 0)
          errors.append(tr("Error Posting Invoice. Expected: %1, returned: %2").arg(itemlocSeries).arg(result));
        continue;
      }
    }
    else if (postPost.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      cleanup.exec();
      failedInvoiceNumbers.append(invoiceNumber);
      errors.append(postPost.lastError().text());
      continue;
    }
    succeeded++;
    post.exec("COMMIT;");
  }

  if (errors.size() > 0)
  {
    QMessageBox dlg(QMessageBox::Critical, "Errors Posting Invoice", "", QMessageBox::Ok, this);
    dlg.setText(tr("%1 Invoices succeeded.\n%2 Invoices failed.").arg(succeeded).arg(failedInvoiceNumbers.size()));

    QString details;
    for (int i=0; i<failedInvoiceNumbers.size(); i++)
      details += tr("Invoice number %1 failed with:\n%2\n").arg(failedInvoiceNumbers[i]).arg(errors[i]);
    dlg.setDetailedText(details);

    dlg.exec();
  }

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("source", "A/R");
    params.append("sourceLit", tr("A/R"));
    params.append("startJrnlnum", journalNumber);
    params.append("endJrnlnum", journalNumber);

    if (_metrics->boolean("UseJournals"))
    {
      params.append("title",tr("Journal Series"));
      params.append("table", "sltrans");
    }
    else
    {
      params.append("title",tr("General Ledger Series"));
      params.append("gltrans", true);
      params.append("table", "gltrans");
    }

    orReport report("GLSeries", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }

  omfgThis->sInvoicesUpdated(-1, true);
  omfgThis->sSalesOrdersUpdated(-1);
  accept();
}
