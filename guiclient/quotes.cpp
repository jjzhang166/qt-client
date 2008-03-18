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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "quotes.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QThread>
#include <QVariant>
#include <QWorkspace>

#include <parameter.h>
#include <openreports.h>
#include <metasql.h>

#include "failedPostList.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"
#include "customer.h"

quotes::quotes(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_quote, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_convert, SIGNAL(clicked()), this, SLOT(sConvert()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showProspects, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _quote->addColumn(tr("Quote #"),    _orderColumn, Qt::AlignRight  );
  _quote->addColumn(tr("Customer"),   -1,           Qt::AlignLeft   );
  _quote->addColumn(tr("P/O Number"), _itemColumn,  Qt::AlignLeft   );
  _quote->addColumn(tr("Quote Date"), _dateColumn,  Qt::AlignCenter );
  _quote->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (_privleges->check("PrintQuotes"))
    connect(_quote, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  if (_privleges->check("ConvertQuotes"))
    connect(_quote, SIGNAL(valid(bool)), _convert, SLOT(setEnabled(bool)));

  if (_privleges->check("MaintainQuotes"))
  {
    connect(_quote, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_quote, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_quote, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_quote, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillList()));

  Preferences _pref = Preferences(omfgThis->username());
  if (_pref.boolean("XCheckBox/forgetful"))
    _showProspects->setChecked(true);

  sFillList();
}

quotes::~quotes()
{
  // no need to delete child widgets, Qt does it all for us
}

void quotes::languageChange()
{
  retranslateUi(this);
}

void quotes::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrint()), 0);
  if (!_privleges->check("PrintQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Convert..."), this, SLOT(sConvert()), 0);
  if (!_privleges->check("ConvertQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privleges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void quotes::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool setupPrinter = TRUE;
  q.prepare( "SELECT findCustomerForm(quhead_cust_id, 'Q') AS reportname "
             "FROM quhead "
             "WHERE (quhead_id=:quheadid); " );
  bool userCanceled = false;
  QList<QTreeWidgetItem *> selected = _quote->selectedItems();

  if (selected.size() > 0 &&
      orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("<p>Could not initialize printing system for "
			   "multiple reports."));
    return;
  }
  for (int i = 0; i < selected.size(); i++)
  {
    int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
    q.bindValue(":quheadid", quheadid);
    q.exec();
    if (q.first())
    {
      ParameterList params;
      params.append("quhead_id", quheadid);

      orReport report(q.value("reportname").toString(), params);
      if (report.isValid() && report.print(&printer, setupPrinter))
	setupPrinter = FALSE;
      else
      {
	report.reportError(this);
	break;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      break;
    }
  }
  if (selected.size() > 0)
    orReport::endMultiPrint(&printer);
}

void quotes::sConvert()
{
  if (QMessageBox::question(this, tr("Convert Selected Quote(s)"),
			    tr("<p>Are you sure that you want to convert "
			       "the selected Quote(s) to Sales Order(s)?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    XSqlQuery prospectq;
    prospectq.prepare("SELECT convertProspectToCustomer(quhead_cust_id) AS result "
		      "FROM quhead "
		      "WHERE (quhead_id=:quhead_id);");

    bool tryagain = false;
    do {
      int soheadid = -1;
      QList<QTreeWidgetItem*> selected = _quote->selectedItems();
      QList<QTreeWidgetItem*> notConverted;

      for (int i = 0; i < selected.size(); i++)
      {
        int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
        convert.bindValue(":quhead_id", quheadid);
        convert.exec();
        if (convert.first())
        {
          soheadid = convert.value("sohead_id").toInt();
          if (soheadid == -3)
          {
            if ((_metrics->value("DefaultSalesRep").toInt() > 0) &&
                (_metrics->value("DefaultTerms").toInt() > 0) &&
                (_metrics->value("DefaultCustType").toInt() > 0) && 
                (_metrics->value("DefaultShipFormId").toInt() > 0)  && 
                (_privleges->check("MaintainCustomerMasters"))) 
              {
                if (QMessageBox::question(this, tr("Quote for Prospect"),
                              tr("<p>This Quote is for a Prospect, not "
                             "a Customer. Do you want to convert "
                             "the Prospect to a Customer using global "
                             "default values?"),
                        QMessageBox::Yes,
                        QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
                {
                  prospectq.bindValue(":quhead_id", quheadid);
                  prospectq.exec();
                  if (prospectq.first())
                  {
                    int result = prospectq.value("result").toInt();
                    if (result < 0)
                    {
                      systemError(this,
                                  storedProcErrorLookup("convertProspectToCustomer",
                                  result), __FILE__, __LINE__);
                      notConverted.append(selected[i]);
                      continue;
                    }
                    convert.exec();
                    if (convert.first())
                    {
                      soheadid = convert.value("sohead_id").toInt();
                      if (soheadid < 0)
                      {
                        QMessageBox::warning(this, tr("Cannot Convert Quote"),
                                storedProcErrorLookup("convertQuote", soheadid)
                                .arg(selected[i] ? selected[i]->text(0) : ""));
                        notConverted.append(selected[i]);
                        continue;
                      }
                    }
                  }
                  else if (prospectq.lastError().type() != QSqlError::None)
                  {
                    systemError(this, prospectq.lastError().databaseText(),
                            __FILE__, __LINE__);
                    notConverted.append(selected[i]);
                    continue;
                  }
                }
                else
                {
                  QMessageBox::information(this, tr("Quote for Prospect"),
                              tr("<p>The prospect must be manually "
                                 "converted to customer from either the "
                                 "CRM Account or Customer windows before "
                                 "coverting this quote."));
                  notConverted.append(selected[i]);
                  continue;
                }
            }
            else
            {
              QMessageBox::information(this, tr("Quote for Prospect"),
                          tr("<p>The prospect must be manually "
                             "converted to customer from either the "
                             "CRM Account or Customer windows before "
                             "coverting this quote."));
              notConverted.append(selected[i]);
              continue;
            }
          }
          else if (soheadid < 0)
          {
            QMessageBox::warning(this, tr("Cannot Convert Quote"),
                    storedProcErrorLookup("convertQuote", soheadid)
                    .arg(selected[i] ? selected[i]->text(0) : ""));
            notConverted.append(selected[i]);
            continue;
          }
          omfgThis->sQuotesUpdated(quheadid);
          omfgThis->sSalesOrdersUpdated(soheadid);

          salesOrder::editSalesOrder(soheadid, true);
        }
        else if (convert.lastError().type() != QSqlError::None)
        {
          notConverted.append(selected[i]);
          systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);
          continue;
        }
      }

      if (notConverted.size() > 0)
      {
	failedPostList newdlg(this, "", true);
	newdlg.setLabel(tr("<p>The following Quotes could not be converted."));
	newdlg.sSetList(notConverted, _quote->headerItem(), _quote->header());
	tryagain = (newdlg.exec() == QDialog::Accepted);
	selected = notConverted;
	notConverted.clear();
      }
    } while (tryagain);
  } // if user wants to convert
}

void quotes::sNew()
{
  ParameterList params;
  params.append("mode", "newQuote");
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void quotes::sEdit()
{
  QList<QTreeWidgetItem *> selected = _quote->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode", "editQuote");
    params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
  
    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
    break;
  }
}

void quotes::sView()
{
  QList<QTreeWidgetItem *> selected = _quote->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    ParameterList params;
    params.append("mode", "viewQuote");
    params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    
    salesOrder *newdlg = new salesOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
    break;
  }
}

void quotes::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Quotes"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Quotes?" ),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default)
			    == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteQuote(:quhead_id) AS result;");

    int counter = 0;
    QList<QTreeWidgetItem *> selected = _quote->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteQuote", result),
		      __FILE__, __LINE__);
	  continue;
	}
	counter++;
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, tr("A System Error occurred deleting Quote #%1\n%2.")
			   .arg(selected[i]->text(0))
			   .arg(q.lastError().databaseText()), __FILE__, __LINE__);
	continue;
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);
  }
}

void quotes::sFillList()
{
  QString sql("SELECT DISTINCT quhead_id, quhead_number, quhead_billtoname,"
              "                quhead_custponumber, formatDate(quhead_quotedate) "
              "FROM quhead "
	      " <? if exists(\"customersOnly\") ?>"
	      "     JOIN custinfo ON (quhead_cust_id=cust_id) "
	      " <? endif ?>"
	      " <? if exists(\"warehous_id\") ?>"
              "     LEFT OUTER JOIN quitem "
	      "     JOIN itemsite ON (quitem_itemsite_id=itemsite_id) "
              "              ON (quitem_quhead_id=quhead_id) "
	      " <? endif ?>"
	      " <? if exists(\"warehous_id\") ?>"
	      " WHERE (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	      " <? endif ?>"
	      "ORDER BY quhead_number;");

  ParameterList params;
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
  if (! _showProspects->isChecked())
    params.append("customersOnly");

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _quote->clear();
  _quote->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
