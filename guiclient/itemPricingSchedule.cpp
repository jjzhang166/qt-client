/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemPricingSchedule.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "itemPricingScheduleItem.h"

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"

itemPricingSchedule::itemPricingSchedule(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery itemitemPricingSchedule;
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sCheckCurrency()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Expires"));

  _ipsitem->addColumn(tr("Target"),          _ynColumn,    Qt::AlignLeft,   true,  "target"  );
  _ipsitem->addColumn(tr("Item/Prod. Cat."), _itemColumn,  Qt::AlignLeft,   true,  "number"  );
  _ipsitem->addColumn(tr("Description"),     -1,           Qt::AlignLeft,   true,  "descrip"  );
  _ipsitem->addColumn(tr("Qty. UOM"),        _uomColumn,   Qt::AlignCenter, true,  "qtyuom");
  _ipsitem->addColumn(tr("Qty. Break"),      _qtyColumn,   Qt::AlignRight,  true,  "qtybreak" );
  _ipsitem->addColumn(tr("Price UOM"),       _uomColumn,   Qt::AlignCenter, true,  "priceuom");
  _ipsitem->addColumn(tr("Price/Percent"),   _priceColumn, Qt::AlignRight,  true,  "price" );
  _ipsitem->addColumn(tr("Fixed Amt."),      _priceColumn, Qt::AlignRight,  true,  "fixedAmt" );
  _ipsitem->addColumn(tr("Net Price"),       _priceColumn, Qt::AlignRight,  true,  "netPrice" );
  _ipsitem->addColumn(tr("Type"),            _uomColumn,   Qt::AlignLeft,   true,  "type" );
  _ipsitem->addColumn(tr("Method"),          _uomColumn,   Qt::AlignLeft,   true,  "method" );

  _currency->setType(XComboBox::Currencies);
  _currency->setLabel(_currencyLit);
  _updated = QDate::currentDate();
  _listpricesched = false;
  
  itemitemPricingSchedule.exec("BEGIN;");
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

itemPricingSchedule::~itemPricingSchedule()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemPricingSchedule::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingSchedule::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("listpricesched", &valid);
  if (valid)
  {
    _listpricesched = true;
    setWindowTitle(tr("List Pricing Schedule"));
  }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _dates->setEnabled(false);
      _warehouse->setEnabled(false);
      _currency->setEnabled(false);
      _new->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  param = pParams.value("ipshead_id", &valid);
  if (valid)
  {
    _ipsheadid = param.toInt();
    populate();
  }

  if ( (_mode == cNew) || (_mode == cEdit) || (_mode == cCopy) )
  {
    connect(_ipsitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }

  if (_mode == cNew)
  {
    itemet.exec("SELECT NEXTVAL('ipshead_ipshead_id_seq') AS ipshead_id;");
    if (itemet.first())
      _ipsheadid = itemet.value("ipshead_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Item Pricing Information"),
                                  itemet, __FILE__, __LINE__))
    {
      reject();
      return UndefinedError;
    }
  }

  return NoError;
}

