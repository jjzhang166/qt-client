/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2013 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 *
 * Originally contributed by Specter Business Solutions - specter.ca/business
 */

#include "quickRelocateLot.h"
#include "xsqlquery.h"
#include <QSqlError>
#include <QMessageBox>
#include "errorReporter.h"

quickRelocateLot::quickRelocateLot(QWidget *parent, const char *name, bool modal, Qt::WindowFlags f1)
    : XDialog(parent, name, modal, f1)
{
    setupUi(this);

    _itemloc->addColumn(tr("Item #"),             -1, Qt::AlignLeft,  true, "item_number");
    _itemloc->addColumn(tr("Location"),          200, Qt::AlignLeft,  true, "locationname");
    _itemloc->addColumn(tr("Qty."),       _qtyColumn, Qt::AlignRight, true, "itemloc_qty");
    _itemloc->setPopulateLinear(true);

    // signals and slots connections
    connect(_lotSerial, SIGNAL(editingFinished()), this, SLOT(sFillList()));
    connect(_warehous, SIGNAL(newID()), this, SLOT(sFillList()));
    connect(_assign, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

quickRelocateLot::~quickRelocateLot()
{
    // nothing to delete
}

void quickRelocateLot::languageChange()
{
    retranslateUi(this);
}

void quickRelocateLot::sFillList()
{
  XSqlQuery lotQuery;
  lotQuery.prepare("SELECT itemsite_id, item_number, itemloc_qty,"
                   "       formatLocationName(itemloc_location_id) AS locationname "
                   "FROM itemloc JOIN itemsite ON (itemsite_id=itemloc_itemsite_id)"
                   "             JOIN item ON (item_id=itemsite_item_id)"
                   "             JOIN ls ON (ls_id=itemloc_ls_id) "
                   "WHERE (UPPER(ls_number)=UPPER(:ls_number))"
                   "  AND (itemsite_warehous_id=:warehous) "
                   "ORDER BY item_number;");
  lotQuery.bindValue(":ls_number", _lotSerial->text());
  lotQuery.bindValue(":warehous", _warehous->id());
  lotQuery.exec();
  _itemloc->clear();
  _itemloc->populate(lotQuery);
  _itemloc->selectAll();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot Information"),
                                lotQuery, __FILE__, __LINE__))
  {
    return;
  }
}

void quickRelocateLot::sPost()
{
    int itemsite_id = -1;
    int ls_id = -1;
    int location_id = -1;
    int qoh = -1;

    QList<XTreeWidgetItem*>selected = _itemloc->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();
      if (itemsite_id == -1)
        itemsite_id = id;
      else if (itemsite_id != id)
      {
        QMessageBox::warning(this, tr("Multiple Items"),
                             tr("Please select a single Item to relocate."));
        return;
      }
    }
  
    XSqlQuery lotQuery;
    lotQuery.prepare("SELECT ls_id "
                     "FROM itemsite JOIN ls ON (ls_item_id=itemsite_item_id) "
                     "WHERE (itemsite_id=:itemsite_id)"
                     "  AND (UPPER(ls_number)=UPPER(:ls_number));");
    lotQuery.bindValue(":itemsite_id", itemsite_id);
    lotQuery.bindValue(":ls_number", _lotSerial->text());
    lotQuery.exec();
    if(lotQuery.first())
    {
        ls_id = lotQuery.value("ls_id").toInt();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot Information"),
                                  lotQuery, __FILE__, __LINE__))
    {
      return;
    }

    if (ls_id == -1)
    {
        QMessageBox::warning(this, tr("Invalid Lot/Serial"),
                             tr("The specified lot/serial does not exist."));
        return;
    }

    XSqlQuery locationQuery;
    locationQuery.prepare("SELECT location_id FROM location "
                          "WHERE (formatLocationName(location_id)=:location)"
                          "  AND (location_warehous_id=:warehous)"
                          "  AND (validLocation(location_id, :itemsite_id));");
    locationQuery.bindValue(":location", _location->text());
    locationQuery.bindValue(":warehous", _warehous->id());
    locationQuery.bindValue(":itemsite_id", itemsite_id);
    locationQuery.exec();
    if (locationQuery.first())
    {
        location_id = locationQuery.value("location_id").toInt();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Location Information"),
                                  locationQuery, __FILE__, __LINE__))
    {
      return;
    }

    if (location_id == -1)
    {
        QMessageBox::warning(this, tr("Invalid Location"),
                             tr("The specified location does not exist or is restricted."));
        return;
    }

    XSqlQuery qohQuery;
    qohQuery.prepare("SELECT itemloc_qty "
                     "FROM itemloc "
                     "WHERE (itemloc_ls_id=:ls_id)"
                     "  AND (itemloc_itemsite_id=:itemsite_id);");
    qohQuery.bindValue(":ls_id", ls_id);
    qohQuery.bindValue(":itemsite_id", itemsite_id);
    qohQuery.exec();
    if (qohQuery.first())
    {
        qoh = qohQuery.value("itemloc_qty").toInt();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot/Quantity Information"),
                                  qohQuery, __FILE__, __LINE__))
    {
      return;
    }

    if (qoh <= 0)
    {
        QMessageBox::warning(this, tr("Unable to relocate lot"),
                             QString("Current lot (%1) has no quantity available to relocate.\n\n"
                                     "Please check quantities before attempting to relocate lot.").arg(_lotSerial->text()));
        return;
    }


    XSqlQuery updateQuery;
    updateQuery.prepare("SELECT relocateInventory(itemloc_id, :location_id,"
                        "                         itemloc_itemsite_id, itemloc_qty,"
                        "                         'Quick Relocate Lot') AS result "
                        "FROM itemloc "
                        "WHERE (itemloc_ls_id=:ls_id)"
                        "  AND (itemloc_itemsite_id=:itemsite_id);");
    updateQuery.bindValue(":location_id", location_id);
    updateQuery.bindValue(":ls_id", ls_id);
    updateQuery.bindValue(":itemsite_id", itemsite_id);
    updateQuery.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Lot Information"),
                                  updateQuery, __FILE__, __LINE__))
    {
      return;
    }
  
    //accept();
    clearFields();
}
void quickRelocateLot::clearFields()
{
    _lotSerial->clear();
    _location->clear();
    _itemloc->clear();

    _lotSerial->setFocus();
}

