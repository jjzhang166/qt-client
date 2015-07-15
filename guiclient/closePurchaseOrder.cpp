/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "closePurchaseOrder.h"

#include <QMessageBox>
#include <QVariant>

closePurchaseOrder::closePurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_closePo, SIGNAL(clicked()), this, SLOT(sClosePo()));

  _captive = false;

  _po->addColumn(tr("Number"),      _orderColumn,   Qt::AlignRight,  true,  "pohead_number"  );
  _po->addColumn(tr("Vendor"),      -1,             Qt::AlignLeft,   true,  "vend_name"   );
  _po->addColumn(tr("Agent"),       _itemColumn,    Qt::AlignCenter, true,  "pohead_agent_username" );
  _po->addColumn(tr("Order Date"),  _dateColumn,    Qt::AlignLeft,   true,  "pohead_orderdate"  );
  _po->addColumn(tr("First Item"),  _itemColumn,    Qt::AlignLeft,   true,  "item_number"  );

  sFillList();
}

closePurchaseOrder::~closePurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void closePurchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse closePurchaseOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
  }

  return NoError;
}

void closePurchaseOrder::sClosePo()
{
  XSqlQuery closeClosePo;
  closeClosePo.prepare("SELECT closePo(:pohead_id) AS result;");

  QList<XTreeWidgetItem*>selected = _po->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      closeClosePo.bindValue(":pohead_id", ((XTreeWidgetItem*)selected[i])->id());
      closeClosePo.exec();
	}
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));
    sFillList();
  }
}

void closePurchaseOrder::sFillList()
{
  QString sql( "SELECT pohead_id, pohead_number,"
               "       vend_name, pohead_agent_username,"
               "       pohead_orderdate,"
               "       item_number "
               "FROM vendinfo, pohead LEFT OUTER JOIN"
               "     poitem ON (poitem_pohead_id=pohead_id AND poitem_linenumber=1)"
               "     LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
               "     LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
               "  AND   (pohead_status='O') "
               " ) "
               "ORDER BY pohead_number DESC;" );
  _po->populate(sql);
}

bool closePurchaseOrder::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkPOSitePrivs(:poheadid) AS result;");
    check.bindValue(":poheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not close this Purchase Order as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

