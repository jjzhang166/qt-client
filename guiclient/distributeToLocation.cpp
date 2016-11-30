/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "distributeToLocation.h"

#include <math.h>

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <errorReporter.h>

distributeToLocation::distributeToLocation(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_distribute, SIGNAL(clicked()), this, SLOT(sDistribute()));
  _availToDistribute   =  0;
  _sourceItemlocdistid = -1;
  _itemlocdistid       = -1;

  _qtyToDistribute->setPrecision(omfgThis->qtyVal());
  _qtyTagged->setPrecision(omfgThis->qtyVal());
  _qtyBalance->setPrecision(omfgThis->qtyVal());
  _locationQty->setValidator(omfgThis->transQtyVal());
}

distributeToLocation::~distributeToLocation()
{
  // no need to delete child widgets, Qt does it all for us
}

void distributeToLocation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse distributeToLocation::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("source_itemlocdist_id", &valid);
  if (valid)
    _sourceItemlocdistid = param.toInt();

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _mode = cItemloc;
    _itemlocdistid = param.toInt();
  }

  param = pParams.value("location_id", &valid);
  if (valid)
  {
    _mode = cLocation;
    _locationid = param.toInt();
  }

  populate();

  param = pParams.value("qty", &valid);
  if (valid)
  {
    // convert the sign of qty to match the desired outcome then choose the lesser of
    // the qty available to distribute and the qty the caller requested
    double locQty = _locationQty->toDouble();
    if (_mode == cItemloc)	// lot/serial
    {
      if (locQty < 0 && param.toDouble() > 0)
	locQty = qMax(-param.toDouble(), locQty);
      else if (locQty < 0 && param.toDouble() < 0)
	locQty = qMax(param.toDouble(), locQty);
    }
    _locationQty->setDouble(locQty);
  }

  param = pParams.value("itemsite_controlmethod", &valid);
  if (valid)
  {
    _controlMethod = param.toString();
  }

  param = pParams.value("distribute", &valid);
  if (valid)
  {
    sDistribute();
    return NoError;
  }

  return NoError;
}

