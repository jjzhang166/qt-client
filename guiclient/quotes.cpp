/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "quotes.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <applock.h>
#include <parameter.h>
#include <openreports.h>

#include "customer.h"
#include "errorReporter.h"
#include "failedPostList.h"
#include "parameterwidget.h"
#include "printQuote.h"
#include "salesOrder.h"
#include "copyQuote.h"
#include "storedProcErrorLookup.h"

quotes::quotes(QWidget* parent, const char *name, Qt::WindowFlags fl)
  : display(parent, "quotes", fl)
{
  setupUi(optionsWidget());
  if (name)
    setObjectName(name);

  setWindowTitle(tr("Quotes"));
  setMetaSQLOptions("quotes", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);
  setAutoUpdateEnabled(true);

  _convertedtoSo->setVisible(false);

  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Exclude Prospects"), "customersOnly", ParameterWidget::Exists);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Customer Type"), "custtype_id", XComboBox::CustomerTypes);
  parameterWidget()->append(tr("Customer Type Pattern"), "custtype_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("P/O Number"), "poNumber", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Sales Rep."), "salesrep_id", XComboBox::SalesRepsActive);
  parameterWidget()->append(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget()->append(tr("End Date"),   "endDate",   ParameterWidget::Date);

  list()->addColumn(tr("Quote #"),     _orderColumn,  Qt::AlignLeft,  true,  "quhead_number");
  list()->addColumn(tr("Customer"),    _itemColumn,   Qt::AlignLeft,  true,  "quhead_billtoname");
  list()->addColumn(tr("P/O Number"),  _orderColumn,  Qt::AlignLeft,  true,  "quhead_custponumber");
  list()->addColumn(tr("Status"),      _dateColumn,   Qt::AlignCenter,true,  "quhead_status");
  list()->addColumn(tr("Quote Date"),  _dateColumn,   Qt::AlignCenter,true,  "quhead_quotedate");
  list()->addColumn(tr("Expire Date"), _dateColumn,   Qt::AlignCenter,false, "quhead_expire");
  list()->addColumn(tr("Total"),       _moneyColumn,  Qt::AlignRight, true,  "ordertotal");
  list()->addColumn(tr("Notes"),       -1,            Qt::AlignLeft,  true,  "notes");
  list()->setSelectionMode(QAbstractItemView::ExtendedSelection);

  setupCharacteristics("QU");
  parameterWidget()->applyDefaultFilterSet();

  if (_privileges->check("MaintainQuotes"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sHandleQuoteEvent(int, bool)));
}

enum SetResponse quotes::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void quotes::sHandleQuoteEvent(int pQuheadid, bool)
{
  if (pQuheadid == -1)
    sFillList();
}

