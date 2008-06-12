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

#include <QMessageBox>
#include <QSqlError>

#include "ordercluster.h"
#include "xsqlquery.h"

OrderCluster::OrderCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new OrderLineEdit(this, pName));

  _name->setVisible(true);
  _grid->removeWidget(_number);
  _grid->removeWidget(_list);
  _grid->removeWidget(_info);
  _grid->removeWidget(_description);
  _grid->removeWidget(_name);

  _grid->addWidget(_number,		0, 1, 1, 2);
  _grid->addWidget(_list,		0, 3);
  _grid->addWidget(_info,		0, 4);
  _grid->addWidget(_name,		1, 1);
  _grid->addWidget(_description,	1, 2);

  _fromLit	= new QLabel(this);	_fromLit->setObjectName("_fromLit");
  _from		= new QLabel(this);	_from->setObjectName("_from");
  _toLit	= new QLabel(this);	_toLit->setObjectName("_toLit");
  _to		= new QLabel(this);	_to->setObjectName("_to");

  _grid->addWidget(_fromLit,	2, 1);
  _grid->addWidget(_from,	2, 2);
  _grid->addWidget(_toLit,	2, 3);
  _grid->addWidget(_to,		2, 4);

  connect(_number, SIGNAL(newId(const int, const QString &)), this, SIGNAL(newId(const int, const QString &)));
  connect(_number, SIGNAL(numberChanged(const QString &, const QString &)), this, SIGNAL(numberChanged(const QString &, const QString &)));

  setAllowedStatuses(OrderLineEdit::AnyStatus);
  setAllowedTypes(OrderLineEdit::AnyType);
}

OrderLineEdit::OrderStatuses OrderCluster::allowedStatuses() const
{
  return ((OrderLineEdit*)_number)->allowedStatuses();
}

OrderLineEdit::OrderTypes OrderCluster::allowedTypes() const
{
  return ((OrderLineEdit*)_number)->allowedTypes();
}

QString OrderCluster::from() const
{
  return ((OrderLineEdit*)_number)->from();
}

bool OrderCluster::isClosed() const
{
  return ((OrderLineEdit*)_number)->isClosed();
}

bool OrderCluster::isOpen() const
{
  return ((OrderLineEdit*)_number)->isOpen();
}

bool OrderCluster::isPO() const
{
  return ((OrderLineEdit*)_number)->isPO();
}

bool OrderCluster::isRA() const
{
  return ((OrderLineEdit*)_number)->isRA();
}

bool OrderCluster::isSO() const
{
  return ((OrderLineEdit*)_number)->isSO();
}

bool OrderCluster::isTO() const
{
  return ((OrderLineEdit*)_number)->isTO();
}

bool OrderCluster::isUnposted() const
{
  return ((OrderLineEdit*)_number)->isUnposted();
}

void OrderCluster::setExtraClause(const QString &pType, const QString &pClause)
{
  ((OrderLineEdit*)_number)->setExtraClause(pType, pClause);
}

void OrderCluster::setExtraClause(const OrderLineEdit::OrderTypes pTypes,
				  const QString &pClause)
{
  ((OrderLineEdit*)_number)->setExtraClause(pTypes, pClause);
}

void OrderCluster::setAllowedStatuses(const OrderLineEdit::OrderStatuses p)
{
  ((OrderLineEdit*)_number)->setAllowedStatuses(p);
}

OrderLineEdit::OrderStatus  OrderCluster::status() const
{
  return ((OrderLineEdit*)_number)->status();
}

QString OrderCluster::to() const
{
  return ((OrderLineEdit*)_number)->to();
}

QString OrderCluster::type() const
{
  return ((OrderLineEdit*)_number)->type();
}

void OrderCluster::setAllowedType(const QString &p)
{
  ((OrderLineEdit*)_number)->setAllowedType(p);
}

