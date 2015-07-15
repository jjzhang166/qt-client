/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBankrecHistory.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>

#include "xtreewidget.h"
#include "xmessagebox.h"

dspSummarizedBankrecHistory::dspSummarizedBankrecHistory(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "dspSummarizedBankrecHistory", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Summarized Bank Reconciliation History"));
  setListLabel(tr("Summarized History"));
  setReportName("SummarizedBankrecHistory");
  setMetaSQLOptions("summarizedBankrecHistory", "detail");

  list()->addColumn(tr("Posted"),             _ynColumn, Qt::AlignLeft,   true,  "bankrec_posted"   );
  list()->addColumn(tr("Post Date"),        _dateColumn, Qt::AlignCenter, true,  "bankrec_postdate" );
  list()->addColumn(tr("User"),                      -1, Qt::AlignLeft,   true,  "bankrec_username"   );
  list()->addColumn(tr("Start Date"),       _dateColumn, Qt::AlignCenter, true,  "bankrec_opendate" );
  list()->addColumn(tr("End Date"),         _dateColumn, Qt::AlignCenter, true,  "bankrec_enddate" );
  list()->addColumn(tr("Opening Bal."), _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_openbal"  );
  list()->addColumn(tr("Ending Bal."),  _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_endbal"  );
  
  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");
}

void dspSummarizedBankrecHistory::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

bool dspSummarizedBankrecHistory::setParams(ParameterList & params)
{
  params.append("bankaccntid", _bankaccnt->id());
  return true;
}

void dspSummarizedBankrecHistory::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *selected, int)
{
  QAction *menuItem;
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  
  if (item->rawValue("bankrec_posted").toBool())
  {
    XSqlQuery menu;
    menu.prepare("SELECT bankrec_id "
                 "FROM bankrec "
                 "WHERE (bankrec_bankaccnt_id=:bankrec_bankaccnt_id) "
                 "  AND (bankrec_opendate > :bankrec_enddate) "
                 "  AND (bankrec_posted);");
    menu.bindValue(":bankrec_bankaccnt_id", _bankaccnt->id());
    menu.bindValue(":bankrec_enddate", item->rawValue("bankrec_enddate").toDate());
    menu.exec();
    if (!menu.first())
    {
      menuItem = pMenu->addAction(tr("Reopen..."), this, SLOT(sReopen()));
      menuItem->setEnabled(_privileges->check("MaintainBankRec"));
    }
  }
}

void dspSummarizedBankrecHistory::sReopen()
{
  if (QMessageBox::question(this, tr("Reopen Bank Reconciliation?"),
                            tr("Reopening a posted Bank Reconciliation "
                               "will delete any unposted reconciliation entries."
                               "<p>Are you sure you want to continue?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery open;
    open.prepare("SELECT reopenBankReconciliation(:bankrec_id) AS result;");
    open.bindValue(":bankrec_id", list()->id());
    open.exec();
    if (open.first())
    {
      int result = open.value("result").toInt();
      if (result < 0)
      {
        QMessageBox::critical(this, tr("Reopen Error"),
                              tr("<p>reopenBankReconciliation failed, result=%1").arg(result));
        return;
      }
    }
    else if (open.lastError().type() != QSqlError::NoError)
    {
      systemError(this, open.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}
