/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "unpostedCreditMemos.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include <parameter.h>
#include <openreports.h>

#include "creditMemo.h"
#include "distributeInventory.h"
#include "failedPostList.h"
#include "getGLDistDate.h"
#include "printCreditMemo.h"
#include "storedProcErrorLookup.h"

unpostedCreditMemos::unpostedCreditMemos(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    connect(_cmhead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

    statusBar()->hide();
    
    _cmhead->addColumn(tr("C/M #"),     _orderColumn, Qt::AlignLeft   );
    _cmhead->addColumn(tr("Prnt'd"),    _orderColumn, Qt::AlignCenter );
    _cmhead->addColumn(tr("Customer"),  -1,           Qt::AlignLeft   );
    _cmhead->addColumn(tr("Memo Date"), _dateColumn,  Qt::AlignCenter );
    _cmhead->addColumn(tr("Hold"),      _whsColumn,   Qt::AlignCenter );
    _cmhead->addColumn(tr("G/L Dist Date"), _dateColumn, Qt::AlignCenter );

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
      _new->setEnabled(FALSE);
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
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainCreditMemos"))
    pMenu->setItemEnabled(menuItem, false);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainCreditMemos")) && (!_privileges->check("ViewCreditMemos")))
    pMenu->setItemEnabled(menuItem, false);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostARDocuments"))
    pMenu->setItemEnabled(menuItem, false);
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
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cmhead_id", _cmhead->id());

  creditMemo *newdlg = new creditMemo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedCreditMemos::sPrint()
{
  QList<QTreeWidgetItem *> selected = _cmhead->selectedItems();
  QList<QTreeWidgetItem *> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    params.append("persistentPrint");

    printCreditMemo newdlg(this, "", TRUE);
    newdlg.set(params);

    if (!newdlg.isSetup())
    {
      newdlg.exec();
      newdlg.setSetup(TRUE);
    }

  }
  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeSOMemoPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Credit Memo Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE cmhead SET cmhead_gldistdate=:distdate "
		  "WHERE cmhead_id=:cmhead_id;");

  QList<QTreeWidgetItem *> selected = _cmhead->selectedItems();
  QList<QTreeWidgetItem *> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();

    if (changeDate)
    {
      setDate.bindValue(":distdate",  newDate);
      setDate.bindValue(":cmhead_id", id);
      setDate.exec();
      if (setDate.lastError().type() != QSqlError::None)
      {
	systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");
    
  XSqlQuery postq;
  postq.prepare("SELECT postCreditMemo(:cmhead_id, 0) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();
 
      XSqlQuery tx;
      tx.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
      
      postq.bindValue(":cmhead_id", id);
      postq.exec();
      if (postq.first())
      {
        int result = postq.value("result").toInt();
        if (result < 0)
        {
          rollback.exec();
          systemError( this, storedProcErrorLookup("postCreditMemo", result),
                __FILE__, __LINE__);
          return;
        }
        else
        {
          if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
          {
            rollback.exec();
            QMessageBox::information( this, tr("Post Credit Memo"), tr("Transaction Canceled") );
            return;
          }

          q.exec("COMMIT;");
        }
      }
      // contains() string is hard-coded in stored procedure
      else if (postq.lastError().databaseText().contains("post to closed period"))
      {
        rollback.exec();
        if (changeDate)
        {
          triedToClosed = selected;
          break;
        }
        else
          triedToClosed.append(selected[i]);
      }
      else if (postq.lastError().type() != QSqlError::None)
      {
        rollback.exec();
        systemError(this, tr("A System Error occurred posting Credit Memo#%1.\n%2")
                .arg(selected[i]->text(0))
                .arg(postq.lastError().databaseText()),
              __FILE__, __LINE__);
      }
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _cmhead->headerItem(), _cmhead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }

  } while (tryagain);

  omfgThis->sCreditMemosUpdated();
}

void unpostedCreditMemos::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Credit Memos?"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Credit Memos?"),
                            QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery delq;
    delq.prepare("SELECT deleteCreditMemo(:cmhead_id) AS result;");

    QList<QTreeWidgetItem *> selected = _cmhead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      delq.bindValue(":cmhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      delq.exec();
      if (delq.first())
      {
	if (! delq.value("result").toBool())
	  systemError(this, tr("Could not delete Credit Memo."),
		      __FILE__, __LINE__);
      }
      else if (delq.lastError().type() != QSqlError::None)
	systemError(this,
		    tr("Error deleting Credit Memo %1\n").arg(selected[i]->text(0)) +
		    delq.lastError().databaseText(), __FILE__, __LINE__);
    }

    omfgThis->sCreditMemosUpdated();
  }
}

void unpostedCreditMemos::sFillList()
{
  _cmhead->clear();
  _cmhead->populate( "SELECT cmhead_id, cmhead_number,"
                     "       formatBoolYN(cmhead_printed),"
                     "       cmhead_billtoname,"
                     "       formatDate(cmhead_docdate),"
                     "       formatBoolYN(cmhead_hold),"
		     "       formatDate(COALESCE(cmhead_gldistdate, cmhead_docdate)) "
                     "FROM cmhead "
                     "WHERE (NOT cmhead_posted) "
                     "ORDER BY cmhead_number;" );
}
