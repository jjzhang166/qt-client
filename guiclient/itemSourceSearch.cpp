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

#include "itemSourceSearch.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a itemSourceSearch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemSourceSearch::itemSourceSearch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _vendid = -1;

  // signals and slots connections
  connect(_searchNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip1, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchDescrip2, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendNumber, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchVendDescrip, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_search, SIGNAL(textChanged(QString)), this, SLOT(sFillList()));

  _itemsrc->addColumn(tr("Item Number"),    _itemColumn, Qt::AlignLeft );
  _itemsrc->addColumn(tr("Description"),    -1,          Qt::AlignLeft );
  _itemsrc->addColumn(tr("Vendor"),         _itemColumn, Qt::AlignLeft );
  _itemsrc->addColumn(tr("Vendor Item"),    _itemColumn, Qt::AlignLeft );
  _itemsrc->addColumn(tr("Vendor Descrip"), _itemColumn, Qt::AlignLeft );
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourceSearch::~itemSourceSearch()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourceSearch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemSourceSearch::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if(valid)
    _vendid = param.toInt();
  
  param = pParams.value("search", &valid);
  if(valid)
    _search->setText(param.toString());

  sFillList();

  return NoError;
}

void itemSourceSearch::sFillList()
{
  _itemsrc->clear();
  
  bool first = true;

  QString sql( "SELECT itemsrc_item_id AS id,"
               "       1 AS altid,"
               "       item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       vend_name,"
               "       itemsrc_vend_item_number,"
               "       itemsrc_vend_item_descrip "
               "  FROM item, vend, itemsrc "
               " WHERE((itemsrc_item_id=item_id)"
               "   AND (itemsrc_vend_id=vend_id)"
               "   AND (vend_id=:vend_id)"
               "   AND (" );
  if(_searchNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_number ~* :searchString)";
    first = false;
  }
  if(_searchVendNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_vend_item_number ~* :searchString)";
    first = false;
  }
  if(_searchDescrip1->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip1 ~* :searchString)";
    first = false;
  }
  if(_searchDescrip2->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip2 ~* :searchString)";
    first = false;
  }
  if(_searchVendDescrip->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (itemsrc_vend_item_descrip ~* :searchString)";
    first = false;
  }
  
  sql +=       "       )"
               "  )"
               " UNION "
               "SELECT DISTINCT COALESCE(item_id, poitem_expcat_id) AS id,"
               "       CASE WHEN(item_id IS NULL) THEN 2 ELSE 1 END AS altid,"
               "       COALESCE(item_number, :non) AS item_number,"
               "       CASE WHEN(item_id IS NOT NULL) THEN"
               "         (item_descrip1 || ' ' || item_descrip2)"
               "            ELSE (expcat_code || ' ' || expcat_descrip)"
               "       END AS item_descrip,"
               "       vend_name,"
               "       poitem_vend_item_number,"
               "       poitem_vend_item_descrip"
               "  FROM vend, pohead, poitem"
               "       LEFT OUTER JOIN expcat ON (poitem_expcat_id=expcat_id)"
               "       LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
               "       LEFT OUTER JOIN item ON (itemsite_item_id=item_id)"
               " WHERE((pohead_vend_id=vend_id)"
               "   AND (COALESCE(poitem_vend_item_number, '')!='')"
               "   AND (poitem_pohead_id=pohead_id)"
               "   AND (vend_id=:vend_id)"
               "   AND ( ";

  first = true;
  if(_searchNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_number ~* :searchString)";
    first = false;
  }
  if(_searchVendNumber->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_vend_item_number ~* :searchString)";
    first = false;
  }
  if(_searchDescrip1->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip1 ~* :searchString)"
               "    OR  (expcat_code ~* :searchString)"
               "    OR  (expcat_descrip ~* :searchString)";
    first = false;
  }
  if(_searchDescrip2->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (item_descrip2 ~* :searchString)";
    first = false;
  }
  if(_searchVendDescrip->isChecked())
  {
    if(!first)
      sql += " OR ";
    sql +=     "        (poitem_vend_item_descrip ~* :searchString)";
    first = false;
  }


  sql +=       "    ) )"
               " ORDER BY item_number, vend_name;";
  q.prepare(sql);
  q.bindValue(":searchString", _search->text());
  q.bindValue(":vend_id", _vendid);
  q.bindValue(":non", tr("Non-Inventory"));
  q.exec();
  _itemsrc->populate(q, TRUE);
}

int itemSourceSearch::itemId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 1)
      return item->id();
  }
  return -1;
}

int itemSourceSearch::expcatId()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
  {
    if(item->altId() == 2)
      return item->id();
  }
  return -1;
}

QString itemSourceSearch::vendItemNumber()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(3);
  return QString();
}

QString itemSourceSearch::vendItemDescrip()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_itemsrc->currentItem();
  if(item)
    return item->text(4);
  return QString();
}
