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

#include <QGridLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QVBoxLayout>

#include "xsqlquery.h"
#include "xlineedit.h"
#include "xcheckbox.h"

#include "virtualCluster.h"

#define DEBUG false

void VirtualCluster::init()
{
  if (DEBUG)
    qDebug("VC %s::init()", qPrintable(objectName()));

    _number = NULL;	// initially there is none

    _label = new QLabel(this, "_label");
    _label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    _list = new QPushButton(tr("..."), this, "_list");
    _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

#ifndef Q_WS_MAC
	_list->setMaximumWidth(25);
#else
    _list->setMinimumWidth(60);
    _list->setMinimumHeight(32);
#endif
   
    _info = new QPushButton(tr("?"), this, "_info");
    _info->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
	_info->setMaximumWidth(25);
#else
    _info->setMinimumWidth(60);
    _info->setMinimumHeight(32);
#endif


    _name = new QLabel(this, "_name");
    _name->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    _name->setVisible(false);

    _description = new QLabel(this, "_description");
    _description->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    _grid = new QGridLayout(this);
    _grid->setMargin(0);
    _grid->setSpacing(6);
    _grid->addWidget(_label,  0, 0);
    _grid->addWidget(_list,   0, 2);
    _grid->addWidget(_info,   0, 3);
    _grid->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding), 0, 4);
    _grid->addWidget(_name,   1, 1, 1, -1);
    _grid->addWidget(_description, 2, 1, 1, -1);

    _grid->setColumnMinimumWidth(0, 0);
    _grid->setColumnStretch(1, 1);	// let number/name/descrip stretch
    _grid->setColumnMinimumWidth(2, 0);
    _grid->setColumnMinimumWidth(3, 0);

    _grid->setRowMinimumHeight(0, 0);
    _grid->setRowMinimumHeight(1, 0);
    _grid->setRowMinimumHeight(2, 0);
}

VirtualCluster::VirtualCluster(QWidget* pParent, const char* pName) :
    QWidget(pParent, pName)
{
  setObjectName(pName ? pName : "VirtualCluster");
  init();
}

VirtualCluster::VirtualCluster(QWidget* pParent,
			       VirtualClusterLineEdit* pNumberWidget,
			       const char* pName) :
    QWidget(pParent, pName)
{
    init();
    if (pNumberWidget)
	addNumberWidget(pNumberWidget);
}

void VirtualCluster::clear()
{
  if (DEBUG)
    qDebug("VC %s::clear()", qPrintable(objectName()));

  _number->clear();
  _name->clear();
  _description->clear();
}

void VirtualCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_number, _fieldName, QByteArray("number"));
  _number->_mapper=m;
}

void VirtualCluster::setEnabled(const bool p)
{
    QList<QWidget*> child = findChildren<QWidget*>();
    for (int i = 0; i < child.size(); i++)
    {
      if (child[i]->inherits("VirtualCluster"))
	((VirtualCluster*)(child[i]))->setEnabled(p);
      else
	child[i]->setEnabled(p || child[i]->inherits("QLabel"));
    }
    _info->setEnabled(true);
}

void VirtualCluster::setStrict(bool b)
{
  _number->setStrict(b);
}

void VirtualCluster::addNumberWidget(VirtualClusterLineEdit* pNumberWidget)
{
    _number = pNumberWidget;
    if (! _number)
      return;

    _grid->addWidget(_number, 0, 1);
    setFocusProxy(pNumberWidget);

    connect(_list,	SIGNAL(clicked()),	_number, SLOT(sEllipses()));
    connect(_info,	SIGNAL(clicked()),	_number, SLOT(sInfo()));
    connect(_number,	SIGNAL(newId(int)),	this,	 SIGNAL(newId(int)));
    connect(_number,	SIGNAL(parsed()), 	this, 	 SLOT(sRefresh()));
    connect(_number,	SIGNAL(valid(bool)),	this,	 SIGNAL(valid(bool)));
}

void VirtualCluster::sRefresh()
{
  if (DEBUG)
    qDebug("VC %s::sRefresh() with id %d",
           qPrintable(objectName()), _number ? _number->_id : -1);
  _name->setText(_number->_name);
  _description->setText(_number->_description);
  _info->setEnabled(_number && _number->_id > 0);
}

void VirtualCluster::setReadOnly(bool b)
{
  _readOnly = b;
  if (b)
  {
    _number->setEnabled(false);
    _list->hide();
  }
  else
  {
    _number->setEnabled(true);
    _list->show();
  }
}

