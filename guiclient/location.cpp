/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "location.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "itemcluster.h"

location::location(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_location, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sHandleWarehouse(int)));

  _locitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number" );
  _locitem->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip" );

  _warehouse->setAllowNull(_metrics->boolean("MultiWhs"));
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
    sHandleWarehouse(_warehouse->id());
  }
  else
    _warehouse->setNull();

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
  XSqlQuery locationet;
  XDialog::set(pParams);
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

      locationet.exec("SELECT NEXTVAL('location_location_id_seq') AS location_id;");
      if (locationet.first())
        _locationid = locationet.value("location_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );

        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _warehouse->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _warehouse->setEnabled(false);
      _whsezone->setEnabled(false);
      _aisle->setEnabled(false);
      _rack->setEnabled(false);
      _bin->setEnabled(false);
      _location->setEnabled(false);
      _netable->setEnabled(false);
      _usable->setEnabled(false);
      _restricted->setEnabled(false);
      _description->setEnabled(false);
      _locitem->setEnabled(false);
      _save->hide();
      _new->hide();
      _delete->hide();
      _close->setText(tr("&Close"));
    }
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehouse->setId(param.toInt());
    _warehouse->setEnabled(false);
  }

  return NoError;
}

void location::sCheck()
{
  XSqlQuery locationCheck;
  _location->setText(_location->text().trimmed());
  if ( (_mode == cNew) &&
       (_warehouse->isEnabled()) &&
       (_location->text().length()) )
  {
    locationCheck.prepare( "SELECT location_id "
               "FROM location "
               "WHERE ( (location_warehous_id=:location_warehous_id)"
               " AND (UPPER(location_name)=UPPER(:location_name)) );" );
    locationCheck.bindValue(":warehous_id", _warehouse->id());
    locationCheck.bindValue(":location_name", _location->text());
    locationCheck.exec();
    if (locationCheck.first())
    {
      _locationid = locationCheck.value("location_id").toInt();
      _mode = cEdit;
      _warehouse->setEnabled(false);
      _location->setEnabled(false);

      populate();
    }
  }
}

void location::sSave()
{
  XSqlQuery locationSave;
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Select a Site"),
                           tr( "<p>You must select a Site for this "
			      "Location before creating it." ) );
    _warehouse->setFocus();
    return;
  }

  if ( (_location->text().trimmed().length() == 0) &&
       (_aisle->text().trimmed().length() == 0) &&
       (_rack->text().trimmed().length() == 0) &&
       (_bin->text().trimmed().length() == 0) )
  {
    QMessageBox::critical( this, tr("Enter Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location.") );
    _location->setFocus();
    return;
  }

  if ( (!_netable->isChecked()) && (_usable->isChecked()) )
  {
    QMessageBox::critical( this, tr("Invalid Status"),
                          tr( "<p>Non Nettable inventory cannot be Usable inventory." ) );
    _netable->setFocus();
    return;
  }
  
  locationSave.prepare("SELECT location_id"
            "  FROM location"
            " WHERE((location_id != :location_id)"
            "   AND (location_warehous_id=:location_warehous_id)"
            "   AND (location_aisle=:location_aisle)"
            "   AND (location_rack=:location_rack)"
            "   AND (location_bin=:location_bin)"
            "   AND (location_name=:location_name))");
  locationSave.bindValue(":location_id", _locationid);
  locationSave.bindValue(":location_warehous_id", _warehouse->id());
  locationSave.bindValue(":location_aisle", _aisle->text());
  locationSave.bindValue(":location_rack", _rack->text());
  locationSave.bindValue(":location_bin", _bin->text());
  locationSave.bindValue(":location_name", _location->text());
  locationSave.exec();
  if(locationSave.first())
  {
    QMessageBox::critical( this, tr("Duplicate Location Name"),
                           tr("<p>You must enter a unique name to identify "
			      "this Location in the specified Site.") );
    _location->setFocus();
    return;
  }
  

  if (_mode == cNew)
    locationSave.prepare( "INSERT INTO location "
               "( location_id, location_warehous_id,"
               "  location_aisle, location_rack, location_bin,  location_name,"
               "  location_whsezone_id, location_descrip,"
               "  location_netable, location_usable, location_restrict ) "
               "VALUES "
               "( :location_id, :location_warehous_id,"
               "  :location_aisle, :location_rack, :location_bin, :location_name,"
               "  :location_whsezone_id, :location_descrip,"
               "  :location_netable, :location_usable, :location_restrict );" );
  else if (_mode == cEdit)
    locationSave.prepare( "UPDATE location "
               "SET location_warehous_id=:location_warehous_id,"
               "    location_aisle=:location_aisle,"
               "    location_rack=:location_rack,"
               "    location_bin=:location_bin,"
               "    location_name=:location_name,"
               "    location_whsezone_id=:location_whsezone_id,"
               "    location_descrip=:location_descrip,"
               "    location_netable=:location_netable,"
               "    location_usable=:location_usable,"
               "    location_restrict=:location_restrict "
               "WHERE (location_id=:location_id);" );

  locationSave.bindValue(":location_id", _locationid);
  locationSave.bindValue(":location_warehous_id", _warehouse->id());
  locationSave.bindValue(":location_aisle", _aisle->text());
  locationSave.bindValue(":location_rack", _rack->text());
  locationSave.bindValue(":location_bin", _bin->text());
  locationSave.bindValue(":location_name", _location->text());
  locationSave.bindValue(":location_whsezone_id", _whsezone->id());
  locationSave.bindValue(":location_descrip", _description->toPlainText().trimmed());
  locationSave.bindValue(":location_netable", QVariant(_netable->isChecked()));
  locationSave.bindValue(":location_usable", QVariant(_usable->isChecked()));
  locationSave.bindValue(":location_restrict", QVariant(_restricted->isChecked()));
  locationSave.exec();

  done(_locationid);
}