bool itemPricingSchedule::sSave(bool p)
{
  XSqlQuery itemSave;
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Name"),
                           tr("You must enter a Name for this Pricing Schedule.") );
    _name->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Effective Date"),
                           tr("You must enter an effective date for this Pricing Schedule.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("You must enter an expiration date for this Pricing Schedule.") );
    _dates->setFocus();
    return false;
  }

  if (_dates->endDate() < _dates->startDate())
  {
    QMessageBox::critical( this, tr("Invalid Expiration Date"),
                           tr("The expiration date cannot be earlier than the effective date.") );
    _dates->setFocus();
    return false;
  }

  itemSave.prepare("SELECT ipshead_id"
            "  FROM ipshead"
            " WHERE ((ipshead_name=:ipshead_name)"
            "   AND  (ipshead_id != :ipshead_id))");
  itemSave.bindValue(":ipshead_id", _ipsheadid);
  itemSave.bindValue(":ipshead_name", _name->text());
  itemSave.exec();
  if(itemSave.first())
  {
    QMessageBox::warning(this, tr("Pricing Schedule Already Exists"),
                      tr("A Pricing Schedule with the entered Name already exists."));
    return false;
  }

  if (_mode == cNew) 
    itemSave.prepare( "INSERT INTO ipshead "
                     "( ipshead_id, ipshead_name, ipshead_descrip,"
                     "  ipshead_effective, ipshead_expires,"
                     "  ipshead_curr_id, ipshead_listprice,"
                     "  ipshead_updated ) "
                     "VALUES "
                     "( :ipshead_id, :ipshead_name, :ipshead_descrip,"
                     "  :ipshead_effective, :ipshead_expires,"
                     "  :ipshead_curr_id, :ipshead_listprice,"
                     "  CURRENT_DATE );" );
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    itemSave.prepare( "UPDATE ipshead "
                     "SET ipshead_name=:ipshead_name, ipshead_descrip=:ipshead_descrip,"
                     "    ipshead_effective=:ipshead_effective, ipshead_expires=:ipshead_expires, "
                     "    ipshead_curr_id=:ipshead_curr_id, "
                     "    ipshead_listprice=:ipshead_listprice, "
                     "    ipshead_updated=CURRENT_DATE "
                     "WHERE (ipshead_id=:ipshead_id);" );

  itemSave.bindValue(":ipshead_id", _ipsheadid);
  itemSave.bindValue(":ipshead_name", _name->text());
  itemSave.bindValue(":ipshead_descrip", _descrip->text());
  itemSave.bindValue(":ipshead_effective", _dates->startDate());
  itemSave.bindValue(":ipshead_expires", _dates->endDate());
  itemSave.bindValue(":ipshead_curr_id", _currency->id());
  itemSave.bindValue(":ipshead_listprice", _listpricesched);
  itemSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Item Pricing Information"),
                                itemSave, __FILE__, __LINE__))
  {
    reject();
  }

  _mode = cEdit;
  
  if (p)
  {
    itemSave.exec("COMMIT;");
    done(_ipsheadid);
  }
  return true;
}

void itemPricingSchedule::sSave()
{
  XSqlQuery itemSave;
  sSave(true) ;  
}

void itemPricingSchedule::sCheck()
{
}

void itemPricingSchedule::sNew()
{
  if(!sSave(false))
    return;
  ParameterList params;
  params.append("mode", "new");
  params.append("ipshead_id", _ipsheadid);
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);
  if (_listpricesched)
    params.append("listpricesched", true);

  itemPricingScheduleItem newdlg(this, "", true);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      reject();
    else
      sFillList(result);
  }
}

void itemPricingSchedule::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);
  if (_listpricesched)
    params.append("listpricesched", true);

  if(_ipsitem->altId() == 1)
    params.append("ipsitem_id", _ipsitem->id());
  else if(_ipsitem->altId() == 2)
    params.append("ipsfreight_id", _ipsitem->id());
  else
    return;
    // ToDo - tell the user why we're not showing the pricing sched?

  itemPricingScheduleItem newdlg(this, "", true);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
  {
    if (result == -1)
      reject();
    else
      sFillList(result);
  }
}

void itemPricingSchedule::sDelete()
{
  XSqlQuery itemDelete;
  if(_ipsitem->altId() == 1)
    itemDelete.prepare( "DELETE FROM ipsiteminfo "
               "WHERE (ipsitem_id=:ipsitem_id);" );
  else if(_ipsitem->altId() == 2)
    itemDelete.prepare( "DELETE FROM ipsfreight "
               "WHERE (ipsfreight_id=:ipsitem_id);" );
  else
    return;
  itemDelete.bindValue(":ipsitem_id", _ipsitem->id());
  itemDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Item Pricing Information"),
                                itemDelete, __FILE__, __LINE__))
  {
    reject();
  }

  sFillList();
}

