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

//  itemCluster.cpp
//  Created 03/07/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QDragEnterEvent>
#include <QApplication>

#include <xsqlquery.h>
#include <parameter.h>

#include "itemcluster.h"
#include "itemList.h"
#include "itemSearch.h"
#include "itemAliasList.h"

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int);

ItemLineEdit::ItemLineEdit(QWidget *pParent, const char *name) : XLineEdit(pParent, name)
{
  setAcceptDrops(TRUE);
  
  _type = cUndefined;
  _useQuery = FALSE;
  _useValidationQuery = FALSE;
  _itemNumber = "";
  _uom = "";
  _itemType = "";
  _id = -1;
  _parsed = TRUE;
  _valid = FALSE;
  _configured = FALSE;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
  connect(this, SIGNAL(requestList()), this, SLOT(sList()));
  connect(this, SIGNAL(requestSearch()), this, SLOT(sSearch()));
  connect(this, SIGNAL(requestAlias()), this, SLOT(sAlias()));
  
  _mapper = new XDataWidgetMapper(this);
}

void ItemLineEdit::setItemNumber(QString pNumber)
{
  XSqlQuery item;
  bool      found = FALSE;

  _parsed = TRUE;

  if (_useValidationQuery)
  {
    item.prepare(_validationSql);
    item.bindValue(":item_number", pNumber);
    item.exec();
    if (item.first())
      found = TRUE;
  }
  else if (_useQuery)
  {
    item.prepare(_sql);
    item.exec();
    found = (item.findFirst("item_number", pNumber) != -1);
  }
  else if (pNumber != QString::Null())
  {
    QString pre( "SELECT DISTINCT item_number, item_descrip1, item_descrip2,"
                 "                uom_name, item_type, item_config");

    QStringList clauses;
    clauses = _extraClauses;
    clauses << "(item_number=:item_number)";

    item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type));
    item.bindValue(":item_number", pNumber);
    item.exec();

    found = item.first();
  }

  if (found)
  {
    _itemNumber = pNumber;
    _uom        = item.value("uom_name").toString();
    _itemType   = item.value("item_type").toString();
    _configured = item.value("item_config").toBool();
    _id         = item.value("item_id").toInt();
    _valid      = TRUE;

    setText(item.value("item_number").toString());

    emit aliasChanged("");
    emit typeChanged(_itemType);
    emit descrip1Changed(item.value("item_descrip1").toString());
    emit descrip2Changed(item.value("item_descrip2").toString());
    emit uomChanged(item.value("uom_name").toString());
    emit configured(item.value("item_config").toBool());
    
    emit valid(TRUE);
  }
  else
  {
    _itemNumber = "";
    _uom        = "";
    _itemType   = "";
    _id         = -1;
    _valid      = FALSE;

    setText("");

    emit aliasChanged("");
    emit typeChanged("");
    emit descrip1Changed("");
    emit descrip2Changed("");
    emit uomChanged("");
    emit configured(FALSE);

    emit valid(FALSE);
  }
}

void ItemLineEdit::silentSetId(int pId)
{
  XSqlQuery item;
  bool      found = FALSE;

  _parsed = TRUE;

  if (_useValidationQuery)
  {
    item.prepare(_validationSql);
    item.bindValue(":item_id", pId);
    item.exec();
    if (item.first())
      found = TRUE;
  }
  else if (_useQuery)
  {
    item.prepare(_sql);
    item.exec();
    found = (item.findFirst("item_id", pId) != -1);
  }
  else if (pId != -1)
  {
    QString pre( "SELECT DISTINCT item_number, item_descrip1, item_descrip2,"
                 "                uom_name, item_type, item_config");

    QStringList clauses;
    clauses = _extraClauses;
    clauses << "(item_id=:item_id)";

    item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type));
    item.bindValue(":item_id", pId);
    item.exec();

    found = item.first();
  }

  if (found)
  {
    _itemNumber = item.value("item_number").toString();
    _uom        = item.value("uom_name").toString();
    _itemType   = item.value("item_type").toString();
    _configured = item.value("item_config").toBool();
    _id         = pId;
    _valid      = TRUE;

    setText(item.value("item_number").toString());

    if (_mapper->model() &&
        _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());

    emit aliasChanged("");
    emit typeChanged(_itemType);
    emit descrip1Changed(item.value("item_descrip1").toString());
    emit descrip2Changed(item.value("item_descrip2").toString());
    emit uomChanged(item.value("uom_name").toString());
    emit configured(item.value("item_config").toBool());
    
    emit valid(TRUE);
  }
  else
  {
    _itemNumber = "";
    _uom        = "";
    _itemType   = "";
    _id         = -1;
    _valid      = FALSE;

    setText("");

    emit aliasChanged("");
    emit typeChanged("");
    emit descrip1Changed("");
    emit descrip2Changed("");
    emit uomChanged("");
    emit configured(FALSE);

    emit valid(FALSE);
  }
} 

