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

#include "unpostedInvoices.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include  <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "invoice.h"
#include "printInvoice.h"
#include "storedProcErrorLookup.h"

unpostedInvoices::unpostedInvoices(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_invchead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _invchead->addColumn(tr("Invoice #"),  _orderColumn, Qt::AlignLeft,   true,  "invchead_invcnumber" );
  _invchead->addColumn(tr("Prnt'd"),     _orderColumn, Qt::AlignCenter, true,  "invchead_printed" );
  _invchead->addColumn(tr("S/O #"),      _orderColumn, Qt::AlignLeft,   true,  "invchead_ordernumber" );
  _invchead->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,   true,  "cust_name" );
  _invchead->addColumn(tr("Invc. Date"), _dateColumn,  Qt::AlignCenter, true,  "invchead_invcdate" );
  _invchead->addColumn(tr("Ship Date"),  _dateColumn,  Qt::AlignCenter, true,  "invchead_shipdate" );
  _invchead->addColumn(tr("G/L Dist Date"),_dateColumn,Qt::AlignCenter, true,  "invchead_gldistdate" );
  _invchead->addColumn(tr("Recurring"),  _ynColumn,    Qt::AlignCenter, false, "invchead_recurring" );
  _invchead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (! _privileges->check("ChangeARInvcDistDate"))
    _invchead->hideColumn(6);

  if (_privileges->check("MaintainMiscInvoices"))
  {
    _new->setEnabled(TRUE);
    connect(_invchead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_invchead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  if (_privileges->check("MaintainMiscInvoices") || _privileges->check("ViewMiscInvoices"))
    connect(_invchead, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  if (_privileges->check("PrintInvoices"))
    connect(_invchead, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  
  if (_privileges->check("PostMiscInvoices"))
  {
    connect(_invchead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(valid(bool)), _printJournal, SLOT(setEnabled(bool)));
  }

  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  sFillList();
}

unpostedInvoices::~unpostedInvoices()
{
    // no need to delete child widgets, Qt does it all for us
}

void unpostedInvoices::languageChange()
{
    retranslateUi(this);
}

void unpostedInvoices::sNew()
{
  invoice::newInvoice(-1);
}

void unpostedInvoices::sEdit()
{
  QList<QTreeWidgetItem *> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      invoice::editInvoice(((XTreeWidgetItem*)(selected[i]))->id());
}

void unpostedInvoices::sView()
{
  QList<QTreeWidgetItem *> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      invoice::viewInvoice(((XTreeWidgetItem*)(selected[i]))->id());
}

void unpostedInvoices::sDelete()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Invoices"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Invoices?"),
                             tr("Delete"), tr("Cancel"), QString::null, 1, 1 ) == 0)
  {
    q.prepare("SELECT deleteInvoice(:invchead_id) AS result;");

    QList<QTreeWidgetItem *> selected = _invchead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteInvoice", result),
		      __FILE__, __LINE__);
	}
      }
      else if (q.lastError().type() != QSqlError::None)
	systemError(this,
		    tr("Error deleting Invoice %1\n").arg(selected[i]->text(0)) +
		    q.lastError().databaseText(), __FILE__, __LINE__);
    }

    omfgThis->sInvoicesUpdated(-1, TRUE);
    omfgThis->sBillingSelectionUpdated(-1, -1);
  }
}

