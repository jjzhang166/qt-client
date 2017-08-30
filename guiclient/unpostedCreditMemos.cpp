/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedCreditMemos.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "creditMemo.h"
#include "distributeInventory.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "printCreditMemo.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

unpostedCreditMemos::unpostedCreditMemos(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

    connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

    _cmhead->addColumn(tr("Sales Credit #"),      _orderColumn, Qt::AlignLeft,   true,  "cmhead_number"   );
    _cmhead->addColumn(tr("Prnt'd"),        _orderColumn, Qt::AlignCenter, true,  "printed" );
    _cmhead->addColumn(tr("Customer"),      -1,           Qt::AlignLeft,   true,  "cmhead_billtoname"   );
    _cmhead->addColumn(tr("Credit Date"),   _dateColumn,  Qt::AlignCenter, true,  "cmhead_docdate" );
    _cmhead->addColumn(tr("Hold"),          _whsColumn,   Qt::AlignCenter, true,  "cmhead_hold" );
    _cmhead->addColumn(tr("G/L Dist Date"), _dateColumn,  Qt::AlignCenter, true,  "distdate" );

    if (! _privileges->check("ChangeSOMemoPostDate"))
      _cmhead->hideColumn(5);

    if (_privileges->check("MaintainCreditMemos"))
    {
      connect(_cmhead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_cmhead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_cmhead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(false);
      connect(_cmhead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
    
    if (_privileges->check("PrintCreditMemos"))
      connect(_cmhead, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

    if (_privileges->check("PostARDocuments"))
      connect(_cmhead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

    connect(omfgThis, SIGNAL(creditMemosUpdated()), this, SLOT(sFillList()));

    sFillList();
}

unpostedCreditMemos::~unpostedCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedCreditMemos::languageChange()
{
    retranslateUi(this);
}

void unpostedCreditMemos::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainCreditMemos"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainCreditMemos") ||
                       _privileges->check("ViewCreditMemos"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  menuItem->setEnabled(_privileges->check("PostARDocuments"));
}

void unpostedCreditMemos::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sEdit()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
	
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sView()
{
  if (!checkSitePrivs(_cmhead->id()))
    return;
	
  ParameterList params;
  params.append("mode", "view");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sPrint()
{
  QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("persistentPrint");

      printCreditMemo newdlg(this, "", true);
      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        newdlg.exec();
        newdlg.setSetup(true);
      }
    }
  }
  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sPost()
{
  XSqlQuery unpostedPost;

  
  unpostedPost.exec("SELECT fetchJournalNumber('AR-CM') AS result");
  if (!unpostedPost.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Journal Number"),
                         unpostedPost, __FILE__, __LINE__);
    return;
  }

  int journalNumber = unpostedPost.value("result").toInt();

  XSqlQuery setDate;
  setDate.prepare("UPDATE cmhead SET cmhead_gldistdate=:distdate "
                  "WHERE cmhead_id=:cmhead_id;");

  // Wrap in do/while in case of date issues
  QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
  QList<XTreeWidgetItem*> triedToClosed;
  bool tryagain = false;
  do {
    bool changeDate = false;
    QDate newDate = QDate::currentDate();

    if (_privileges->check("ChangeSOMemoPostDate"))
    {
      getGLDistDate newdlg(this, "", true);
      newdlg.sSetDefaultLit(tr("Credit Memo Date"));
      if (newdlg.exec() == XDialog::Accepted)
      {
        newDate = newdlg.date();
        changeDate = (newDate.isValid());

        if (changeDate)
        { // Update credit memo dates
          for (int i = 0; i < selected.size(); i++)
          {
            int id = ((XTreeWidgetItem*)(selected[i]))->id();
            if (checkSitePrivs(id))
            {
              setDate.bindValue(":distdate",  newDate);
              setDate.bindValue(":cmhead_id", id);
              setDate.exec();
              if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Changing Credit Memo GL Distribution Date"),
                setDate, __FILE__, __LINE__))
              {
                return;
              }
            }
          }
        }
      }
      else
        return;
    }

    // Loop through the selected credit memos
    int succeeded = 0;
    QList<QString> failedMemos;
    QList<QString> errors;
    for (int i = 0; i < selected.size(); i++)
    {
      int memoId = ((XTreeWidgetItem*)(selected[i]))->id();
      QString memoNumber;

      // Skip this CM if no privilege
      if (!checkSitePrivs(memoId))
        continue;

      // Check if the date is in a closed period
      XSqlQuery closedPeriod;
      closedPeriod.prepare("SELECT period_closed, cmhead_number "
                           "FROM cmhead "
                           "  JOIN period ON COALESCE(cmhead_gldistdate, cmhead_docdate) BETWEEN period_start AND period_end "
                           "WHERE cmhead_id=:cmhead_id;");
      closedPeriod.bindValue(":cmhead_id", memoId);
      closedPeriod.exec();
      if (!closedPeriod.first() || closedPeriod.value("period_closed").toBool())
      {
        if (_privileges->check("ChangeSOMemoPostDate"))
        {
          if (changeDate)
          {
            triedToClosed = selected;
            break;
          }
          else
            triedToClosed.append(selected[i]);
        }
        continue;  
      }
      else
        memoNumber = closedPeriod.value("cmhead_number").toString();

      // Set the series for this memo
      int itemlocSeries = distributeInventory::SeriesCreate(0, 0, QString(), QString());
      if (itemlocSeries < 0)
      {
        failedMemos.append(memoNumber);
        errors.append(tr("Failed to create a new series for the memo."));
        continue;
      }

      XSqlQuery cleanup; // Stage cleanup function to be called on error
      cleanup.prepare("SELECT deleteitemlocseries(:itemlocSeries, TRUE);");
      cleanup.bindValue(":itemlocSeries", itemlocSeries);

      bool memoLineFailed = false;
      bool hasControlledItems = false;

      // Loop through controlled items that have qty to return and create itemlocdist record for each
      XSqlQuery items;
      items.prepare("SELECT itemsite_id, item_number, "
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
      items.bindValue(":cmheadId", memoId);
      items.exec();
      while (items.next())
      {
        int result = distributeInventory::SeriesCreate(items.value("itemsite_id").toInt(), 
          items.value("qty").toDouble(), "CM", "RS", items.value("cmitem_id").toInt(), itemlocSeries);
        if (result < 0)
        {
          cleanup.exec();
          failedMemos.append(memoNumber);
          errors.append(tr("Error Creating itemlocdist Record for item %1").arg(items.value("item_number").toString()));
          memoLineFailed = true;
          break;
        }
        else if (itemlocSeries == 0) // The first time through the loop, set itemlocSeries
          itemlocSeries = result;

        hasControlledItems = true;
      }

      if (memoLineFailed)
        continue;

      // Distribute the items from above
      if (hasControlledItems && distributeInventory::SeriesAdjust(itemlocSeries, this, QString(), QDate(), QDate(), true)
        == XDialog::Rejected)
      {
        cleanup.exec();
        // If it's not the last memo in the loop, ask the user to exit loop or continue
        if (i != (selected.size() -1))
        {
          if (QMessageBox::question(this,  tr("Post Credit Memo"),
            tr("Posting distribution detail for credit memo number %1 was cancelled but "
               "there other credit memos to Post. Continue posting the remaining credit memos?")
            .arg(memoNumber), 
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
          {
            failedMemos.append(memoNumber);
            errors.append(tr("Detail Distribution Cancelled"));
            continue;
          }
          else
          {
            failedMemos.append(memoNumber);
            errors.append(tr("Detail Distribution Cancelled"));
            break;
          }
        }
        else 
        {
          failedMemos.append(memoNumber);
          errors.append(tr("Detail Distribution Cancelled"));
          continue;
        }
      }

      // Post credit memo
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      XSqlQuery postPost;
      postPost.exec("BEGIN;");  // TODO - remove this when postCreditMemo no longer returns negative error codes
      postPost.prepare("SELECT postCreditMemo(:cmheadId, :journalNumber, :itemlocSeries, TRUE) AS result;");
      postPost.bindValue(":cmheadId", memoId);
      postPost.bindValue(":journalNumber", journalNumber);
      postPost.bindValue(":itemlocSeries", itemlocSeries);
      postPost.exec();
      if (postPost.first())
      {
        int result = postPost.value("result").toInt();

        if (result < 0 || result != itemlocSeries)
        {
          rollback.exec();
          cleanup.exec();
          failedMemos.append(memoNumber);
          errors.append(tr("Error Posting Credit Memo %1")
            .arg(storedProcErrorLookup("postCreditMemo", result)));
          continue;
        }

        postPost.exec("COMMIT;");
        succeeded++;
      }
      else if (postPost.lastError().databaseText().contains("post to closed period"))
      {
        if (changeDate)
        {
          triedToClosed = selected;
          break;
        }
        else
          triedToClosed.append(selected[i]);
      }
      else
      {
        rollback.exec();
        cleanup.exec();
        failedMemos.append(memoNumber);
        errors.append(tr("Error Posting Credit Memo %1")
          .arg(postPost.lastError().databaseText()));
        continue;
      }
    }

    // Report any errors in a single dialog
    if (errors.size() > 0)
    {
      QMessageBox dlg(QMessageBox::Critical, "Errors Posting Credit Memo ", "", QMessageBox::Ok, this);
      dlg.setText(tr("%1 Credit Memos Succeeded.\n%2 Credit Memos Failed.").arg(succeeded).arg(failedMemos.size()));

      QString details;
      for (int i=0; i<failedMemos.size(); i++)
        details += tr("Credit Memo %1 failed with:\n%2\n").arg(failedMemos[i]).arg(errors[i]);
      dlg.setDetailedText(details);

      dlg.exec();
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _cmhead->headerItem(), _cmhead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
    else 
      tryagain = false;

  } while (tryagain); // selected credit memo loop

  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Sales Credits?"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Sales Credits?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");

    QList<XTreeWidgetItem*> selected = _cmhead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        delq.bindValue(":cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        delq.exec();
        if (delq.first())
        {
          int result = delq.value("result").toInt();
          if (result < 0)
            ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Memo Information"),
                                 storedProcErrorLookup("deleteCreditMemo", result),
                                 __FILE__, __LINE__);
        }
        else if (delq.lastError().type() != QSqlError::NoError)
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Credit Memo Information %1\n")
                             .arg(selected[i]->text(0)) + delq.lastError().databaseText(),
                             delq, __FILE__, __LINE__);
      }
    }

    omfgThis->sCreditMemosUpdated();
  }
}

