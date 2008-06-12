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

#include "itemSubstitute.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a itemSubstitute as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemSubstitute::itemSubstitute(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_substitute, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSubstitute::~itemSubstitute()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSubstitute::languageChange()
{
    retranslateUi(this);
}


#define cItemSub    0x01
#define cBOMItemSub 0x02

void itemSubstitute::init()
{
  _uomRatio->setText("1.0");
}

enum SetResponse itemSubstitute::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _bomitemid = param.toInt();
    q.prepare( "SELECT bomitem_item_id "
               "FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    q.bindValue(":bomitem_id", _bomitemid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("bomitem_item_id").toInt());
      _item->setReadOnly(TRUE);
    }
  }

  param = pParams.value("bomitem_item_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("itemsub_id", &valid);
  if (valid)
  {
    _type = cItemSub;
    _itemsubid = param.toInt();
    populate();
  }

  param = pParams.value("bomitemsub_id", &valid);
  if (valid)
  {
    _type = cBOMItemSub;
    _itemsubid = param.toInt();

    _item->setEnabled(FALSE);

    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _substitute->setReadOnly(TRUE);
      _uomRatio->setEnabled(FALSE);
      _ranking->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemSubstitute::sSave()
{
  if (_type == cItemSub)
  {
    if (_item->id() == _substitute->id())
    {
      QMessageBox::critical( this, tr("Cannot Create Substitute"),
                             tr("You cannot define an Item to be a substitute for itself.") );
      _substitute->setFocus();
      return;
    }

    q.prepare( "SELECT itemsub_id "
               "  FROM itemsub "
               " WHERE((itemsub_parent_item_id=:parentItem_id)"
               "   AND (itemsub_sub_item_id=:item_id)"
               "   AND (itemsub_id != :itemsub_id) );" );
    q.bindValue(":parentItem_id", _item->id());
    q.bindValue(":item_id", _substitute->id());
    q.bindValue(":itemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                             tr( "This substitution has already been defined for the selected Item.\n"
                                 "You may edit the existing Substitution but you may not create a duplicate." ) );
      _substitute->setFocus();
      return;
    }

    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('itemsub_itemsub_id_seq') AS _itemsub_id");
      if (q.first())
        _itemsubid = q.value("_itemsub_id").toInt();
//  ToDo

      q.prepare( "INSERT INTO itemsub "
                 "( itemsub_id, itemsub_parent_item_id, itemsub_sub_item_id,"
                 "  itemsub_uomratio, itemsub_rank ) "
                 "VALUES "
                 "( :itemsub_id, :itemsub_parent_item_id, :itemsub_sub_item_id,"
                 "  :itemsub_uomratio, :itemsub_rank );" );
    }
    else if (_mode == cEdit)
      q.prepare( "UPDATE itemsub "
                 "   SET itemsub_uomratio=:itemsub_uomratio,"
                 "       itemsub_rank=:itemsub_rank,"
                 "       itemsub_sub_item_id=:itemsub_sub_item_id"
                 " WHERE(itemsub_id=:itemsub_id);" );

    q.bindValue(":itemsub_id", _itemsubid);
    q.bindValue(":itemsub_parent_item_id", _item->id());
    q.bindValue(":itemsub_sub_item_id", _substitute->id());
    q.bindValue(":itemsub_uomratio", _uomRatio->toDouble());
    q.bindValue(":itemsub_rank", _ranking->value());
    q.exec();
  }
  else if (_type == cBOMItemSub)
  {
    if (_mode == cNew)
    {
      q.prepare( "SELECT bomitemsub_id "
                 "FROM bomitemsub "
                 "WHERE ( (bomitemsub_bomitem_id=:bomitem_id)"
                 " AND (bomitemsub_item_id=:item_id) );" );
      q.bindValue(":bomitem_id", _bomitemid);
      q.bindValue(":item_id", _substitute->id());
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Create Duplicate Substitute"),
                               tr( "This substitution has already been defined for the selected BOM Item.\n"
                                   "You may edit the existing Substitution but you may not create a duplicate." ) );
        _substitute->setFocus();
        return;
      }
    }

    if (_mode == cNew)
    {
      q.exec("SELECT NEXTVAL('bomitemsub_bomitemsub_id_seq') AS bomitemsub_id");
      if (q.first())
        _itemsubid = q.value("bomitemsub_id").toInt();
//  ToDo

      q.prepare( "INSERT INTO bomitemsub "
                 "( bomitemsub_id, bomitemsub_bomitem_id, bomitemsub_item_id,"
                 "  bomitemsub_uomratio, bomitemsub_rank ) "
                 "VALUES "
                 "( :bomitemsub_id, :bomitemsub_bomitem_id, :bomitemsub_item_id,"
                 "  :bomitemsub_uomratio, :bomitemsub_rank );" );
    }
    else if (_mode == cEdit)
      q.prepare( "UPDATE bomitemsub "
                 "SET bomitemsub_uomratio=:bomitemsub_uomratio, bomitemsub_rank=:bomitemsub_rank "
                 "WHERE (bomitemsub_id=:bomitemsub_id);" );

    q.bindValue(":bomitemsub_id", _itemsubid);
    q.bindValue(":bomitemsub_bomitem_id", _bomitemid);
    q.bindValue(":bomitemsub_item_id", _substitute->id());
    q.bindValue(":bomitemsub_uomratio", _uomRatio->toDouble());
    q.bindValue(":bomitemsub_rank", _ranking->value());
    q.exec();
  }

  done(_itemsubid);
}

void itemSubstitute::populate()
{
  if (_type == cItemSub)
  {
    q.prepare( "SELECT itemsub_parent_item_id, itemsub_sub_item_id,"
               "       formatUOMRatio(itemsub_uomratio) AS uomratio, itemsub_rank "
               "FROM itemsub "
               "WHERE (itemsub_id=:itemsub_id);" );
    q.bindValue(":itemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("itemsub_parent_item_id").toInt());
      _substitute->setId(q.value("itemsub_sub_item_id").toInt());
      _ranking->setValue(q.value("itemsub_rank").toInt());
      _uomRatio->setText(q.value("uomratio").toString());
    }
  }
  else if (_type == cBOMItemSub)
  {
    q.prepare( "SELECT bomitemsub_bomitem_id, item_id,  bomitemsub_item_id,"
               "       formatUOMRatio(bomitemsub_uomratio) AS uomratio, bomitemsub_rank "
               "FROM bomitemsub, bomitem, item "
               "WHERE ( (bomitemsub_bomitem_id=bomitem_id)"
               " AND (bomitem_item_id=item_id)"
               " AND (bomitemsub_id=:bomitemsub_id) );" );
    q.bindValue(":bomitemsub_id", _itemsubid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("item_id").toInt());
      _substitute->setId(q.value("bomitemsub_item_id").toInt());
      _ranking->setValue(q.value("bomitemsub_rank").toInt());
      _uomRatio->setText(q.value("uomratio").toString());
    }
  }
}