void ItemLineEdit::setId(int pId)
{
  if (pId != _id)
  {
    silentSetId(pId);
    emit privateIdChanged(_id);
    emit newId(_id);
  }
}

void ItemLineEdit::setItemsiteid(int pItemsiteid)
{
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_item_id, itemsite_warehous_id "
                    "FROM itemsite "
                    "WHERE (itemsite_id=:itemsite_id);" );
  itemsite.bindValue(":itemsite_id", pItemsiteid);
  itemsite.exec();
  if (itemsite.first())
  {
    setId(itemsite.value("itemsite_item_id").toInt());
    emit warehouseIdChanged(itemsite.value("itemsite_warehous_id").toInt());
  }
  else
  {
    setId(-1);
    emit warehouseIdChanged(-1);
  }
}

void ItemLineEdit::sEllipses()
{
  if(_x_preferences)
  {
    if(_x_preferences->value("DefaultEllipsesAction") == "search")
    {
      sSearch();
      return;
    }
    else if(_x_preferences->value("DefaultEllipsesAction") == "alias")
    {
      sAlias();
      return;
    }
  }

  sList();
}

void ItemLineEdit::sList()
{
  ParameterList params;
  params.append("item_id", _id);

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _type);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  itemList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id = newdlg.exec())!= _id)
  {
    setId(id);

    if (id != -1 && hasFocus())
      focusNextPrevChild(TRUE);
  }
}

void ItemLineEdit::sSearch()
{
  ParameterList params;
  params.append("item_id", _id);

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _type);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  itemSearch newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id = newdlg.exec()) != QDialog::Rejected)
  {
    setId(id);

    if (id != -1 && hasFocus())
      focusNextPrevChild(TRUE);
  }
}

void ItemLineEdit::sAlias()
{
  ParameterList params;

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _type);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  itemAliasList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int itemaliasid;
  if ((itemaliasid = newdlg.exec()) != QDialog::Rejected)
  {
    XSqlQuery itemalias;
    itemalias.prepare( "SELECT itemalias_number, itemalias_item_id "
                       "FROM itemalias "
                       "WHERE (itemalias_id=:itemalias_id);" );
    itemalias.bindValue(":itemalias_id", itemaliasid);
    itemalias.exec();
    if (itemalias.first())
    {
      setId(itemalias.value("itemalias_item_id").toInt());
      emit aliasChanged(itemalias.value("itemalias_number").toString());
      focusNextPrevChild(TRUE);
    }
  }
}

QString ItemLineEdit::itemNumber()
{
  sParse();
  return _itemNumber;
}

QString ItemLineEdit::uom()
{
  sParse();
  return _uom;
}

QString ItemLineEdit::itemType()
{
  sParse();
  return _itemType;
}

bool ItemLineEdit::isConfigured()
{
  sParse();
  return _configured;
}

void ItemLineEdit::mousePressEvent(QMouseEvent *pEvent)
{
  dragStartPosition = pEvent->pos();
  QLineEdit::mousePressEvent(pEvent);
}