void OrderCluster::setAllowedTypes(const OrderLineEdit::OrderTypes p)
{
  ((OrderLineEdit*)_number)->setAllowedTypes(p);

  switch (p)
  {
    case OrderLineEdit::Purchase:	setLabel(tr("P/O #:")); break;
    case OrderLineEdit::Sales:		setLabel(tr("S/O #:")); break;
    case OrderLineEdit::Transfer:	setLabel(tr("T/O #:")); break;
    case OrderLineEdit::Return:		setLabel(tr("R/A #:")); break;
    default:				setLabel(tr("Order #:")); break;
  }
}

void OrderCluster::setId(const int pId, const QString &pType)
{
  ((OrderLineEdit*)_number)->setId(pId, pType);
}

void OrderCluster::sRefresh()
{
  VirtualCluster::sRefresh();

  _grid->removeWidget(_fromLit);
  _grid->removeWidget(_toLit);
  _grid->removeWidget(_from);
  _grid->removeWidget(_to);

  if (type() == "PO")
  {
    _grid->addWidget(_fromLit,	2, 1);
    _grid->addWidget(_from,	2, 2, 1, -1);
    _grid->addWidget(_toLit,	2, 0);
    _grid->addWidget(_to,	2, 0);

    _fromLit->setText("Vendor:");
    _toLit->setText("");
  }
  else if (type() == "RA")
  {
    _grid->addWidget(_fromLit,	2, 1);
    _grid->addWidget(_from,	2, 2, 1, -1);
    _grid->addWidget(_toLit,	2, 0);
    _grid->addWidget(_to,	2, 0);

    _fromLit->setText("Customer:");
    _toLit->setText("");
  }
  else if (type() == "SO")
  {
    _grid->addWidget(_fromLit,	2, 0);
    _grid->addWidget(_from,	2, 0);
    _grid->addWidget(_toLit,	2, 1);
    _grid->addWidget(_to,	2, 2, 1, -1);

    _fromLit->setText("");
    _toLit->setText("Customer:");
  }
  else if (type() == "TO")
  {
    _grid->addWidget(_fromLit,	2, 1);
    _grid->addWidget(_from,	2, 2);
    _grid->addWidget(_toLit,	2, 3);
    _grid->addWidget(_to,	2, 4);

    _fromLit->setText(tr("From:"));
    _toLit->setText(tr("To:"));
  }
  else
  {
    _grid->addWidget(_fromLit,	2, 1);
    _grid->addWidget(_from,	2, 2);
    _grid->addWidget(_toLit,	2, 3);
    _grid->addWidget(_to,	2, 4);

    _fromLit->setText("");
    _toLit->setText("");
  }

  _from->setText(((OrderLineEdit*)_number)->from());
  _to->setText(((OrderLineEdit*)_number)->to());
}

// LineEdit ///////////////////////////////////////////////////////////////////

OrderLineEdit::OrderLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "orderhead", "orderhead_id",
			 "orderhead_number", "orderhead_type",
			 "orderhead_status", pName)
{
  setTitles(tr("Order"), tr("Orders"));

  connect(this, SIGNAL(newId(int)), this, SLOT(sNewId(int)));

  _query = "SELECT orderhead_id AS id, orderhead_number AS number,"
	   "       orderhead_type AS name, orderhead_status AS description,"
	   "       orderhead_from, orderhead_to "
	   "FROM orderhead WHERE (TRUE) ";
}

void OrderLineEdit::sNewId(const int p)
{
  emit newId(p, _name);
  emit numberChanged(text(), _name);
}

