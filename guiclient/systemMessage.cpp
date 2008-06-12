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

#include "systemMessage.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a systemMessage as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
systemMessage::systemMessage(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
systemMessage::~systemMessage()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void systemMessage::languageChange()
{
    retranslateUi(this);
}


#define cAcknowledge 0x80

void systemMessage::init()
{
  _scheduledDate->setNullString(tr("ASAP"));
  _scheduledDate->setNullDate(omfgThis->startOfTime());
  _scheduledDate->setAllowNullDate(TRUE);

  _expiresDate->setNullString(tr("Never"));
  _expiresDate->setNullDate(omfgThis->endOfTime());
  _expiresDate->setAllowNullDate(TRUE);
  _expiresDate->setNull();
}

enum SetResponse systemMessage::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("msguser_id", &valid);
  if (valid)
  {
    _msguserid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _scheduledDate->setFocus();
    }
    else if (param.toString() == "acknowledge")
    {
      _mode = cAcknowledge;

      _scheduledDate->setEnabled(FALSE);
      _scheduledTime->setEnabled(FALSE);
      _expiresDate->setEnabled(FALSE);
      _expiresTime->setEnabled(FALSE);
      _message->setReadOnly(TRUE);
      _close->setText("&Acknowlege");
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void systemMessage::sClose()
{
  if (_mode == cNew)
    reject();

  else if (_mode == cAcknowledge)
  {
    q.prepare( "SELECT acknowledgeMessage(msguser_msg_id) "
               "FROM msguser "
               "WHERE (msguser_id=:msguser_id);" );
    q.bindValue(":msguser_id", _msguserid);
    q.exec();

    accept();
  }
}

void systemMessage::sSave()
{
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
      q.prepare("SELECT postMessage(:scheduled, :expires, :message) AS msgid;");

      QDateTime scheduled;
      QDateTime expires;
      scheduled.setDate(_scheduledDate->date());
      scheduled.setTime(_scheduledTime->time());
      expires.setDate(_expiresDate->date());
      expires.setTime(_expiresTime->time());

      q.bindValue(":scheduled", scheduled);
      q.bindValue(":expires", expires);
      q.bindValue(":message", (_message->text()));
      q.exec();
      if (q.first())
        done(q.value("msgid").toInt());
      else
        reject();
    }
  }
}

void systemMessage::populate()
{
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
  q.exec(sql);
  if (q.first())
  {
    _postedDate->setDate(q.value("postdate").toDate());
    _postedTime->setTime(q.value("posttime").toTime());
    _scheduledDate->setDate(q.value("scheduleddate").toDate());
    _scheduledTime->setTime(q.value("scheduledtime").toTime());
    _expiresDate->setDate(q.value("expiresdate").toDate());

    if (q.value("expiresdate") != tr("Never"))
      _expiresTime->setTime(q.value("expirestime").toTime());

    _user->setText(q.value("msg_username").toString());
    _message->setText(q.value("msg_text").toString());
  }
}

