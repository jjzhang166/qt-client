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

#include "incidentPriority.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentPriority as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentPriority::incidentPriority(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

incidentPriority::~incidentPriority()
{
}

void incidentPriority::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentPriority::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("incdtpriority_id", &valid);
  if (valid)
  {
    _incdtpriorityId = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _order->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  return NoError;
}

void incidentPriority::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name));" );
    q.bindValue(":incdtpriority_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtpriorityId = q.value("incdtpriority_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentPriority::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtpriority_incdtpriority_id_seq') AS _incdtpriority_id");
    if (q.first())
      _incdtpriorityId = q.value("_incdtpriority_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtpriority "
               "(incdtpriority_id, incdtpriority_name, incdtpriority_order, incdtpriority_descrip)"
               " VALUES "
               "(:incdtpriority_id, :incdtpriority_name, :incdtpriority_order, :incdtpriority_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtpriority_id "
               "FROM incdtpriority "
               "WHERE ( (UPPER(incdtpriority_name)=UPPER(:incdtpriority_name))"
               " AND (incdtpriority_id<>:incdtpriority_id) );" );
    q.bindValue(":incdtpriority_id", _incdtpriorityId);
    q.bindValue(":incdtpriority_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Priority"),
                            tr("You may not rename this Incident Priority with "
			       "the entered value as it is in use by another "
			       "Incident Priority.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtpriority "
               "SET incdtpriority_name=:incdtpriority_name, "
	       "    incdtpriority_order=:incdtpriority_order, "
	       "    incdtpriority_descrip=:incdtpriority_descrip "
               "WHERE (incdtpriority_id=:incdtpriority_id);" );
  }

  q.bindValue(":incdtpriority_id", _incdtpriorityId);
  q.bindValue(":incdtpriority_name", _name->text());
  if (! _order->text().isEmpty())
    q.bindValue(":incdtpriority_order", _order->text());
  q.bindValue(":incdtpriority_descrip", _descrip->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtpriorityId);
}

void incidentPriority::populate()
{
  q.prepare( "SELECT * "
             "FROM incdtpriority "
             "WHERE (incdtpriority_id=:incdtpriority_id);" );
  q.bindValue(":incdtpriority_id", _incdtpriorityId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtpriority_name").toString());
    if (! q.value("incdtpriority_order").isNull())
      _order->setText(q.value("incdtpriority_order").toString());
    _descrip->setText(q.value("incdtpriority_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
