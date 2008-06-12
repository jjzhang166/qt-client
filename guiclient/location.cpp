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

#include "location.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "itemList.h"

location::location(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_location, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sHandleWarehouse(int)));

  _locitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft );
  _locitem->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    sHandleWarehouse(_warehouse->id());
  }
  else
  {
    _warehouse->setAllowNull(TRUE);
    _warehouse->setNull();
  }

}

location::~location()
{
  // no need to delete child widgets, Qt does it all for us
}

void location::languageChange()
{
  retranslateUi(this);
}

enum SetResponse location::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("location_id", &valid);
  if (valid)
  {
    _locationid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('location_location_id_seq') AS location_id;");
      if (q.first())
        _locationid = q.value("location_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }

      _warehouse->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _warehouse->setEnabled(FALSE);

      _location->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(FALSE);
      _whsezone->setEnabled(FALSE);
      _aisle->setEnabled(FALSE);
      _rack->setEnabled(FALSE);
      _bin->setEnabled(FALSE);
      _location->setEnabled(FALSE);
      _netable->setEnabled(FALSE);
      _restricted->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _locitem->setEnabled(FALSE);
      _save->hide();
      _new->hide();
      _delete->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(FALSE);
  }

  return NoError;
}

void location::sCheck()
{
  _location->setText(_location->text().stripWhiteSpace());
  if ( (_mode == cNew) &&
       (_warehouse->isEnabled()) &&
       (_location->text().length()) )
  {
    q.prepare( "SELECT location_id "
               "FROM location "
               "WHERE ( (location_warehous_id=:location_warehous_id)"
               " AND (UPPER(location_name)=UPPER(:location_name)) );" );
    q.bindValue(":warehous_id", _warehouse->id());
    q.bindValue(":location_name", _location->text());
    q.exec();
    if (q.first())
    {
      _locationid = q.value("location_id").toInt();
      _mode = cEdit;
      _warehouse->setEnabled(FALSE);
      _location->setEnabled(FALSE);

      populate();
    }
  }
}

void location::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Select a Warehouse"),
                           tr( "<p>You must select a Warehouse for this "
			      "Location before creating it." ) );
    _warehouse->setFocus();
    return;
  }

  if ( (_location->text().stripWhiteSpace().length() == 0) &&
       (_aisle->text().stripWhiteSpace().length() == 0) &&
       (_rack->text().stripWhiteSpace().length() == 0) &&
       (_bin->text().stripWhiteSpace().length() == 0) )
  {
    QMessageBox::critical( this, tr("Enter Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location.") );
    _location->setFocus();
    return;
  }

  q.prepare("SELECT location_id"
            "  FROM location"
            " WHERE((location_id != :location_id)"
            "   AND (location_warehous_id=:location_warehous_id)"
            "   AND (location_aisle=:location_aisle)"
            "   AND (location_rack=:location_rack)"
            "   AND (location_bin=:location_bin)"
            "   AND (location_name=:location_name))");
  q.bindValue(":location_id", _locationid);
  q.bindValue(":location_warehous_id", _warehouse->id());
  q.bindValue(":location_aisle", _aisle->text());
  q.bindValue(":location_rack", _rack->text());
  q.bindValue(":location_bin", _bin->text());
  q.bindValue(":location_name", _location->text());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Duplicate Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location in the specified Warehouse.") );
    _location->setFocus();
    return;
  }
  

  if (_mode == cNew)
    q.prepare( "INSERT INTO location "
               "( location_id, location_warehous_id,"
               "  location_aisle, location_rack, location_bin,  location_name,"
               "  location_whsezone_id, location_descrip,"
               "  location_netable, location_restrict ) "
               "VALUES "
               "( :location_id, :location_warehous_id,"
               "  :location_aisle, :location_rack, :location_bin, :location_name,"
               "  :location_whsezone_id, :location_descrip,"
               "  :location_netable, :location_restrict );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE location "
               "SET location_warehous_id=:location_warehous_id,"
               "    location_aisle=:location_aisle, location_rack=:location_rack,"
               "    location_bin=:location_bin, location_name=:location_name,"
               "    location_whsezone_id=:location_whsezone_id, location_descrip=:location_descrip,"
               "    location_netable=:location_netable, location_restrict=:location_restrict "
               "WHERE (location_id=:location_id);" );

  q.bindValue(":location_id", _locationid);
  q.bindValue(":location_warehous_id", _warehouse->id());
  q.bindValue(":location_aisle", _aisle->text());
  q.bindValue(":location_rack", _rack->text());
  q.bindValue(":location_bin", _bin->text());
  q.bindValue(":location_name", _location->text());
  q.bindValue(":location_whsezone_id", _whsezone->id());
  q.bindValue(":location_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":location_netable", QVariant(_netable->isChecked(), 0));
  q.bindValue(":location_restrict", QVariant(_restricted->isChecked(), 0));
  q.exec();

  done(_locationid);
}

void location::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM locitem "
                "WHERE (locitem_location_id=:location_id);" );
    q.bindValue(":location_id", _locationid);
    q.exec();
  }

  reject();
}