void quotes::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrint()));
  menuItem->setEnabled(_privileges->check("PrintQuotes"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Convert to S/O..."), this, SLOT(sConvertSalesOrder()));
  menuItem->setEnabled(_privileges->check("ConvertQuotes"));

  menuItem = pMenu->addAction(tr("Convert to Invoice..."), this, SLOT(sConvertInvoice()));
  menuItem->setEnabled(_privileges->check("ConvertQuotesInvoice"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Copy"), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));

  menuItem = pMenu->addAction(tr("Copy to Cust./Prospect"), this, SLOT(sCopyToCustomer()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainQuotes"));
}

void quotes::sPrint()
{
  printQuote newdlg(this);

  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    ParameterList params;
    params.append("quhead_id", item->id());
    params.append("persistentPrint");

    newdlg.set(params);

    if (! newdlg.isSetup())
    {
      if (newdlg.exec() == QDialog::Rejected)
        break;
      newdlg.setSetup(true);
    }
  }
}

void quotes::sConvert(int pType)
{
  AppLock _lock;
  QString docType = "Sales Order";
  if (pType == 1)
    docType = "Invoice";

  if (QMessageBox::question(this, tr("Convert Selected Quote(s)"),
			    tr("<p>Are you sure that you want to convert "
                               "the selected Quote(s) to %1(s)?").arg(docType),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");
    if (pType == 1)
      convert.prepare("SELECT convertQuoteToInvoice(:quhead_id) AS sohead_id;");

    XSqlQuery prospectq;
    prospectq.prepare("SELECT convertProspectToCustomer(quhead_cust_id) AS result "
		      "FROM quhead "
		      "WHERE (quhead_id=:quhead_id);");

    bool tryagain = false;
    QList<int> converted;
    do {
      tryagain = false;
      int soheadid = -1;
      QList<XTreeWidgetItem*> selected = list()->selectedItems();
      QList<XTreeWidgetItem*> notConverted;

      foreach (XTreeWidgetItem *item, list()->selectedItems())
      {
        if (!_lock.acquire("quhead", item->id(), AppLock::Interactive))
        {
          QMessageBox::critical(this, tr("Cannot Convert"),
                                tr("<p>One or more of the selected Quotes is"
                                   " being edited.  You cannot convert a Quote"
                                   " that is being edited."));
          return;
        }
        if (! _lock.release())
        {
          ErrorReporter::error(QtCriticalMsg, this, tr("Locking Error"),
                               _lock.lastError(), __FILE__, __LINE__);
          return;
        }
        
        if (checkSitePrivs(item->id()))
        {
          int quheadid = item->id();
          XSqlQuery check;
          check.prepare("SELECT * FROM quhead WHERE (quhead_id = :quhead_id) AND (quhead_status ='C');");
          check.bindValue(":quhead_id", quheadid);
          check.exec();
          if (check.first())
          {
            QMessageBox::critical(this, tr("Cannot Convert"),
                                tr("<p>One or more of the selected Quotes have"
                                   " been converted.  You cannot convert an already"
                                   " converted Quote."));
            return;
          }
          else
          {
            convert.bindValue(":quhead_id", quheadid);
            convert.exec();
          }
          if (convert.first())
          {
            soheadid = convert.value("sohead_id").toInt();
            if (soheadid == -3)
            {
              if ((_metrics->value("DefaultSalesRep").toInt() > 0) &&
                  (_metrics->value("DefaultTerms").toInt() > 0) &&
                  (_metrics->value("DefaultCustType").toInt() > 0) && 
                  (_metrics->value("DefaultShipFormId").toInt() > 0)  && 
                  (_privileges->check("MaintainCustomerMasters"))) 
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
                          ErrorReporter::error(QtCriticalMsg, this, tr("Error Converting Prospect To Customer"),
                                               storedProcErrorLookup("convertProspectToCustomer", result),
                                               __FILE__, __LINE__);
                          notConverted.append(item);
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
                                  .arg(item->id() ? item->text(0) : ""));
                          notConverted.append(item);
                          continue;
                        }
                      }
                    }
                    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Converting Prospect To Customer"),
                                                  prospectq, __FILE__, __LINE__))
                    {
                        notConverted.append(item);
                        continue;
                    }
                  }
                  else
                  {
                    QMessageBox::information(this, tr("Quote for Prospect"),
                                tr("<p>The prospect must be manually "
                                   "converted to customer from either the "
                                   "Account or Customer windows before "
                                   "coverting this quote."));
                    notConverted.append(item);
                    continue;
                  }
              }
              else
              {
                QMessageBox::information(this, tr("Quote for Prospect"),
                            tr("<p>The prospect must be manually "
                               "converted to customer from either the "
                               "Account or Customer windows before "
                               "coverting this quote."));
                notConverted.append(item);
                continue;
              }
            }
            else if (soheadid < 0)
            {
              QMessageBox::warning(this, tr("Cannot Convert Quote"),
                      storedProcErrorLookup("convertQuote", soheadid)
                      .arg(item->id() ? item->text(0) : ""));
              notConverted.append(item);
              continue;
            }
            converted << quheadid;

            if (pType == 0)
            {
              omfgThis->sSalesOrdersUpdated(soheadid);

              salesOrder::editSalesOrder(soheadid, true);
            }
            else
              omfgThis->sQuotesUpdated(soheadid);
          }
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Convert Quote"),
                                        convert, __FILE__, __LINE__))
          {
            notConverted.append(item);
            continue;
          }
        }
      }

      if (notConverted.size() > 0)
      {
	failedPostList newdlg(this, "", true);
	newdlg.setLabel(tr("<p>The following Quotes could not be converted."));
        newdlg.sSetList(notConverted, list()->headerItem(), list()->header());
	tryagain = (newdlg.exec() == XDialog::Accepted);
	selected = notConverted;
	notConverted.clear();
      }
    } while (tryagain);
    for (int i = 0; i < converted.size(); i++)
    {
      omfgThis->sQuotesUpdated(converted[i]);
    }
  } // if user wants to convert
}

