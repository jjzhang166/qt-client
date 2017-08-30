/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCreditMemos.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "distributeInventory.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

postCreditMemos::postCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);
}

postCreditMemos::~postCreditMemos()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCreditMemos::languageChange()
{
  retranslateUi(this);
}

void postCreditMemos::sPost()
{
  XSqlQuery postPost;
  postPost.exec( "SELECT cmhead_printed, COUNT(*) AS number "
          "FROM ( SELECT cmhead_id, cmhead_printed "
          "       FROM cmhead "
          "       WHERE (NOT cmhead_posted) ) AS data "
          "WHERE (checkCreditMemoSitePrivs(cmhead_id)) "
          "GROUP BY cmhead_printed;" );
  if (postPost.first())
  {
    int printed   = 0;
    int unprinted = 0;

    do
    {
      if (postPost.value("cmhead_printed").toBool())
        printed = postPost.value("number").toInt();
      else
        unprinted = postPost.value("number").toInt();
    }
    while (postPost.next());

    if ( ( (unprinted) && (!printed) ) && (!_postUnprinted->isChecked()) )
    {
      QMessageBox::warning( this, tr("No Sales Credits to Post"),
                            tr( "Although there are unposted Sales Credits, there are no unposted Sales Credits that have been printed.\n"
                                "You must manually print these Sales Credits or select 'Post Unprinted Sales Credits' before these Sales Credits\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Sales Credits to Post"),
                          tr("There are no Sales Credits, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  postPost.exec("SELECT fetchJournalNumber('AR-CM') AS result");
  if (!postPost.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Journal Number"),
                         postPost, __FILE__, __LINE__);
    return;
  }

  int journalNumber = postPost.value("result").toInt();

  // Cycle through each credit memo and handle the itemlocSeries and itemlocdist creation
  int succeeded = 0;
  QList<QString> failedItems;
  QList<QString> errors;
  XSqlQuery creditMemos;
  creditMemos.prepare("SELECT cmhead_id, cmhead_number "
                      "FROM cmhead "  
                      "WHERE (NOT cmhead_posted) "
                      " AND (NOT cmhead_hold) "
                      " AND checkCreditMemoSitePrivs(cmhead_id) "
                      " AND (:postUnprinted OR cmhead_printed) "
                      "ORDER BY cmhead_id;");
  creditMemos.bindValue(":postUnprinted", _postUnprinted->isChecked());
  creditMemos.exec();
  while (creditMemos.next())
  {
    QString creditMemoNumber = creditMemos.value("cmhead_number").toString();
    int creditMemoId = creditMemos.value("cmhead_id").toInt();
    int itemlocSeries = distributeInventory::SeriesCreate(0, 0, QString(), QString());
    if (itemlocSeries < 0)
    {
      failedItems.append(creditMemoNumber);
      errors.append(tr("Failed to create a new series for credit memo %1")
        .arg(creditMemos.lastError().databaseText()));
      continue;
    }

    XSqlQuery cleanup; // Stage cleanup function to be called on error
    cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
    cleanup.bindValue(":itemlocSeries", itemlocSeries);
    
    // Cycle through credit memo items that are controlled and have qty returned, create an itemlocdist record for each
    bool hasControlledItems = false;
    bool cmitemFail = false;
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
    cmitems.bindValue(":cmheadId", creditMemos.value("cmhead_id").toInt());
    cmitems.exec();
    while (cmitems.next())
    {
      if (distributeInventory::SeriesCreate(cmitems.value("itemsite_id").toInt(), 
        cmitems.value("qty").toDouble(), "CM", "RS", cmitems.value("cmitem_id").toInt(), itemlocSeries) < 0)
      {
        failedItems.append(creditMemoNumber);
        errors.append(tr("Failed to create itemlocdist record for item %1")
          .arg(cmitems.value("item_number").toString()));
        cmitemFail = true;
        break;
      }

      hasControlledItems = true;
    }

    // Don't continue on this credit memo because there was an issue with one of it's line items
    if (cmitemFail)
    {
      cleanup.exec();
      continue;
    }

    // Distribute detail for the records created above
    if (hasControlledItems && distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(),
      QDate(), true) == XDialog::Rejected)
    {
      cleanup.exec();
      failedItems.append(creditMemoNumber);
      errors.append(tr("Detail Distribution Cancelled"));

      // If it's not the last credit memo, ask user if they want to continue
      if (creditMemos.at() != creditMemos.size() -1)
      {
        if (QMessageBox::question(this,  tr("Post Credit Memo"),
          tr("Posting distribution detail for credit memo number %1 was cancelled but "
             "there other credit memos to Post. Continue posting the remaining credit memos?")
          .arg(creditMemoNumber), 
          QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
        {
          continue;
        }
        else
        {
          break;
        }
      }
      continue;
    }

    XSqlQuery rollback;
    rollback.prepare("ROLLBACK;");

    postPost.exec("BEGIN;");  // TODO - remove this when postCreditMemo no longer returns negative error codes
    postPost.prepare("SELECT postCreditMemo(:cmheadId, :journalNumber, :itemlocSeries, TRUE) AS result;");
    postPost.bindValue(":cmheadId", creditMemoId);
    postPost.bindValue(":journalNumber", journalNumber);
    postPost.bindValue(":itemlocSeries", itemlocSeries);
    postPost.exec();
    if (postPost.first())
    {
      int result = postPost.value("result").toInt();

      if (result < 0)
      {
        rollback.exec();
        cleanup.exec();
        failedItems.append(creditMemoNumber);
        errors.append(tr("Error Posting Credit Memo %1")
          .arg(storedProcErrorLookup("postCreditMemo", result)));
        continue;
      }

      postPost.exec("COMMIT;");
      succeeded++;
    }
    else
    {
      rollback.exec();
      cleanup.exec();
      failedItems.append(creditMemoNumber);
      errors.append(tr("Error Posting Credit Memo %1")
        .arg(postPost.lastError().databaseText()));
      continue;
    }
  }

  if (errors.size() > 0)
  {
    QMessageBox dlg(QMessageBox::Critical, "Errors Posting Credit Memo", "", QMessageBox::Ok, this);
    dlg.setText(tr("%1 Credit Memos succeeded.\n%2 Credit Memos failed.").arg(succeeded).arg(failedItems.size()));

    QString details;
    for (int i=0; i<failedItems.size(); i++)
      details += tr("Credit Memo %1 failed with:\n%2\n").arg(failedItems[i]).arg(errors[i]);
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

  omfgThis->sCreditMemosUpdated();

  accept();
}
