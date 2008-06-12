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

#include "incidentCategory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a incidentCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
incidentCategory::incidentCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

incidentCategory::~incidentCategory()
{
}

void incidentCategory::languageChange()
{
    retranslateUi(this);
}

enum SetResponse incidentCategory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("incdtcat_id", &valid);
  if (valid)
  {
    _incdtcatId = param.toInt();
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

void incidentCategory::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT incdtcat_id "
               "FROM incdtcat "
               "WHERE (UPPER(incdtcat_name)=UPPER(:incdtcat_name));" );
    q.bindValue(":incdtcat_name", _name->text());
    q.exec();
    if (q.first())
    {
      _incdtcatId = q.value("incdtcat_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void incidentCategory::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('incdtcat_incdtcat_id_seq') AS _incdtcat_id");
    if (q.first())
      _incdtcatId = q.value("_incdtcat_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO incdtcat "
               "(incdtcat_id, incdtcat_name, incdtcat_order, incdtcat_descrip)"
               " VALUES "
               "(:incdtcat_id, :incdtcat_name, :incdtcat_order, :incdtcat_descrip);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT incdtcat_id "
               "FROM incdtcat "
               "WHERE ( (UPPER(incdtcat_name)=UPPER(:incdtcat_name))"
               " AND (incdtcat_id<>:incdtcat_id) );" );
    q.bindValue(":incdtcat_id", _incdtcatId);
    q.bindValue(":incdtcat_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Incident Category"),
                            tr("You may not rename this Incident Category with "
			       "the entered value as it is in use by another "
			       "Incident Category.") );
      return;
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "UPDATE incdtcat "
               "SET incdtcat_name=:incdtcat_name, "
	       "    incdtcat_order=:incdtcat_order, "
	       "    incdtcat_descrip=:incdtcat_descrip "
               "WHERE (incdtcat_id=:incdtcat_id);" );
  }

  q.bindValue(":incdtcat_id", _incdtcatId);
  q.bindValue(":incdtcat_name", _name->text());
  if (! _order->text().isEmpty())
    q.bindValue(":incdtcat_order", _order->text());
  q.bindValue(":incdtcat_descrip", _descrip->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_incdtcatId);
}

void incidentCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM incdtcat "
             "WHERE (incdtcat_id=:incdtcat_id);" );
  q.bindValue(":incdtcat_id", _incdtcatId);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("incdtcat_name").toString());
    if (! q.value("incdtcat_order").isNull())
      _order->setText(q.value("incdtcat_order").toString());
    _descrip->setText(q.value("incdtcat_descrip").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
