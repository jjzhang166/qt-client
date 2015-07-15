/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "absoluteCalendarItem.h"

#include <QVariant>

absoluteCalendarItem::absoluteCalendarItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

absoluteCalendarItem::~absoluteCalendarItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void absoluteCalendarItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse absoluteCalendarItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
    _calheadid = param.toInt();
  else
    _calheadid = -1;

  XSqlQuery setCalendarItem;
  param = pParams.value("calendarName", &valid);
  if (valid)
    _calendarName->setText(param.toString());
  else if (_calheadid != -1)
  {
    setCalendarItem.prepare( "SELECT calhead_name "
               "FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );
    setCalendarItem.bindValue(":calhead_id", _calheadid);
    setCalendarItem.exec();
    if (setCalendarItem.first())
      _calendarName->setText(setCalendarItem.value("calhead_name").toString());
  }

  param = pParams.value("calitem_id", &valid);
  if (valid)
  {
    _calitemid = param.toInt();
    populate();
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
  }

  return NoError;
}

void absoluteCalendarItem::sSave()
{
  XSqlQuery saveCalendarItem;
  if (_mode == cNew)
  {
    saveCalendarItem.exec("SELECT NEXTVAL('xcalitem_xcalitem_id_seq') AS _calitem_id");
    if (saveCalendarItem.first())
      _calitemid = saveCalendarItem.value("_calitem_id").toInt();
//  ToDo

    saveCalendarItem.prepare( "INSERT INTO acalitem "
               "( acalitem_id, acalitem_calhead_id, acalitem_name,"
               "  acalitem_periodstart, acalitem_periodlength )"
               "VALUES "
               "( :acalitem_id, :acalitem_calhead_id, :acalitem_name,"
               "  :acalitem_periodstart, :acalitem_periodlength )" );
  }
  else
    saveCalendarItem.prepare( "UPDATE acalitem "
               "SET acalitem_name=:acalitem_name,"
               "    acalitem_periodstart=:acalitem_periodstart, acalitem_periodlength=:acalitem_periodlength "
               "WHERE (acalitem_id=:acalitem_id);" );

  saveCalendarItem.bindValue(":acalitem_id", _calitemid);
  saveCalendarItem.bindValue(":acalitem_calhead_id", _calheadid);
  saveCalendarItem.bindValue(":acalitem_name", _name->text());
  saveCalendarItem.bindValue(":acalitem_periodstart", _startDate->date());
  saveCalendarItem.bindValue(":acalitem_periodlength", _periodLength->value());
  saveCalendarItem.exec();

  done (_calitemid);
}

void absoluteCalendarItem::populate()
{
  XSqlQuery populateCalendarItem;
  populateCalendarItem.prepare( "SELECT calhead_id, calhead_name "
             "FROM calhead, acalitem "
             "WHERE ( (acalitem_calhead_id=calhead_id)"
             " AND (acalitem_id=:acalitem_id));" );
  populateCalendarItem.bindValue(":acalitem_id", _calitemid);
  populateCalendarItem.exec();
  if (populateCalendarItem.first())
  {
    _calheadid = populateCalendarItem.value("calhead_id").toInt();
    _calendarName->setText(populateCalendarItem.value("calhead_name").toString());
  }

  populateCalendarItem.prepare( "SELECT acalitem_name,"
             "       acalitem_periodstart, acalitem_periodlength "
             "FROM acalitem "
             "WHERE (acalitem_id=:acalitem_id);" );
  populateCalendarItem.bindValue(":acalitem_id", _calitemid);
  populateCalendarItem.exec();
  if (populateCalendarItem.first())
  {
    _name->setText(populateCalendarItem.value("acalitem_name").toString());
    _startDate->setDate(populateCalendarItem.value("acalitem_periodstart").toDate());
    _periodLength->setValue(populateCalendarItem.value("acalitem_periodlength").toInt());
  }
//  ToDo
}
