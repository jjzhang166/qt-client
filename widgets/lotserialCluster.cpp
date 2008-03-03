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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

// lotserialCluster.cpp
// Created 11/04/2006 GJM
// Copyright (c) 2006-2007, OpenMFG, LLC

#include "lotserialcluster.h"

#include <QMessageBox>
#include <QSqlError>

LotserialList::LotserialList(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualList(pParent, flags)
{
  setObjectName("LotserialList");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
  _listTab->headerItem()->setText(2, tr("Lot/Serial Creation Timestamp"));
}

LotserialSearch::LotserialSearch(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualSearch(pParent, flags)
{
  setObjectName("LotserialSearch");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
  _listTab->headerItem()->setText(2, tr("Lot/Serial Creation Timestamp"));
}

LotserialCluster::LotserialCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new LotserialLineEdit(this, pName));
}

LotserialLineEdit::LotserialLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "lsdetail", "lsdetail_id", "lsdetail_lotserial", "item_number", "lsdetail_created", 0, pName)
{
    setTitles(tr("Lot/Serial Number"), tr("Lot/Serial Numbers"));
    _query = QString("SELECT lsdetail_id AS id, "
                     "       lsdetail_lotserial AS number, "
                     "       item_number AS name, "
                     "       lsdetail_created AS description "
                     "FROM lsdetail, itemsite, item "
                     "WHERE (lsdetail_itemsite_id=itemsite_id) "
                     "  AND (itemsite_item_id = item_id)");
    _idClause = QString("AND (lsdetail_id=:id)");
    _numClause = QString("AND (UPPER(lsdetail_lotserial)=UPPER(:number))");
    _strict = true;
}

int LotserialCluster::itemId() const
{
  if (_number && _number->inherits("LotserialLineEdit"))
    return ((LotserialLineEdit*)(_number))->itemId();
  return true;
}

void LotserialCluster::setItemId(const int p)
{
  if (_number && _number->inherits("LotserialLineEdit"))
    ((LotserialLineEdit*)(_number))->setItemId(p);
}

bool LotserialCluster::strict() const
{
  if (_number && _number->inherits("LotserialLineEdit"))
    return ((LotserialLineEdit*)(_number))->strict();
  return true;
}

void LotserialCluster::setStrict(const bool p)
{
  if (_number && _number->inherits("LotserialLineEdit"))
    ((LotserialLineEdit*)(_number))->setStrict(p);
}

void LotserialLineEdit::setItemId(const int itemid)
{
  if (itemid <= 0)
  {
    _itemid = -1;
    _extraClause = "";
  }
  else
  {
    _itemid = itemid;
    _extraClause = QString(" (item_id=%1) ").arg(itemid);
  }
}

void LotserialLineEdit::clear()
{
  VirtualClusterLineEdit::clear();
  setItemId(-1);
}

/* copied from virtualCluster.cpp but with one important difference:
   if a not-strict flag is set then warn the user but don't clear the
   lotserial field
 */
void LotserialLineEdit::sParse()
{
    if (! _parsed)
    {
        QString stripped = text().stripWhiteSpace().upper();
        if (stripped.length() == 0)
            setId(-1);
        else
        {
            XSqlQuery numQ;
            numQ.prepare(_query + _numClause +
                         (_extraClause.isEmpty() ? "" : " AND " + _extraClause) +
                         QString(";"));
            numQ.bindValue(":number", stripped);
            numQ.exec();
            if (numQ.first())
            {
                _valid = true;
                setId(numQ.value("id").toInt());
                _name = (numQ.value("name").toString());
                if (_hasDescription)
                    _description = numQ.value("description").toString();
            }
            else if (numQ.lastError().type() != QSqlError::None)
            {
              QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                            .arg(__FILE__)
                                            .arg(__LINE__),
                                    numQ.lastError().databaseText());
              return;
            }
            else if (_strict)
              VirtualClusterLineEdit::clear();
            else if (isVisible() && QMessageBox::question(this, tr("Lot/Serial # Not Found"),
                                           tr("This Lot/Serial # was not found" +
                                              QString(_itemid > 0 ? " for this item" : "") +
                                              ". Are you sure it is correct?"),
                                           QMessageBox::Yes,
                                           QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
            {
              VirtualClusterLineEdit::clear();
              return;
            }
            else
            {
              VirtualClusterLineEdit::clear();
              setText(stripped);
            }
        }
    }

    _parsed = TRUE;
    emit parsed();
}

LotserialList* LotserialLineEdit::listFactory()
{
  return new LotserialList(this);
}

LotserialSearch* LotserialLineEdit::searchFactory()
{
  return new LotserialSearch(this);
}

/*
LotserialInfo* LotserialLineEdit::infoFactory()
{
  return new LotserialInfo(this, "LotserialInfo", true);
}
*/