void ItemLineEdit::mouseMoveEvent(QMouseEvent * event)
{
  if (!_valid)
    return;
  if (!(event->buttons() & Qt::LeftButton))
    return;
  if ((event->pos() - dragStartPosition).manhattanLength()
        < QApplication::startDragDistance())
    return;

  QString dragData = QString("itemid=%1").arg(_id);
  
  QDrag *drag = new QDrag(this);
  QMimeData * mimeData = new QMimeData;

  mimeData->setText(dragData);
  drag->setMimeData(mimeData);

  /*Qt::DropAction dropAction =*/ drag->start(Qt::CopyAction);
}

void ItemLineEdit::dragEnterEvent(QDragEnterEvent *pEvent)
{
  const QMimeData * mimeData = pEvent->mimeData();
  if (mimeData->hasFormat("text/plain"))
  {
    if (mimeData->text().contains("itemid=") || mimeData->text().contains("itemsiteid="))
      pEvent->acceptProposedAction();
  }
}

void ItemLineEdit::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (pEvent->mimeData()->hasFormat("text/plain"))
  {
    dropData = pEvent->mimeData()->text();
    if (dropData.contains("itemid="))
    {
      QString target = dropData.mid((dropData.find("itemid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));

      pEvent->acceptProposedAction();
      pEvent->accept();
      setId(target.toInt());
    }
    else if (dropData.contains("itemsiteid="))
    {
      QString target = dropData.mid((dropData.find("itemsiteid=") + 11), (dropData.length() - 11));

      if (target.contains(","))
        target = target.left(target.find(","));

      pEvent->acceptProposedAction();
      pEvent->accept();
      setItemsiteid(target.toInt());
    }
  }
}


void ItemLineEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    if (text().length() == 0)
    {
      setId(-1);
      return;
    }

    else if (_useValidationQuery)
    {
      XSqlQuery item;
      item.prepare("SELECT item_id FROM item WHERE (item_number = :searchString);");
      item.bindValue(":searchString", text().stripWhiteSpace().upper());
      item.exec();
      if (item.first())
      {
        int itemid = item.value("item_id").toInt();
        item.prepare(_validationSql);
        item.bindValue(":item_id", itemid);
        item.exec();
        if (item.first())
        {
          setId(itemid);
          return;
        }
      }
    }

    else if (_useQuery)
    {
      XSqlQuery item;
      item.prepare(_sql);
      item.exec();
      if (item.findFirst("item_number", text().stripWhiteSpace().upper()) != -1)
      {
        setId(item.value("item_id").toInt());
        return;
      }
    }
    else
    {
      XSqlQuery item;

      QString pre( "SELECT DISTINCT item_id" );

      QStringList clauses;
      clauses = _extraClauses;
      clauses << "(item_number=:searchString)";

      item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type));
      item.bindValue(":searchString", text().stripWhiteSpace().upper());
      item.exec();
      if (item.first())
      {
        setId(item.value("item_id").toInt());
        return;
      }
    }

    setId(-1);
    focusNextPrevChild(FALSE);
    QMessageBox::warning( this, tr("Invalid Item Number"),
                          tr( "<p>The Item Number you entered is Invalid.</p>") );
  }
}

void ItemLineEdit::addExtraClause(const QString & pClause)
{
  _extraClauses << pClause;
}