void OrderLineEdit::sParse()
{
  bool oldvalid = _valid;
  if (! _parsed)
  {
    QString stripped = text().stripWhiteSpace().upper();
    if (stripped.length() == 0)
    {
      _parsed = true;
      setId(-1);
    }
    else
    {
      QString oldExtraClause = _extraClause;

      XSqlQuery countQ;
      countQ.prepare("SELECT COUNT(*) AS count FROM orderhead WHERE (TRUE) " +
		      _numClause +
		      (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
		      QString(";"));
      countQ.bindValue(":number", text());
      countQ.exec();
      if (countQ.first())
      {
	int result = countQ.value("count").toInt();
	if (result <= 0)
	{
	  _id = -1;
	  XLineEdit::clear();
	}
	else if (result == 1)
	{
	  XSqlQuery numQ;
	  numQ.prepare(_query + _numClause +
		      (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
		      QString(";"));
	  numQ.bindValue(":number", text());
	  numQ.exec();
	  if (numQ.first())
	  {
	    _valid = true;
	    setId(numQ.value("id").toInt(), numQ.value("name").toString());
	    _description	= numQ.value("description").toString();
	    _from		= numQ.value("orderhead_from").toString();
	    _to		= numQ.value("orderhead_to").toString();
	  }
	  else if (numQ.lastError().type() != QSqlError::None)
	    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					  .arg(__FILE__)
					  .arg(__LINE__),
				  numQ.lastError().databaseText());
	}
	else
	{
	  _extraClause += "AND (orderhead_number=" + text() + ")";
	  sEllipses();
	  _extraClause += "AND (orderhead_type='" + type() + "')";
	}
      }
      else if (countQ.lastError().type() != QSqlError::None)
      {
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      countQ.lastError().databaseText());
      }

      _extraClause = oldExtraClause;
    }
  }

  _parsed = true;
  if (_valid != oldvalid)
    emit valid(_valid);
  emit parsed();
  emit numberChanged(text(), _name);
}

OrderLineEdit::OrderStatuses OrderLineEdit::allowedStatuses() const
{
  return _allowedStatuses;
}

OrderLineEdit::OrderTypes OrderLineEdit::allowedTypes() const
{
  return _allowedTypes;
}

void OrderLineEdit::clear()
{
  _from = "";
  _to = "";
  VirtualClusterLineEdit::clear();
}

QString OrderLineEdit::from() const
{
  return _from;
}

bool OrderLineEdit::isClosed() const
{
  return _description == "C";
}

bool OrderLineEdit::isOpen() const
{
  return _description == "O";
}

bool OrderLineEdit::isPO() const
{
  return _name == "PO";
}

bool OrderLineEdit::isRA() const
{
  return _name == "RA";
}

bool OrderLineEdit::isSO() const
{
  return _name == "SO";
}

bool OrderLineEdit::isTO() const
{
  return _name == "TO";
}

bool OrderLineEdit::isUnposted() const
{
  return _description == "U";
}

void OrderLineEdit::setExtraClause(const QString &pType, const QString &pClause)
{
  if (pType == "" || pType.compare("All", Qt::CaseInsensitive) == 0 ||
	   pType.compare("Any", Qt::CaseInsensitive) == 0)
  {
    _poClause = _raClause = _soClause = _toClause = "";
    _allClause = pClause;
  }
  else if (pType == "PO")
    _poClause = " (((orderhead_type='PO') AND (" + pClause + ")) OR (orderhead_type != 'PO'))";
  else if (pType == "RA")
    _raClause = " (((orderhead_type='RA') AND (" + pClause + ")) OR (orderhead_type != 'RA'))";
  else if (pType == "SO")
    _soClause = " (((orderhead_type='SO') AND (" + pClause + ")) OR (orderhead_type != 'SO'))";
  else if (pType == "TO")
    _toClause = " (((orderhead_type='TO') AND (" + pClause + ")) OR (orderhead_type != 'TO'))";
  else
  {
    QMessageBox::critical(this, tr("Invalid Order Type"),
			  tr("%1 is not a valid Order Type.").arg(pType));
    _allClause = _poClause = _raClause = _soClause = _toClause = "";
  }

  VirtualClusterLineEdit::setExtraClause(buildExtraClause());
}

