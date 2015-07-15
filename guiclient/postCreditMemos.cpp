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
      QMessageBox::warning( this, tr("No Returns to Post"),
                            tr( "Although there are unposted Returns, there are no unposted Returns that have been printed.\n"
                                "You must manually print these Returns or select 'Post Unprinted Returns' before these Returns\n"
                                "may be posted." ) );
      _postUnprinted->setFocus();
      return;
    }
  }
  else
  {
    QMessageBox::warning( this, tr("No Returns to Post"),
                          tr("There are no Returns, printed or not, to post.\n" ) );
    _close->setFocus();
    return;
  }

  postPost.exec("SELECT fetchJournalNumber('AR-CM') AS result");
  if (!postPost.first())
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  int journalNumber = postPost.value("result").toInt();
  
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  postPost.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  postPost.prepare("SELECT postCreditMemos(:postUnprinted, :journalNumber) AS result;");
  postPost.bindValue(":postUnprinted", QVariant(_postUnprinted->isChecked()));
  postPost.bindValue(":journalNumber", journalNumber);
  postPost.exec();
  if (postPost.first())
  {
    int result = postPost.value("result").toInt();

    if (result == -5)
    {
      rollback.exec();
      QMessageBox::critical( this, tr("Cannot Post one or more Returns"),
                             tr( "The Ledger Account Assignments for one or more of the Returns that you are trying to post are not\n"
                                 "configured correctly.  Because of this, G/L Transactions cannot be posted for these Returns.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post these Returns." ) );
      return;
    }
    else if (result < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at postCreditMemos::%1, Error #%2.")
                         .arg(__LINE__)
                         .arg(postPost.value("result").toInt()) );
      return;
    }
    else if (distributeInventory::SeriesAdjust(postPost.value("result").toInt(), this) == XDialog::Rejected)
    {
      rollback.exec();
      QMessageBox::information( this, tr("Post Returns"), tr("Transaction Canceled") );
      return;
    }

    postPost.exec("COMMIT;");

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
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at postCreditMemos::%1.")
                       .arg(__LINE__) );
    return;
  }

  omfgThis->sCreditMemosUpdated();

  accept();
}
