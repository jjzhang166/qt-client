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

// TODO: drag/drop support?
// TODO: make a base class for the similarities of this and poitemTableModel

#include "toitemTableModel.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QString>

#include "guiclient.h"
#include "currcluster.h"

ToitemTableModel::ToitemTableModel(QObject * parent, QSqlDatabase db) :
  QSqlRelationalTableModel(parent, db)
{
  setTable("toitem");

  // select statement on which everything else is based
  _selectStatement =
      QString("SELECT tohead_number, "
	     "       item_number,"
	     "       prj_number,"
	     "       CURRENT_DATE AS earliestdate, "
	     "       toitem.* "
	     "FROM tohead"
	     "     JOIN toitem ON (toitem_tohead_id=tohead_id)"
	     "     LEFT OUTER JOIN item ON (toitem_item_id=item_id)"
	     "     LEFT OUTER JOIN prj ON (toitem_prj_id=prj_id)"
	     );

  setEditStrategy(QSqlTableModel::OnManualSubmit); // OnRow?
  setSort(TOITEM_LINENUMBER_COL, Qt::AscendingOrder);

  // insert only those columns not directly part of the toitem table
  insertColumns(0, 4);

  setHeaderData(TOITEM_LINENUMBER_COL,	Qt::Horizontal, tr("#"));
  setHeaderData(ITEM_NUMBER_COL,	Qt::Horizontal, tr("Item"));
  setHeaderData(TOITEM_QTY_ORDERED_COL,	Qt::Horizontal, tr("Qty."));
  setHeaderData(TOITEM_STDCOST_COL,	Qt::Horizontal, tr("Std. Cost"));
  setHeaderData(TOITEM_FREIGHT_COL,	Qt::Horizontal, tr("Freight"));
  setHeaderData(TOITEM_DUEDATE_COL,	Qt::Horizontal, tr("Due Date"));
  setHeaderData(PRJ_NUMBER_COL,		Qt::Horizontal, tr("Project #"));

  _toheadid	= -1;
  _toitemid	= -1;
  findHeadData();
  _dirty = false;

  select();

  connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}

