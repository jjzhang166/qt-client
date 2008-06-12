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

#include "maintainBudget.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <xlistbox.h>
#include <accountList.h>
#include <glcluster.h>
#include <openreports.h>

/*
 *  Constructs a maintainBudget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
maintainBudget::maintainBudget(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_accountsAdd, SIGNAL(clicked()), this, SLOT(sAccountsAdd()));
  connect(_accountsDown, SIGNAL(clicked()), this, SLOT(sAccountsMoveDown()));
  connect(_accountsRemove, SIGNAL(clicked()), this, SLOT(sAccountsRemove()));
  connect(_accountsUp, SIGNAL(clicked()), this, SLOT(sAccountsMoveUp()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_generate, SIGNAL(clicked()), this, SLOT(sGenerateTable()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_table, SIGNAL(valueChanged(int,int)), this, SLOT(sValueChanged(int,int)));
  connect(_periodsAll, SIGNAL(clicked()), this, SLOT(sPeriodsAll()));
  connect(_periodsInvert, SIGNAL(clicked()), _periods, SLOT(invertSelection()));
  connect(_periodsNone, SIGNAL(clicked()), _periods, SLOT(clearSelection()));

  _dirty = false;
  _budgheadid = -1;
  _mode = cNew;
  
  // populate _periods
  q.exec("SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end)) AS f_name "
         "  FROM period "
         "ORDER BY period_start DESC;" );
  while(q.next())
  {
    XListBoxText *item = new XListBoxText(q.value("f_name").toString(), q.value("period_id").toInt());
    _periods->insertItem(item);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
maintainBudget::~maintainBudget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void maintainBudget::languageChange()
{
  retranslateUi(this);
}

enum SetResponse maintainBudget::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("budghead_id", &valid);
  if(valid)
  {
    _budgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
    {
      _mode = cNew;
      _print->setEnabled(false);
    }
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _name->setReadOnly(true);
      _descrip->setReadOnly(true);
      _save->setEnabled(false);
      _save->hide();
      _table->setReadOnly(true);
      _accountsAdd->setEnabled(false);
      _accountsDown->setEnabled(false);
      _accountsUp->setEnabled(false);
      _accountsRemove->setEnabled(false);
      _periodsAll->setEnabled(false);
      _periodsInvert->setEnabled(false);
      _periodsNone->setEnabled(false);
      _generate->setEnabled(false);
      _close->setFocus();
    }
  }

  return NoError;
}

void maintainBudget::sSave()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Budget"),
        tr("You must specify a name for this budget before saving."));
    _name->setFocus();
    return;
  }

  _save->setFocus();

  if(cEdit == _mode)
    q.prepare("UPDATE budghead "
              "   SET budghead_name=:name,"
              "       budghead_descrip=:descrip "
              " WHERE(budghead_id=:budghead_id);");
  else if(cNew == _mode)
  {
    q.prepare("SELECT nextval('budghead_budghead_id_seq') AS result;");
    q.exec();
    if(q.first())
      _budgheadid = q.value("result").toInt();
    else
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("INSERT INTO budghead"
              "      (budghead_id, budghead_name, budghead_descrip) "
              "VALUES(:budghead_id, :name, :descrip);");
  }

  q.bindValue(":budghead_id", _budgheadid);
  q.bindValue(":name", _name->text());
  q.bindValue(":descrip", _descrip->text());
  if(!q.exec())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT setBudget(:budghead_id, :period_id, :accnt_id, :amount) AS result;");

  for(int r = 0; r < _table->numRows(); r++)
  {
    int accountid = *(_accountsRef.at(r));
    for(int c = 1; c < _table->numCols(); c++)
    {
      QString amount = _table->text(r, c);
      if(!amount.isEmpty())
      {
        q.bindValue(":budghead_id", _budgheadid);
        q.bindValue(":period_id", *(_periodsRef.at(c)));
        q.bindValue(":accnt_id", accountid);
        q.bindValue(":amount", amount.toDouble());
        q.exec();
      }
    }
  }
 
  _dirty = false;

  omfgThis->sBudgetsUpdated(_budgheadid, true);

  if(cNew == _mode)
  {
    _name->clear();
    _descrip->clear();
    _accounts->clear();
    _periods->clearSelection();
    _accountsRef.clear();
    _periodsRef.clear();
    _table->setNumRows(0);
    _table->setNumCols(0);
  }
  else
    close();
}

void maintainBudget::closeEvent( QCloseEvent * e )
{
  _close->setFocus();
  if(!_dirty)
  {
    e->accept();
    return;
  }

  switch( QMessageBox::information( this, tr("Unsaved Changes"),
                                    "The document has been changed since "
                                    "the last save.",
                                    "Save Now", "Cancel", "Close Anyway",
                                    0, 1 ) ) {
    case 0:
      sSave();
      e->accept();
      break;
    case 1:
    default: // just for sanity
      e->ignore();
      break;
    case 2:
      e->accept();
      break;
  }
}

void maintainBudget::sAccountsAdd()
{
  ParameterList params;
  params.append("type", (GLCluster::cAsset | GLCluster::cLiability | GLCluster::cExpense | GLCluster::cRevenue | GLCluster::cEquity));

  accountList newdlg(this, "", true);
  newdlg._accnt->setSelectionMode(QAbstractItemView::ExtendedSelection);
  newdlg.set(params);
  int accnt_id=newdlg.exec();

  if (accnt_id == -1)
    return;

  QList<QTreeWidgetItem*> selected = newdlg._accnt->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *child = (XTreeWidgetItem*)selected[i];
    q.prepare("SELECT formatGLAccountLong(:accnt_id) AS result;");
    q.bindValue(":accnt_id", child->id());
    q.exec();
    if(q.first())
    {
      XListBoxText *item = new XListBoxText(q.value("result").toString(), child->id());
      _accounts->insertItem(item);
    }
  }
}

void maintainBudget::sAccountsRemove()
{
  int idx = _accounts->currentItem();
  if(idx != -1)
    _accounts->removeItem(idx);
}

void maintainBudget::sAccountsMoveUp()
{
  int idx = _accounts->currentItem();
  if(idx <= 0)
    return;

  Q3ListBoxItem * item = _accounts->item(idx);
  _accounts->takeItem(item);
  _accounts->insertItem(item, idx - 1);
  _accounts->setCurrentItem(item);
}

void maintainBudget::sAccountsMoveDown()
{
  int idx = _accounts->currentItem();
  if(idx == -1)
    return;

  Q3ListBoxItem * item = _accounts->item(idx);
  _accounts->takeItem(item);
  _accounts->insertItem(item, idx + 1);
  _accounts->setCurrentItem(item);
}

void maintainBudget::sPeriodsAll()
{
  _periods->selectAll(TRUE);
}

void maintainBudget::sValueChanged( int, int )
{
  _dirty = true;
}

void maintainBudget::sGenerateTable()
{
  _generate->setFocus();
  if(_dirty)
  {
    switch( QMessageBox::information( this, tr("Unsaved Changes"),
                                      "The document has been changed since "
                                      "the last save.",
                                      "Save Now", "Cancel", "Generate Anyway",
                                      0, 1 ) ) {
      case 0:
        sSave();
        break;
      case 1:
      default: // just for sanity
        return;
        break;
      case 2:
        break;
    }
  }

  _dirty = false;

  _accountsRef.clear();
  _periodsRef.clear();
  _table->setNumRows(0);
  _table->setNumCols(0);

  QStringList accounts;
  QStringList periods;

  XListBoxText* item = (XListBoxText*)_accounts->firstItem();
  while(item)
  {
    _accountsRef.append(item->id());
    accounts.append(item->text());
    item = (XListBoxText*)item->next();
  }
  _table->setNumRows(accounts.count());
  //_table->setRowLabels(accounts);

  item = (XListBoxText*)_periods->firstItem();
  while(item)
  {
    if(item->isSelected())
    {
      _periodsRef.prepend(item->id());
      periods.prepend(item->text());
    }
    item = (XListBoxText*)item->next();
  }
  _periodsRef.prepend(-1);
  periods.prepend(tr("Account"));
  _table->setNumCols(periods.count());
  _table->setColumnLabels(periods);

  periods.clear();
  Q3ValueList<int>::iterator it;
  for( it = _periodsRef.begin(); it != _periodsRef.end(); ++it )
  {
    if(-1 != (*it))
      periods.append(QString::number((*it)));
  }

  QString sql = "SELECT budgitem_period_id, budgitem_amount"
                "  FROM budgitem"
                " WHERE ((budgitem_accnt_id=:accnt_id)"
                "   AND  (budgitem_budghead_id=:budghead_id)"
                "   AND  (budgitem_period_id IN (" + periods.join(",") + ")) ); ";
  q.prepare(sql);
  
  for(int i = 0; i < _accountsRef.count(); i++)
  {
    _table->setText(i, 0, (accounts.at(i)));
    q.bindValue(":accnt_id", *(_accountsRef.at(i)));
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
      _table->setText(i, _periodsRef.findIndex(q.value("budgitem_period_id").toInt()), q.value("budgitem_amount").toString());
  }
  _table->setColumnReadOnly(0, true);
}

void maintainBudget::populate()
{
  q.prepare("SELECT budghead_name, budghead_descrip"
            "  FROM budghead"
            " WHERE(budghead_id=:budghead_id);");
  q.bindValue(":budghead_id", _budgheadid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("budghead_name").toString());
    _descrip->setText(q.value("budghead_descrip").toString());

    q.prepare("SELECT DISTINCT budgitem_accnt_id, formatGLAccountLong(budgitem_accnt_id) AS result"
              "  FROM budgitem"
              " WHERE(budgitem_budghead_id=:budghead_id);");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      XListBoxText *item = new XListBoxText(q.value("result").toString(), q.value("budgitem_accnt_id").toInt());
      _accounts->insertItem(item);
    }

    q.prepare("SELECT DISTINCT budgitem_period_id"
              "  FROM budgitem"
              " WHERE(budgitem_budghead_id=:budghead_id);");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      XListBoxText* item = (XListBoxText*)_periods->firstItem();
      while(item)
      {
        if(item->id() == q.value("budgitem_period_id").toInt())
        {
          _periods->setSelected(item, true);
          break;
        }
        item = (XListBoxText*)item->next();
      }
    }
    sGenerateTable();
  }
}

void maintainBudget::sPrint()
{
  ParameterList params;
  params.append("budghead_id", _budgheadid);

  orReport report("Budget", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}