///////////////////////////////////////////////////////////////////////////

VirtualClusterLineEdit::VirtualClusterLineEdit(QWidget* pParent,
					       const char* pTabName,
					       const char* pIdColumn,
					       const char* pNumberColumn,
					       const char* pNameColumn,
					       const char* pDescripColumn,
					       const char* pExtra,
					       const char* pName) :
    XLineEdit(pParent, pName)
{
    if (DEBUG)
      qDebug("VirtualClusterLineEdit(%p, %s, %s, %s, %s, %s, %s, %s)",
             pParent ? pParent : 0,         pTabName ? pTabName : "",
             pIdColumn ? pIdColumn : "",     pNumberColumn ? pNumberColumn : "",
             pNameColumn ? pNameColumn : "",
             pDescripColumn ? pDescripColumn : "",
             pExtra ? pExtra : "",           pName ? pName : "");

    setObjectName(pName ? pName : "VirtualClusterLineEdit");

    _valid  = false;
    _parsed = true;
    _strict = true;

    setTableAndColumnNames(pTabName, pIdColumn, pNumberColumn, pNameColumn, pDescripColumn);

    if (pExtra && QString(pExtra).stripWhiteSpace().length())
	_extraClause = pExtra;

    connect(this, SIGNAL(lostFocus()),		this, SLOT(sParse()));
    connect(this, SIGNAL(requestInfo()),	this, SLOT(sInfo()));
    connect(this, SIGNAL(requestList()),	this, SLOT(sList()));
    connect(this, SIGNAL(requestSearch()),	this, SLOT(sSearch()));

    setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMaximumWidth(100);

    clear();
    _titleSingular = tr("Object");
    _titlePlural = tr("Objects");
    
    _mapper = new XDataWidgetMapper(this);
}

void VirtualClusterLineEdit::setTableAndColumnNames(const char* pTabName,
						    const char* pIdColumn,
						    const char* pNumberColumn,
						    const char* pNameColumn,
						    const char* pDescripColumn)
{
  _idColName = QString(pIdColumn);
  _numColName = QString(pNumberColumn);
  _nameColName = QString(pNameColumn);
  _descripColName = QString(pDescripColumn);

  _query = QString("SELECT %1 AS id, %2 AS number ")
		  .arg(pIdColumn).arg(pNumberColumn);

  _hasName = (pNameColumn && QString(pNameColumn).stripWhiteSpace().length());
  if (_hasName)
    _query += QString(", %1 AS name ").arg(pNameColumn);

  _hasDescription = (pDescripColumn &&
		     QString(pDescripColumn).stripWhiteSpace().length());
  if (_hasDescription)
       _query += QString(", %1 AS description ").arg(pDescripColumn);

  _query += QString("FROM %1 WHERE (TRUE) ").arg(pTabName);

  _idClause = QString(" AND (%1=:id) ").arg(pIdColumn);
  _numClause = QString(" AND (%1=:number) ").arg(pNumberColumn);

  _extraClause = "";
}

void VirtualClusterLineEdit::setTitles(const QString& s, const QString& p)
{
    _titleSingular = s;
    _titlePlural = !p.isEmpty() ? p : s;
}

void VirtualClusterLineEdit::clear()
{
    if (DEBUG)
      qDebug("VCLE %s::clear()", qPrintable(objectName()));

    int oldid = _id;
    bool oldvalid = _valid;

    XLineEdit::clear();
    _description = "";
    _name = "";
    _id = -1;	// calling setId() or silentSetId() is recursive
    _valid = false;
    if (oldvalid != _valid)
      emit valid(_valid);
    if (oldid != _id)
      emit newId(_id);
}

void VirtualClusterLineEdit::sEllipses()
{
    if (_x_preferences && _x_preferences->value("DefaultEllipsesAction") == "search")
	sSearch();
    else
	sList();
}

void VirtualClusterLineEdit::setId(const int pId)
{
  if (DEBUG)
    qDebug("VCLE %s::setId(%d)", qPrintable(objectName()), pId);

    if (pId == -1)
	clear();
    else if (pId == _id)
	return;
    else
    {
      silentSetId(pId);
      emit newId(pId);
      emit valid(_valid);
    }
}

