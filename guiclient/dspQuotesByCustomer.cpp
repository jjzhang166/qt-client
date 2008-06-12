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

#include "dspQuotesByCustomer.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include "salesOrder.h"

/*
 *  Constructs a dspQuotesByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspQuotesByCustomer::dspQuotesByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));
    connect(_selectedPO, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_selectedPO, SIGNAL(toggled(bool)), _poNumber, SLOT(setEnabled(bool)));
    connect(_poNumber, SIGNAL(activated(int)), this, SLOT(sFillList()));
    connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspQuotesByCustomer::~dspQuotesByCustomer()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspQuotesByCustomer::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspQuotesByCustomer::init()
{
  statusBar()->hide();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date"));

  _so->addColumn(tr("Quote #"),     _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Quote Date"),     _dateColumn,  Qt::AlignRight  );
  _so->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft   );
  _so->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft   );
  
  _cust->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspQuotesByCustomer::sPopulatePo()
{
  _poNumber->clear();

  if ((_cust->isValid()) && (_dates->allValid()))
  {
    q.prepare( "SELECT DISTINCT -2, quhead_custponumber "
               "FROM quhead "
               "WHERE ( (quhead_cust_id=:cust_id)"
               " AND (quhead_quotedate BETWEEN :startDate AND :endDate) ) "
               "ORDER BY quhead_custponumber;" );
    _dates->bindValue(q);
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    _poNumber->populate(q);
  }

  sFillList();
}

void dspQuotesByCustomer::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
  
  if (_privileges->check("ConvertQuotes"))
  {
    menuThis->insertSeparator();
    menuThis->insertItem(tr("Convert..."), this, SLOT(sConvert()), 0);
  }

}

void dspQuotesByCustomer::sEditOrder()
{
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _so->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sViewOrder()
{
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _so->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sFillList()
{
  if ( ( (_allPOs->isChecked()) ||
         ( (_selectedPO->isChecked()) && (_poNumber->currentItem() != -1) ) ) &&
       (_dates->allValid())  )
  {
    QString sql( "SELECT quhead_id, quhead_number,"
                 "       formatDate(quhead_quotedate),"
                 "       quhead_shiptoname, quhead_custponumber "
                 "FROM quhead, quitem as soitem "
                 "WHERE ( (quhead_cust_id=:cust_id)"
                 " AND (soitem.quitem_quhead_id=quhead_id)"
                 " AND (quhead_quotedate BETWEEN :startDate AND :endDate)" );

    if (_selectedPO->isChecked())
      sql += " AND (quhead_custponumber=:poNumber)";

    sql += ") "
           "GROUP BY quhead_id, quhead_number, quhead_quotedate,"
           "         quhead_shiptoname, quhead_custponumber "
           "ORDER BY quhead_number;";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":cust_id", _cust->id());
    q.bindValue(":poNumber", _poNumber->currentText());
    q.exec();
    _so->populate(q);
  }
  else
    _so->clear();
}

void dspQuotesByCustomer::sConvert()
{
  if ( QMessageBox::information( this, tr("Convert Selected Quote(s)"),
                                 tr("Are you sure that you want to convert the selected Quote(s) to Sales Order(s)?" ),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    XSqlQuery check;
    check.prepare( "SELECT quhead_number, cust_creditstatus "
                   "FROM quhead, cust "
                   "WHERE ( (quhead_cust_id=cust_id)"
                   " AND (quhead_id=:quhead_id) );" );

    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    int counter = 0;
    int soheadid = -1;

    QList<QTreeWidgetItem*> selected = _so->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
      check.bindValue(":quhead_id", cursor->id());
      check.exec();
      if (check.first())
      {
	if ( (check.value("cust_creditstatus").toString() == "H") && (!_privileges->check("CreateSOForHoldCustomer")) )
	{
	  QMessageBox::warning( this, tr("Cannot Convert Quote"),
				tr( "Quote #%1 is for a Customer that has been placed on a Credit Hold and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Hold.  The selected\n"
				    "Customer must be taken off of Credit Hold before you may create convert this Quote." )
				.arg(check.value("quhead_number").toString()) );
	  return;
	}	

	if ( (check.value("cust_creditstatus").toString() == "W") && (!_privileges->check("CreateSOForWarnCustomer")) )
	{
	  QMessageBox::warning( this, tr("Cannot Convert Quote"),
				tr( "Quote #%1 is for a Customer that has been placed on a Credit Warning and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Warning.  The selected\n"
				    "Customer must be taken off of Credit Warning before you may create convert this Quote." )
				.arg(check.value("quhead_number").toString()) );
	  return;
	}	
      }
      else
      {
	systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	continue;
      }

      convert.bindValue(":quhead_id", cursor->id());
      convert.exec();
      if (convert.first())
      { 
	soheadid = convert.value("sohead_id").toInt();
	if(soheadid < 0)
	{
	  QMessageBox::warning( this, tr("Cannot Convert Quote"),
				tr( "Quote #%1 has one or more line items without a warehouse specified.\n"
				    "These line items must be fixed before you may convert this quote." )
				.arg(check.value("quhead_number").toString()) );
	  return;
	}
	counter++;
	omfgThis->sSalesOrdersUpdated(soheadid);
      }
      else
      {
	systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	return;
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);

    if (counter == 1)
      salesOrder::editSalesOrder(soheadid, true);
  }
}