void distributeToLocation::sDistribute()
{
  XSqlQuery distributeDistribute;
  double qty = _locationQty->toDouble();

  if ((_balance < 0) && (qty < _balance))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You must not distribute a quantity to this Location that is greater than the total quantity to distribute.") );
    _locationQty->setFocus();
    return;
  }
  else if ((_balance > 0) && (qty > _balance))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You may not distribute a quantity to this Location that is greater than the total quantity to distribute.") );
    _locationQty->setFocus();
    return;
  }
  else if ((_balance > 0) && (qty < 0))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You may not distribute a negative value when the balance to distribute is positive.") );
    _locationQty->setFocus();
    return;
  }
  else if ((_balance < 0) && (qty > 0))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You may not distribute a positive value when the balance to distribute is negative.") );
    _locationQty->setFocus();
    return;
  }

  if (qAbs(qty)>1 && _controlMethod=="S")
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You may not distribute more than one of the same serial controlled item.") );
    _locationQty->setFocus();
    return;
  }
  

  if (qty < 0 && _availToDistribute < qAbs(qty) &&
      QMessageBox::question(this, tr("Distribute More Than Available?"),
			    tr("<p>It appears you are trying to distribute "
			       "more than is available to be distributed. "
			       "Are you sure you want to distribute this "
			       "quantity?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _locationQty->setFocus();
    return;
  }

  if (qty == 0.0)
  {
    if (_mode == cLocation)
    {
      distributeDistribute.prepare( "DELETE FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='L')"
                 " AND (itemlocdist_source_id=:location_id) );" );
      distributeDistribute.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      distributeDistribute.bindValue(":location_id", _locationid);
      distributeDistribute.exec();
    }
    else if (_mode == cItemloc)
    {
      distributeDistribute.prepare( "DELETE FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='I')"
                 " AND (itemlocdist_source_id=:itemloc_id));" );
      distributeDistribute.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      distributeDistribute.bindValue(":itemloc_id", _itemlocdistid);
      distributeDistribute.exec();
    }
  }
  else if (_mode == cLocation)
  {
    if (_metrics->boolean("LotSerialControl"))
      distributeDistribute.prepare( "SELECT itemlocdist_id "
                 "FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='L')"
                 " AND (itemlocdist_source_id=:location_id)"
                 " AND (formatlotserialnumber(itemlocdist_ls_id)=:lotSerial) );" );
    else
      distributeDistribute.prepare( "SELECT itemlocdist_id "
                 "FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='L')"
                 " AND (itemlocdist_source_id=:location_id)"
                 "  );" );
    distributeDistribute.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    distributeDistribute.bindValue(":location_id", _locationid);
    distributeDistribute.bindValue(":lotSerial", _lotSerial);
    distributeDistribute.exec();
    if (distributeDistribute.first())
    {
      int itemlocdistid = distributeDistribute.value("itemlocdist_id").toInt();

      distributeDistribute.prepare( "UPDATE itemlocdist "
                 "SET itemlocdist_qty=:qty "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      distributeDistribute.bindValue(":qty", qty);
      distributeDistribute.bindValue(":itemlocdist_id", itemlocdistid);
      distributeDistribute.exec();
    }
    else
    {
      distributeDistribute.prepare( "INSERT INTO itemlocdist "
                 "( itemlocdist_itemlocdist_id,"
                 "  itemlocdist_source_type, itemlocdist_source_id,"
                 "  itemlocdist_qty, itemlocdist_ls_id, itemlocdist_expiration ) "
                 "SELECT itemlocdist_id,"
                 "       'L', :location_id,"
                 "       :qty, itemlocdist_ls_id, endOfTime() "
                 "FROM itemlocdist "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      distributeDistribute.bindValue(":location_id", _locationid);
      distributeDistribute.bindValue(":qty", qty);
      distributeDistribute.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      distributeDistribute.exec();
    }
  }

  else if (_mode == cItemloc)
  {
    distributeDistribute.prepare( "SELECT itemlocdist_id "
               "FROM itemlocdist, itemloc "
               "WHERE ( (itemlocdist_itemlocdist_id=:sItemlocdist_id)"
               " AND (itemlocdist_source_type='I')"
               " AND (itemlocdist_source_id=:itemlocdist_id) );" );
    distributeDistribute.bindValue(":sItemlocdist_id", _sourceItemlocdistid);
    distributeDistribute.bindValue(":itemlocdist_id", _itemlocdistid);
    distributeDistribute.exec();
    if (distributeDistribute.first())
    {
      int itemlocdistid = distributeDistribute.value("itemlocdist_id").toInt();

      distributeDistribute.prepare( "UPDATE itemlocdist "
                 "SET itemlocdist_qty=:qty "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      distributeDistribute.bindValue(":qty", qty);
      distributeDistribute.bindValue(":itemlocdist_id", itemlocdistid);
      distributeDistribute.exec();
    }
    else
    {
      distributeDistribute.prepare( "INSERT INTO itemlocdist "
                 "( itemlocdist_itemlocdist_id,"
                 "  itemlocdist_source_type, itemlocdist_source_id,"
                 "  itemlocdist_qty, itemlocdist_expiration ) "
                 "VALUES "
                 "( :sItemlocdist_id,"
                 "  'I',  :itemlocdist_id,"
                 "  :qty, endOfTime() );" );
      distributeDistribute.bindValue(":sItemlocdist_id", _sourceItemlocdistid);
      distributeDistribute.bindValue(":itemlocdist_id", _itemlocdistid);
      distributeDistribute.bindValue(":qty", qty);
      distributeDistribute.exec();
    }
  }

  accept();
}

void distributeToLocation::populate()
{
  XSqlQuery distributepopulate;
  if (_mode == cLocation)
  {
    distributepopulate.prepare( "SELECT formatLocationName(location_id) AS locationname, COALESCE(subild.itemlocdist_qty, 0) AS qty, "
               "       qtyLocation(location_id, NULL, NULL, NULL, topild.itemlocdist_itemsite_id, topild.itemlocdist_order_type, topild.itemlocdist_order_id, topild.itemlocdist_id) AS availqty "
               "FROM location LEFT OUTER JOIN itemlocdist AS subild"
               "               ON ( (subild.itemlocdist_source_type='L')"
               "                   AND (subild.itemlocdist_source_id=location_id)"
               "                   AND (subild.itemlocdist_itemlocdist_id=:itemlocdist_id) ) "
               "     LEFT OUTER JOIN itemlocdist AS topild"
               "               ON (topild.itemlocdist_id=:itemlocdist_id) "
               "WHERE (location_id=:location_id);" );
    distributepopulate.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    distributepopulate.bindValue(":location_id", _locationid);
  }
  else if (_mode == cItemloc)
  {
    distributepopulate.prepare( "SELECT formatLocationName(location_id) AS locationname,"
	       "       COALESCE(itemlocdist_qty, 0) AS qty, "
	       "       itemloc_qty AS availqty "
               "FROM itemloc LEFT OUTER JOIN"
	       "     location ON (itemloc_location_id=location_id) LEFT OUTER JOIN"
	       "     itemlocdist ON ((itemlocdist_source_type='I')"
               "                 AND (itemlocdist_source_id=itemloc_id)"
               "                 AND (itemlocdist_itemlocdist_id=:itemlocdist_id) ) "
               "WHERE (itemloc_id=:itemloc_id);" );
    distributepopulate.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    distributepopulate.bindValue(":itemloc_id", _itemlocdistid);
  }

  distributepopulate.exec();
  if (distributepopulate.first())
  {
    _locationQty->setDouble(distributepopulate.value("qty").toDouble());
    _location->setText(distributepopulate.value("locationname").toString());
    _availToDistribute = distributepopulate.value("availqty").toDouble();
  }

  distributepopulate.prepare( "SELECT parent.lotserial AS lotserial,"
             "       qtydistrib,"
             "       qtytagged,"
             "       (qtydistrib - qtytagged) AS qtybalance "
             "FROM ( SELECT formatlotserialnumber(itemlocdist_ls_id) AS lotserial, itemlocdist_qty AS qtydistrib"
             "       FROM itemlocdist"
             "       WHERE (itemlocdist_id=:itemlocdist_id) ) AS parent,"
             "     ( SELECT COALESCE(SUM(itemlocdist_qty), 0) AS qtytagged"
             "       FROM itemlocdist"
             "       WHERE (itemlocdist_itemlocdist_id=:itemlocdist_id) ) AS child;" );
  distributepopulate.bindValue(":itemlocdist_id", _sourceItemlocdistid);
  distributepopulate.exec();
  if (distributepopulate.first())
  {
    _lotSerial = distributepopulate.value("lotserial").toString();

    _qtyToDistribute->setDouble(distributepopulate.value("qtydistrib").toDouble());
    _qtyTagged->setDouble(distributepopulate.value("qtytagged").toDouble());
    _qtyBalance->setDouble(distributepopulate.value("qtybalance").toDouble());
//    _balance = (double)qRound(distributepopulate.value("qtybalance").toDouble() * pow(10.0, decimalPlaces("qty"))) / pow(10.0, decimalPlaces("qty"));
    _balance = _qtyBalance->toDouble();

    double locQty = _balance;
    if (_mode == cItemloc || _mode == cLocation)	// lot/serial or Location
    {
      // if we want to take stuff away and we have stuff to take away
      if (locQty < 0 && _availToDistribute > 0)
	locQty = qMax(-_availToDistribute, locQty);
      // if we want to take stuff away but don't have anything to take away
      else if (locQty < 0)
	locQty = 0;
    }
    _locationQty->setDouble(locQty);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Location Information"),
                                distributepopulate, __FILE__, __LINE__))
  {
    return;
  }
}
