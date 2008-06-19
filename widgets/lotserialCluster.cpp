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

// lotserialCluster.cpp
// Created 11/04/2006 GJM
// Copyright (c) 2006-2008, OpenMFG, LLC

#include "lotserialcluster.h"

#include <QMessageBox>
#include <QSqlError>

#include "xsqlquery.h"

LotserialList::LotserialList(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualList(pParent, flags)
{
  setObjectName("LotserialList");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialSearch::LotserialSearch(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualSearch(pParent, flags)
{
  setObjectName("LotserialSearch");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialCluster::LotserialCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    LotserialLineEdit* lsle=new LotserialLineEdit(this, pName);
    addNumberWidget(lsle);
    
    connect(lsle, SIGNAL(newItemId(int)), this, SIGNAL(newItemId(int)));
}

LotserialLineEdit::LotserialLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "ls", "ls_id", "ls_number", "item_number", "item_id", 0, pName)
{
    _itemid = -1;
    setTitles(tr("Lot/Serial Number"), tr("Lot/Serial Numbers"));
    _query = QString("SELECT ls_id AS id, "
                     "       ls_number AS number, "
                     "       item_id,"
                     "       item_number AS name, "
                     "       ls_notes AS description "
                     "FROM ls, item "
                     "WHERE (ls_item_id=item_id) ");
    _idClause = QString("AND (ls_id=:id)");
    _numClause = QString("AND (UPPER(ls_number)=UPPER(:number))");
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
  if (_itemid == itemid)
    return;
  else if (itemid <= 0)
  {
    _itemid = -1;
    _extraClause = "";
    VirtualClusterLineEdit::clear();
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
}

void LotserialLineEdit::setId(const int lsid)
{
  if (_x_metrics)
    if (_x_metrics->boolean("LotSerialControl"))
    {
      VirtualClusterLineEdit::setId(lsid);
      XSqlQuery qq;
      qq.prepare("SELECT ls_item_id"
                 "  FROM ls"
                 " WHERE(ls_id=:id);");
      qq.bindValue(":id", id());
      qq.exec();
      if(qq.first())
        emit newItemId(qq.value("ls_item_id").toInt());
    }
}

/* copied from virtualCluster.cpp but with one important difference:
   if a not-strict flag is set then warn the user but don't clear the
   lotserial field
 */
void LotserialLineEdit::sParse()
{
  if (_x_metrics)
    if (_x_metrics->boolean("LotSerialControl"))
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
                  _itemid = (numQ.value("item_id").toInt());
              }
              else if (numQ.lastError().type() != QSqlError::None)
              {
                QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                              .arg(__FILE__)
                                              .arg(__LINE__),
                                      numQ.lastError().databaseText());
                return;
              }
              else if (_strict || _itemid == -1)
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
                int lsid;
                numQ.exec("SELECT nextval('ls_ls_id_seq') AS ls_id;");
                if (numQ.first())
                  lsid= numQ.value("ls_id").toInt();
                else if (numQ.lastError().type() != QSqlError::None)
                {
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                                .arg(__FILE__)
                                                .arg(__LINE__),
                                        numQ.lastError().databaseText());
                  return;
                }
                numQ.prepare("INSERT INTO ls (ls_id,ls_item_id,ls_number) "
                             "VALUES (:ls_id,:item_id,:number)");
                numQ.bindValue(":ls_id", lsid);
                numQ.bindValue(":item_id", _itemid);
                numQ.bindValue(":number", stripped);
                numQ.exec();
                if (numQ.lastError().type() != QSqlError::None)
                {
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                                .arg(__FILE__)
                                                .arg(__LINE__),
                                        numQ.lastError().databaseText());
                  return;
                }
                setId(lsid);
              }
          }
      }

      _parsed = TRUE;
      emit parsed();
    }
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