void unpostedCreditMemos::sFillList()
{
  _cmhead->clear();
  _cmhead->populate( "SELECT cmhead_id, cmhead_number,"
                     "       COALESCE(cmhead_printed, false) AS printed,"
                     "       cmhead_billtoname, cmhead_docdate, cmhead_hold,"
                     "       COALESCE(cmhead_gldistdate, cmhead_docdate) AS distdate "
                     "FROM cmhead "
                     "WHERE ( (NOT cmhead_posted) "
                     "  AND   ( ((SELECT COUNT(*)"
                     "            FROM cmitem, itemsite, site()"
                     "            WHERE ( (cmitem_cmhead_id=cmhead_id)"
                     "              AND   (itemsite_id=cmitem_itemsite_id)"
                     "              AND   (warehous_id=itemsite_warehous_id) )) > 0) OR"
                     "          ((SELECT COUNT(*)"
                     "            FROM cmitem"
                     "            WHERE ((cmitem_cmhead_id=cmhead_id)"
                     "              AND  (cmitem_number IS NOT NULL) )) > 0) OR"
                     "          ((SELECT COUNT(*)"
                     "            FROM cmitem"
                     "            WHERE (cmitem_cmhead_id=cmhead_id)) = 0) ) ) "
                     "ORDER BY cmhead_number;" );
}

bool unpostedCreditMemos::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkCreditMemoSitePrivs(:cmheadid) AS result;");
    check.bindValue(":cmheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Sales Credit as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
