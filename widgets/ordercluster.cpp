/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>

#include "ordercluster.h"
#include "xsqlquery.h"

#define DEBUG false

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

bool OrderCluster::fromSitePrivsEnforced() const
{
  return ((OrderLineEdit*)_number)->fromSitePrivsEnforced();
}

bool OrderCluster::toSitePrivsEnforced() const
{
  return ((OrderLineEdit*)_number)->toSitePrivsEnforced();
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

void OrderCluster::setAllowedStatuses(const int p)
{
  ((OrderLineEdit*)_number)->setAllowedStatuses((OrderLineEdit::OrderStatuses)p);
}

void OrderCluster::setFromSitePrivsEnforced(const bool p)
{
  ((OrderLineEdit*)_number)->setFromSitePrivsEnforced(p);
}

void OrderCluster::setToSitePrivsEnforced(const bool p)
{
  ((OrderLineEdit*)_number)->setFromSitePrivsEnforced(p);
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

void OrderCluster::setAllowedTypes(const int p)
{
  setAllowedTypes((OrderLineEdit::OrderTypes)p);
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

  _toPrivs=false;
  _fromPrivs=false;
  
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
  if (DEBUG)
    qDebug("VCLE %s::sParse() entered with _parsed %d and text() %s",
           qPrintable(objectName()), _parsed, qPrintable(text()));

    if (! _parsed)
    {
      QString stripped = text().trimmed().toUpper();
      if (stripped.length() == 0)
      {
        _parsed = TRUE;
        setId(-1);
      }
      else
      {
        XSqlQuery countQ;
        countQ.prepare("SELECT COUNT(*) AS count FROM orderhead WHERE (TRUE) " + _numClause +
                        (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                        (_hasActive ? _activeClause : "" ) + QString(";"));
        countQ.bindValue(":number", "^" + stripped);
        countQ.exec(); 
        if(countQ.first())
        {
          int result = countQ.value("count").toInt();
          if (result <= 0)
            clear();
          else if (result == 1)
          {

            XSqlQuery numQ;
            numQ.prepare(_query + _numClause +
                        (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                        (_hasActive ? _activeClause : "" ) +
                        QString("ORDER BY %1 LIMIT 1;").arg(_numColName));
            numQ.bindValue(":number", "^" + stripped);
            numQ.exec();
            if (numQ.first())
            {
                _valid = true;
                setId(numQ.value("id").toInt(), numQ.value("name").toString());
                _name = (numQ.value("name").toString());
                _description = numQ.value("description").toString();
                _from = numQ.value("orderhead_from").toString();
                _to	= numQ.value("orderhead_to").toString();
            }
            else
            {
              clear();
              if (numQ.lastError().type() != QSqlError::NoError)
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                                .arg(__FILE__)
                                                .arg(__LINE__),
                          numQ.lastError().databaseText());
            }
          }
          else
          {
            _extraClause += "AND (orderhead_number='" + stripped + "')";
            sEllipses();
            _extraClause += "AND (orderhead_type='" + type() + "')";
          }
        }
        else
        {
          clear();
          if (countQ.lastError().type() != QSqlError::NoError)
              QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                            .arg(__FILE__)
                                            .arg(__LINE__),
                      countQ.lastError().databaseText());
        }
      }
      emit valid(_valid);
      emit parsed();
      emit numberChanged(text(), _name);
    }
    _parsed = TRUE;
}

void OrderLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  OrderList* newdlg = new OrderList(this);
  if (newdlg)
  {
    int id = newdlg->exec();
    if(id != -1)
      setId(id, newdlg->type());
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined").arg(objectName()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void OrderLineEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  OrderSearch* newdlg = new OrderSearch(this);
  if (newdlg)
  {
    QString stripped = text().trimmed();
    if(stripped.length())
    {
      XSqlQuery numQ;
      numQ.prepare(_query + _numClause +
                   (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                   (_hasActive ? _activeClause : "" ) +
                   QString("ORDER BY %1;").arg(_numColName));
      numQ.bindValue(":number", "^" + stripped);
      numQ.exec();
      if (numQ.first())
        newdlg->setQuery(numQ);
    }
    newdlg->setSearchText(text());
    int id = newdlg->exec();
    if(id != -1)
      setId(id, newdlg->type());
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sSearch() not yet defined").arg(objectName()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
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
    _poClause = " (((orderhead_type='PO') AND (" + pClause + ")) OR (orderhead_type != 'PO')) ";
  else if (pType == "RA")
    _raClause = " (((orderhead_type='RA') AND (" + pClause + ")) OR (orderhead_type != 'RA')) ";
  else if (pType == "SO")
    _soClause = " (((orderhead_type='SO') AND (" + pClause + ")) OR (orderhead_type != 'SO')) ";
  else if (pType == "TO")
    _toClause = " (((orderhead_type='TO') AND (" + pClause + ")) OR (orderhead_type != 'TO')) ";
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

void OrderLineEdit::setToSitePrivsEnforced(const bool p)
{
  if (_toPrivs == p)
    return;
  
  _toPrivs=p;
  if (p)
    _toPrivsClause = " AND ((orderhead_type='TO' "
                     " AND orderhead_to_id IN (SELECT warehous_id FROM site()))"
                     " OR (orderhead_type != 'TO'))";
  else
    _toPrivsClause.clear();
}

void OrderLineEdit::setFromSitePrivsEnforced(const bool p)
{
  if (_fromPrivs == p)
    return;
  
  _fromPrivs=p;
  if (p)
    _fromPrivsClause = " AND ((orderhead_type='TO' "
                     " AND orderhead_from_id IN (SELECT warehous_id FROM site()))"
                     " OR (orderhead_type != 'TO'))";
  else
    _fromPrivsClause.clear();
}

VirtualList * OrderLineEdit::listFactory()
{
  return new OrderList(this);
}

VirtualSearch * OrderLineEdit::searchFactory()
{
  return new OrderSearch(this);
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
    if(!pType.isNull())
      setAllowedType(pType);
    silentSetId(pId);
    if(!pType.isNull())
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

  QString tmpClause = clauses.join(" AND ");
  if (DEBUG) qDebug("buildExtraClause returning %s",
                    qPrintable(tmpClause));
  return tmpClause;
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
    else if (countQ.lastError().type() != QSqlError::NoError)
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
      else if (idQ.lastError().type() != QSqlError::NoError)
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

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  QTreeWidgetItem *headerItem = _listTab->headerItem();
  headerItem->setText(1, tr("Order Type"));
  headerItem->setText(2, tr("Status"));

  _listTab->addColumn(tr("From"), -1, Qt::AlignLeft, true, "orderhead_from");
  _listTab->addColumn(tr("To"),   -1, Qt::AlignLeft, true, "orderhead_to");
  
  if (! ((OrderLineEdit*)_parent)->fromPrivsClause().isEmpty())
     ((OrderLineEdit*)_parent)->setExtraClause(((OrderLineEdit*)_parent)->extraClause() + ((OrderLineEdit*)_parent)->fromPrivsClause());
  if (! ((OrderLineEdit*)_parent)->toPrivsClause().isEmpty())
     ((OrderLineEdit*)_parent)->setExtraClause(((OrderLineEdit*)_parent)->extraClause() + ((OrderLineEdit*)_parent)->toPrivsClause());
    
  sFillList();
}

void OrderList::sClose()
{
  done(-1);
}

QString OrderList::type() const
{
  if(selectedAtDone.count() > 0)
  {
    return selectedAtDone.at(0)->text(1);
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
  
  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );
  
  _searchName->setText(tr("Search through Order Type"));
  _searchDescrip->setText(tr("Search through Status"));

  _listTab->addColumn(tr("From"), -1, Qt::AlignLeft, true, "orderhead_from");
  _listTab->addColumn(tr("To"),   -1, Qt::AlignLeft, true, "orderhead_to");
  
  if (! ((OrderLineEdit*)_parent)->fromPrivsClause().isEmpty())
     ((OrderLineEdit*)_parent)->setExtraClause(((OrderLineEdit*)_parent)->extraClause() + ((OrderLineEdit*)_parent)->fromPrivsClause());
  if (! ((OrderLineEdit*)_parent)->toPrivsClause().isEmpty())
     ((OrderLineEdit*)_parent)->setExtraClause(((OrderLineEdit*)_parent)->extraClause() + ((OrderLineEdit*)_parent)->toPrivsClause());
       
  sFillList();
}

void OrderSearch::sClose()
{
  done(-1);
}

QString OrderSearch::type() const
{
  QList<XTreeWidgetItem*> items = (selectedAtDone.size() > 0) ? selectedAtDone :
						      _listTab->selectedItems();
  if(items.count() > 0)
    return items.at(0)->text(1);

  return "";
}

void OrderSearch::done(int p)
{
  selectedAtDone = _listTab->selectedItems();
  VirtualSearch::done(p);
}
