/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "calendar.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "absoluteCalendarItem.h"
#include "relativeCalendarItem.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

static const char *originTypes[] = { "D", "E", "W", "X", "M", "N", "L", "Y", "Z" };

calendar::calendar(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
}

calendar::~calendar()
{
  // no need to delete child widgets, Qt does it all for us
}

void calendar::languageChange()
{
  retranslateUi(this);
}

enum SetResponse calendar::set(const ParameterList &pParams)
{
  XSqlQuery calendaret;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
  {
    _calheadid = param.toInt();
    populate();
  }

  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "absolute")
    {
      _type = 'A';
      _absolute->setChecked(true);
      _relative->setEnabled(false);
      _absolute->setEnabled(false);
    }
    else if (param.toString() == "relative")
    {
      _type = 'R';
      _relative->setChecked(true);
      _relative->setEnabled(false);
      _absolute->setEnabled(false);
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      calendaret.exec("SELECT NEXTVAL('calhead_calhead_id_seq') AS _calhead_id;");
      if (calendaret.first())
        _calheadid = calendaret.value("_calhead_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating New Calendar"),
                                    calendaret, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _relative->setEnabled(false);
      _absolute->setEnabled(false);
    }
  }

  return NoError;
}

void calendar::sSave()
{
  XSqlQuery calendarSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                          tr("You must enter a valid Calendar Name."))
    ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Calendar"), errors))
    return;

  calendarSave.prepare("SELECT calhead_id"
            "  FROM calhead"
            " WHERE((calhead_id != :calhead_id)"
            "   AND (calhead_name=:calhead_name))");
  calendarSave.bindValue(":calhead_id",       _calheadid);
  calendarSave.bindValue(":calhead_name",   _name->text().trimmed());
  calendarSave.exec();
  if(calendarSave.first())
  {
    QMessageBox::critical(this, tr("Duplicate Calendar Name"),
      tr("A Calendar already exists for the Name specified.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (_type =='R')
      calendarSave.prepare( "INSERT INTO calhead "
                 "(calhead_id, calhead_name, calhead_descrip, calhead_type, calhead_origin) "
                 "VALUES "
                 "(:calhead_id, :calhead_name, :calhead_descrip, 'R', :calhead_origin);" );
    else
      calendarSave.prepare( "INSERT INTO calhead "
                 "(calhead_id, calhead_name, calhead_descrip, calhead_type) "
                 "VALUES "
                 "(:calhead_id, :calhead_name, :calhead_descrip, 'A');" );
  }
  else if (_mode == cEdit)
  {
    if (_type =='R')
      calendarSave.prepare( "UPDATE calhead "
                 "SET calhead_name=:calhead_name, calhead_descrip=:calhead_descrip, calhead_origin=:calhead_origin "
                 "WHERE (calhead_id=:calhead_id);" );
    else
      calendarSave.prepare( "UPDATE calhead "
                 "SET calhead_name=:calhead_name, calhead_descrip=:calhead_descrip "
                 "WHERE (calhead_id=:calhead_id);" );
  }

  calendarSave.bindValue(":calhead_id", _calheadid);
  calendarSave.bindValue(":calhead_name", _name->text().trimmed());
  calendarSave.bindValue(":calhead_descrip", _descrip->text());
  calendarSave.bindValue(":calhead_origin", originTypes[_origin->currentIndex()]);
  calendarSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Calendar"),
                                calendarSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_calheadid);
}

