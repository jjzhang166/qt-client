/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "relativeCalendarItem.h"

#include <QVariant>
#include <QMessageBox>

static const char *offsetTypes[] = { "D", "W", "M", "Y" };

relativeCalendarItem::relativeCalendarItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_offsetType, SIGNAL(activated(int)), this, SLOT(sHandleNewOffsetType(int)));
}

relativeCalendarItem::~relativeCalendarItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void relativeCalendarItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse relativeCalendarItem::set(const ParameterList &pParams)
{
  XSqlQuery relativeet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
    _calheadid = param.toInt();
  else
    _calheadid = -1;

  param = pParams.value("calendarName", &valid);
  if (valid)
    _calendarName->setText(param.toString());
  else if (_calheadid != -1)
  {
    relativeet.prepare( "SELECT calhead_name "
               "FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );
    relativeet.bindValue(":calhead_id", _calheadid);
    relativeet.exec();
    if (relativeet.first())
      _calendarName->setText(relativeet.value("calhead_name").toString());
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

void relativeCalendarItem::populate()
{
  XSqlQuery relativepopulate;
  relativepopulate.prepare( "SELECT calhead_id, calhead_name "
             "FROM calhead, rcalitem "
             "WHERE ( (rcalitem_calhead_id=calhead_id)"
             " AND (rcalitem_id=:rcalitem_id) );" );
  relativepopulate.bindValue(":rcalitem_id", _calitemid);
  relativepopulate.exec();
  if (relativepopulate.first())
  {
    _calheadid = relativepopulate.value("calhead_id").toInt();
    _calendarName->setText(relativepopulate.value("calhead_name").toString());
  }

  relativepopulate.prepare( "SELECT rcalitem_name,"
             "       rcalitem_offsettype, rcalitem_offsetcount,"
             "       rcalitem_periodtype, rcalitem_periodcount "
             "FROM rcalitem "
             "WHERE (rcalitem_id=:rcalitem_id);" );
  relativepopulate.bindValue(":rcalitem_id", _calitemid);
  relativepopulate.exec();
  if (relativepopulate.first())
  {
    int counter;

    _name->setText(relativepopulate.value("rcalitem_name").toString());

    _offsetCount->setValue(relativepopulate.value("rcalitem_offsetcount").toInt());

    for (counter = 0; counter < _offsetType->count(); counter++)
    {
      if (relativepopulate.value("rcalitem_offsettype").toString() == offsetTypes[counter])
      {
        _offsetType->setCurrentIndex(counter);
        break;
      }
    }

    _periodCount->setValue(relativepopulate.value("rcalitem_periodcount").toInt());

    for (counter = 0; counter < _offsetType->count(); counter++)
    {
      if (relativepopulate.value("rcalitem_periodtype").toString() == offsetTypes[counter])
      {
        _periodType->setCurrentIndex(counter);
        break;
      }
    }

  }
  else
    reject();
}

void relativeCalendarItem::sHandleNewOffsetType(int pOffsetType)
{
  _periodType->setCurrentIndex(pOffsetType);
}

void relativeCalendarItem::sSave()
{
  XSqlQuery relativeSave;
  if (_mode == cNew)
    relativeSave.prepare( "SELECT rcalitem_id "
               "FROM rcalitem "
               "WHERE ( (rcalitem_calhead_id=:calhead_id)"
               " AND (rcalitem_offsettype=:offsetType)"
               " AND (rcalitem_offsetcount=:offsetCount)"
               " AND (rcalitem_periodtype=:periodType)"
               " AND (rcalitem_periodcount=:periodCount) );" );
  else if (_mode == cEdit)
    relativeSave.prepare( "SELECT rcalitem_id "
               "FROM rcalitem "
               "WHERE ( (rcalitem_calhead_id=:calhead_id)"
               " AND (rcalitem_offsettype=:offsetType)"
               " AND (rcalitem_offsetcount=:offsetCount)"
               " AND (rcalitem_periodtype=:periodType)"
               " AND (rcalitem_periodcount=:periodCount)"
               " AND (rcalitem_id<>:rcalitem_id) );" );

  relativeSave.bindValue(":rcalitem_id", _calitemid);
  relativeSave.bindValue(":calhead_id", _calheadid);
  relativeSave.bindValue(":offsetType", offsetTypes[_offsetType->currentIndex()]);
  relativeSave.bindValue(":offsetCount", _offsetCount->value());
  relativeSave.bindValue(":periodType", offsetTypes[_periodType->currentIndex()]);
  relativeSave.bindValue(":periodCount", _periodCount->value());
  relativeSave.exec();
  if (relativeSave.first())
  {
    QMessageBox::critical( this, tr("Cannon Create Duplicate Calendar Item"),
                           tr( "A Relative Calendar Item for the selected Calendar exists that has the save Offset and Period as this Calendar Item.\n"
                               "You may not create duplicate Calendar Items." ) );
    return;
  }

  if (_mode == cNew)
  {
    relativeSave.exec("SELECT NEXTVAL('xcalitem_xcalitem_id_seq') AS _calitem_id;");
    if (relativeSave.first())
      _calitemid = relativeSave.value("_calitem_id").toInt();

    relativeSave.prepare( "INSERT INTO rcalitem "
               "( rcalitem_id, rcalitem_calhead_id, rcalitem_name,"
               "  rcalitem_offsettype, rcalitem_offsetcount,"
               "  rcalitem_periodtype, rcalitem_periodcount ) "
               "VALUES "
               "( :rcalitem_id, :rcalitem_calhead_id, :rcalitem_name,"
               "  :rcalitem_offsettype, :rcalitem_offsetcount,"
               "  :rcalitem_periodtype, :rcalitem_periodcount );" );
  }
  else if (_mode == cEdit)
    relativeSave.prepare( "UPDATE rcalitem "
               "SET rcalitem_name=:rcalitem_name,"
               "    rcalitem_offsettype=:rcalitem_offsettype, rcalitem_offsetcount=:rcalitem_offsetcount,"
               "    rcalitem_periodtype=:rcalitem_periodtype, rcalitem_periodcount=:rcalitem_periodcount "
               "WHERE (rcalitem_id=:rcalitem_id);" );

  relativeSave.bindValue(":rcalitem_id", _calitemid);
  relativeSave.bindValue(":rcalitem_calhead_id", _calheadid);
  relativeSave.bindValue(":rcalitem_name", _name->text());
  relativeSave.bindValue(":rcalitem_offsettype", offsetTypes[_offsetType->currentIndex()]);
  relativeSave.bindValue(":rcalitem_offsetcount", _offsetCount->value());
  relativeSave.bindValue(":rcalitem_periodtype", offsetTypes[_periodType->currentIndex()]);
  relativeSave.bindValue(":rcalitem_periodcount", _periodCount->value());
  relativeSave.exec();

  done(_calitemid);
}


