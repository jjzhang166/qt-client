/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspReturnAuthorizationsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "returnAuthorization.h"

dspReturnAuthorizationsByItem::dspReturnAuthorizationsByItem(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspReturnAuthorizationsByItem", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Return Authorization Lookup by Item"));
  setListLabel(tr("Return Authorizations"));
  setMetaSQLOptions("returnAuthorizationItems", "detail");

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Starting Order Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Ending Order Date:"));

  _item->setType(ItemLineEdit::cSold);

  list()->addColumn(tr("Return #"),        _orderColumn, Qt::AlignLeft,   true,  "rahead_number"   );
  list()->addColumn(tr("Created"),         _dateColumn,  Qt::AlignCenter, true,  "rahead_authdate" );
  list()->addColumn(tr("Customer"),        -1,           Qt::AlignLeft,   true,  "rahead_billtoname"   );
  list()->addColumn(tr("Status"),          _orderColumn, Qt::AlignLeft,   true,  "raitem_status"   );
  list()->addColumn(tr("Order UOM"),       _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Authorized"),      _qtyColumn,   Qt::AlignRight,  true,  "raitem_qtyauthorized"  );
  list()->addColumn(tr("Received"),        _qtyColumn,   Qt::AlignRight,  true,  "raitem_qtyreceived"  );

  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated(int, bool)), this, SLOT(sFillList()));
}

void dspReturnAuthorizationsByItem::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

enum SetResponse dspReturnAuthorizationsByItem::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  return NoError;
}

void dspReturnAuthorizationsByItem::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  if(_privileges->check("MaintainReturns"))
    menuThis->addAction(tr("Edit..."), this, SLOT(sEditRA()));
  menuThis->addAction(tr("View..."), this, SLOT(sViewRA()));
}

void dspReturnAuthorizationsByItem::sEditRA()
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

void dspReturnAuthorizationsByItem::sViewRA()
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

bool dspReturnAuthorizationsByItem::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning(this, tr("Item Required"),
      tr("You must specify an Item Number."));
    return false;
  }

  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Dates Required"),
      tr("You must specify a valid Date range."));
    return false;
  }

  _dates->appendValue(params);
  params.append("item_id", _item->id());
  if (_preferences->boolean("selectedSites"))
    params.append("selectedSites");
  params.append("undefined", tr("Undefined"));
  params.append("credit", tr("Credit"));
  params.append("return", tr("Return"));
  params.append("replace", tr("Replace"));
  params.append("service", tr("Service"));
  params.append("substitute", tr("Substitute"));

  return true;
}

bool dspReturnAuthorizationsByItem::checkSitePrivs(int orderid)
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
