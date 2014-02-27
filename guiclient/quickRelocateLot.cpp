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

quickRelocateLot::quickRelocateLot(QWidget *parent, const char *name, bool modal, Qt::WFlags f1)
    : XDialog(parent, name, modal, f1)
{
    setupUi(this);

    // signals and slots connections
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


void quickRelocateLot::sPost()
{
    int ls_id = -1;
    int location_id = -1;
    int qoh = -1;
    XSqlQuery lotQuery;
    lotQuery.prepare("SELECT ls_id FROM ls WHERE (ls_number=:ls_number);");
    lotQuery.bindValue(":ls_number", _lotSerial->text());
    lotQuery.exec();
    if(lotQuery.first())
    {
        ls_id = lotQuery.value("ls_id").toInt();
    }
    else if (lotQuery.lastError().type() != QSqlError::NoError)
    {
        systemError(this, lotQuery.lastError().databaseText(), __FILE__, __LINE__);
        return;
    }

    if (ls_id == -1)
    {
        QMessageBox::warning(this, tr("Invalid Lot/Serial"),
                             tr("The specified lot/serial does not exist."));
        return;
    }

    XSqlQuery locationQuery;
    locationQuery.prepare("SELECT location_id FROM location WHERE (formatLocationName(location_id)=:location);");
    locationQuery.bindValue(":location", _location->text());
    locationQuery.exec();
    if (locationQuery.first())
    {
        location_id = locationQuery.value("location_id").toInt();
    }
    else if(locationQuery.lastError().type() != QSqlError::NoError)
    {
      systemError(this, locationQuery.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (location_id == -1)
    {
        QMessageBox::warning(this, tr("Invalid Location"),
                             tr("The specified location does not exist."));
        return;
    }

    XSqlQuery qohQuery;
    qohQuery.prepare("SELECT itemloc_qty FROM itemloc WHERE itemloc_ls_id=:ls_id;");
    qohQuery.bindValue(":ls_id", ls_id);
    qohQuery.exec();
    if (qohQuery.first())
    {
        qoh = qohQuery.value("itemloc_qty").toInt();
    }
    else if (qohQuery.lastError().type() != QSqlError::NoError)
    {
    systemError(this, qohQuery.lastError().databaseText(), __FILE__, __LINE__);
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
                        "WHERE itemloc_ls_id=:ls_id;");
    updateQuery.bindValue(":location_id", location_id);
    updateQuery.bindValue(":ls_id", ls_id);
    updateQuery.exec();
    if (qohQuery.lastError().type() != QSqlError::NoError)
    {
     systemError(this, qohQuery.lastError().databaseText(), __FILE__, __LINE__);
     return;
    }

    //accept();
    clearFields();
}
void quickRelocateLot::clearFields()
{
    _lotSerial->clear();
    _location->clear();

    _lotSerial->setFocus();
}