void unpostedInvoices::sPrint()
{
  QList<QTreeWidgetItem *> selected = _invchead->selectedItems();
  printInvoice newdlg(this, "", TRUE);

  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    params.append("persistentPrint");

    newdlg.set(params);

    if (!newdlg.isSetup())
    {
      newdlg.exec();
      newdlg.setSetup(TRUE);
    }
  }

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void unpostedInvoices::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeARInvcDistDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Invoice Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  int journal = -1;
  q.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (q.first())
  {
    journal = q.value("result").toInt();
    if (journal < 0)
    {
      systemError(this, storedProcErrorLookup("fetchJournalNumber", journal), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery xrate;
  xrate.prepare("SELECT curr_rate "
		"FROM curr_rate, invchead "
		"WHERE ((curr_id=invchead_curr_id)"
		"  AND  (invchead_id=:invchead_id)"
		"  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
  // if SUM becomes dependent on curr_id then move XRATE before it in the loop
  XSqlQuery sum;
  sum.prepare("SELECT COALESCE(SUM(round((invcitem_billed * invcitem_qty_invuomratio) *"
	      "                 (invcitem_price / "
	      "                  CASE WHEN (item_id IS NULL) THEN 1"
	      "                       ELSE invcitem_price_invuomratio"
	      "                  END), 2)),0) + "
	      "       invchead_freight + invchead_tax + "
	      "       invchead_misc_amount AS subtotal "
	      "  FROM invchead LEFT OUTER JOIN invcitem ON (invcitem_invchead_id=invchead_id) LEFT OUTER JOIN"
	      "       item ON (invcitem_item_id=item_id) "
	      " WHERE(invchead_id=:invchead_id) "
	      " GROUP BY invchead_freight, invchead_tax, invchead_misc_amount;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  QList<QTreeWidgetItem *> selected = _invchead->selectedItems();
  QList<QTreeWidgetItem *> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();

    if (changeDate)
    {
      setDate.bindValue(":distdate",    newDate);
      setDate.bindValue(":invchead_id", id);
      setDate.exec();
      if (setDate.lastError().type() != QSqlError::None)
      {
	systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      sum.bindValue(":invchead_id", id);
      if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
      {
	if (QMessageBox::question(this, tr("Invoice Has Value 0"),
				  tr("Invoice #%1 has a total value of 0.\n"
				     "Would you like to post it anyway?")
				    .arg(selected[i]->text(0)),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default)
	      == QMessageBox::No)
	  continue;
      }
      else if (sum.lastError().type() != QSqlError::NoError)
      {
	systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
	continue;
      }
      else if (sum.value("subtotal").toDouble() != 0)
      {
	xrate.bindValue(":invchead_id", id);
	xrate.exec();
	if (xrate.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, tr("System Error posting Invoice #%1\n%2")
			      .arg(selected[i]->text(0))
			      .arg(xrate.lastError().databaseText()),
		      __FILE__, __LINE__);
	  continue;
	}
	else if (!xrate.first() || xrate.value("curr_rate").isNull())
	{
	  systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
			      .arg(selected[i]->text(0)));
	  continue;
	}
      }

      post.bindValue(":invchead_id", id);
      post.bindValue(":journal",     journal);
      post.exec();
      if (post.first())
      {
	int result = post.value("result").toInt();
	if (result < 0)
	  systemError(this, storedProcErrorLookup("postInvoice", result),
		      __FILE__, __LINE__);
      }
      // contains() string is hard-coded in stored procedure
      else if (post.lastError().databaseText().contains("post to closed period"))
      {
	if (changeDate)
	{
	  triedToClosed = selected;
	  break;
	}
	else
	  triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::None)
	systemError(this, tr("A System Error occurred posting Invoice #%1.\n%2")
			    .arg(selected[i]->text(0))
			    .arg(post.lastError().databaseText()),
		    __FILE__, __LINE__);
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _invchead->headerItem(), _invchead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("journalNumber", journal);

    orReport report("SalesJournal", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }

  omfgThis->sInvoicesUpdated(-1, TRUE);
}

void unpostedInvoices::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainMiscInvoices")) && (!_privileges->check("ViewMiscInvoices")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrint()), 0);
  if (!_privileges->check("PrintInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Post..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void unpostedInvoices::sFillList()
{
  _invchead->clear();
  XSqlQuery fill;
  fill.prepare("SELECT invchead_id, invchead_invcnumber,"
	    "       formatBoolYN(invchead_printed) AS invchead_printed,"
	    "       invchead_ordernumber, cust_name,"
	    "       invchead_invcdate, "
	    "       invchead_shipdate, "
	    "       COALESCE(invchead_gldistdate, invchead_invcdate) AS invchead_gldistdate,"
            "       formatBoolYN(invchead_recurring) AS invchead_recurring "
	    "FROM invchead, cust "
	    "WHERE ( (invchead_cust_id=cust_id)"
	    " AND (NOT(invchead_posted)) ) "
	    "ORDER BY invchead_invcnumber;" );
  fill.exec();
  _invchead->populate(fill);
  if (fill.lastError().type() != QSqlError::None)
  {
    systemError(this, fill.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
