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

#include "characteristic.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

characteristic::characteristic(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

characteristic::~characteristic()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristic::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristic::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("char_id", &valid);
  if (valid)
  {
    _charid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
      _mode = cEdit;
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _useGroup->setEnabled(FALSE);

      _items->setEnabled(FALSE);
      _customers->setEnabled(FALSE);
      _lotSerial->setEnabled(FALSE);
      _addresses->setEnabled(FALSE);
      _crmaccounts->setEnabled(FALSE);
      _contacts->setEnabled(FALSE);
      _opportunity->setEnabled(FALSE);
      _employees->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void characteristic::sSave()
{
  if (_name->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Characteristic before "
			     "saving it."));
    _name->setFocus();
    return;
  }
  if (! (_items->isChecked()       || _customers->isChecked() ||
	 _lotSerial->isChecked()   || _addresses->isChecked() ||
	 _crmaccounts->isChecked() || _contacts->isChecked()  ||
	 _opportunity->isChecked() || _employees->isChecked() ))
  {
    QMessageBox::critical(this, tr("Apply Characteristic"),
			  tr("<p>You must apply this Characteristic to at "
			     "least one type of application object."));
    _items->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('char_char_id_seq') AS char_id;");
    if (q.first())
      _charid = q.value("char_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO char "
               "( char_id, char_name, char_items, char_customers, "
	       "  char_contacts, char_crmaccounts, char_addresses, "
	       "  char_options, char_opportunity,"
               "  char_attributes, char_lotserial, char_employees,"
               "  char_notes ) "
               "VALUES "
               "( :char_id, :char_name, :char_items, :char_customers, "
	       "  :char_contacts, :char_crmaccounts, :char_addresses, "
	       "  :char_options, :char_opportunity,"
               "  :char_attributes, :char_lotserial, :char_employees,"
               "  :char_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE char "
               "SET char_name=:char_name, char_items=:char_items, "
	       "    char_customers=:char_customers, "
	       "    char_contacts=:char_contacts, "
	       "    char_crmaccounts=:char_crmaccounts, "
	       "    char_addresses=:char_addresses, "
	       "    char_options=:char_options,"
               "    char_attributes=:char_attributes, "
               "    char_opportunity=:char_opportunity,"
	       "    char_lotserial=:char_lotserial,"
               "    char_employees=:char_employees,"
               "    char_notes=:char_notes "
               "WHERE (char_id=:char_id);" );

  q.bindValue(":char_id", _charid);
  q.bindValue(":char_name", _name->text());
  q.bindValue(":char_items",            QVariant(_items->isChecked(), 0));
  q.bindValue(":char_customers",	QVariant(_customers->isChecked(), 0));
  q.bindValue(":char_crmaccounts",	QVariant(_crmaccounts->isChecked(), 0));
  q.bindValue(":char_contacts",		QVariant(_contacts->isChecked(), 0));
  q.bindValue(":char_addresses",	QVariant(_addresses->isChecked(), 0));
  q.bindValue(":char_options",          QVariant(FALSE, 0));
  q.bindValue(":char_attributes",       QVariant(FALSE, 0));
  q.bindValue(":char_lotserial",        QVariant(_lotSerial->isChecked(), 0));
  q.bindValue(":char_opportunity",      QVariant(_opportunity->isChecked(), 0));
  q.bindValue(":char_employees",        QVariant(_employees->isChecked(), 0));
  q.bindValue(":char_notes", _description->text().stripWhiteSpace());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_charid);
}

void characteristic::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().stripWhiteSpace().length()))
  {
    q.prepare( "SELECT char_id "
               "FROM char "
               "WHERE (UPPER(char_name)=UPPER(:char_name));" );
    q.bindValue(":char_name", _name->text());
    q.exec();
    if (q.first())
    {
      _charid = q.value("char_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void characteristic::populate()
{
  q.prepare( "SELECT * "
             "FROM char "
             "WHERE (char_id=:char_id);" );
  q.bindValue(":char_id", _charid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("char_name").toString());
    _items->setChecked(q.value("char_items").toBool());
    _customers->setChecked(q.value("char_customers").toBool());
    _contacts->setChecked(q.value("char_contacts").toBool());
    _crmaccounts->setChecked(q.value("char_crmaccounts").toBool());
    _addresses->setChecked(q.value("char_addresses").toBool());
    _lotSerial->setChecked(q.value("char_lotserial").toBool());
    _opportunity->setChecked(q.value("char_opportunity").toBool());
    _employees->setChecked(q.value("char_employees").toBool());
    _description->setText(q.value("char_notes").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

