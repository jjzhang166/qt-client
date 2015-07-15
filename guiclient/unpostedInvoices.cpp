/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedInvoices.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include  <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "invoice.h"
#include "mqlutil.h"
#include "printInvoice.h"
#include "storedProcErrorLookup.h"
#include "distributeInventory.h"

unpostedInvoices::unpostedInvoices(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : display(parent, "unpostedInvoices", fl)
{
  if (name)
    setObjectName(name);

  setupUi(optionsWidget());
  setWindowTitle(tr("Unposted Invoices"));
  setListLabel(tr("Unposted Invoices"));
  setMetaSQLOptions("invoices", "detail");
  setNewVisible(true);

  list()->addColumn(tr("Invoice #"),     _orderColumn,    Qt::AlignLeft,   true,  "invchead_invcnumber" );
  list()->addColumn(tr("Prnt'd"),        _orderColumn,    Qt::AlignCenter, true,  "invchead_printed" );
  list()->addColumn(tr("S/O #"),         _orderColumn,    Qt::AlignLeft,   true,  "invchead_ordernumber" );
  list()->addColumn(tr("Customer"),      -1,              Qt::AlignLeft,   true,  "cust_name" );
  list()->addColumn(tr("Ship-to"),       100,             Qt::AlignLeft,   false, "invchead_shipto_name" );
  list()->addColumn(tr("Invc. Date"),    _dateColumn,     Qt::AlignCenter, true,  "invchead_invcdate" );
  list()->addColumn(tr("Ship Date"),     _dateColumn,     Qt::AlignCenter, true,  "invchead_shipdate" );
  list()->addColumn(tr("G/L Dist Date"), _dateColumn,     Qt::AlignCenter, true,  "gldistdate" );
  list()->addColumn(tr("Recurring"),     _ynColumn,       Qt::AlignCenter, false, "isRecurring" );
  list()->addColumn(tr("Ship Date"),     _dateColumn,     Qt::AlignCenter, false, "invchead_shipdate" );
  list()->addColumn(tr("P/O #"),         _orderColumn,    Qt::AlignCenter, false, "invchead_ponumber" );
  list()->addColumn(tr("Total Amount"),  _bigMoneyColumn, Qt::AlignRight,  true,  "extprice" );
  list()->addColumn(tr("Margin"),        _bigMoneyColumn, Qt::AlignRight,  true,  "margin" );
  list()->addColumn(tr("Margin %"),      _prcntColumn,    Qt::AlignRight,  true,  "marginpercent");
  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (! _privileges->check("ChangeARInvcDistDate"))
    list()->hideColumn(7);

  if (_privileges->check("MaintainMiscInvoices"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

void unpostedInvoices::sNew()
{
  invoice::newInvoice(-1);
}

void unpostedInvoices::sEdit()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::editInvoice(((XTreeWidgetItem*)(selected[i]))->id());
      
}

void unpostedInvoices::sView()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::viewInvoice(((XTreeWidgetItem*)(selected[i]))->id());
}

void unpostedInvoices::sDelete()
{
  XSqlQuery unpostedDelete;
  if ( QMessageBox::warning( this, tr("Delete Selected Invoices"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Invoices?"),
                             tr("Delete"), tr("Cancel"), QString::null, 1, 1 ) == 0)
  {
    unpostedDelete.prepare("SELECT deleteInvoice(:invchead_id) AS result;");

    QList<XTreeWidgetItem*> selected = list()->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
	  {
        unpostedDelete.bindValue(":invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        unpostedDelete.exec();
        if (unpostedDelete.first())
        {
	      int result = unpostedDelete.value("result").toInt();
	      if (result < 0)
	      {
	        systemError(this, storedProcErrorLookup("deleteInvoice", result),
		            __FILE__, __LINE__);
	      }
        }
        else if (unpostedDelete.lastError().type() != QSqlError::NoError)
	      systemError(this,
		          tr("Error deleting Invoice %1\n").arg(selected[i]->text(0)) +
		          unpostedDelete.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    omfgThis->sInvoicesUpdated(-1, true);
    omfgThis->sBillingSelectionUpdated(-1, -1);
  }
}

void unpostedInvoices::sPrint()
{
  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  printInvoice newdlg(this, "", true);

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("invchead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("persistentPrint");

      newdlg.set(params);

      if (!newdlg.isSetup())
      {
        if(newdlg.exec() == QDialog::Rejected)
          break;
        newdlg.setSetup(true);
      }
    }
  }

  omfgThis->sInvoicesUpdated(-1, true);
}

void unpostedInvoices::sPost()
{
  XSqlQuery unpostedPost;
  bool changeDate = false;
  QDate newDate = QDate();

  if (_privileges->check("ChangeARInvcDistDate"))
  {
    getGLDistDate newdlg(this, "", true);
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
  unpostedPost.exec("SELECT fetchJournalNumber('AR-IN') AS result;");
  if (unpostedPost.first())
  {
    journal = unpostedPost.value("result").toInt();
    if (journal < 0)
    {
      systemError(this, storedProcErrorLookup("fetchJournalNumber", journal), __FILE__, __LINE__);
      return;
    }
  }
  else if (unpostedPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, unpostedPost.lastError().databaseText(), __FILE__, __LINE__);
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
  sum.prepare("SELECT invoicetotal(:invchead_id) AS subtotal;");

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery post;
  post.prepare("SELECT postInvoice(:invchead_id, :journal) AS result;");

  XSqlQuery setDate;
  setDate.prepare("UPDATE invchead SET invchead_gldistdate=:distdate "
		  "WHERE invchead_id=:invchead_id;");

  QList<XTreeWidgetItem*> selected = list()->selectedItems();
  QList<XTreeWidgetItem*> triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      // always change gldistdate.  if using invoice date then will set to null
      //if (changeDate)
      //{
        setDate.bindValue(":distdate",    newDate);
        setDate.bindValue(":invchead_id", id);
        setDate.exec();
        if (setDate.lastError().type() != QSqlError::NoError)
        {
	      systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
        }
      //}
    }
  }

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
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

        unpostedPost.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
        post.bindValue(":invchead_id", id);
        post.bindValue(":journal",     journal);
        post.exec();
        if (post.first())
        {
	      int result = post.value("result").toInt();
	      if (result < 0)
              {
                rollback.exec();
                systemError(this, storedProcErrorLookup("postInvoice", result),
		            __FILE__, __LINE__);
              }
              else if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
              {
                rollback.exec();
                QMessageBox::information( this, tr("Post Invoices"), tr("Transaction Canceled") );
                return;
              }

              unpostedPost.exec("COMMIT;");
        }
        // contains() string is hard-coded in stored procedure
        else if (post.lastError().databaseText().contains("post to closed period"))
        {
          rollback.exec();
            if (changeDate)
              triedToClosed = selected;
            else
              triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::NoError)
      {
        rollback.exec();
        systemError(this, tr("A System Error occurred posting Invoice #%1.\n%2")
                    .arg(selected[i]->text(0))
                        .arg(post.lastError().databaseText()),
                        __FILE__, __LINE__);
      }
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, list()->headerItem(), list()->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
      }
    }
  } while (tryagain);

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("source", "A/R");
    params.append("sourceLit", tr("A/R"));
    params.append("startJrnlnum", journal);
    params.append("endJrnlnum", journal);

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
}

void unpostedInvoices::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  if (!_privileges->check("MaintainMiscInvoices"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  if ((!_privileges->check("MaintainMiscInvoices")) && (!_privileges->check("ViewMiscInvoices")))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  if (!_privileges->check("MaintainMiscInvoices"))
    menuItem->setEnabled(false);

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrint()));
  if (!_privileges->check("PrintInvoices"))
    menuItem->setEnabled(false);

  menuItem = pMenu->addAction(tr("Post..."), this, SLOT(sPost()));
  if (!_privileges->check("PostMiscInvoices"))
    menuItem->setEnabled(false);
}

bool unpostedInvoices::checkSitePrivs(int invcid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkInvoiceSitePrivs(:invcheadid) AS result;");
    check.bindValue(":invcheadid", invcid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Invoice as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

void unpostedInvoices::sFillList()
{
  ParameterList params;
  params.append("unpostedOnly");
  display::sFillList(params, true);
}
