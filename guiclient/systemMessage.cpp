/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "systemMessage.h"

#include <QVariant>
#include <QMessageBox>

#define cAcknowledge 0x80


/* systemMessage::open is part of a window-handling experiment.
 * set() returns Error_AlreadyOpen if there is already a window open for the given id.
 * windowForId(id) returns the open window for the given id or 0 if no such window exists.
 * TODO: generalize for all windows and have QHashMap<classname, QHashMap<id, window>>?
 * This might be a better answer than the salesOrder:: funkiness
 */
QHash<int, systemMessage*> systemMessage::open;

systemMessage::systemMessage(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,  SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _scheduledDate->setNullString(tr("ASAP"));
  _scheduledDate->setNullDate(omfgThis->startOfTime());
  _scheduledDate->setAllowNullDate(true);

  _expiresDate->setNullString(tr("Never"));
  _expiresDate->setNullDate(omfgThis->endOfTime());
  _expiresDate->setAllowNullDate(true);
  _expiresDate->setNull();
}

systemMessage::~systemMessage()
{
  // no need to delete child widgets, Qt does it all for us
  if (_msguserid > 0)
    open.remove(_msguserid);
}

void systemMessage::languageChange()
{
  retranslateUi(this);
}

systemMessage *systemMessage::windowForId(int id)
{
  return systemMessage::open.value(id, 0);
}

enum SetResponse systemMessage::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("msguser_id", &valid);
  if (valid)
  {
    _msguserid = param.toInt();
    if (systemMessage::open.value(_msguserid, 0))
    {
      return Error_AlreadyOpen;
    }
    populate();
    systemMessage::open.insert(_msguserid, this);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "acknowledge")
    {
      _mode = cAcknowledge;

      _scheduledDate->setEnabled(false);
      _scheduledTime->setEnabled(false);
      _expiresDate->setEnabled(false);
      _expiresTime->setEnabled(false);
      _message->setReadOnly(true);
      _close->setText("&Acknowlege");
      _save->setText("&Snooze");
    }
  }

  return NoError;
}

void systemMessage::sClose()
{
  XSqlQuery systemClose;
  if (_mode == cNew)
    reject();

  else if (_mode == cAcknowledge)
  {
    systemClose.prepare( "SELECT acknowledgeMessage(msguser_msg_id) "
               "FROM msguser "
               "WHERE (msguser_id=:msguser_id);" );
    systemClose.bindValue(":msguser_id", _msguserid);
    systemClose.exec();

    accept();
  }
}

void systemMessage::sSave()
{
  XSqlQuery systemSave;
  if (_mode == cNew)
  {
    if (!_scheduledDate->isValid())
    {
      QMessageBox::warning( this, tr("Enter Valid Schedule Date"),
                            tr("You must enter a valid Schedule Date before saving this Message.") );
      _scheduledDate->setFocus();
    }
    else
    {
      systemSave.prepare("SELECT postMessage(:scheduled::timestamp without time zone, :expires::timestamp without time zone, :message) AS msgid;");

      QDateTime scheduled;
      QDateTime expires;
      scheduled.setDate(_scheduledDate->date());
      scheduled.setTime(_scheduledTime->time());
      expires.setDate(_expiresDate->date());
      expires.setTime(_expiresTime->time());

      systemSave.bindValue(":scheduled", scheduled);
      systemSave.bindValue(":expires", expires);
      systemSave.bindValue(":message", (_message->toPlainText()));
      systemSave.exec();
      if (systemSave.first())
        done(systemSave.value("msgid").toInt());
      else
        reject();
    }
  }
  else if (_mode == cAcknowledge)
  {
    systemSave.prepare( "SELECT snoozeMessage(msguser_msg_id) "
               "FROM msguser "
               "WHERE (msguser_id=:msguser_id);" );
    systemSave.bindValue(":msguser_id", _msguserid);
    systemSave.exec();

    accept();
  }
}

void systemMessage::populate()
{
  XSqlQuery systempopulate;
  QString sql = QString( "SELECT DATE(msg_posted) AS postdate,"
                         "       msg_posted::TIME AS posttime,"
                         "       DATE(msg_scheduled) AS scheduleddate,"
                         "       msg_scheduled::TIME AS scheduledtime,"
                         "       DATE(msg_expires) AS expiresdate,"
                         "       msg_expires::TIME AS expirestime,"
                         "       msg_username, msg_text "
                         "FROM msg, msguser "
                         "WHERE ( (msguser_msg_id=msg_id)"
                         " AND (msguser_id=%1) );" )
                .arg(_msguserid);
  systempopulate.exec(sql);
  if (systempopulate.first())
  {
    _postedDate->setDate(systempopulate.value("postdate").toDate());
    _postedTime->setTime(systempopulate.value("posttime").toTime());
    _scheduledDate->setDate(systempopulate.value("scheduleddate").toDate());
    _scheduledTime->setTime(systempopulate.value("scheduledtime").toTime());
    _expiresDate->setDate(systempopulate.value("expiresdate").toDate());

    if (systempopulate.value("expiresdate") != tr("Never"))
      _expiresTime->setTime(systempopulate.value("expirestime").toTime());

    _user->setText(systempopulate.value("msg_username").toString());
    _message->setText(systempopulate.value("msg_text").toString());
  }
}