void location::sClose()
{
  XSqlQuery locationClose;
  if (_mode == cNew)
  {
    locationClose.prepare( "DELETE FROM locitem "
                "WHERE (locitem_location_id=:location_id);" );
    locationClose.bindValue(":location_id", _locationid);
    locationClose.exec();
  }

  reject();
}

void location::sHandleWarehouse(int pWarehousid)
{
  XSqlQuery locationHandleWarehouse;
  locationHandleWarehouse.prepare( "SELECT warehous_enforcearbl, warehous_usezones,"
             "       warehous_aislesize, warehous_aislealpha,"
             "       warehous_racksize, warehous_rackalpha,"
             "       warehous_binsize, warehous_binalpha,"
             "       warehous_locationsize, warehous_locationalpha "
             "FROM whsinfo "
             "WHERE (warehous_id=:warehous_id);" );
  locationHandleWarehouse.bindValue(":warehous_id", pWarehousid);
  locationHandleWarehouse.exec();
  if (locationHandleWarehouse.first())
  {
    if (locationHandleWarehouse.value("warehous_enforcearbl").toBool())
    {
      QString regex;

      if (locationHandleWarehouse.value("warehous_aislesize").toInt() > 0)
      {
        _aisleLit->show();
        _aisle->show();

        if (locationHandleWarehouse.value("warehous_aislealpha").toBool())
          regex = QString("\\w{1%1}").arg(locationHandleWarehouse.value("warehous_aislesize").toInt());
        else
          regex = QString("\\d{1%1}").arg(locationHandleWarehouse.value("warehous_aislesize").toInt());

        _aisle->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _aisle->setMaxLength(locationHandleWarehouse.value("warehous_aislesize").toInt());
      }
      else
      {
        _aisleLit->hide();
        _aisle->hide();
      }

      if (locationHandleWarehouse.value("warehous_racksize").toInt() > 0)
      {
        _rackLit->show();
        _rack->show();

        if (locationHandleWarehouse.value("warehous_rackalpha").toBool())
          regex = QString("\\w{1%1}").arg(locationHandleWarehouse.value("warehous_racksize").toInt());
        else
          regex = QString("\\d{1%1}").arg(locationHandleWarehouse.value("warehous_racksize").toInt());

        _rack->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _rack->setMaxLength(locationHandleWarehouse.value("warehous_racksize").toInt());
      }
      else
      {
        _rackLit->hide();
        _rack->hide();
      }

      if (locationHandleWarehouse.value("warehous_binsize").toInt() > 0)
      {
        _binLit->show();
        _bin->show();

        if (locationHandleWarehouse.value("warehous_binalpha").toBool())
          regex = QString("\\w{1%1}").arg(locationHandleWarehouse.value("warehous_binsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(locationHandleWarehouse.value("warehous_binsize").toInt());

        _bin->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _bin->setMaxLength(locationHandleWarehouse.value("warehous_binsize").toInt());
      }
      else
      {
        _binLit->hide();
        _bin->hide();
      }

      if (locationHandleWarehouse.value("warehous_locationsize").toInt() > 0)
      {
        _locationLit->show();
        _location->show();

        if (locationHandleWarehouse.value("warehous_locationalpha").toBool())
          regex = QString("\\w{1%1}").arg(locationHandleWarehouse.value("warehous_locationsize").toInt());
        else
          regex = QString("\\d{1%1}").arg(locationHandleWarehouse.value("warehous_locationsize").toInt());

        _location->setValidator(new QRegExpValidator(QRegExp(regex), this));
        _location->setMaxLength(locationHandleWarehouse.value("warehous_locationsize").toInt());
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

    _whsezone->setAllowNull(!locationHandleWarehouse.value("warehous_usezones").toBool());
    locationHandleWarehouse.prepare( "SELECT whsezone_id, whsezone_name "
               "FROM whsezone "
               "WHERE (whsezone_warehous_id=:warehous_id) "
               "ORDER BY whsezone_name;" );
    locationHandleWarehouse.bindValue(":warehous_id", pWarehousid);
    locationHandleWarehouse.exec();
    _whsezone->populate(locationHandleWarehouse);
    if (!_whsezone->count())
      _whsezone->setEnabled(false);
  }
}

void location::sNew()
{
  XSqlQuery locationNew;
  ParameterList params;
  itemList* newdlg = new itemList(this);
  newdlg->set(params);

  int itemid = newdlg->exec();
  if (itemid != -1)
  {
//  Make sure that a locitem does not already exist for this
//  location and the selected item
    locationNew.prepare( "SELECT locitem_id "
               "FROM locitem "
               "WHERE ( (locitem_location_id=:location_id)"
               " AND (locitem_item_id=:item_id) );" );
    locationNew.bindValue(":location_id", _locationid);
    locationNew.bindValue(":item_id", itemid);
    locationNew.exec();
    if (locationNew.first())
//  Tell the user that a locitem already exists for this location/item
      QMessageBox::information( this, tr("Location/Item Exists"),
                                tr( "<p>An Item record already exists in the "
				    "Location. You may not add another record "
				    "for the same Item") );
    else
    {
      locationNew.prepare( "INSERT INTO locitem "
                 "(locitem_location_id, locitem_item_id) "
                 "VALUES "
                 "(:location_id, :item_id);" );
      locationNew.bindValue(":location_id", _locationid);
      locationNew.bindValue(":item_id", itemid);
      locationNew.exec();

      sFillList();
    }
  }
}

void location::sDelete()
{
  XSqlQuery locationDelete;
//  ToDo - add a check to make sure that am itemloc does not exist for the selected locitem pair?
  locationDelete.prepare( "DELETE FROM locitem "
             "WHERE (locitem_id=:locitem_id);" );
  locationDelete.bindValue(":locitem_id", _locitem->id());
  locationDelete.exec();

  sFillList();
}

void location::sFillList()
{
  XSqlQuery locationFillList;
  locationFillList.prepare( "SELECT locitem_id, item_number,"
             "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip  "
             "FROM locitem, item "
             "WHERE ( (locitem_item_id=item_id)"
             " AND (locitem_location_id=:location_id) );" );
  locationFillList.bindValue(":location_id", _locationid);
  locationFillList.exec();
  _locitem->populate(locationFillList);
}

void location::populate()
{
  XSqlQuery locationpopulate;
  locationpopulate.prepare( "SELECT * "
             "FROM location "
             "WHERE (location_id=:location_id);" );
  locationpopulate.bindValue(":location_id", _locationid);
  locationpopulate.exec();
  if (locationpopulate.first())
  {
    _aisle->setText(locationpopulate.value("location_aisle").toString());
    _rack->setText(locationpopulate.value("location_rack").toString());
    _bin->setText(locationpopulate.value("location_bin").toString());
    _location->setText(locationpopulate.value("location_name").toString());
    _description->setText(locationpopulate.value("location_descrip").toString());
    _netable->setChecked(locationpopulate.value("location_netable").toBool());
    _usable->setChecked(locationpopulate.value("location_usable").toBool());
    _restricted->setChecked(locationpopulate.value("location_restrict").toBool());

    int whsezoneid = locationpopulate.value("location_whsezone_id").toInt();
    _warehouse->setId(locationpopulate.value("location_warehous_id").toInt());
    sHandleWarehouse(_warehouse->id());

    _whsezone->setId(whsezoneid);

    sFillList();
  }
}