void VirtualClusterLineEdit::silentSetId(const int pId)
{
  if (DEBUG)
    qDebug("VCLE %s::silentSetId(%d)", qPrintable(objectName()), pId);

    if (pId == -1)
	XLineEdit::clear();
    else
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
	    if (_hasName)
	      _name = (idQ.value("name").toString());
	    if (_hasDescription)
	      _description = idQ.value("description").toString();
            if (_mapper->model() &&
                _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
              _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());
	}
	else if (idQ.lastError().type() != QSqlError::None)
	    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					  .arg(__FILE__)
					  .arg(__LINE__),
				  idQ.lastError().databaseText());
    }

    _parsed = TRUE;
    emit parsed();
}

void VirtualClusterLineEdit::setNumber(const QString& pNumber)
{
    _parsed = false;
    setText(pNumber);
    sParse();
}

void VirtualClusterLineEdit::sParse()
{
  if (DEBUG)
    qDebug("VCLE %s::sParse() entered with _parsed %d and text() %s",
           qPrintable(objectName()), _parsed, qPrintable(text()));

    if (! _parsed)
    {
      QString stripped = text().stripWhiteSpace().upper();
      if (stripped.length() == 0)
      {
	_parsed = TRUE;
	setId(-1);
      }
      else
      {
	XSqlQuery numQ;
	numQ.prepare(_query + _numClause +
		    (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
		    QString(";"));
	numQ.bindValue(":number", stripped);
	numQ.exec();
	if (numQ.first())
	{
	    _valid = true;
	    setId(numQ.value("id").toInt());
	    if (_hasName)
	      _name = (numQ.value("name").toString());
	    if (_hasDescription)
		_description = numQ.value("description").toString();
	}
	else
	{
	    clear();
	    if (numQ.lastError().type() != QSqlError::None)
		QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					      .arg(__FILE__)
					      .arg(__LINE__),
			numQ.lastError().databaseText());
	}
      }
    }

    _parsed = TRUE;
    emit valid(_valid);
    emit parsed();
}

void VirtualClusterLineEdit::sList()
{
    VirtualList* newdlg = listFactory();
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("%1::sList() not yet defined").arg(className()));
}

void VirtualClusterLineEdit::sSearch()
{
    VirtualSearch* newdlg = searchFactory();
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("%1::sSearch() not yet defined").arg(className()));
}

void VirtualClusterLineEdit::sInfo()
{
    VirtualInfo* newdlg = infoFactory();
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("%1::sInfo() not yet defined").arg(className()));
}

void VirtualClusterLineEdit::setStrict(bool b)
{
  _strict = b;
}

VirtualInfo* VirtualClusterLineEdit::infoFactory()
{
    return new VirtualInfo(this);
}

VirtualList* VirtualClusterLineEdit::listFactory()
{
    return new VirtualList(this);
}

VirtualSearch* VirtualClusterLineEdit::searchFactory()
{
    return new VirtualSearch(this);
}

///////////////////////////////////////////////////////////////////////////////

void VirtualList::init()
{
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);
    _search	= new QLineEdit(this, "_search");
    _searchLit	= new QLabel(_search, tr("S&earch for:"), this, "_searchLit");
    _close	= new QPushButton(tr("&Cancel"), this, "_close");
    _select	= new QPushButton(tr("&Select"), this, "_select");
    _listTab	= new XTreeWidget(this);
    _titleLit	= new QLabel(_listTab, "", this, "_titleLit");

    _listTab->setName("_listTab");

    _searchLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _select->setEnabled(false);
    _select->setAutoDefault(true);
    _select->setDefault(true);
    _listTab->setMinimumHeight(250);
    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    QVBoxLayout* dialogLyt    = new QVBoxLayout(this,      5, -1, "dialogLyt");
    QHBoxLayout* topLyt	      = new QHBoxLayout(dialogLyt,    -1, "topLyt");
    QVBoxLayout* searchLyt    = new QVBoxLayout(topLyt,       -1, "searchLyt");
    topLyt->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
					    QSizePolicy::Minimum));
    QVBoxLayout* buttonsLyt   = new QVBoxLayout(topLyt,       -1, "buttonsLyt");
    QHBoxLayout* searchStrLyt = new QHBoxLayout(searchLyt,    -1, "searchStrLyt");
    QVBoxLayout* tableLyt     = new QVBoxLayout(dialogLyt,    -1, "tableLyt");

    searchStrLyt->addWidget(_searchLit);
    searchStrLyt->addWidget(_search);
    buttonsLyt->addWidget(_close);
    buttonsLyt->addWidget(_select);

    tableLyt->addWidget(_titleLit);
    tableLyt->addWidget(_listTab);

    ((QBoxLayout*)topLyt)->setStretchFactor(searchLyt, 0);
    ((QBoxLayout*)dialogLyt)->setStretchFactor(topLyt, 0);
    ((QBoxLayout*)dialogLyt)->setStretchFactor(tableLyt, 1);

    connect(_close,   SIGNAL(clicked()),	 this,   SLOT(sClose()));
    connect(_listTab, SIGNAL(itemSelected(int)), this,	 SLOT(sSelect()));
    connect(_listTab, SIGNAL(valid(bool)),     _select, SLOT(setEnabled(bool)));
    connect(_search,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_select,  SIGNAL(clicked()),         this,	 SLOT(sSelect()));
}

