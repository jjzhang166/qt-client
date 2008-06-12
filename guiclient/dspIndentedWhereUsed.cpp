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

#include "dspIndentedWhereUsed.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <QStack>
#include "dspInventoryHistoryByItem.h"

/*
 *  Constructs a dspIndentedWhereUsed as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspIndentedWhereUsed::dspIndentedWhereUsed(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq. #"),        80,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),       _dateColumn,  Qt::AlignCenter );
  _bomitem->setIndentation(10);

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspIndentedWhereUsed::~dspIndentedWhereUsed()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspIndentedWhereUsed::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspIndentedWhereUsed::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Print;
  }

  return NoError;
}

void dspIndentedWhereUsed::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  q.prepare("SELECT indentedWhereUsed(:item_id) AS result;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    int worksetid = q.value("result").toInt();

    ParameterList params;
    params.append("item_id", _item->id());
    params.append("bomworkset_id", worksetid);

    if(_showExpired->isChecked())
      params.append("showExpired");

    if(_showFuture->isChecked())
      params.append("showFuture");

    orReport report("IndentedWhereUsed", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);

    q.prepare("SELECT deleteBOMWorkset(:bomworkset_id) AS result;");
    q.bindValue(":bomworkset_id", worksetid);
    q.exec();
  }
  else
      QMessageBox::critical( this, tr("Error Executing Report"),
                             tr( "Was unable to create/collect the required information to create this report." ) );
}

void dspIndentedWhereUsed::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspIndentedWhereUsed::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspIndentedWhereUsed::sFillList()
{
  _bomitem->clear();

  if (_item->isValid())
  {
    q.prepare("SELECT indentedWhereUsed(:item_id) AS workset_id;");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      int worksetid = q.value("workset_id").toInt();

      QString sql( "SELECT bomwork_level, bomwork_id, item_id, bomwork_parent_id,"
		           "       bomworkitemsequence(bomwork_id) as seqord, "
                   "       bomwork_seqnumber, item_number, uom_name,"
                   "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
                   "       formatQtyPer(bomwork_qtyper) AS qtyper,"
                   "       formatScrap(bomwork_scrap) AS scrap,"
                   "       formatDate(bomwork_effective, 'Always') AS effective,"
                   "       formatDate(bomwork_expires, 'Never') AS expires "
                   "FROM bomwork, item, uom "
                   "WHERE ( (bomwork_item_id=item_id)"
                   " AND (item_inv_uom_id=uom_id)"
                   " AND (bomwork_set_id=:bomwork_set_id)" );

      if (!_showExpired->isChecked())
        sql += " AND (bomwork_expires > CURRENT_DATE)";

      if (!_showFuture->isChecked())
        sql += " AND (bomwork_effective <= CURRENT_DATE)";

      sql += ") "
             "ORDER BY seqord;";

      q.prepare(sql);
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();

      QStack<XTreeWidgetItem*> parent;
      XTreeWidgetItem *last = 0;
      int level = 0;
      while(q.next())
      {
        // If the level this item is on is lower than the last level we just did then we need
        // to pop the stack a number of times till we are equal.
        while(q.value("bomwork_level").toInt() < level)
        {
          level--;
          last = parent.pop();
        }

        // If the level this item is on is higher than the last level we need to push the last
        // item onto the stack a number of times till we are equal. (Should only ever be 1.)
        while(q.value("bomwork_level").toInt() > level)
        {
          level++;
          parent.push(last);
          last = 0;
        }

        // If there is an item in the stack use that as the parameter to the new xlistviewitem
        // otherwise we'll just use the xlistview _layout
        if(!parent.isEmpty() && parent.top())
          last = new XTreeWidgetItem(parent.top(), last, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                             q.value("bomwork_seqnumber"), q.value("item_number"),
                             q.value("itemdescription"), q.value("uom_name"),
                             q.value("qtyper"), q.value("scrap"),
                             q.value("effective"), q.value("expires") );
        else
          last = new XTreeWidgetItem(_bomitem, last, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                             q.value("bomwork_seqnumber"), q.value("item_number"),
                             q.value("itemdescription"), q.value("uom_name"),
                             q.value("qtyper"), q.value("scrap"),
                             q.value("effective"), q.value("expires") );

      }
      _bomitem->expandAll();

      q.prepare("SELECT deleteBOMWorkset(:workset_id) AS result;");
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();
    }
  }
}
