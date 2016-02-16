/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reserveSalesOrderItem.h"

#include <QApplication>
#include <QInputDialog>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include <metasql.h>
#include "mqlutil.h"


#include "characteristic.h"
#include "inputManager.h"
#include "xmessagebox.h"
#include "storedProcErrorLookup.h"

reserveSalesOrderItem::reserveSalesOrderItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _item->setReadOnly(true);

  _qtyToReserve->setValidator(omfgThis->qtyVal());

  _ordered->setPrecision(omfgThis->qtyVal());
  _shipped->setPrecision(omfgThis->qtyVal());
  _atShipping->setPrecision(omfgThis->qtyVal());
  _reserved->setPrecision(omfgThis->qtyVal());
  _onHand->setPrecision(omfgThis->qtyVal());
  _allocated->setPrecision(omfgThis->qtyVal());
  _unreserved->setPrecision(omfgThis->qtyVal());

  if (_metrics->boolean("SOManualReservations"))
  {
    connect(_bcReserve,       SIGNAL(clicked()), this, SLOT(sBcReserve()));
    connect(_reserve,         SIGNAL(clicked()), this, SLOT(sReserveLocation()));
    connect(_unreserve,       SIGNAL(clicked()), this, SLOT(sUnreserveLocation()));
    connect(_itemloc, SIGNAL(itemSelected(int)), this, SLOT(sReserveLocation()));
    connect(_bc,   SIGNAL(textChanged(QString)), this, SLOT(sBcChanged(QString)));
    
    omfgThis->inputManager()->notify(cBCLotSerialNumber, this, this, SLOT(sCatchLotSerialNumber(QString)));
    
    _itemloc->addColumn(tr("Location"),       _itemColumn, Qt::AlignLeft,  true, "location");
    _itemloc->addColumn(tr("Lot/Serial #"),   -1,          Qt::AlignLeft,  true, "lotserial");
    _itemloc->addColumn(tr("Expiration"),     _dateColumn, Qt::AlignCenter,true, "f_expiration");
    _itemloc->addColumn(tr("This Reserved"),  _qtyColumn,  Qt::AlignRight, true, "reserved");
    _itemloc->addColumn(tr("Total Reserved"), _qtyColumn,  Qt::AlignRight, true, "totalreserved");
    _itemloc->addColumn(tr("Unreserved"),     _qtyColumn,  Qt::AlignRight, true, "unreserved");

    //If not lot serial control, hide info
    if (!_metrics->boolean("LotSerialControl"))
    {
      _bcLit->hide();
      _bc->hide();
      _bcQtyLit->hide();
      _bcQty->hide();
      _bcReserve->hide();
    }
    else
    {
      // Add columns for lotserial characteristics
      QString column;
      QString name;
      QString sql = QString("SELECT char_id, char_name, char_type "
                            "FROM char "
                            "WHERE (char_lotserial) "
                            " AND (char_search) "
                            "ORDER BY char_name;");
      XSqlQuery chars;
      chars.exec(sql);
      while (chars.next())
      {
        characteristic::Type chartype = (characteristic::Type)chars.value("char_type").toInt();
        column = QString("char%1").arg(chars.value("char_id").toString());
        name = chars.value("char_name").toString();
        _itemloc->addColumn(name, -1, Qt::AlignLeft , true, column );
        if (chartype == characteristic::Text)
        {
          _charidstext.append(chars.value("char_id").toInt());
        }
        else if (chartype == characteristic::List)
        {
          _charidslist.append(chars.value("char_id").toInt());
        }
        else if (chartype == characteristic::Date)
        {
          _charidsdate.append(chars.value("char_id").toInt());
        }
      }
    }
  }
  else
  {
    _locationGroup->hide();
  }
  
  XSqlQuery reserveSalesOrderItem;
  reserveSalesOrderItem.exec("BEGIN;");

}

reserveSalesOrderItem::~reserveSalesOrderItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void reserveSalesOrderItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reserveSalesOrderItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _soitemid = param.toInt();
    //_orderNumberLit->setText(tr("Sales Order #:"));
    populate();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qtyToReserve->setDouble(param.toDouble());

  return NoError;
}

