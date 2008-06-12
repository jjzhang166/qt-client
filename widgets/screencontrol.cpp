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

#include "screencontrol.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>

#define DEBUG true

ScreenControl::ScreenControl(QWidget *parent) : 
  QWidget(parent)
{
  setupUi(this);
  _autoSave=false;
  _searchType=Query;
  _shown=false;
  _listReportName=QString();
  _print->setVisible(! _listReportName.isEmpty());
  
  _model = new XSqlTableModel;
  
  connect (_new,	SIGNAL(clicked()),	this,	SLOT(newRow()));
  connect (_save,       SIGNAL(clicked()),      this,   SIGNAL(saveClicked()));
  connect (_save,	SIGNAL(clicked()),	this,	SLOT(save()));
  connect (_print,	SIGNAL(clicked()),	this,	SIGNAL(printClicked()));
  connect (_print,      SIGNAL(clicked()),      this,   SLOT(print()));
  connect (_prev,	SIGNAL(clicked()),	this,	SLOT(toPrevious()));
  connect (_next,	SIGNAL(clicked()),	this,	SLOT(toNext()));
  connect (_search,	SIGNAL(clicked()),	this,	SLOT(search()));
  connect (_model,      SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(enableSave()));
  
  _view->setVisible(FALSE);
  _save->setEnabled(false);

  _model->setEditStrategy(QSqlTableModel::OnManualSubmit);
}

ScreenControl::~ScreenControl()
{
  qDebug("check desctroyed");
  if (isDirty())
    if (QMessageBox::question(this, tr("Unsaved work"),
                                   tr("Would you like to save your work so you do not lose your changes?"),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No ) == QMessageBox::Yes)
      save();
}

ScreenControl::Modes ScreenControl::mode()
{
  return _mode;
}

ScreenControl::SearchTypes ScreenControl::searchType()
{
  return _searchType;
}

bool ScreenControl::isDirty()
{
  for (int i = 0; i < _model->columnCount()-1; i++)
    if (_model->isDirty(_model->index(_mapper.currentIndex(),i)))
      return true;
  return false;
}

void ScreenControl::enableSave()
{
  _save->setEnabled(true);
}

void ScreenControl::languageChange()
{
    retranslateUi(this);
}

/* When the widget is first shown, set up table mappings if they exist*/
void ScreenControl::showEvent(QShowEvent *event)
{
  if (DEBUG)
    qDebug("ScreenControl::showEvent() entered with mode = %d and shown %d",
           _mode, _shown);
  if(!_shown)
  {
    _shown = true;
    setTable(_schemaName, _tableName);

    // now set the sort order
    for (int i = 0; i < _model->columnCount(); i++)
    { 
      if (_model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString() == _sortColumn)
      {
        _model->setSort(i, Qt::AscendingOrder);
        break;
      }
    }

    if (_mode != New)
      search();
  }
  QWidget::showEvent(event);
  if (DEBUG)
    qDebug("ScreenControl::showEvent() returning");
}

void ScreenControl::toNext()
{
  emit movingNext();
  _mapper.toNext();
  _prev->setEnabled(true);
  _next->setEnabled(_mapper.currentIndex() < _model->rowCount()-1);
  emit movedNext();
}

void ScreenControl::toPrevious()
{
  emit movingPrev();
  _mapper.toPrevious();
  _next->setEnabled(true);
  _prev->setEnabled(_mapper.currentIndex());
  emit movedPrev();
}

void ScreenControl::newRow()
{
  qDebug("insert row %d", _model->insertRows(_model->rowCount(),1));
  _mapper.toLast();
  _prev->setEnabled(_model->rowCount()-1);
  _next->setEnabled(false);
}

void ScreenControl::print()
{
  // TODO: how do we choose between list and form print?
  printList();
}

