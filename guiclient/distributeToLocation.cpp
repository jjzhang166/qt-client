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

#include "distributeToLocation.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

distributeToLocation::distributeToLocation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_distribute, SIGNAL(clicked()), this, SLOT(sDistribute()));
  _availToDistribute   =  0;
  _sourceItemlocdistid = -1;
  _itemlocdistid       = -1;
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
    double locQty = _locationQty->text().toDouble();
    if (_mode == cItemloc)	// lot/serial
    {
      if (locQty < 0 && param.toDouble() > 0)
	locQty = qMax(-param.toDouble(), locQty);
      else if (locQty < 0 && param.toDouble() < 0)
	locQty = qMax(param.toDouble(), locQty);
    }
    _locationQty->setText(formatNumber(locQty, 6));
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
  double qty = _locationQty->toDouble();

  if ((_balance < 0) && (qty < _balance))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You must not distribute a quantity to this Location that is greater than to total quantity to distribute.") );
    _locationQty->setFocus();
    return;
  }
  else if ((_balance > 0) && (qty > _balance))
  {
    QMessageBox::warning( this, tr("Cannot Distribute Quantity"),
                          tr("You may not distribute a quantity to this Location that is greater than to total quantity to distribute.") );
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
      q.prepare( "DELETE FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='L')"
                 " AND (itemlocdist_source_id=:location_id) );" );
      q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      q.bindValue(":location_id", _locationid);
      q.exec();
    }
    else if (_mode == cItemloc)
    {
      q.prepare( "DELETE FROM itemlocdist "
                 "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
                 " AND (itemlocdist_source_type='I')"
                 " AND (itemlocdist_source_id=:itemloc_id));" );
      q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      q.bindValue(":itemloc_id", _itemlocdistid);
      q.exec();
    }
  }
  else if (_mode == cLocation)
  {
    q.prepare( "SELECT itemlocdist_id "
               "FROM itemlocdist, ls "
               "WHERE ( (itemlocdist_itemlocdist_id=:itemlocdist_id)"
               " AND (itemlocdist_source_type='L')"
               " AND (itemlocdist_source_id=:location_id)"
               " AND (formatlotserialnumber(itemlocdist_ls_id)=:lotSerial) );" );
    q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    q.bindValue(":location_id", _locationid);
    q.bindValue(":lotSerial", _lotSerial);
    q.exec();
    if (q.first())
    {
      int itemlocdistid = q.value("itemlocdist_id").toInt();

      q.prepare( "UPDATE itemlocdist "
                 "SET itemlocdist_qty=:qty "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      q.bindValue(":qty", qty);
      q.bindValue(":itemlocdist_id", itemlocdistid);
      q.exec();
    }
    else
    {
      q.prepare( "INSERT INTO itemlocdist "
                 "( itemlocdist_itemlocdist_id,"
                 "  itemlocdist_source_type, itemlocdist_source_id,"
                 "  itemlocdist_qty, itemlocdist_ls_id, itemlocdist_expiration ) "
                 "SELECT itemlocdist_id,"
                 "       'L', :location_id,"
                 "       :qty, itemlocdist_ls_id, endOfTime() "
                 "FROM itemlocdist "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      q.bindValue(":location_id", _locationid);
      q.bindValue(":qty", qty);
      q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
      q.exec();
    }
  }

  else if (_mode == cItemloc)
  {
    q.prepare( "SELECT itemlocdist_id "
               "FROM itemlocdist, itemloc "
               "WHERE ( (itemlocdist_itemlocdist_id=:sItemlocdist_id)"
               " AND (itemlocdist_source_type='I')"
               " AND (itemlocdist_source_id=:itemlocdist_id) );" );
    q.bindValue(":sItemlocdist_id", _sourceItemlocdistid);
    q.bindValue(":itemlocdist_id", _itemlocdistid);
    q.exec();
    if (q.first())
    {
      int itemlocdistid = q.value("itemlocdist_id").toInt();

      q.prepare( "UPDATE itemlocdist "
                 "SET itemlocdist_qty=:qty "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      q.bindValue(":qty", qty);
      q.bindValue(":itemlocdist_id", itemlocdistid);
      q.exec();
    }
    else
    {
      q.prepare( "INSERT INTO itemlocdist "
                 "( itemlocdist_itemlocdist_id,"
                 "  itemlocdist_source_type, itemlocdist_source_id,"
                 "  itemlocdist_qty, itemlocdist_expiration ) "
                 "VALUES "
                 "( :sItemlocdist_id,"
                 "  'I',  :itemlocdist_id,"
                 "  :qty, endOfTime() );" );
      q.bindValue(":sItemlocdist_id", _sourceItemlocdistid);
      q.bindValue(":itemlocdist_id", _itemlocdistid);
      q.bindValue(":qty", qty);
      q.exec();
    }
  }

  accept();
}

void distributeToLocation::populate()
{
  if (_mode == cLocation)
  {
    q.prepare( "SELECT formatLocationName(location_id) AS locationname, COALESCE(itemlocdist_qty, 0) AS qty, "
	       "       0 AS availqty "
               "FROM location LEFT OUTER JOIN itemlocdist"
               "               ON ( (itemlocdist_source_type='L')"
               "                   AND (itemlocdist_source_id=location_id)"
               "                   AND (itemlocdist_itemlocdist_id=:itemlocdist_id) ) "
               "WHERE (location_id=:location_id);" );
    q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    q.bindValue(":location_id", _locationid);
  }
  else if (_mode == cItemloc)
  {
    q.prepare( "SELECT formatLocationName(location_id) AS locationname,"
	       "       COALESCE(itemlocdist_qty, 0) AS qty, "
	       "       itemloc_qty AS availqty "
               "FROM itemloc LEFT OUTER JOIN"
	       "     location ON (itemloc_location_id=location_id) LEFT OUTER JOIN"
	       "     itemlocdist ON ((itemlocdist_source_type='I')"
               "                 AND (itemlocdist_source_id=itemloc_id)"
               "                 AND (itemlocdist_itemlocdist_id=:itemlocdist_id) ) "
               "WHERE (itemloc_id=:itemloc_id);" );
    q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
    q.bindValue(":itemloc_id", _itemlocdistid);
  }

  q.exec();
  if (q.first())
  {
    _locationQty->setText(q.value("qty").toString());
    _location->setText(q.value("locationname").toString());
    _availToDistribute = q.value("availqty").toDouble();
  }

  q.prepare( "SELECT parent.lotserial AS lotserial,"
             "       qtydistrib,"
             "       qtytagged,"
             "       (qtydistrib - qtytagged) AS qtybalance "
             "FROM ( SELECT formatlotserialnumber(itemlocdist_ls_id) AS lotserial, itemlocdist_qty AS qtydistrib"
             "       FROM itemlocdist"
             "       WHERE (itemlocdist_id=:itemlocdist_id) ) AS parent,"
             "     ( SELECT COALESCE(SUM(itemlocdist_qty), 0) AS qtytagged"
             "       FROM itemlocdist"
             "       WHERE (itemlocdist_itemlocdist_id=:itemlocdist_id) ) AS child;" );
  q.bindValue(":itemlocdist_id", _sourceItemlocdistid);
  q.exec();
  if (q.first())
  {
    _lotSerial = q.value("lotserial").toString();

    _qtyToDistribute->setText(formatNumber(q.value("qtydistrib").toDouble(), 6));
    _qtyTagged->setText(formatNumber(q.value("qtytagged").toDouble(),6));
    _qtyBalance->setText(formatNumber(q.value("qtybalance").toDouble(),6));
    _balance = q.value("qtybalance").toDouble();

    double locQty = _balance;
    if (_mode == cItemloc)	// lot/serial
    {
      // if we want to take stuff away and we have stuff to take away
      if (locQty < 0 && _availToDistribute > 0)
	locQty = qMax(-_availToDistribute, locQty);
      // if we want to take stuff away but don't have anything to take away
      else if (locQty < 0)
	locQty = 0;
    }
    _locationQty->setText(formatNumber(locQty, 6));
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