void OrderLineEdit::setExtraClause(const OrderTypes pTypes, const QString &pClause)
{
  if (pTypes == AnyType ||
      ((pTypes & Purchase) &&
       ((pTypes & Return) || ! _x_metrics->boolean("EnableReturnAuth")) &&
       (pTypes & Sales) &&
       ((pTypes & Transfer) || ! _x_metrics->boolean("MultiWhs"))))
    _allClause = pClause;
  else
  {
    // not else if because multiple flags can be set
    if (pTypes | Purchase)	setExtraClause("PO", pClause);
    if (pTypes | Return)		setExtraClause("RA", pClause);
    if (pTypes | Sales)		setExtraClause("SO", pClause);
    if (pTypes | Transfer)	setExtraClause("TO", pClause);
  }

  VirtualClusterLineEdit::setExtraClause(buildExtraClause());
}

void OrderLineEdit::sList()
{
  OrderList newdlg(this);
  int id = newdlg.exec();
  QString type = newdlg.type();
  setId(id, type);
}

void OrderLineEdit::sSearch()
{
  OrderSearch newdlg(this);
  int id = newdlg.exec();
  QString type = newdlg.type();
  setId(id, type);
}

OrderLineEdit::OrderStatus  OrderLineEdit::status()
{
  if (_description.isEmpty())	return OrderLineEdit::AnyStatus;
  else if (_description == "O")	return OrderLineEdit::Open;
  else if (_description == "C")	return OrderLineEdit::Closed;
  else
  {
    QMessageBox::critical(this, tr("Invalid Order Status"),
			  tr("Order %1 has an invalid status %2.")
			    .arg(text()).arg(_description));
    return AnyStatus;
  }
}

QString OrderLineEdit::to() const
{
  return _to;
}

QString OrderLineEdit::type()
{
  return _name;
}

void OrderLineEdit::setAllowedStatuses(const OrderLineEdit::OrderStatuses p)
{
  if (p && (p != Unposted + Open + Closed))
  {
    QStringList statusList;

    if (p & Unposted)	statusList << "'U'";
    if (p & Open)	statusList << "'O'";
    if (p & Closed)	statusList << "'C'";

    _statusClause = "(orderhead_status IN (" +
		    statusList.join(", ") +
		    "))";
  }
  else
    _statusClause = "";

  VirtualClusterLineEdit::setExtraClause(buildExtraClause());

  _allowedStatuses = p;
}

void OrderLineEdit::setAllowedType(const QString &p)
{
  if (p == "PO")	setAllowedTypes(Purchase);
  else if (p == "RA")	setAllowedTypes(Return);
  else if (p == "SO")	setAllowedTypes(Sales);
  else if (p == "TO")	setAllowedTypes(Transfer);
  else			setAllowedTypes(AnyType);
}

void OrderLineEdit::setAllowedTypes(const OrderTypes p)
{
  if ((p & Purchase) &&
      ((p & Return) || ! _x_metrics->boolean("EnableReturnAuth")) &&
      (p & Sales) &&
      ((p & Transfer) || ! _x_metrics->boolean("MultiWhs")))
  {
    _typeClause = "";
    _allowedTypes = AnyType;
  }

  else if (p)
  {
    QStringList typeList;

    if (p & Purchase)
      typeList << "'PO'";

    if (p & Sales)
      typeList << "'SO'";

    if (p & Transfer && _x_metrics->boolean("MultiWhs"))
      typeList << "'TO'";

    if (p & Return)
      typeList << "'RA'";

    _typeClause = "(orderhead_type IN (" + typeList.join(", ") + "))" ;
    _allowedTypes = p;
  }
  else
  {
    _typeClause = "";
    _allowedTypes = AnyType;
  }

  VirtualClusterLineEdit::setExtraClause(buildExtraClause());
}

void OrderLineEdit::setId(const int pId, const QString &pType)
{
  if (pId == -1)
    clear();
  else if (pId == _id && pType == _name)
    return;
  else
  {
    OrderTypes oldTypes = _allowedTypes;
    setAllowedType(pType);
    silentSetId(pId);
    setAllowedTypes(oldTypes);
    emit newId(pId, pType);
    emit valid(_valid);
  }
}

