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

#include "countTagList.h"

#include <QVariant>

/*
 *  Constructs a countTagList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
countTagList::countTagList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    connect(_cnttag, SIGNAL(valid(bool)), _select, SLOT(setEnabled(bool)));
    connect(_cnttag, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
    connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
countTagList::~countTagList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void countTagList::languageChange()
{
    retranslateUi(this);
}

void countTagList::init()
{
  _cnttag->addColumn(tr("Tag #"),       _orderColumn, Qt::AlignLeft   );
  _cnttag->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _cnttag->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _cnttag->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
}

enum SetResponse countTagList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
    _cnttagid = param.toInt();

  param = pParams.value("tagType", &valid);
  if (valid)
    _type = param.toInt();

  sFillList();

  return NoError;
}

void countTagList::sFillList()
{
  QString sql( "SELECT invcnt_id, invcnt_tagnumber, item_number, item_descrip1, warehous_code "
               "FROM invcnt, itemsite, item, warehous "
               "WHERE ((invcnt_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  if (_type & cPostedCounts)
    sql += " AND (invcnt_posted)";
  else if (_type & cUnpostedCounts)
    sql += " AND (NOT invcnt_posted)";

  sql += ") "
         "ORDER BY invcnt_tagnumber";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.exec();

  _cnttag->populate(q, _cnttagid);
}

void countTagList::sClose()
{
  done(_cnttagid);
}

void countTagList::sSelect()
{
  done(_cnttag->id());
}

void countTagList::sSearch(const QString &pTarget)
{
  _cnttag->clearSelection();
  int i;
  for (i = 0; i < _cnttag->topLevelItemCount(); i++)
  {
    if (_cnttag->topLevelItem(i)->text(1).contains(pTarget, Qt::CaseInsensitive))
    break;
  }

  if (i < _cnttag->topLevelItemCount())
  {
    _cnttag->setCurrentItem(_cnttag->topLevelItem(i));
    _cnttag->scrollToItem(_cnttag->topLevelItem(i));
  }
}

