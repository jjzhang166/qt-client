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

#include "form.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

form::form(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

form::~form()
{
  // no need to delete child widgets, Qt does it all for us
}

void form::languageChange()
{
  retranslateUi(this);
}

enum SetResponse form::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("form_id", &valid);
  if (valid)
  {
    _formid = param.toInt();
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
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _report->setEnabled(FALSE);
      _key->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void form::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Form Name is Invalid"),
                          tr("<p>You must enter a valid name for this Form.") );
    _name->setFocus();
    return;
  }

  else if (_report->id() == -1)
  {
    QMessageBox::warning( this, tr("Report is Invalid"),
                          tr("<p>You must select a report for this Form.") );
    _report->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('form_form_id_seq') AS _form_id");
    if (q.first())
      _formid = q.value("_form_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO form "
               "(form_id, form_name, form_descrip, form_report_id, form_key) "
               "VALUES "
               "(:form_id, :form_name, :form_descrip, :form_report_id, :form_key);" );

  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE form "
               "SET form_name=:form_name, form_descrip=:form_descrip,"
               "    form_report_id=:form_report_id, form_key=:form_key "
               "WHERE (form_id=:form_id);" );

  q.bindValue(":form_id", _formid);
  q.bindValue(":form_name", _name->text());
  q.bindValue(":form_descrip", _descrip->text());
  q.bindValue(":form_report_id", _report->id());

  if (_key->currentItem() == 0)
    q.bindValue(":form_key", "Cust");
  else if (_key->currentItem() == 1)
    q.bindValue(":form_key", "Item");
  else if (_key->currentItem() == 2)
    q.bindValue(":form_key", "ItSt");
  else if (_key->currentItem() == 3)
    q.bindValue(":form_key", "PO");
  else if (_key->currentItem() == 4)
    q.bindValue(":form_key", "SO");
  else if (_key->currentItem() == 5)
    q.bindValue(":form_key", "Vend");
  else if (_key->currentItem() == 6)
    q.bindValue(":form_key", "WO");
  else if (_key->currentItem() == 7)
    q.bindValue(":form_key", "SASC");
  else if (_key->currentItem() == 8)
    q.bindValue(":form_key", "PES");
  else if (_key->currentItem() == 9)
    q.bindValue(":form_key", "RA");
  else
    q.bindValue(":form_key", "");

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_formid);
}

void form::populate()
{
  q.prepare( "SELECT * "
      	     "FROM form "
             "WHERE (form_id=:form_id);" );
  q.bindValue(":form_id", _formid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("form_name").toString());
    _descrip->setText(q.value("form_descrip").toString());
    _report->setId(q.value("form_report_id").toInt());
  
    if (q.value("form_key").toString() == "Cust")
      _key->setCurrentItem(0);
    else if (q.value("form_key").toString() == "Item")
      _key->setCurrentItem(1);
    else if (q.value("form_key").toString() == "ItSt")
      _key->setCurrentItem(2);
    else if (q.value("form_key").toString() == "PO")
      _key->setCurrentItem(3);
    else if (q.value("form_key").toString() == "SO")
      _key->setCurrentItem(4);
    else if (q.value("form_key").toString() == "Vend")
      _key->setCurrentItem(5);
    else if (q.value("form_key").toString() == "WO")
      _key->setCurrentItem(6);
    else if (q.value("form_key").toString() == "SASC")
      _key->setCurrentItem(7);
    else if (q.value("form_key").toString() == "PES")
      _key->setCurrentItem(8);
    else if (q.value("form_key").toString() == "RA")
      _key->setCurrentItem(9);
    else
      _key->setCurrentItem(-1);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
