/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uninvoicedShipments.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include <parameter.h>
#include <openreports.h>
#include "mqlutil.h"
#include "selectOrderForBilling.h"
#include "errorReporter.h"

uninvoicedShipments::uninvoicedShipments(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showUnselected, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_shipitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _shipitem->setRootIsDecorated(true);
  _shipitem->addColumn(tr("Order/Line #"),           _itemColumn, Qt::AlignRight,  true,  "orderline" );
  _shipitem->addColumn(tr("Cust./Item Number"),      _itemColumn, Qt::AlignLeft,   true,  "custitem"  );
  _shipitem->addColumn(tr("Cust. Name/Description"), -1,          Qt::AlignLeft,   true,  "custdescrip"  );
  _shipitem->addColumn(tr("UOM"),                    _uomColumn,  Qt::AlignLeft,   true,  "uom_name"  );
  _shipitem->addColumn(tr("Shipped"),                _qtyColumn,  Qt::AlignRight,  true,  "shipped" );
  _shipitem->addColumn(tr("Approved"),               _qtyColumn,  Qt::AlignRight,  true,  "selected" );
  
  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

uninvoicedShipments::~uninvoicedShipments()
{
  // no need to delete child widgets, Qt does it all for us
}

void uninvoicedShipments::languageChange()
{
  retranslateUi(this);
}

void uninvoicedShipments::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  if (_showUnselected->isChecked())
    params.append("showUnselected");
	
  orReport report("UninvoicedShipments", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void uninvoicedShipments::sPopulateMenu(QMenu *menu)
{
  QAction *menuItem;

  menuItem = menu->addAction(tr("Approve This Order for Billing..."), this, SLOT(sSelectForBilling()));
  menuItem->setEnabled(_privileges->check("SelectBilling"));
}

void uninvoicedShipments::sSelectForBilling()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _shipitem->id());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void uninvoicedShipments::sFillList()
{
  MetaSQLQuery mql = mqlLoad("uninvoicedShipments", "detail");

  ParameterList params;
  if (_warehouse->isSelected())
    _warehouse->appendValue(params);
  if (_showUnselected->isChecked())
    params.append("showUnselected", true);

  XSqlQuery qry = mql.toQuery(params);
  _shipitem->populate(qry, true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Shipment Information"),
                                qry, __FILE__, __LINE__))
  {
    return;
  }
}