ItemCluster::ItemCluster(QWidget *pParent, const char *name) : QWidget(pParent)
{
  setObjectName(name);

//  Create the component Widgets
  QVBoxLayout *mainLayout  = new QVBoxLayout(); 
  mainLayout->setMargin(0); mainLayout->setSpacing(2);
  QHBoxLayout *itemLayout  = new QHBoxLayout(); 
  itemLayout->setMargin(0); itemLayout->setSpacing(5);
  QHBoxLayout *uomLayout   = new QHBoxLayout(); 
  uomLayout->setMargin(0); uomLayout->setSpacing(5);
  QHBoxLayout *line1Layout = new QHBoxLayout(); 
  line1Layout->setMargin(0); line1Layout->setSpacing(7);

  QLabel *_itemNumberLit = new QLabel(tr("Item Number:"), this, "_itemNumberLit");
  _itemNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  itemLayout->addWidget(_itemNumberLit);

  _itemNumber = new ItemLineEdit(this, "_itemNumber");
  _itemNumber->setMinimumWidth(100);
  itemLayout->addWidget(_itemNumber);

  _itemList = new QPushButton(tr("..."), this, "_itemList");
#ifndef Q_WS_MAC
  _itemList->setMaximumWidth(25);
#else
  _itemList->setMinimumWidth(60);
  _itemList->setMinimumHeight(32);
#endif
  _itemList->setFocusPolicy(Qt::NoFocus);
  itemLayout->addWidget(_itemList);
  line1Layout->addLayout(itemLayout);

  QLabel *_uomLit = new QLabel(tr("UOM:"), this, "_uomLit");
  _uomLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  uomLayout->addWidget(_uomLit);

  _uom = new QLabel(this, "_uom");
  _uom->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _uom->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _uom->setMinimumWidth(50);
  uomLayout->addWidget(_uom);
  line1Layout->addLayout(uomLayout);
  mainLayout->addLayout(line1Layout);

  _descrip1 = new QLabel(this, "_descrip1");
  _descrip1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  mainLayout->addWidget(_descrip1);

  _descrip2 = new QLabel(this, "_descrip2");
  _descrip2->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  mainLayout->addWidget(_descrip2);
  setLayout(mainLayout);

//  Make some internal connections
  connect(_itemNumber, SIGNAL(aliasChanged(const QString &)), this, SIGNAL(aliasChanged(const QString &)));
  connect(_itemNumber, SIGNAL(privateIdChanged(int)), this, SIGNAL(privateIdChanged(int)));
  connect(_itemNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_itemNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));
  connect(_itemNumber, SIGNAL(warehouseIdChanged(int)), this, SIGNAL(warehouseIdChanged(int)));
  connect(_itemNumber, SIGNAL(typeChanged(const QString &)), this, SIGNAL(typeChanged(const QString &)));
  connect(_itemNumber, SIGNAL(configured(bool)), this, SIGNAL(configured(bool)));

  connect(_itemNumber, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(_itemNumber, SIGNAL(descrip1Changed(const QString &)), _descrip1, SLOT(setText(const QString &)));
  connect(_itemNumber, SIGNAL(descrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));

  connect(_itemList, SIGNAL(clicked()), _itemNumber, SLOT(sEllipses()));

  setFocusProxy(_itemNumber);
}

void ItemCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_itemNumber, _fieldName, QByteArray("number"));
  _itemNumber->_mapper=m;
}

void ItemCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _itemNumber->setEnabled(FALSE);
    _itemList->hide();
  }
  else
  {
    _itemNumber->setEnabled(TRUE);
    _itemList->show();
  }
}

void ItemCluster::setEnabled(bool pEnabled)
{
  setReadOnly(!pEnabled);
}

void ItemCluster::setDisabled(bool pDisabled)
{
  setReadOnly(pDisabled);
}

void ItemCluster::setId(int pId)
{
  _itemNumber->setId(pId);
}

void ItemCluster::silentSetId(int pId)
{
  _itemNumber->silentSetId(pId);
}

void ItemCluster::setItemNumber(QString pNumber)
{
  _itemNumber->setItemNumber(pNumber);
}

void ItemCluster::setItemsiteid(int intPItemsiteid)
{
  _itemNumber->setItemsiteid(intPItemsiteid);
}