void reserveSalesOrderItem::sSave()
{
  XSqlQuery reserveq;

  bool _update = false;
  if (_qtyToReserve->toDouble() > 0.0)
    _update = true;
  
  if (_update && _metrics->boolean("SOManualReservations"))
  {
    if (QMessageBox::question(this, tr("Reserve Balance?"),
                              tr("<p>Reserve the balance using the Reservations "
                                 "by Location configuration setting?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      _update = false;
    }
  }

  if (_update)
  {
    reserveq.prepare("SELECT reserveSoLineQty(:lineitem_id, :qty) AS result;");
    reserveq.bindValue(":lineitem_id", _soitemid);
    reserveq.bindValue(":qty", _qtyToReserve->toDouble());
    reserveq.exec();
    
    if (reserveq.first())
    {
      int result = reserveq.value("result").toInt();
      if (result < 0)
      {
        systemError( this, storedProcErrorLookup("reserveSoLineQty", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (reserveq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, reserveq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reserveq.exec("COMMIT;");
  accept();
}

void reserveSalesOrderItem::reject()
{
  XSqlQuery itemreject;
  itemreject.exec("ROLLBACK;");
  XDialog::reject();
}

void reserveSalesOrderItem::populate()
{
  XSqlQuery distributepopulate;
  distributepopulate.prepare("SELECT itemsite_controlmethod "
                             "FROM coitem JOIN itemsite ON (itemsite_id=coitem_itemsite_id) "
                             "WHERE (coitem_id=:soitem_id);");
  
  distributepopulate.bindValue(":soitem_id", _soitemid);
  distributepopulate.exec();
  if (distributepopulate.first())
  {
    _controlMethod = distributepopulate.value("itemsite_controlmethod").toString();
    _bc->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    _bcQty->setEnabled(_controlMethod == "L");
    _bcReserve->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    if (_controlMethod == "S")
      _bcQty->setText("1");
    else
      _bcQty->clear();
  }
  else if (distributepopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, distributepopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList itemp;
  itemp.append("soitem_id", _soitemid);

  QString sql = "SELECT cohead_number AS order_number,"
                "       coitem_linenumber,"
                "       item_id, warehous_code, uom_name,"
                "       itemsite_id, qtyAvailable(itemsite_id) AS availableqoh,"
                "       qtyReserved(itemsite_id) AS totreserved,"
                "       qtyUnreserved(itemsite_id) AS totunreserved,"
                "       coitem_qtyord AS ordered,"
                "       (coitem_qtyshipped - coitem_qtyreturned) AS shipped,"
                "       qtyAtShipping(coitem_id) AS atShipping,"
                "       (coitem_qtyreserved / coitem_qty_invuomratio) AS reserved,"
                "       noNeg(coitem_qtyord - coitem_qtyshipped +"
                "             coitem_qtyreturned - qtyAtShipping(coitem_id) -"
                "             (coitem_qtyreserved / coitem_qty_invuomratio)) AS balance "
                "FROM coitem JOIN cohead ON (cohead_id=coitem_cohead_id)"
                "            JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                "            JOIN item ON (item_id=itemsite_item_id)"
                "            JOIN whsinfo ON (warehous_id=itemsite_warehous_id)"
                "            JOIN uom ON (uom_id=coitem_qty_uom_id) "
                "WHERE ((coitem_status <> 'X')"
                "  AND  (coitem_id=<? value('soitem_id') ?>) );";

  MetaSQLQuery itemm(sql);
  XSqlQuery itemq = itemm.toQuery(itemp);

  if (itemq.first())
  {
    _itemsiteid = itemq.value("itemsite_id").toInt();
    _salesOrderNumber->setText(itemq.value("order_number").toString());
    _salesOrderLine->setText(itemq.value("coitem_linenumber").toString());
    _item->setId(itemq.value("item_id").toInt());
    _warehouse->setText(itemq.value("warehous_code").toString());
    _qtyUOM->setText(itemq.value("uom_name").toString());
    _ordered->setDouble(itemq.value("ordered").toDouble());
    _shipped->setDouble(itemq.value("shipped").toDouble());
    _atShipping->setDouble(itemq.value("atShipping").toDouble());
    _reserved->setDouble(itemq.value("reserved").toDouble());
    _onHand->setDouble(itemq.value("availableqoh").toDouble());
    _allocated->setDouble(itemq.value("totreserved").toDouble());
    _unreserved->setDouble(itemq.value("totunreserved").toDouble());
  }
  else if (itemq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_qtyToReserve->text().length() == 0)
    _qtyToReserve->setDouble(itemq.value("balance").toDouble());
  sFillList();
}

void reserveSalesOrderItem::sFillList()
{
  ParameterList params;
  
  params.append("locationType",   cLocation);
  params.append("itemlocType",    cItemloc);
  params.append("yes",            tr("Yes"));
  params.append("no",             tr("No"));
  params.append("na",             tr("N/A"));
  params.append("undefined",      tr("Undefined"));
  params.append("soitem_id",      _soitemid);
  params.append("itemsite_id",    _itemsiteid);
  if(_metrics->value("SOReservationLocationMethod").toInt() == 1)
    params.append("orderByLowest", true);
  else if (_metrics->value("SOReservationLocationMethod").toInt() == 2)
    params.append("orderByHighest", true);
  else if(_metrics->value("SOReservationLocationMethod").toInt() == 3)
    params.append("orderByAlpha", true);
  
  // Put together the list of text and date based charids used to build joins
  params.append("char_id_text_list", _charidstext);
  params.append("char_id_list_list", _charidslist);
  params.append("char_id_date_list", _charidsdate);
  
  MetaSQLQuery mql = mqlLoad("reserveInventory", "locations");
  XSqlQuery reserveFillList = mql.toQuery(params);
    
  _itemloc->populate(reserveFillList, true);
  if (reserveFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reserveFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void reserveSalesOrderItem::sReserveLocation()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_itemloc->currentItem();
  double locreserve = QLocale().toDouble(item->text(5));
  if (locreserve > _qtyToReserve->toDouble())
    locreserve = _qtyToReserve->toDouble();
  bool ok;
  locreserve = QInputDialog::getDouble(this, tr("Qty. to Reserve"),
                                       tr("Qty:"),
                                       locreserve, 0, locreserve, 5, &ok);
  if ( !ok )
    return;
  
  XSqlQuery reserveq;
  reserveq.prepare("SELECT reserveSoLineQty(:lineitem_id, true, :qty, :itemloc_id) AS result;");
  reserveq.bindValue(":lineitem_id", _soitemid);
  reserveq.bindValue(":qty", locreserve);
  reserveq.bindValue(":itemloc_id", _itemloc->id());
  reserveq.exec();
  
  if (reserveq.first())
  {
    int result = reserveq.value("result").toInt();
    if (result < 0)
    {
      systemError( this, storedProcErrorLookup("reserveSoLineQty", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (reserveq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reserveq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  populate();
  _qtyToReserve->setDouble(_qtyToReserve->toDouble() - locreserve);
}

void reserveSalesOrderItem::sUnreserveLocation()
{
  XTreeWidgetItem *item = (XTreeWidgetItem*)_itemloc->currentItem();
  double locreserve = QLocale().toDouble(item->text(3));
  bool ok;
  locreserve = QInputDialog::getDouble(this, tr("Qty. to Unreserve"),
                                       tr("Qty:"),
                                       locreserve, 0, locreserve, 5, &ok);
  if ( !ok )
    return;
  
  XSqlQuery reserveq;
  reserveq.prepare("SELECT unreserveSoLineQty(:lineitem_id, :qty, :itemloc_id) AS result;");
  reserveq.bindValue(":lineitem_id", _soitemid);
  reserveq.bindValue(":qty", locreserve);
  reserveq.bindValue(":itemloc_id", _itemloc->id());
  reserveq.exec();
  
  if (reserveq.first())
  {
    int result = reserveq.value("result").toInt();
    if (result < 0)
    {
      systemError( this, storedProcErrorLookup("unreserveSoLineQty", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (reserveq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reserveq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  populate();
  _qtyToReserve->setDouble(_qtyToReserve->toDouble() + locreserve);
}

void reserveSalesOrderItem::sBcReserve()
{
  XSqlQuery reserveBc;
  if (_bc->text().isEmpty())
  {
    QMessageBox::warning(this, tr("No Bar Code scanned"),
                         tr("<p>Cannot search for Items by Bar Code without a "
                            "Bar Code."));
    _bc->setFocus();
    return;
  }
  
  reserveBc.prepare( "SELECT itemloc_id "
                     "FROM  coitem JOIN itemsite ON (itemsite_id=coitem_itemsite_id)"
                     "             JOIN itemloc ON (itemloc_itemsite_id=itemsite_id)"
                     "             JOIN ls ON (ls_id=itemloc_ls_id) "
                     "WHERE ((itemsite_controlmethod IN ('L', 'S'))"
                     "  AND  (ls_number=:lotserial)"
                     "  AND  (coitem_id=:soitem_id));");
  
  reserveBc.bindValue(":soitem_id", _soitemid);
  reserveBc.bindValue(":lotserial",      _bc->text());
  reserveBc.exec();
  
  if(!reserveBc.first())
  {
    if (reserveBc.lastError().type() != QSqlError::NoError)
    {
      systemError(this, reserveBc.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    QMessageBox::warning(this, tr("No Match Found"),
                         tr("<p>No available lines match the specified Barcode."));
    _bc->clear();
    return;
  }
  
  XSqlQuery reserveq;
  reserveq.prepare("SELECT reserveSoLineQty(:lineitem_id, true, :qty, :itemloc_id) AS result;");
  reserveq.bindValue(":lineitem_id", _soitemid);
  reserveq.bindValue(":qty", _bcQty->text().toDouble());
  reserveq.bindValue(":itemloc_id", reserveBc.value("itemloc_id").toInt());
  reserveq.exec();
  
  if (reserveq.first())
  {
    int result = reserveq.value("result").toInt();
    if (result < 0)
    {
      systemError( this, storedProcErrorLookup("reserveSoLineQty", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (reserveq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, reserveq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  double _savebcQty = _bcQty->text().toDouble();
  populate();
  _qtyToReserve->setDouble(_qtyToReserve->toDouble() - _savebcQty);
  
  _bc->clear();
  if (_controlMethod == "S")
    _bcQty->setText("1");
  else
    _bcQty->clear();
  sFillList();
  _bc->setFocus();
}

void reserveSalesOrderItem::sCatchLotSerialNumber(const QString plotserial)
{
  _bc->setText(plotserial);
  if (_controlMethod == "S")
    _bcReserve->setFocus();
  else
    _bcQty->setFocus();
}

void reserveSalesOrderItem::sBcChanged(const QString p)
{
  _save->setDefault(p.isEmpty());
  _bcReserve->setDefault(! p.isEmpty());
}