void ToitemTableModel::findHeadData()
{
  _toheadcurrid = CurrDisplay::baseId();
  _toheaddate	= QDate();
  _tostatus	= "O";	// safest default
  _toheadsrcwhsid = -1;

  XSqlQuery toheadq;
  if (_toheadid > 0)
  {
    toheadq.prepare("SELECT * "
		    "FROM tohead "
		    "WHERE (tohead_id=:tohead_id);");
    toheadq.bindValue(":tohead_id", _toheadid);
  }
  else if (_toitemid > 0)
  {
    toheadq.prepare("SELECT * "
		    "FROM tohead "
		    "WHERE tohead_id IN (SELECT toitem_tohead_id "
		    "                    FROM toitem WHERE toitem_id=:toitem_id);");
    toheadq.bindValue(":toitem_id", _toitemid);
  }
  else
    return;

  toheadq.exec();
  if (toheadq.first())
  {
    _toheadcurrid = toheadq.value("tohead_freight_curr_id").toInt();
    _toheaddate   = toheadq.value("tohead_orderdate").toDate();
    _tostatus	  = toheadq.value("tohead_status").toString();
    _toheadsrcwhsid=toheadq.value("tohead_src_warehous_id").toInt();
  }
  else if (toheadq.lastError().type() != QSqlError::None)
  {
    systemError(0, toheadq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void ToitemTableModel::setHeadId(const int pId)
{
  setFilter(QString("toitem_tohead_id=%1").arg(pId));
  _toheadid = pId;
  _toitemid = -1;

  findHeadData();
}

void ToitemTableModel::setItemId(const int pId)
{
  setFilter(QString("toitem_id=%1").arg(pId));
  _toheadid = -1;
  _toitemid = pId;

  findHeadData();
}

void ToitemTableModel::setShipDate(const QDate pDate)
{
  _toshipdate = pDate;
}

void ToitemTableModel::setSrcWhsId(const int pId)
{
  _toheadsrcwhsid = pId;
}

void ToitemTableModel::setTransDate(const QDate pDate)
{
  _toheaddate = pDate;
}

void ToitemTableModel::setCurrId(const int pId)
{
  _toheadcurrid = pId;
}

QString ToitemTableModel::selectStatement() const
{
  return _selectStatement + " WHERE " +
	  QString((filter().isEmpty()) ? " (toitem_tohead_id=-1) " : filter()) +
	  " ORDER BY toitem_linenumber"
	  ;
}

bool ToitemTableModel::select()
{
  bool returnVal = QSqlRelationalTableModel::select();
  if (returnVal)
  {
    insertRow(rowCount());
    _dirty = false;
    connect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
  }
  return returnVal;
}

bool ToitemTableModel::submitAll()
{
  XSqlQuery begin("BEGIN;");
  bool returnVal = QSqlRelationalTableModel::submitAll();
  if (returnVal)
  {
    _dirty = false;
    XSqlQuery commit("COMMIT;");
  }
  else
    XSqlQuery rollback("ROLLBACK;");

  return returnVal;
}

Qt::ItemFlags ToitemTableModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags = QSqlRelationalTableModel::flags(index);
  if (index.column() == TOITEM_STDCOST_COL)
    flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

  return flags;
}

bool ToitemTableModel::isDirty() const
{
  return _dirty;
}

bool ToitemTableModel::removeRow(int row, const QModelIndex & parent)
{
  return QSqlRelationalTableModel::removeRow(row, parent);
}

/*
    Make sure the row is internally consistent. If so then fix it so the
    parent class has a valid row to insert (some of the SELECTed columns
    shouldn't be directly modified in the db 'cause they're not part of the
    model's current table).
*/
bool ToitemTableModel::validRow(QSqlRecord& record)
{
  QString errormsg;
  QString warningmsg;

  if (record.value("item_number").toString().isEmpty())
    errormsg = tr("<p>You must select an Item Number before you may save.");

  else if (record.value("toitem_qty_ordered").toDouble() <= 0)
    errormsg = tr("<p>You must enter a quantity before you may save this "
		  "Purchase Order Item.");

  else if (! record.value("toitem_duedate").toDate().isValid())
    errormsg = tr("<p>You must enter a due date.");

  else if (record.value("toitem_tohead_id").toInt() <= 0 &&
	   _toheadid <= 0)
    errormsg = tr("<p>There is no Transfer Order header yet. "
	     "Try entering ????.");

  int index = record.indexOf("toitem_tohead_id");
  if (index < 0)
  {
    QSqlField field("toitem_tohead_id", QVariant::Int);
    field.setValue(_toheadid);
    record.append(field);
  }
  else
    record.setValue(index, _toheadid);

  XSqlQuery ln;
  ln.prepare("SELECT COUNT(*) + 1 AS newln "
	     "FROM toitem "
	     "WHERE (toitem_tohead_id=:tohead_id);");
  ln.bindValue(":tohead_id", _toheadid);
  if (record.indexOf("toitem_linenumber") < 0)
  {
    ln.exec();
    if (ln.first())
    {
      QSqlField field("toitem_linenumber", QVariant::Int);
      field.setValue(ln.value("newln"));
      record.append(field);
    }
    else if (ln.lastError().type() != QSqlError::None)
    {
      errormsg = ln.lastError().databaseText();
    }
  }
  else if (record.value("toitem_linenumber").toInt() <= 0)
  {
    ln.exec();
    if (ln.first())
      record.setValue("toitem_linenumber", ln.value("newln"));
    else if (ln.lastError().type() != QSqlError::None)
    {
      errormsg = ln.lastError().databaseText();
    }
  }

  if (record.value("toitem_id").isNull())
  {
    XSqlQuery idq("SELECT NEXTVAL('toitem_toitem_id_seq') AS toitem_id;");
    if (idq.first())
      record.setValue("toitem_id", idq.value("toitem_id"));
    else if (idq.lastError().type() != QSqlError::None)
      errormsg = idq.lastError().databaseText();
  }

  if (_tostatus.isEmpty())
    findHeadData();

  index = record.indexOf("toitem_freight_curr_id");
  if (index < 0)
  {
    QSqlField field("toitem_freight_curr_id", QVariant::String);
    field.setValue(_toheadcurrid);
    record.append(field);
  }
  else if (record.field(index).value().toString().isEmpty())
    record.setValue(index, _toheadcurrid);

  index = record.indexOf("toitem_freight");
  if (index >= 0)	// this is different than the others!
  {
    XSqlQuery calcq;
    calcq.prepare("SELECT tax_ratea, tax_rateb, tax_ratec, tax_id,"
		  "       calculateTax(tax_id, COALESCE(:ext, 0.0), 0, 'A') AS tax_a,"
		  "       calculateTax(tax_id, COALESCE(:ext, 0.0), 0, 'B') AS tax_b,"
		  "       calculateTax(tax_id, COALESCE(:ext, 0.0), 0, 'C') AS tax_c "
		  "FROM tohead, tax "
		  "WHERE ((tohead_id=:tohead_id)"
		  "  AND  (tax_id=getFreightTaxSelection(tohead_taxauth_id)));");

    calcq.bindValue(":tohead_id", record.value("toitem_tohead_id"));
    calcq.bindValue(":ext",       record.value("toitem_freight"));
    calcq.exec();
    if (calcq.first() && ! calcq.value("tax_id").isNull())
    {
      if ((index = record.indexOf("toitem_freighttax_id")) < 0)
      {
	QSqlField field("toitem_freighttax_id", QVariant::Int);
	field.setValue(calcq.value("tax_id"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_id"));

      if ((index = record.indexOf("toitem_freighttax_ratea")) < 0)
      {
	QSqlField field("toitem_freighttax_ratea", QVariant::Int);
	field.setValue(calcq.value("tax_a"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_a"));

      if ((index = record.indexOf("toitem_freighttax_rateb")) < 0)
      {
	QSqlField field("toitem_freighttax_rateb", QVariant::Int);
	field.setValue(calcq.value("tax_b"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_b"));

      if ((index = record.indexOf("toitem_freighttax_ratec")) < 0)
      {
	QSqlField field("toitem_freighttax_ratec", QVariant::Int);
	field.setValue(calcq.value("tax_c"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_c"));

      if ((index = record.indexOf("toitem_freighttax_pcta")) < 0)
      {
	QSqlField field("toitem_freighttax_pcta", QVariant::Int);
	field.setValue(calcq.value("tax_ratea"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_ratea"));

      if ((index = record.indexOf("toitem_freighttax_pctb")) < 0)
      {
	QSqlField field("toitem_freighttax_pctb", QVariant::Int);
	field.setValue(calcq.value("tax_rateb"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_rateb"));

      if ((index = record.indexOf("toitem_freighttax_pctc")) < 0)
      {
	QSqlField field("toitem_freighttax_pctc", QVariant::Int);
	field.setValue(calcq.value("tax_ratec"));
	record.append(field);
      }
      else if (record.field(index).value().isNull())
	record.setValue(index, calcq.value("tax_ratec"));
    }
    else if (calcq.lastError().type() != QSqlError::None)
      errormsg = calcq.lastError().databaseText();
  }

  index = record.indexOf("toitem_status");
  if (index < 0)
  {
    QSqlField field("toitem_status", QVariant::String);
    field.setValue(_tostatus);
    record.append(field);
  }
  else if (record.field(index).value().toString().isEmpty())
    record.setValue(index, _tostatus);

  index = record.indexOf("toitem_schedshipdate");
  if (index < 0)
  {
    QSqlField field("toitem_schedshipdate", QVariant::Date);
    field.setValue(_toshipdate);
    record.append(field);
  }
  else if (record.field(index).value().isNull())
    record.setValue(index, _toshipdate);

  if (! errormsg.isEmpty())
  {
    setLastError(QSqlError(QString("ToitemTableModel::validRow() error"),
			   errormsg, QSqlError::UnknownError));
    return false;
  }
  else if (! warningmsg.isEmpty())
  {
    if (QMessageBox::question(0, tr("Are you sure you want to continue?"),
		    warningmsg + tr("<p>Do you wish to Save this Order?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return false;
  }

  record.remove(record.indexOf("prj_number"));
  record.remove(record.indexOf("item_number"));
  record.remove(record.indexOf("tohead_number"));
  record.remove(record.indexOf("earliestdate"));

  return true;
}

bool ToitemTableModel::insertRowIntoTable(const QSqlRecord& record)
{
  if (record.isEmpty())
    return true;

  bool isNull = true;
  for (int i = 0; i < record.count(); i++)
  {
    if (i == record.indexOf("toitem_tohead_id") ||
	(record.value(i).toString().isEmpty() &&
	  (i == record.indexOf("toitem_status") ||
	   i == record.indexOf("toitem_comments") )))
      continue;
    isNull &= record.isNull(i);
  }
  if (isNull)
    return true;

  QSqlRecord newRecord(record);
  if (! validRow(newRecord))
    return false;

  return QSqlRelationalTableModel::insertRowIntoTable(newRecord);
}

bool ToitemTableModel::updateRowInTable(int row, const QSqlRecord& record)
{
  // touch everything so we can distinguish unchanged fields from NULL/0 as new val
  for (int i = 0; i < columnCount(); i++)
    setData(index(row, i), data(index(row, i)));

  QSqlRecord newRecord(record);
  if (! validRow(newRecord))
    return false;

  return QSqlRelationalTableModel::updateRowInTable(row, newRecord);
}

void ToitemTableModel::markDirty(QModelIndex, QModelIndex)
{
  _dirty = true;

  disconnect(this, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this, SLOT(markDirty(QModelIndex, QModelIndex)));
}