void calendar::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("calhead_id", _calheadid);

  if (_mode == cNew)
    params.append("calendarName", _name->text());

  if (_type == 'A')
  {
    absoluteCalendarItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (_type == 'R')
  {
    relativeCalendarItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendar::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("calitem_id", _calitem->id());

  if (_type == 'A')
  {
    absoluteCalendarItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
  else if (_type == 'R')
  {
    relativeCalendarItem newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void calendar::sDelete()
{
  XSqlQuery calendarDelete;
  if (_type == 'A')
    calendarDelete.prepare( "DELETE FROM acalitem "
               "WHERE (acalitem_id=:xcalitem_id);" );
  else if (_type == 'R')
    calendarDelete.prepare( "DELETE FROM rcalitem "
               "WHERE (rcalitem_id=:xcalitem_id);" );

  calendarDelete.bindValue(":xcalitem_id", _calitem->id());
  calendarDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Calendar Item"),
                                calendarDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void calendar::sFillList()
{
  XSqlQuery calendarFillList;
  _calitem->setColumnCount(0);
  _calitem->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "name");
  if (_type == 'A')
  {
    _calitem->addColumn(tr("Start Date"), _dateColumn, Qt::AlignCenter, true, "acalitem_periodstart");
    _calitem->addColumn(tr("Period"),     _qtyColumn,  Qt::AlignRight,  true, "periodlength");
    calendarFillList.prepare( "SELECT acalitem_id, acalitem_name AS name, acalitem_periodstart,"
               "       (TEXT(acalitem_periodlength) || TEXT(:days)) AS periodlength "
               "FROM acalitem "
               "WHERE (acalitem_calhead_id=:calhead_id) "
               "ORDER BY acalitem_periodstart;" );
    calendarFillList.bindValue(":days", tr("Day(s)"));
  }
  else if (_type == 'R')
  {
    _calitem->addColumn(tr("Offset"),        _itemColumn, Qt::AlignRight, true, "offsetdays");
    _calitem->addColumn(tr("Period Length"), -1,          Qt::AlignLeft,  true, "periodlength");
    calendarFillList.prepare( "SELECT rcalitem_id, rcalitem_name AS name,"
               "       CASE WHEN (rcalitem_periodtype='D') THEN (TEXT(rcalitem_periodcount) || :days)"
               "            WHEN (rcalitem_periodtype='B') THEN (TEXT(rcalitem_periodcount) || :businessDays)"
               "            WHEN (rcalitem_periodtype='W') THEN (TEXT(rcalitem_periodcount) || :weeks)"
               "            WHEN (rcalitem_periodtype='M') THEN (TEXT(rcalitem_periodcount) || :months)"
               "            WHEN (rcalitem_periodtype='Q') THEN (TEXT(rcalitem_periodcount) || :quarters)"
               "            WHEN (rcalitem_periodtype='Y') THEN (TEXT(rcalitem_periodcount) || :years)"
               "            ELSE :userDefined"
               "       END AS periodlength,"
               "       CASE WHEN (rcalitem_offsettype='D') THEN (rcalitem_offsetcount)"
               "            WHEN (rcalitem_offsettype='B') THEN (rcalitem_offsetcount)"
               "            WHEN (rcalitem_offsettype='W') THEN (rcalitem_offsetcount * 7)"
               "            WHEN (rcalitem_offsettype='M') THEN (rcalitem_offsetcount * 30)"
               "            WHEN (rcalitem_offsettype='Q') THEN (rcalitem_offsetcount * 120)"
               "            WHEN (rcalitem_offsettype='Y') THEN (rcalitem_offsetcount * 365)"
               "            ELSE (rcalitem_offsetcount)"
               "       END AS offsetdays,"
               "       CASE WHEN (rcalitem_offsettype='D') THEN (TEXT(rcalitem_offsetcount) || :days)"
               "            WHEN (rcalitem_offsettype='B') THEN (TEXT(rcalitem_offsetcount) || :businessDays)"
               "            WHEN (rcalitem_offsettype='W') THEN (TEXT(rcalitem_offsetcount) || :weeks)"
               "            WHEN (rcalitem_offsettype='M') THEN (TEXT(rcalitem_offsetcount) || :months)"
               "            WHEN (rcalitem_offsettype='Q') THEN (TEXT(rcalitem_offsetcount) || :quarters)"
               "            WHEN (rcalitem_offsettype='Y') THEN (TEXT(rcalitem_offsetcount) || :years)"
               "            ELSE :userDefined"
               "       END AS offsetdays_qtdisplayrole "
               "FROM rcalitem "
               "WHERE (rcalitem_calhead_id=:calhead_id) "
               "ORDER BY offsetdays;" );

    calendarFillList.bindValue(":days", tr("Day(s)"));
    calendarFillList.bindValue(":businessDays", tr("Business Day(s)"));
    calendarFillList.bindValue(":weeks", tr("Week(s)"));
    calendarFillList.bindValue(":months", tr("Month(s)"));
    calendarFillList.bindValue(":quarters", tr("Quarter(s)"));
    calendarFillList.bindValue(":years", tr("Year(s)"));
    calendarFillList.bindValue(":userDefined", tr("User-Defined"));
  }
  
  calendarFillList.bindValue(":calhead_id", _calheadid);
  calendarFillList.exec();
  _calitem->populate(calendarFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Calendar Information"),
                                calendarFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void calendar::populate()
{
  XSqlQuery calendarpopulate;
  calendarpopulate.prepare( "SELECT calhead_name, calhead_descrip, calhead_type, calhead_origin "
             "FROM calhead "
             "WHERE (calhead_id=:calhead_id);");
  calendarpopulate.bindValue(":calhead_id", _calheadid);
  calendarpopulate.exec();
  if (calendarpopulate.first())
  {
    _name->setText(calendarpopulate.value("calhead_name"));
    _descrip->setText(calendarpopulate.value("calhead_descrip"));

    _type = calendarpopulate.value("calhead_type").toString()[0].toLatin1();

    if (_type == 'A')
    {
      _absolute->setChecked(true);
    }
    else if (_type == 'R')
    {
      _relative->setChecked(true);

      for (int counter = 0; counter < _origin->count(); counter++)
        if (calendarpopulate.value("calhead_origin").toString() == originTypes[counter])
        {
          _origin->setCurrentIndex(counter);
          break;
        }
    }

    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Calendar Information"),
                                calendarpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
