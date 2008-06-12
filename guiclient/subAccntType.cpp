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

#include "subAccntType.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a subAccntType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
subAccntType::subAccntType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
subAccntType::~subAccntType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void subAccntType::languageChange()
{
    retranslateUi(this);
}


enum SetResponse subAccntType::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("subaccnttype_id", &valid);
  if (valid)
  {
    _subaccnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _code->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _type->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void subAccntType::sSave()
{
  if (_mode == cEdit)
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE ( (subaccnttype_id<>:subaccnttype_id)"
               " AND (subaccnttype_code=:subaccnttype_code) );");
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists."
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    q.prepare( "UPDATE subaccnttype "
               "SET subaccnttype_code=:subaccnttype_code,"
               "       subaccnttype_descrip=:subaccnttype_descrip,"
               "       subaccnttype_accnt_type=:subaccnttype_accnt_type "
               "WHERE (subaccnttype_id=:subaccnttype_id);" );
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentItem() == 0)
      q.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentItem() == 1)
      q.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentItem() == 2)
      q.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentItem() == 3)
      q.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentItem() == 4)
      q.bindValue(":subaccnttype_accnt_type", "Q");
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);");
    q.bindValue(":subaccnttype_code", _code->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Subaccount Type"),
                             tr( "A Subaccount Type with the entered code already exists.\n"
                                 "You may not create a Subaccount Type with this code." ) );
      _code->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('subaccnttype_subaccnttype_id_seq') AS subaccnttype_id;");
    if (q.first())
      _subaccnttypeid = q.value("subaccnttype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO subaccnttype "
               "( subaccnttype_id, subaccnttype_code,"
               "  subaccnttype_descrip, subaccnttype_accnt_type ) "
               "VALUES "
               "( :subaccnttype_id, :subaccnttype_code, :subaccnttype_descrip, :subaccnttype_accnt_type );" );
    q.bindValue(":subaccnttype_id", _subaccnttypeid);
    q.bindValue(":subaccnttype_code", _code->text());
    q.bindValue(":subaccnttype_descrip", _description->text());
    if (_type->currentItem() == 0)
      q.bindValue(":subaccnttype_accnt_type", "A");
    else if (_type->currentItem() == 1)
      q.bindValue(":subaccnttype_accnt_type", "L");
    else if (_type->currentItem() == 2)
      q.bindValue(":subaccnttype_accnt_type", "E");
    else if (_type->currentItem() == 3)
      q.bindValue(":subaccnttype_accnt_type", "R");
    else if (_type->currentItem() == 4)
      q.bindValue(":subaccnttype_accnt_type", "Q");
    q.exec();
  }

  done(_subaccnttypeid);
}

void subAccntType::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    q.prepare( "SELECT subaccnttype_id "
               "FROM subaccnttype "
               "WHERE (subaccnttype_code=:subaccnttype_code);" );
    q.bindValue(":subaccnttype_code", _code->text());
    q.exec();
    if (q.first())
    {
      _subaccnttypeid = q.value("subaccnttype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void subAccntType::populate()
{
  q.prepare( "SELECT subaccnttype_code, subaccnttype_accnt_type, subaccnttype_descrip "
             "FROM subaccnttype "
             "WHERE (subaccnttype_id=:subaccnttype_id);" );
  q.bindValue(":subaccnttype_id", _subaccnttypeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("subaccnttype_code").toString());
    _description->setText(q.value("subaccnttype_descrip").toString());
    
    if (q.value("subaccnttype_accnt_type").toString() == "A")
      _type->setCurrentItem(0);
    else if (q.value("subaccnttype_accnt_type").toString() == "L")
      _type->setCurrentItem(1);
    else if (q.value("subaccnttype_accnt_type").toString() == "E")
      _type->setCurrentItem(2);
    else if (q.value("subaccnttype_accnt_type").toString() == "R")
      _type->setCurrentItem(3);
    else if (q.value("subaccnttype_accnt_type").toString() == "Q")
      _type->setCurrentItem(4);
  }
}