void itemPricingSchedule::sCheckCurrency()
{
  XSqlQuery currCheck;
  currCheck.prepare( "SELECT curr_rate "
                     "FROM curr_rate "
                     "WHERE ( (curr_id=:curr_id) "
                     "  AND   (CURRENT_DATE BETWEEN curr_effective AND curr_expires) );" );
  currCheck.bindValue(":curr_id", _currency->id());
  currCheck.exec();
  if (!currCheck.first())
  {
    QMessageBox::critical( this, tr("Currency Exchange Rate"),
                          tr("Currency Exchange Rate not found.  You should correct before proceeding.") );
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Currency Information"),
                                currCheck, __FILE__, __LINE__))
  {
    reject();
  }
}

void itemPricingSchedule::sFillList()
{
  XSqlQuery itemFillList;
  sFillList(-1);
}

void itemPricingSchedule::sFillList(int pIpsitemid)
{
  XSqlQuery itemFillList;
  MetaSQLQuery mql = mqlLoad("itemPricingSchedule", "detail");
  ParameterList params;
  params.append("ipshead_id", _ipsheadid);
  params.append("warehous_id", _warehouse->id());
  params.append("item",tr("Item"));
  params.append("prodcat", tr("Prod. Cat."));
  params.append("flatrate", tr("Flat Rate"));
  params.append("peruom", tr("Price Per UOM"));
  params.append("nominal",tr("Nominal"));
  params.append("discount",tr("Discount"));
  params.append("markup",tr("Markup"));
  params.append("freight", tr("Freight"));
  params.append("price", tr("Price"));
  params.append("fixed", tr("Fixed"));
  params.append("percent", tr("Percent"));
  params.append("mixed", tr("Mixed"));
  params.append("allsites", tr("All Sites"));
  params.append("allzones", tr("All Shipping Zones"));

  itemFillList = mql.toQuery(params);

  if (pIpsitemid == -1)
    _ipsitem->populate(itemFillList, true);
  else
    _ipsitem->populate(itemFillList, pIpsitemid, true);

  _currency->setEnabled(_ipsitem->topLevelItemCount() <= 0);
}

void itemPricingSchedule::populate()
{
  XSqlQuery itempopulate;
  XSqlQuery pop;
  pop.prepare( "SELECT * "
             "FROM ipshead "
             "WHERE (ipshead_id=:ipshead_id);" );
  pop.bindValue(":ipshead_id", _ipsheadid);
  pop.exec();
  if (pop.first())
  {
    _name->setText(pop.value("ipshead_name").toString());
    _descrip->setText(pop.value("ipshead_descrip").toString());
    _dates->setStartDate(pop.value("ipshead_effective").toDate());
    _dates->setEndDate(pop.value("ipshead_expires").toDate());
    _currency->setId(pop.value("ipshead_curr_id").toInt());
    _currency->setEnabled(false);
    _listpricesched = pop.value("ipshead_listprice").toBool();
    QDate tmpDate = pop.value("ipshead_updated").toDate();
    if (tmpDate.isValid() && ! tmpDate.isNull())
	_updated = tmpDate;

    sFillList(-1);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Pricing Schedule Information"),
                                pop, __FILE__, __LINE__))
  {
    reject();
  }
}

void itemPricingSchedule::reject()
{
  XSqlQuery itemreject;
  itemreject.exec("ROLLBACK;");
  if(_mode == cCopy) 
  {
    itemreject.prepare( "DELETE FROM ipsiteminfo "
               "WHERE (ipsitem_ipshead_id=:ipshead_id); "
               "DELETE FROM ipsfreight "
               "WHERE (ipsfreight_ipshead_id=:ipshead_id); "
               "DELETE FROM ipshead "
               "WHERE (ipshead_id=:ipshead_id);" );
    itemreject.bindValue(":ipshead_id", _ipsheadid);
    itemreject.exec();
  }
  
  XDialog::reject();
}