void location::sHandleWarehouse(int pWarehousid)
{
  q.prepare( "SELECT warehous_enforcearbl, warehous_usezones,"
             "       warehous_aislesize, warehous_aislealpha,"
             "       warehous_racksize, warehous_rackalpha,"
             "       warehous_binsize, warehous_binalpha,"
             "       warehous_locationsize, warehous_locationalpha "
             "FROM warehous "
             "WHERE (warehous_id=:warehous_id);" );
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (q.first())
  {
    if (q.value("warehous_enforcearbl").toBool())
    {
      QString regex;

      if (q.value("warehous_aislesize").toInt() > 0)
      {
        _aisleLit->show();
        _aisle->show();

        if (q.value("warehous_aislealpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_aislesize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_aislesize").toInt());

        _aisle->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _aisle->setMaxLength(q.value("warehous_aislesize").toInt());
      }
      else
      {
        _aisleLit->hide();
        _aisle->hide();
      }

      if (q.value("warehous_racksize").toInt() > 0)
      {
        _rackLit->show();
        _rack->show();

        if (q.value("warehous_rackalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_racksize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_racksize").toInt());

        _rack->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _rack->setMaxLength(q.value("warehous_racksize").toInt());
      }
      else
      {
        _rackLit->hide();
        _rack->hide();
      }

      if (q.value("warehous_binsize").toInt() > 0)
      {
        _binLit->show();
        _bin->show();

        if (q.value("warehous_binalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_binsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_binsize").toInt());

        _bin->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _bin->setMaxLength(q.value("warehous_binsize").toInt());
      }
      else
      {
        _binLit->hide();
        _bin->hide();
      }

      if (q.value("warehous_locationsize").toInt() > 0)
      {
        _locationLit->show();
        _location->show();

        if (q.value("warehous_locationalpha").toBool())
          regex = QString("\\w{1%1}").arg(q.value("warehous_locationsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(q.value("warehous_locationsize").toInt());

        _location->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _location->setMaxLength(q.value("warehous_locationsize").toInt());
      }
      else
      {
        _locationLit->hide();
        _location->hide();
      }
    }
    else
    {
      _aisleLit->hide();
      _aisle->hide();
      _rackLit->hide();
      _rack->hide();
      _binLit->hide();
      _bin->hide();
      _locationLit->show();
      _location->show();
    }

    _whsezone->setAllowNull(!q.value("warehous_usezones").toBool());
    q.prepare( "SELECT whsezone_id, whsezone_name "
               "FROM whsezone "
               "WHERE (whsezone_warehous_id=:warehous_id) "
               "ORDER BY whsezone_name;" );
    q.bindValue(":warehous_id", pWarehousid);
    q.exec();
    _whsezone->populate(q);
    if (!_whsezone->count())
      _whsezone->setEnabled(FALSE);
  }
}

void location::sNew()
{
  ParameterList params;
  itemList newdlg(this, "", TRUE);
  newdlg.set(params);

  int itemid = newdlg.exec();
  if (itemid != -1)
  {
//  Make sure that a locitem does not already exist for this
//  location and the selected item
    q.prepare( "SELECT locitem_id "
               "FROM locitem "
               "WHERE ( (locitem_location_id=:location_id)"
               " AND (locitem_item_id=:item_id) );" );
    q.bindValue(":location_id", _locationid);
    q.bindValue(":item_id", itemid);
    q.exec();
    if (q.first())
//  Tell the user that a locitem already exists for this location/item
      QMessageBox::information( this, tr("Location/Item Exists"),
                                tr( "<p>An Item record already exists in the "
				    "Location. You may not add another record "
				    "for the same Item") );
    else
    {
      q.prepare( "INSERT INTO locitem "
                 "(locitem_location_id, locitem_item_id) "
                 "VALUES "
                 "(:location_id, :item_id);" );
      q.bindValue(":location_id", _locationid);
      q.bindValue(":item_id", itemid);
      q.exec();

      sFillList();
    }
  }
}

void location::sDelete()
{
//  ToDo - add a check to make sure that am itemloc does not exist for the selected locitem pair?
  q.prepare( "DELETE FROM locitem "
             "WHERE (locitem_id=:locitem_id);" );
  q.bindValue(":locitem_id", _locitem->id());
  q.exec();

  sFillList();
}

void location::sFillList()
{
  q.prepare( "SELECT locitem_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2)  "
             "FROM locitem, item "
             "WHERE ( (locitem_item_id=item_id)"
             " AND (locitem_location_id=:location_id) );" );
  q.bindValue(":location_id", _locationid);
  q.exec();
  _locitem->populate(q);
}

void location::populate()
{
  q.prepare( "SELECT * "
             "FROM location "
             "WHERE (location_id=:location_id);" );
  q.bindValue(":location_id", _locationid);
  q.exec();
  if (q.first())
  {
    _aisle->setText(q.value("location_aisle").toString());
    _rack->setText(q.value("location_rack").toString());
    _bin->setText(q.value("location_bin").toString());
    _location->setText(q.value("location_name").toString());
    _description->setText(q.value("location_descrip").toString());
    _netable->setChecked(q.value("location_netable").toBool());
    _restricted->setChecked(q.value("location_restrict").toBool());

    int whsezoneid = q.value("location_whsezone_id").toInt();
    _warehouse->setId(q.value("location_warehous_id").toInt());
    sHandleWarehouse(_warehouse->id());

    _whsezone->setId(whsezoneid);

    sFillList();
  }
}