void quotes::sConvertSalesOrder()
{
  sConvert(0);
  sFillList();
}

void quotes::sConvertInvoice()
{
  sConvert(1);
  sFillList();
}

void quotes::sCopy()
{
  int lastid = -1;
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (checkSitePrivs(item->id()))
    {
      int qid = item->id();
      XSqlQuery qq;
      qq.prepare("SELECT copyQuote(:qid, null) AS result;");
      qq.bindValue(":qid", qid);
      if(qq.exec() && qq.first())
      {
        lastid = qq.value("result").toInt();
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Quote"),
                                    qq, __FILE__, __LINE__))
      {
        return;
      }
    }
  }
  if(lastid != -1)
    omfgThis->sQuotesUpdated(-1);
}

void quotes::sCopyToCustomer()
{
    int lastid = -1;
    foreach (XTreeWidgetItem *item, list()->selectedItems())
    {
      if (checkSitePrivs(item->id()))
      {
        int qid = item->id();
        ParameterList params;
        params.append("quhead_id", qid);

        copyQuote newdlg(this, "", true);
        newdlg.set(params);
        lastid = newdlg.exec();
      }
    }
    if(lastid != -1)
      omfgThis->sQuotesUpdated(-1);
}

void quotes::sNew()
{
  ParameterList params;
  params.append("mode", "newQuote");
  parameterWidget()->appendValue(params); // To pick up customer id, if any
      
  salesOrder *newdlg = new salesOrder(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void quotes::sEdit()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (checkSitePrivs(item->id()))
    {
      ParameterList params;
      params.append("mode", "editQuote");
      params.append("quhead_id", item->id());
    
      salesOrder *newdlg = new salesOrder(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sView()
{
  foreach (XTreeWidgetItem *item, list()->selectedItems())
  {
    if (checkSitePrivs(item->id()))
    {
      ParameterList params;
      params.append("mode", "viewQuote");
      params.append("quhead_id", item->id());
      
      salesOrder *newdlg = new salesOrder(this);
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sDelete()
{
  XSqlQuery quotesDelete;
  if (QMessageBox::question(this, tr("Delete Selected Quotes"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Quotes?" ),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default)
			    == QMessageBox::Yes)
  {
    quotesDelete.prepare("SELECT deleteQuote(:quhead_id) AS result;");

    int counter = 0;
    foreach (XTreeWidgetItem *item, list()->selectedItems())    
    {
      if (checkSitePrivs(item->id()))
      {
        quotesDelete.bindValue(":quhead_id", item->id());
        quotesDelete.exec();
        if (quotesDelete.first())
        {
          int result = quotesDelete.value("result").toInt();
          if (result < 0)
          {
              ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote"),
                                   storedProcErrorLookup("deleteQuote", result),
                                   __FILE__, __LINE__);
              continue;
          }
          counter++;
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Quote #%1")
                                      .arg(item->text(0)),
                                      quotesDelete, __FILE__, __LINE__))
        {
            continue;
        }
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);
  }
}

bool quotes::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkQuoteSitePrivs(:quheadid) AS result;");
    check.bindValue(":quheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view, edit, or convert this Quote as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

bool quotes::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if(_showExpired->isChecked())
    params.append("showExpired");
  if(_convertedtoSo->isChecked())
    params.append("showConverted");

  params.append("open", tr("Open"));
  params.append("converted", tr("Converted"));
  params.append("undefined", tr("Undefined"));

  return true;
}