VirtualList::VirtualList() : QDialog()
{
  init();
}

VirtualList::VirtualList(QWidget* pParent, Qt::WindowFlags pFlags ) :
    QDialog(pParent, pFlags)
{
    init();

    setObjectName("virtualList");

    _listTab->addColumn(tr("Number"), -1, Qt::AlignLeft);

    if (pParent->inherits("VirtualClusterLineEdit"))
    {
      _parent = (VirtualClusterLineEdit*)(pParent);
      setCaption(_parent->_titlePlural);
      if (_parent->_hasName)
      {
	_listTab->addColumn(tr("Name"),	 -1, Qt::AlignLeft);
	_listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      if (_parent->_hasDescription)
      {
	  _listTab->addColumn(tr("Description"),  -1, Qt::AlignLeft);
	  _listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      _id = _parent->_id;
      _titleLit->setText(_parent->_titlePlural);
    }
    else
      _parent = 0;

    sFillList();
}

void VirtualList::sClose()
{
    done(_id);
}

void VirtualList::sSelect()
{
    done(_listTab->id());
}

void VirtualList::sSearch(const QString& pTarget)
{
  QList<QTreeWidgetItem*> matches = _listTab->findItems(pTarget, Qt::MatchStartsWith);

  if (matches.size() > 0)
  {
    _listTab->setCurrentItem(matches[0]);
    _listTab->scrollToItem(matches[0]);
  }
}

void VirtualList::sFillList()
{
    if (! _parent)
      return;

    _listTab->clear();
    XSqlQuery query(_parent->_query +
		    (_parent->_extraClause.isEmpty() ? "" :
					    " AND " + _parent->_extraClause) +
		    QString(" ORDER BY ") +
		    QString((_parent->_hasName) ? "name" : "number"));
    if (query.first())
    {
	XTreeWidgetItem *last = NULL;
	do
	{
	    if (_parent->_hasName && _parent->_hasDescription)
		last = new XTreeWidgetItem(_listTab, last, query.value("id").toInt(),
					 query.value("number"),
					 query.value("name").toString(),
					 query.value("description").toString());
	    else if (_parent->_hasDescription)
		last = new XTreeWidgetItem(_listTab, last, query.value("id").toInt(),
					 query.value("number"),
					 query.value("description").toString());
	    else if (_parent->_hasName)
		last = new XTreeWidgetItem(_listTab, last, query.value("id").toInt(),
					 query.value("number"),
					 query.value("name").toString());
	    else
		last = new XTreeWidgetItem(_listTab, last, query.value("id").toInt(),
					 query.value("number"));
	} while (query.next());
    }
}

///////////////////////////////////////////////////////////////////////////////

VirtualSearch::VirtualSearch(QWidget* pParent, Qt::WindowFlags pFlags) :
    QDialog(pParent, pFlags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
    setObjectName("virtualSearch");

    _search = new QLineEdit(this, "_search");
    _searchLit = new QLabel(_search, tr("S&earch for:"), this, "_searchLit");
    _searchNumber = new XCheckBox(tr("Search through Numbers"));
    _searchNumber->setObjectName("_searchNumber");
    _searchName = new XCheckBox(tr("Search through Names"));
    _searchName->setObjectName("_searchName");
    _searchDescrip = new XCheckBox(tr("Search through Descriptions"));
    _searchDescrip->setObjectName("_searchDescrip");
    _close = new QPushButton(tr("&Cancel"), this, "_close");
    _select = new QPushButton(tr("&Select"), this, "_select");
    _listTab = new XTreeWidget(this);
    _titleLit = new QLabel(_listTab, "", this, "_titleLit");

    _listTab->setName("_listTab");

    _searchLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _select->setEnabled(false);
    _select->setAutoDefault(true);
    _select->setDefault(true);
    _listTab->setMinimumHeight(250);
    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    dialogLyt    = new QVBoxLayout(this,      5, -1, "dialogLyt");
    QHBoxLayout* topLyt = new QHBoxLayout(dialogLyt, -1, "topLyt");
    searchLyt    = new QVBoxLayout(topLyt,    -1, "searchLyt");
    topLyt->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
					    QSizePolicy::Minimum));
    buttonsLyt   = new QVBoxLayout(topLyt,    -1, "buttonsLyt");
    searchStrLyt = new QHBoxLayout(searchLyt, -1, "searchStrLyt");
    selectorsLyt = new QGridLayout(searchLyt,  1, 1, -1, "selectorsLyt");
    tableLyt     = new QVBoxLayout(dialogLyt, -1, "tableLyt");

    searchStrLyt->addWidget(_searchLit);
    searchStrLyt->addWidget(_search);
    selectorsLyt->addWidget(_searchNumber,  0, 0);
    selectorsLyt->addWidget(_searchName,    1, 0);
    selectorsLyt->addWidget(_searchDescrip, 2, 0);
    buttonsLyt->addWidget(_close);
    buttonsLyt->addWidget(_select);
    buttonsLyt->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum,
						QSizePolicy::Expanding));
    tableLyt->addWidget(_titleLit);
    tableLyt->addWidget(_listTab);

    ((QBoxLayout*)topLyt)->setStretchFactor(searchLyt, 0);
    ((QBoxLayout*)dialogLyt)->setStretchFactor(topLyt, 0);
    ((QBoxLayout*)dialogLyt)->setStretchFactor(tableLyt, 1);

    connect(_listTab,	    SIGNAL(itemSelected(int)),	this, SLOT(sSelect()));
    connect(_select,	    SIGNAL(clicked()),		this, SLOT(sSelect()));
    connect(_close,	    SIGNAL(clicked()),		this, SLOT(sClose()));
    connect(_searchNumber,  SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
    connect(_searchDescrip, SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
    connect(_search,	    SIGNAL(lostFocus()),	this, SLOT(sFillList()));
    connect(_listTab,	    SIGNAL(valid(bool)),	_select, SLOT(setEnabled(bool)));

    _listTab->addColumn(tr("Number"), -1, Qt::AlignLeft);

    if (pParent->inherits("VirtualClusterLineEdit"))
    {
      _parent = (VirtualClusterLineEdit*)(pParent);
      setCaption(_parent->_titlePlural);
      if (_parent->_hasName)
      {
	_listTab->addColumn(tr("Name"),	 -1, Qt::AlignLeft);
	_listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      if (_parent->_hasDescription)
      {
	  _listTab->addColumn(tr("Description"),  -1, Qt::AlignLeft);
	  _listTab->setColumnWidth(_listTab->columnCount() - 1, 100);
      }

      _searchName->setHidden(! _parent->_hasName);
      _searchDescrip->setHidden(! _parent->_hasDescription);
      _id = _parent->_id;
      _titleLit->setText(_parent->_titlePlural);
    }
    else
      _parent = 0;

    sFillList();
}

void VirtualSearch::sClose()
{
    done(_id);
}

void VirtualSearch::sSelect()
{
    done(_listTab->id());
}

void VirtualSearch::sFillList()
{
    if (! _parent)
      return;

    _listTab->clear();

    _search->setText(_search->text().stripWhiteSpace().upper());
    if (_search->text().length() == 0)
	return;

    XSqlQuery qry(_parent->_query +
		    (_parent->_extraClause.isEmpty() ? "" :
					    " AND " + _parent->_extraClause) +
		    QString(" ORDER BY ") +
		    QString((_parent->_hasName) ? "name" : "number"));
    if (qry.first())
    {
	XTreeWidgetItem *last = NULL;
	bool addToList;
	do
	{
	    addToList = FALSE;
	    if ((_searchNumber->isChecked()) &&
		(qry.value("number").toString().contains(_search->text(), FALSE)))
		addToList = TRUE;
	    if (_parent->_hasName &&
	        (_searchName->isChecked()) &&
		(qry.value("name").toString().contains(_search->text(), FALSE)))
		addToList = TRUE;
	    if (_parent->_hasDescription &&
		(_searchDescrip->isChecked()) &&
		(qry.value("description").toString().contains(_search->text(), FALSE)))
		addToList = TRUE;
	    if (addToList)
	    {
	      if (_parent->_hasName && _parent->_hasDescription)
		  last = new XTreeWidgetItem(_listTab, last, qry.value("id").toInt(),
					   qry.value("number"),
					   qry.value("name").toString(),
					   qry.value("description").toString());
	      else if (_parent->_hasDescription)
		  last = new XTreeWidgetItem(_listTab, last, qry.value("id").toInt(),
					   qry.value("number"),
					   qry.value("description").toString());
	      else if (_parent->_hasName)
		  last = new XTreeWidgetItem(_listTab, last, qry.value("id").toInt(),
					   qry.value("number"),
					   qry.value("name").toString());
	      else
		  last = new XTreeWidgetItem(_listTab, last, qry.value("id").toInt(),
					   qry.value("number"));
	    }
	}
	while (qry.next());
    }
}

///////////////////////////////////////////////////////////////////////////////

VirtualInfo::VirtualInfo(QWidget* pParent, Qt::WindowFlags pFlags) :
    QDialog(pParent, pFlags)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowModality(Qt::WindowModal);
    _parent = (VirtualClusterLineEdit*)(pParent);
    setObjectName("virtualInfo");
    setCaption(_parent->_titleSingular);
    _id = _parent->_id;
    
    _titleLit	= new QLabel(_parent->_titleSingular, this, "_titleLit");
    _numberLit	= new QLabel(tr("Number:"), this, "_numberLit");
    _number	= new QLabel(this, "_number");
    _nameLit	= new QLabel(tr("Name:"), this, "_nameLit");
    _name	= new QLabel(this, "_name");
    _descripLit	= new QLabel(tr("Description:"), this, "_descripLit");
    _descrip	= new QLabel(this, "_descrip");

    _titleLit->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    _numberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    _nameLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    _close	= new QPushButton(tr("&Close"), this, "_close");
    _close->setDefault(true);

    QHBoxLayout* dialogLyt = new QHBoxLayout(this, 5, -1, "dialogLyt");
    QVBoxLayout* titleLyt  = new QVBoxLayout(dialogLyt, -1, "titleLyt");
    titleLyt->addWidget(_titleLit);
    QHBoxLayout* dataLyt   = new QHBoxLayout(titleLyt,  -1, "mainLyt");
    QVBoxLayout* litLyt	   = new QVBoxLayout(dataLyt,   -1, "litLyt");
    litLyt->addWidget(_numberLit);
    litLyt->addWidget(_nameLit);
    litLyt->addWidget(_descripLit);
    QVBoxLayout* infoLyt   = new QVBoxLayout(dataLyt,   -1, "infoLyt");
    infoLyt->addWidget(_number);
    infoLyt->addWidget(_name);
    infoLyt->addWidget(_descrip);
    QSpacerItem* dataHtSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum,
						QSizePolicy::Expanding);
    titleLyt->addItem(dataHtSpacer);
    QHBoxLayout* buttonLyt = new QHBoxLayout(dialogLyt, -1, "buttonLyt");
    QSpacerItem* wdSpacer  = new QSpacerItem(20, 20, QSizePolicy::Minimum,
					     QSizePolicy::Expanding);
    buttonLyt->addItem(wdSpacer);
    QVBoxLayout* buttonColLyt = new QVBoxLayout(buttonLyt, -1, "buttonColLyt");

    buttonColLyt->addWidget(_close);
    QSpacerItem* htSpacer  = new QSpacerItem(20, 20, QSizePolicy::Minimum,
					     QSizePolicy::Expanding);
    buttonColLyt->addItem(htSpacer);

    connect(_close, SIGNAL(clicked()), this, SLOT(close()));

    _nameLit->setHidden(!_parent->_hasName);
    _name->setHidden(!_parent->_hasName);
    _descripLit->setHidden(!_parent->_hasDescription);
    _descrip->setHidden(!_parent->_hasDescription);

    sPopulate();
}

void VirtualInfo::sPopulate()
{
    XSqlQuery qry;
    qry.prepare(_parent->_query + _parent->_idClause + ";");
    qry.bindValue(":id", _parent->_id);
    qry.exec();
    if (qry.first())
    {
	_number->setText(qry.value("number").toString());
	if (_parent->_hasName)
	  _name->setText(qry.value("name").toString());
	if (_parent->_hasDescription)
	    _descrip->setText(qry.value("description").toString());
    }
    else if (qry.lastError().type() != QSqlError::None)
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					  .arg(__FILE__)
					  .arg(__LINE__),
				  qry.lastError().databaseText());
}