QString buildItemLineEditQuery(const QString pPre, const QStringList pClauses, const QString pPost, const unsigned int pType)
{
  QStringList clauses = pClauses;
  QString sql = pPre + " FROM item, uom";
  clauses << "(item_inv_uom_id=uom_id)";

  if (pType & (ItemLineEdit::cLocationControlled | ItemLineEdit::cLotSerialControlled | ItemLineEdit::cDefaultLocation | ItemLineEdit::cActive))
  {
    sql += ", itemsite";
    clauses << "(itemsite_item_id=item_id)";
  }

  if (pType & ItemLineEdit::cAllItemTypes_Mask)
  {
    QStringList types;

    if (pType & ItemLineEdit::cPurchased)
      types << "'P'";

    if (pType & ItemLineEdit::cManufactured)
      types << "'M'";

    if (pType & ItemLineEdit::cJob)
      types << "'J'";

    if (pType & ItemLineEdit::cPhantom)
      types << "'F'";

    if (pType & ItemLineEdit::cBreeder)
      types << "'B'";

    if (pType & ItemLineEdit::cCoProduct)
      types << "'C'";

    if (pType & ItemLineEdit::cByProduct)
      types << "'Y'";

    if (pType & ItemLineEdit::cReference)
      types << "'R'";

    if (pType & ItemLineEdit::cCosting)
      types << "'S'";

    if (pType & ItemLineEdit::cTooling)
      types << "'T'";

    if (pType & ItemLineEdit::cOutsideProcess)
      types << "'O'";

    if (pType & ItemLineEdit::cPlanning)
      types << "'L'";

    if (!types.isEmpty())
      clauses << QString("(item_type IN (" + types.join(",") + "))");
  }

  if (pType & ItemLineEdit::cPlanningAny)
  {
    QStringList plantypes;

    if (pType & ItemLineEdit::cPlanningMRP)
      plantypes << "'M'";

    if (pType & ItemLineEdit::cPlanningMPS)
      plantypes << "'S'";

    if (pType & ItemLineEdit::cPlanningNone)
      plantypes << "'N'";

    if (!plantypes.isEmpty())
      clauses << QString("(item_planning_type IN (" + plantypes.join(",") + "))");
  }

  if (pType & (ItemLineEdit::cLocationControlled | ItemLineEdit::cLotSerialControlled | ItemLineEdit::cDefaultLocation))
  {
    QString sub;
    if (pType & ItemLineEdit::cLocationControlled)
      sub = "(itemsite_loccntrl)";

    if (pType & ItemLineEdit::cLotSerialControlled)
    {
      if (!sub.isEmpty())
        sub += " OR ";
      sub += "(itemsite_controlmethod IN ('L', 'S'))";
    }

    if (pType & ItemLineEdit::cDefaultLocation)
    {
      if (!sub.isEmpty())
        sub += " OR ";
      sub += "(useDefaultLocation(itemsite_id))";
    }

    clauses << QString("( " + sub + " )");
  }

  if (pType & ItemLineEdit::cSold)
    clauses << "(item_sold)";

  if (pType & (ItemLineEdit::cActive | ItemLineEdit::cItemActive))
    clauses << "(item_active)";

  if (!clauses.isEmpty())
    sql += " WHERE (" + clauses.join(" AND ") + ")";

  if (!pPost.isEmpty())
    sql += " " + pPost;

  sql += ";";

  return sql;
}

QString buildItemLineEditTitle(unsigned int pType, const QString pPost)
{
  QString caption;
  unsigned int items = (0xFFFF & pType); // mask so this value only has the individual items

  if (pType & ItemLineEdit::cSold)
    caption += ItemLineEdit::tr("Sold ");

  if (pType & ItemLineEdit::cActive)
    caption = ItemLineEdit::tr("Active ");
 
  if (pType & ItemLineEdit::cLocationControlled)
    caption += ItemLineEdit::tr("Multiply Located ");
  else if (pType & ItemLineEdit::cLotSerialControlled)
    caption += ItemLineEdit::tr("Lot/Serial # Controlled ");

  else if (items == (ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured))
    caption += ItemLineEdit::tr("Purchased and Manufactured ");
  else if ((items == ItemLineEdit::cGeneralPurchased) || (items == ItemLineEdit::cPurchased) )
    caption += ItemLineEdit::tr("Purchased ");
  else if ( (items == ItemLineEdit::cGeneralManufactured) || (items == ItemLineEdit::cManufactured) )
    caption += ItemLineEdit::tr("Manufactured ");
  else if (items == ItemLineEdit::cBreeder)
    caption += ItemLineEdit::tr("Breeder ");
  else if (items == (ItemLineEdit::cCoProduct | ItemLineEdit::cByProduct))
    caption += ItemLineEdit::tr("Co-Product and By-Product ");
  else if (items == ItemLineEdit::cCosting)
    caption += ItemLineEdit::tr("Costing ");
  else if (items == ItemLineEdit::cGeneralComponents)
    caption += ItemLineEdit::tr("Component ");

  if(!pPost.isEmpty())
    caption += pPost;

  return caption;
}

