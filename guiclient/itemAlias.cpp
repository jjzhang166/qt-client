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

#include "itemAlias.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a itemAlias as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemAlias::itemAlias(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _itemaliasid = -1;
  _itemid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemAlias::~itemAlias()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemAlias::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemAlias::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _itemid = param.toInt();
    q.prepare("SELECT item_number FROM item WHERE (item_id=:item_id);");
    q.bindValue(":item_id", _itemid);
    q.exec();
    if(q.first())
      _item->setText(q.value("item_number").toString());
  }

  param = pParams.value("itemalias_id", &valid);
  if (valid)
  {
    _itemaliasid = param.toInt();
    populate();
  }

  param = pParams.value("item_number", &valid);
  if (valid)
    _item->setText(param.toString());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _useDescription->setEnabled(FALSE);
      _descrip1->setEnabled(FALSE);
      _descrip2->setEnabled(FALSE);
      _comments->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemAlias::sSave()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT itemalias_id "
               "FROM itemalias "
               "WHERE ( (itemalias_item_id=:item_id)"
               " AND (itemalias_number=:itemalias_number) );" );
    q.bindValue(":item_id", _itemid);
    q.bindValue(":itemalias_number", _number->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Item Alias"),
                             tr( "An Item Alias for the selected Item Number has already been defined with the selected Alias Item Number.\n"
                                 "You may not create duplicate Item Aliases." ) );
      _number->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('itemalias_itemalias_id_seq') AS _itemalias_id;");
    if (q.first())
      _itemaliasid = q.value("_itemalias_id").toInt();

    q.prepare( "INSERT INTO itemalias "
               "( itemalias_id, itemalias_item_id, itemalias_number,"
               "  itemalias_usedescrip, itemalias_descrip1, itemalias_descrip2,"
               "  itemalias_comments ) "
               "VALUES "
               "( :itemalias_id, :itemalias_item_id, :itemalias_number,"
               "  :itemalias_usedescrip, :itemalias_descrip1, :itemalias_descrip2,"
               "  :itemalias_comments );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE itemalias "
               "SET itemalias_number=:itemalias_number, itemalias_comments=:itemalias_comments,"
               "    itemalias_usedescrip=:itemalias_usedescrip,"
               "    itemalias_descrip1=:itemalias_descrip1, itemalias_descrip2=:itemalias_descrip2 "
               "WHERE (itemalias_id=:itemalias_id);" );

  q.bindValue(":itemalias_id", _itemaliasid);
  q.bindValue(":itemalias_item_id", _itemid);
  q.bindValue(":itemalias_number", _number->text().stripWhiteSpace());
  q.bindValue(":itemalias_descrip1", _descrip1->text().stripWhiteSpace());
  q.bindValue(":itemalias_descrip2", _descrip2->text().stripWhiteSpace());
  q.bindValue(":itemalias_comments", _comments->text());
  q.bindValue(":itemalias_usedescrip", QVariant(_useDescription->isChecked(), 0));
  q.exec();

  done(_itemaliasid);
}

void itemAlias::populate()
{
  q.prepare( "SELECT itemalias_item_id, itemalias_number,"
             "       itemalias_usedescrip, itemalias_descrip1, itemalias_descrip2,"
             "       itemalias_comments, item_number "
             "FROM itemalias LEFT OUTER JOIN item ON (itemalias_item_id=item_id) "
             "WHERE (itemalias_id=:itemalias_id);" );
  q.bindValue(":itemalias_id", _itemaliasid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("itemalias_item_id").toInt();
    _item->setText(q.value("item_number").toString());
    _number->setText(q.value("itemalias_number").toString());

    if (q.value("itemalias_usedescrip").toBool())
    {
      _useDescription->setChecked(TRUE);
      _descriptionGroup->setEnabled(TRUE);
      _descrip1->setText(q.value("itemalias_descrip1").toString());
      _descrip2->setText(q.value("itemalias_descrip2").toString());
    }
    else
    {
      _useDescription->setChecked(FALSE);
      _descriptionGroup->setEnabled(FALSE);
    }

    _comments->setText(q.value("itemalias_comments").toString());
  }
}

