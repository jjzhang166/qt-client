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

#include "itemGroup.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <Q3DragObject>
#include "itemList.h"

/*
 *  Constructs a itemGroup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemGroup::itemGroup(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemGroup::~itemGroup()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemGroup::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDropEvent>

void itemGroup::init()
{
  statusBar()->hide();
  setAcceptDrops(TRUE);

  _itemgrpitem->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft );
  _itemgrpitem->addColumn(tr("Description"), -1,           Qt::AlignLeft );
}

enum SetResponse itemGroup::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _itemgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('itemgrp_itemgrp_id_seq') AS itemgrp_id;");
      if (q.first())
        _itemgrpid = q.value("itemgrp_id").toInt();
//  ToDo

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_itemgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void itemGroup::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT itemgrp_id "
               "FROM itemgrp "
               "WHERE (UPPER(itemgrp_name)=UPPER(:itemgrp_name));" );
    q.bindValue(":itemgrp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _itemgrpid = q.value("itemgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void itemGroup::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM itemgrpitem "
               "WHERE (itemgrpitem_itemgrp_id=:itemgrp_id);"

                "DELETE FROM itemgrp "
                "WHERE (itemgrp_id=:itemgrp_id);" );
    q.bindValue(":itemgrp_id", _itemgrpid);
    q.exec();
  }

  close();
}

void itemGroup::sSave()
{
  if (_name->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Item Group"),
                          tr("You must enter a Name for this Item Group before you may save it.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO itemgrp "
               "(itemgrp_id, itemgrp_name, itemgrp_descrip) "
               "VALUES "
               "(:itemgrp_id, :itemgrp_name, :itemgrp_descrip);" );
  else
    q.prepare( "UPDATE itemgrp "
               "SET itemgrp_name=:itemgrp_name, itemgrp_descrip=:itemgrp_descrip "
               "WHERE (itemgrp_id=:itemgrp_id);" );

  q.bindValue(":itemgrp_id", _itemgrpid);
  q.bindValue(":itemgrp_name", _name->text());
  q.bindValue(":itemgrp_descrip", _descrip->text());
  q.exec();

  omfgThis->sItemGroupsUpdated(_itemgrpid, TRUE);

  close();
}

void itemGroup::sDelete()
{
  q.prepare( "DELETE FROM itemgrpitem "
             "WHERE (itemgrpitem_id=:itemgrpitem_id);" );
  q.bindValue(":itemgrpitem_id", _itemgrpitem->id());
  q.exec();

  sFillList();
}

void itemGroup::sNew()
{
  ParameterList params;
  itemList newdlg(this, "", TRUE);
  newdlg.set(params);

  int itemid = newdlg.exec();
  if (itemid != QDialog::Rejected)
  {
    q.prepare( "SELECT itemgrpitem_id "
               "FROM itemgrpitem "
               "WHERE ( (itemgrpitem_itemgrp_id=:itemgrp_id)"
               " AND (itemgrpitem_item_id=:item_id) );" );
    q.bindValue(":itemgrp_id", _itemgrpid);
    q.bindValue(":item_id", itemid);
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Add Item to Item Group"),
                            tr("The selected Item is already a member of this Item Group") );
      return;
    }

    q.prepare( "INSERT INTO itemgrpitem "
               "(itemgrpitem_itemgrp_id, itemgrpitem_item_id) "
               "VALUES "
               "(:itemgrpitem_itemgrp_id, :itemgrpitem_item_id);" );
    q.bindValue(":itemgrpitem_itemgrp_id", _itemgrpid);
    q.bindValue(":itemgrpitem_item_id", itemid);
    q.exec();

    sFillList();
  }
}

void itemGroup::sFillList()
{
  q.prepare( "SELECT itemgrpitem_id, item_number, (item_descrip1 || ' ' || item_descrip2) "
             "FROM itemgrpitem, item "
             "WHERE ( (itemgrpitem_item_id=item_id) "
             " AND (itemgrpitem_itemgrp_id=:itemgrp_id) ) "
             "ORDER BY item_number;" );
  q.bindValue(":itemgrp_id", _itemgrpid);
  q.exec();
  _itemgrpitem->populate(q);
}

void itemGroup::populate()
{
  q.prepare( "SELECT itemgrp_name, itemgrp_descrip "
             "FROM itemgrp "
             "WHERE (itemgrp_id=:itemgrp_id);" );
  q.bindValue(":itemgrp_id", _itemgrpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("itemgrp_name").toString());
    _descrip->setText(q.value("itemgrp_descrip").toString());

    sFillList();
  }
}

void itemGroup::dragEnterEvent(QDragEnterEvent *pEvent)
{
  QString dragData;

  if (Q3TextDrag::decode(pEvent, dragData))
  {
    if (dragData.contains("itemid="))
      pEvent->accept(TRUE);
  }
  else
    pEvent->accept(FALSE);
}

void itemGroup::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("itemid="))
    {
      QString target = dropData.mid((dropData.find("itemid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));

      q.prepare( "SELECT itemgrpitem_id "
                 "FROM itemgrpitem "
                 "WHERE ( (itemgrpitem_itemgrp_id=:itemgrp_id)"
                 " AND (itemgrpitem_item_id=:item_id) );" );
      q.bindValue(":itemgrp_id", _itemgrpid);
      q.bindValue(":item_id", target.toInt());
      q.exec();
      if (q.first())
      {
        QMessageBox::warning( this, tr("Cannot Add Item to Item Group"),
                              tr("The selected Item is already a member of this Item Group") );
        return;
      }

      q.prepare( "INSERT INTO itemgrpitem "
                 "(itemgrpitem_itemgrp_id, itemgrpitem_item_id) "
                 "VALUES "
                 "(:itemgrpitem_itemgrp_id, :itemgrpitem_item_id);" );
      q.bindValue(":itemgrpitem_itemgrp_id", _itemgrpid);
      q.bindValue(":itemgrpitem_item_id", target.toInt());
      q.exec();

      sFillList();
    }
  }
}
