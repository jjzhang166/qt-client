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

#include "itemAttribute.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a itemAttribute as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemAttribute::itemAttribute(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemAttribute::~itemAttribute()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemAttribute::languageChange()
{
    retranslateUi(this);
}


void itemAttribute::init()
{
  _costModifier->setValidator(omfgThis->negMoneyVal());
  _priceModifier->setValidator(omfgThis->negMoneyVal());

  _char->setAllowNull(TRUE);
  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_attributes) "
                   "ORDER BY char_name; ");
}

enum SetResponse itemAttribute::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemopn_id", &valid);
  if (valid)
  {
    _itemopnid = param.toInt();
    
    q.prepare( "SELECT char_name "
               "FROM itemopn, char "
               "WHERE ( (itemopn_char_id=char_id)"
               " AND (itemopn_id=:itemopn_id) )" );
    q.bindValue(":itemopn_id", _itemopnid);
    q.exec();
    if (q.first())
      _option->setText(q.value("char_name").toString());
    else
      systemError( this, tr("A System Error occurred at itemAttribute::%1.")
                         .arg(__LINE__) );
  }

  param = pParams.value("itematr_id", &valid);
  if (valid)
  {
    _itematrid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('itematr_itematr_id_seq') AS _itematr_id");
      if (q.first())
        _itematrid = q.value("_itematr_id").toInt();
      else
        systemError( this, tr("A System Error occurred at itemAttribute::%1.")
                           .arg(__LINE__) );

      _char->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemAttribute::sSave()
{
  if (!_char->isValid())
  {
    QMessageBox::critical( this, tr("Enter Attribute Name"),
                           tr("You must enter a name for this Attribute before saving it.") );
    _char->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO itematr "
               "( itematr_id, itematr_itemopn_id, itematr_char_id, itematr_default,"
               "  itematr_suffixval, itematr_costmod, itematr_pricemod ) "
               "VALUES "
               "( :itematr_id, :itematr_itemopn_id, :itematr_char_id, :itematr_default,"
               "  :itematr_suffixval, :itematr_costmod, :itematr_pricemod );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE itematr "
               "SET itematr_char_id=:itematr_char_id, itematr_default=:itematr_default,"
               "    itematr_suffixval=:itematr_suffixval,"
               "    itematr_costmod=:itematr_costmod, itematr_pricemod=:itematr_pricemod "
               "WHERE (itematr_id=:itematr_id);" );

  q.bindValue(":itematr_id", _itematrid);
  q.bindValue(":itematr_itemopn_id", _itemopnid);
  q.bindValue(":itematr_char_id", _char->id());
  q.bindValue(":itematr_default", QVariant(_default->isChecked(),0));
  q.bindValue(":itematr_suffixval", _suffixValue->text());
  q.bindValue(":itematr_costmod", _costModifier->toDouble());
  q.bindValue(":itematr_pricemod", _priceModifier->toDouble());
  q.exec();
  
  if (_default->isChecked())
  {
    q.prepare( "UPDATE itematr "
               "SET itematr_default=FALSE "
               "WHERE ( (itematr_itemopn_id=:itemopn_id) "
               " AND (itematr_id<>:itematr_id) );" );
    q.bindValue(":itematr_id", _itematrid);
    q.bindValue(":itemopn_id", _itemopnid);
    q.exec();
  }

  done(_itematrid);
}

void itemAttribute::populate()
{
  q.prepare( "SELECT char_name, itematr_char_id, itematr_default, itematr_suffixval,"
             "       formatMoney(itematr_costmod) AS f_costmod,"
             "       formatMoney(itematr_pricemod) AS f_pricemod "
             "FROM itematr, itemopn, char "
             "WHERE ( (itematr_itemopn_id=itemopn_id)"
             " AND (itemopn_char_id=char_id)"
             " AND (itematr_id=:itematr_id) );" );
  q.bindValue(":itematr_id", _itematrid);
  q.exec();
  if (q.first())
  {
    _option->setText(q.value("char_name").toString());
    _char->setId(q.value("itematr_char_id").toInt());
    _default->setChecked(q.value("itematr_default").toBool());
    _suffixValue->setText(q.value("itematr_suffixval").toString());
    _costModifier->setText(q.value("f_costmod").toString());
    _priceModifier->setText(q.value("f_pricemod").toString());
  }
}

