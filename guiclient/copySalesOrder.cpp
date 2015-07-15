/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copySalesOrder.h"

#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "salesOrderList.h"

copySalesOrder::copySalesOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _so->setAllowedTypes(OrderLineEdit::Sales);

  connect(_so, SIGNAL(newId(int,QString)), this, SLOT(sPopulateSoInfo(int)));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sCopy()));

  _captive = false;

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));

  _soitem->addColumn(tr("#"),        _seqColumn, Qt::AlignRight, true, "coitem_linenumber" );
  _soitem->addColumn(tr("Item"),    _itemColumn, Qt::AlignLeft,  true, "item_number");
  _soitem->addColumn(tr("Description"),      -1, Qt::AlignLeft,  true, "item_descrip");
  _soitem->addColumn(tr("Site"),     _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _soitem->addColumn(tr("Ordered"),  _qtyColumn, Qt::AlignRight, true, "coitem_qtyord");
  _soitem->addColumn(tr("Price"),    _qtyColumn, Qt::AlignRight, true, "coitem_price");
  _soitem->addColumn(tr("Extended"), _qtyColumn, Qt::AlignRight, true, "extended");
}

copySalesOrder::~copySalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void copySalesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copySalesOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _captive = true;
    _so->setId(param.toInt());
    _so->setEnabled(false);
  }

  return NoError;
}

void copySalesOrder::sPopulateSoInfo(int)
{
  XSqlQuery copyPopulateSoInfo;
  if (_so->id() != -1)
  {
    copyPopulateSoInfo.prepare( "SELECT cohead_number,"
              "        cohead_orderdate,"
              "        cohead_custponumber, cust_name, cntct_phone "
              " FROM cohead "
              " JOIN custinfo ON (cohead_cust_id=cust_id)"
              " LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
              " WHERE (cohead_id=:sohead_id);" );
    copyPopulateSoInfo.bindValue(":sohead_id", _so->id());
    copyPopulateSoInfo.exec();
    if (copyPopulateSoInfo.first())
    {
      _orderDate->setDate(copyPopulateSoInfo.value("cohead_orderdate").toDate());
      _poNumber->setText(copyPopulateSoInfo.value("cohead_custponumber").toString());
      _custName->setText(copyPopulateSoInfo.value("cust_name").toString());
      _custPhone->setText(copyPopulateSoInfo.value("cntct_phone").toString());
    }
    else if (copyPopulateSoInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, copyPopulateSoInfo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    copyPopulateSoInfo.prepare( "SELECT coitem.*, item_number,"
	       "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
	       "       warehous_code,"
               "       ((coitem_qtyord * coitem_qty_invuomratio) * (coitem_price / coitem_price_invuomratio)) AS extended,"
               "       'qty' AS coitem_qtyord_xtnumericrole,"
               "       'price' AS coitem_price_xtnumericrole,"
               "       'curr' AS extended_xtnumericrole "
               "FROM coitem, itemsite, item, whsinfo "
               "WHERE ( (coitem_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (coitem_status != 'X')"
               " AND (coitem_cohead_id=:sohead_id) "
               " AND (coitem_subnumber = 0) ) "
               "ORDER BY coitem_linenumber;" );
    copyPopulateSoInfo.bindValue(":sohead_id", _so->id());
    copyPopulateSoInfo.exec();
    _soitem->populate(copyPopulateSoInfo);
    if (copyPopulateSoInfo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, copyPopulateSoInfo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _soitem->clear();
  }
}


void copySalesOrder::sCopy()
{
  XSqlQuery copyCopy;
  copyCopy.prepare("SELECT copySo(:sohead_id, :customer, :scheddate) AS sohead_id;");
  copyCopy.bindValue(":sohead_id", _so->id());

  if (_useNewCust->isChecked())
    copyCopy.bindValue(":customer", _cust->id());

  if (_reschedule->isChecked())
    copyCopy.bindValue(":scheddate", _scheduleDate->date());

  copyCopy.exec();

  if (_captive)
  {
    if (copyCopy.first())
    {
      int soheadid = copyCopy.value("sohead_id").toInt();
      omfgThis->sSalesOrdersUpdated(soheadid);
      done(soheadid);
    }
    else if (copyCopy.lastError().type() != QSqlError::NoError)
    {
      systemError(this, copyCopy.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _so->setId(-1);
    _so->setFocus();
  }
}
