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
 * Graphic Image as pmakrovided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "characteristicPrice.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

/*
 *  Constructs a characteristicPrice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
characteristicPrice::characteristicPrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCharIdChanged()));
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  connect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));

  _char->setAllowNull(TRUE);
  
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

/*
 *  Destroys the object and frees any allocated resources
 */
characteristicPrice::~characteristicPrice()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void characteristicPrice::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristicPrice::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _char->populate(QString( "SELECT DISTINCT charass_char_id, char_name "
                             "FROM charass, char "
                             "WHERE ((charass_char_id=char_id) "
                             "AND (charass_target_type='I') "
                             "AND (charass_target_id= %1)) "
                             "ORDER BY char_name; ").arg(param.toInt()));
                             
  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  
  param = pParams.value("ipsitem_id", &valid);
  if (valid)
    _ipsitemid = param.toInt();

  param = pParams.value("ipsitemchar_id", &valid);
  if (valid)
    _ipsitemcharid = param.toInt();
  
  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _char->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
      populate();
      populateItemcharInfo();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(FALSE);
      _value->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      populate();

      _close->setFocus();
    }
  }

  return NoError;
}

void characteristicPrice::sSave()
{
  if (_char->id() == -1)
  {
    QMessageBox::information( this, tr("No Characteristic Selected"),
                              tr("You must select a Characteristic before saving this Item Characteristic.") );
    _char->setFocus();
    return;
  }
  if (_mode == cNew)
  {
    q.exec("SELECT nextval('ipsitemchar_ipsitemchar_id_seq') AS result;");
    if (q.first())
      _ipsitemcharid=q.value("result").toInt();
    q.prepare( "INSERT INTO ipsitemchar "
               "( ipsitemchar_id, ipsitemchar_ipsitem_id, ipsitemchar_char_id, ipsitemchar_value, ipsitemchar_price ) "
               "VALUES "
               "( :ipsitemchar_id, :ipsitem_id, :ipsitemchar_char_id, :ipsitemchar_value, :ipsitemchar_price );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE ipsitemchar "
               "SET ipsitemchar_char_id=:ipsitemchar_char_id, ipsitemchar_value=:ipsitemchar_value, ipsitemchar_price=:ipsitemchar_price "
               "WHERE (ipsitemchar_id=:ipsitemchar_id);" );

  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":ipsitemchar_id", _ipsitemcharid);
  q.bindValue(":ipsitemchar_char_id", _char->id());
  q.bindValue(":ipsitemchar_value", _value->currentText());
  q.bindValue(":ipsitemchar_price", _price->localValue());;
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }

  done(_ipsitemcharid);
}

void characteristicPrice::sCheck()
{
  q.prepare( "SELECT ipsitemchar_id "
             "FROM ipsitemchar "
             "WHERE ( (ipsitemchar_char_id=:char_id)"
             " AND (ipsitemchar_value=:ipsitemchar_value) "
             " AND (ipsitemchar_ipsitem_id=:ipsitem_id));" );
  q.bindValue(":ipsitem_id", _ipsitemid);
  q.bindValue(":char_id", _char->id());
  q.bindValue(":ipsitemchar_value", _value->currentText());
  q.exec();
  if (q.first())
  {
    _ipsitemcharid = q.value("ipsitemchar_id").toInt();
    _mode = cEdit;
    populate();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  else
    _mode = cNew;
  populateItemcharInfo();
}

void characteristicPrice::populate()
{
  disconnect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  disconnect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));
  q.prepare( "SELECT ipsitemchar_char_id, ipsitemchar_value, ipsitemchar_price  "
             "FROM ipsitemchar "
             "WHERE (ipsitemchar_id=:ipsitemchar_id);" );
  q.bindValue(":ipsitemchar_id", _ipsitemcharid);
  q.exec();
  if (q.first())
  {
    _char->setId(q.value("ipsitemchar_char_id").toInt());
    _value->setText(q.value("ipsitemchar_value").toString());
    _price->setLocalValue(q.value("ipsitemchar_price").toDouble());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  connect(_char, SIGNAL(newID(int)), this, SLOT(sCheck()));
  connect(_value, SIGNAL(activated(int)), this, SLOT(sCheck()));
}

void characteristicPrice::populateItemcharInfo()
{
  q.prepare("SELECT charass_price "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (charass_value=:value)) ");
  q.bindValue(":item_id", _itemid);
  q.bindValue(":char_id", _char->id());
  q.bindValue(":value", _value->currentText());
  q.exec();
  if (q.first())
    _listPrice->setLocalValue(q.value("charass_price").toDouble());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  else
    _listPrice->setLocalValue(0);
}

void characteristicPrice::sCharIdChanged()
{
  XSqlQuery charass;
  charass.prepare("SELECT charass_id, charass_value "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (COALESCE(charass_value,'')!='')); ");
  charass.bindValue(":item_id", _itemid);
  charass.bindValue(":char_id", _char->id());
  charass.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
    done(-1);
  }
  _value->populate(charass);
}