void ScreenControl::printList()
{
  orReport report(_listReportName);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void ScreenControl::save()
{
  emit saving();
  int i=_mapper.currentIndex();
  _mapper.submit();
  _model->submitAll();
  _mapper.setCurrentIndex(i);
  qDebug("error %d",_model->lastError().isValid());
  if (_model->lastError().type() != QSqlError::NoError && _model->lastError().driverText() != "No Fields to update")
  {
    QMessageBox::critical(this, tr("Error Saving %1").arg(_tableName),
                          tr("Error saving %1 at %2::%3\n\n%4")
                          .arg(_tableName).arg(__FILE__).arg(__LINE__)
                          .arg(_model->lastError().databaseText()));
    return;
  }
  if (_mode==New)
    newRow();
  _save->setEnabled(false);
  emit saved(true);
}

void ScreenControl::search()
{
  if (DEBUG)
    qDebug("ScreenControl::search() entered with _searchText '%s'",
           _searchText->text().toAscii().data());
  QString filter;
  if (_searchText->text().stripWhiteSpace().length())
  {
    QStringList parts;
    for (int i = 0; i < _model->columnCount(); i++)
    { 
      if (_model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString().length())
        parts.append("(CAST(" +
                     _model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString() +
                     " AS TEXT) ~* '" + _searchText->text() + "')");
    }
    filter=parts.join(" OR ");
  }
  if (DEBUG)
    qDebug("ScreenControl::search() filter = %s", filter.toAscii().data());
  setFilter(filter);
  select();
  if (DEBUG)
    qDebug("ScreenControl::search() returning");
}

void ScreenControl::select()
{
  _model->select();
  _new->setEnabled(_mode!=View);
  if (_model->rowCount())
  {
    _mapper.toFirst();
    //_save->setEnabled(_mode!=View);
    _view->setEnabled(true);
    _next->setEnabled(_model->rowCount() > 1);
    emit newModel(_model);
  }
  else
  {
    _view->setEnabled(false);
    _next->setEnabled(false);
    _prev->setEnabled(false);
    _next->setEnabled(false);
  }
}

void ScreenControl::setFilter(QString p)
{
  _model->setFilter(p);
}

void ScreenControl::setMode(Modes p)
{
  if (DEBUG)
    qDebug("ScreenControl::setMode(%d)", p);

  if (p==New)
  {
    _mode=New;
    if (_new)  _new->setEnabled(true);
    newRow();
  }
  else if (p==Edit)
  {
    _mode=Edit;
    if (_new)  _new->setEnabled(true);
  }
  else
  {
    _mode=View;
    if (_new)  _new->setEnabled(false);
    if (_save) _save->setEnabled(false);
  }

  if (DEBUG)
    qDebug("ScreenControl::setMode() about to enable/disable widgets ");

  for (int i = 0; _mapper.model() && i < _mapper.model()->columnCount(); i++)
    if (_mapper.mappedWidgetAt(i))
      _mapper.mappedWidgetAt(i)->setEnabled(_mode != View);

  if (DEBUG)
    qDebug("ScreenControl::setMode() returning");
}

void ScreenControl::setSearchType(SearchTypes p)
{
  _searchType=p;
  //to do: set button to change label between list and query
}

void ScreenControl::setSortColumn(QString p)
{
  _sortColumn = p;
  // _model->setSort is called in setVisible
}

/* Pass in a schema name and a table name.  If schema name is blank, it will be ignored.
     If there is a table name to set, data widget mappings will be set as well. */
void ScreenControl::setTable(QString s, QString t)
{
  if (t.length())
  {
    QString tableName="";
    if (s.length())
      tableName=s + ".";
    tableName+=t;
    if (_model->tableName() != tableName)
    {
      //QSqlIndex _index;
      //_model->setPrimaryKey(_index);
      _model->setTable(tableName);
      setDataWidgetMapper(_model);
      _search->setEnabled(true);
      _searchText->setEnabled(true);
      emit newModel(_model);
    }
  }
  else
  {
    _search->setEnabled(false);
    _searchText->setEnabled(false);
  }
}

void ScreenControl::setDataWidgetMapper(QSqlTableModel *p)
{
  _mapper.setSqlTableModel(p);
  emit newDataWidgetMapper(&_mapper);
}
