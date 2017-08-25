/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspReturnAuthorizations.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "parameterwidget.h"
#include "returnAuthorization.h"
#include "xtreewidget.h"

dspReturnAuthorizations::dspReturnAuthorizations(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspReturnAuthorizations", fl)
{
  setWindowTitle(tr("Return Authorization Lookup"));
  setListLabel(tr("Return Authorizations"));
  setMetaSQLOptions("returnAuthorizations", "lookupDetail");
  setReportName("ReturnAuthorizations");
  setParameterWidgetVisible(true);
  setSearchVisible(true);

  list()->addColumn(tr("Return #"),        _orderColumn, Qt::AlignLeft,   true,  "rahead_number"   );
  list()->addColumn(tr("Created"),         _dateColumn,  Qt::AlignCenter, true,  "rahead_authdate" );
  list()->addColumn(tr("Original Order#"), _orderColumn, Qt::AlignLeft,   true,  "cohead_number"   );
  list()->addColumn(tr("Customer"),        -1,           Qt::AlignLeft,   true,  "rahead_billtoname"   );
  list()->addColumn(tr("Status"),          _orderColumn, Qt::AlignLeft,   true,  "status"   );
  list()->addColumn(tr("Disposition"),     _orderColumn, Qt::AlignLeft,   true,  "disposition"   );
  list()->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,   true,  "item_number" );
  list()->addColumn(tr("Description"),     -1,           Qt::AlignLeft,   true,  "item_descrip1" );
  list()->addColumn(tr("Order UOM"),       _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Authorized"),      _qtyColumn,   Qt::AlignRight,  true,  "raitem_qtyauthorized"  );
  list()->addColumn(tr("Received"),        _qtyColumn,   Qt::AlignRight,  true,  "raitem_qtyreceived"  );
  list()->addColumn(tr("Credited"),        _qtyColumn,   Qt::AlignRight,  true,  "raitem_qtycredited"  );
  list()->addColumn(tr("Auth. Value"),     _priceColumn, Qt::AlignRight,  true,  "extauthorized"  );
  list()->addColumn(tr("Credited Value"),  _priceColumn, Qt::AlignRight,  true,  "raitem_amtcredited"  );

  parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);
  parameterWidget()->append(tr("Customer"), "cust_id", ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->append(tr("Date From"), "dateFrom", ParameterWidget::Date);
  parameterWidget()->append(tr("Date To"), "dateTo", ParameterWidget::Date);

  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated(int, bool)), this, SLOT(sFillList()));
}

enum SetResponse dspReturnAuthorizations::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;

  return NoError;
}

void dspReturnAuthorizations::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(_privileges->check("MaintainReturns"))
    menuThis->addAction(tr("Edit..."), this, SLOT(sEditRA()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewRA()));
}

void dspReturnAuthorizations::sEditRA()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", list()->id());
  
  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspReturnAuthorizations::sViewRA()
{
  if (!checkSitePrivs(list()->id()))
    return;
    
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", list()->id());
  
  returnAuthorization *newdlg = new returnAuthorization(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

bool dspReturnAuthorizations::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (_preferences->boolean("selectedSites"))
    params.append("selectedSites");
  params.append("undefined", tr("Undefined"));
  params.append("credit", tr("Credit"));
  params.append("return", tr("Return"));
  params.append("replace", tr("Replace"));
  params.append("service", tr("Service"));
  params.append("substitute", tr("Substitute"));
  params.append("closed", tr("Closed"));
  params.append("expired", tr("Expired"));
  params.append("unauthorized", tr("Unauthorized"));
  params.append("open", tr("Open"));

  return true;
}

bool dspReturnAuthorizations::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkRASitePrivs(:raheadid) AS result;");
    check.bindValue(":raheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Return Authorization "
                                 "as it references a Site for which you have "
                                 "not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
