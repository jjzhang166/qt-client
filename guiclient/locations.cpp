/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "locations.h"

#include <QVariant>
#include <QMessageBox>
//#include <QWorkspace>
#include <openreports.h>
#include <parameter.h>
#include "location.h"
#include <metasql.h>

locations::locations(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_location, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(updateZoneList()));
  connect(_zone, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillList()));

  _location->addColumn(tr("Site"),        _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  _location->addColumn(tr("Zone"),        -1, Qt::AlignLeft,   true,  "zone"   );
  _location->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,   true,  "name"   );
  _location->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "locationname"   );
  _location->addColumn(tr("Netable"),     80,          Qt::AlignCenter, true,  "netable" );
  _location->addColumn(tr("Usable"),      80,          Qt::AlignCenter, true,  "usable" );
  _location->addColumn(tr("Restricted"),  80,          Qt::AlignCenter, true,  "restricted" );

  if (_privileges->check("MaintainLocations"))
  {
    connect(_location, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_location, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_location, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_location, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  updateZoneList();
  sFillList();
}

locations::~locations()
{
  // no need to delete child widgets, Qt does it all for us
}

void locations::languageChange()
{
  retranslateUi(this);
}

void locations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  location newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void locations::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("location_id", _location->id());

  location newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void locations::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("location_id", _location->id());

  location newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}


void locations::sDelete()
{
  XSqlQuery locationsDelete;
  locationsDelete.prepare("SELECT deleteLocation(:location_id) AS result;");
  locationsDelete.bindValue(":location_id", _location->id());
  locationsDelete.exec();
  if (locationsDelete.first())
  {
    QString caption = tr("Cannot Delete Selected Location");
    QString msg;

    switch (locationsDelete.value("result").toInt())
    {
      case -1:
        msg = tr("<p>There are one or more Item Sites that use the selected Location as their default Location. "
		 "You must assign the default Location for all Item Sites that use the selected Location before you may delete it.</p>" );
        break;

      case -2:
        msg = tr("<p>There is Inventory contained in the selected Location. "
		 "You must move all Inventory out of the selected Location and may then set its status to inactive.</p>" );
        break;

      case -3:
        msg = tr("<p>There are one or more undistributed Location records that have been posted against the selected Locations. "
		 "This probably indicates a system error.<br><br>"
		 "Please contact your Systems Adminstrator to have this resolved.</p>" );
        break;

      case -4:
        msg = tr("<p>The selected Location cannot be deleted as there has been Inventory Transaction History posted against it.</p> " );
        break;
    }
    if (! msg.isEmpty())
    {
      QMessageBox::critical(this, caption, msg);
      return;
    }

    sFillList();
  }

//  ToDo
}


void locations::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  orReport report("WarehouseLocationMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}


void locations::sFillList()
{
  QString sql( "SELECT location_id, warehous_code, formatLocationName(location_id) AS name,"
               "       whsezone_name||'-'||whsezone_descrip as zone,"
               "       firstLine(location_descrip) AS locationname,"
               "       formatBoolYN(location_netable) AS netable,"
               "       formatBoolYN(location_usable) AS usable,"
               "       formatBoolYN(location_restrict) AS restricted "
               "FROM location  "
               " JOIN whsinfo ON (location_warehous_id=warehous_id) "
               " LEFT OUTER JOIN whsezone ON (location_whsezone_id=whsezone_id) "
               " WHERE ( (true)" 
               " <? if exists('warehous_id') ?>"
               " AND (warehous_id=<? value('warehous_id') ?>) "
               " <? endif ?> "
               " <? if exists('zone_id') ?>"
               " AND (location_whsezone_id=<? value('zone_id') ?>) "
               " <? endif ?> ) "
               "ORDER BY warehous_code, locationname;");

  MetaSQLQuery  mql(sql);
  ParameterList params;

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
  if (_zone->id() > 0)
    params.append("zone_id", _zone->id());

  XSqlQuery locationsFillList = mql.toQuery(params);
  if (locationsFillList.first())
    _location->populate(locationsFillList);
}

void locations::updateZoneList()
{
  QString zoneSql( "SELECT whsezone_id, whsezone_name||'-'||whsezone_descrip "
             " FROM whsezone  "
             " <? if exists('warehous_id') ?> "
             " WHERE (whsezone_warehous_id = <? value('warehous_id') ?>) "
             " <? endif ?> "
             " ORDER BY whsezone_name;");

  MetaSQLQuery  mql(zoneSql);
  ParameterList params;

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  XSqlQuery zoneFillList = mql.toQuery(params);
  if (zoneFillList.first())
    _zone->populate(zoneFillList);
}