QString OrderLineEdit::buildExtraClause()
{
  QStringList clauses;
  if (! _statusClause.isEmpty()) clauses << _statusClause;
  if (! _typeClause.isEmpty())   clauses << _typeClause;
  if (! _allClause.isEmpty())	 clauses << _allClause;
  if (! _poClause.isEmpty())	 clauses << _poClause;
  if (! _raClause.isEmpty())	 clauses << _raClause;
  if (! _soClause.isEmpty())	 clauses << _soClause;
  if (! _toClause.isEmpty())	 clauses << _toClause;

  return clauses.join(" AND ");
}

void OrderLineEdit::silentSetId(const int pId)
{
  if (pId == -1)
    XLineEdit::clear();
  else
  {
    QString oldExtraClause = _extraClause;

    XSqlQuery countQ;
    countQ.prepare("SELECT COUNT(*) AS count FROM orderhead WHERE (TRUE) " + _idClause +
		    (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
		    QString(";"));
    countQ.bindValue(":id", pId);
    countQ.exec();
    if (countQ.first())
    {
      int result = countQ.value("count").toInt();
      if (result <= 0)
      {
	_id = -1;
	XLineEdit::clear();
      }
      else if (result == 1)
	_id = pId;
      else
      {
	_extraClause += "AND (orderhead_id=" + QString::number(pId) + ")";
	sEllipses();
	_extraClause += "AND (orderhead_type='" + type() + "')";
      }
    }
    else if (countQ.lastError().type() != QSqlError::None)
    {
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
			    countQ.lastError().databaseText());
    }

    if (_id > 0)
    {
      XSqlQuery idQ;
      idQ.prepare(_query + _idClause +
		  (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
		  QString(";"));
      idQ.bindValue(":id", pId);
      idQ.exec();
      if (idQ.first())
      {
	_id = pId;
	_valid = true;
	setText(idQ.value("number").toString());
	_name		= idQ.value("name").toString();
	_description	= idQ.value("description").toString();
	_from		= idQ.value("orderhead_from").toString();
	_to		= idQ.value("orderhead_to").toString();
      }
      else if (idQ.lastError().type() != QSqlError::None)
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      idQ.lastError().databaseText());
    }
  }

  _parsed = TRUE;
  emit parsed();
}

// List ///////////////////////////////////////////////////////////////////////

OrderList::OrderList(QWidget *pParent, Qt::WindowFlags pFlags) :
  VirtualList(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose, false);
  QTreeWidgetItem *headerItem = _listTab->headerItem();
  headerItem->setText(1, tr("Order Type"));
  headerItem->setText(2, tr("Status"));

  _listTab->addColumn(tr("From"), -1, Qt::AlignLeft);
  _listTab->addColumn(tr("To"),   -1, Qt::AlignLeft);
}

QString OrderList::type() const
{
  if(selectedAtDone.count() > 0)
  {
    XTreeWidgetItem * item = (XTreeWidgetItem*)selectedAtDone.at(0);
    return item->text(1);
  }

  return "";
}

void OrderList::done(int p)
{
  selectedAtDone = _listTab->selectedItems();
  VirtualList::done(p);
}

// Search /////////////////////////////////////////////////////////////////////

OrderSearch::OrderSearch(QWidget *pParent, Qt::WindowFlags pFlags) :
  VirtualSearch(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose, false);
  QTreeWidgetItem *headerItem = _listTab->headerItem();
  headerItem->setText(1, tr("Order Type"));
  headerItem->setText(2, tr("Status"));

  _listTab->addColumn(tr("From"), -1, Qt::AlignLeft);
  _listTab->addColumn(tr("To"),   -1, Qt::AlignLeft);
}

QString OrderSearch::type() const
{
  QList<QTreeWidgetItem*> items = (selectedAtDone.size() > 0) ? selectedAtDone :
						      _listTab->selectedItems();
  if(items.count() > 0)
  {
    XTreeWidgetItem * item = (XTreeWidgetItem*)items.at(0);
    return item->text(1);
  }

  return "";
}

void OrderSearch::done(int p)
{
  selectedAtDone = _listTab->selectedItems();
  VirtualSearch::done(p);
}
