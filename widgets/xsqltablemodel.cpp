/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlDriver>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRelation>

#include "format.h"
#include "xsqltablemodel.h"

XSqlTableModel::XSqlTableModel(QObject *parent) : 
  QSqlRelationalTableModel(parent)
{
  _locales << "money" << "qty" << "curr" << "percent" << "cost" << "qtyper" 
    << "salesprice" << "purchprice" << "uomratio" << "extprice" << "weight";
}

XSqlTableModel::~XSqlTableModel()
{
}

bool XSqlTableModel::select()
{
  bool result;
  result = QSqlRelationalTableModel::select();
  applyColumnRoles();
  if (result && rowCount());
    emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1));
  return result;
}

void XSqlTableModel::applyColumnRole(int column, int role, QVariant value)
{
  QSqlQuery qry = query();
  
  // Apply to the model
  qry.first();
  for (int row=0; row < qry.size(); ++row) {
    setData(index(row,column), value, role);
    qry.next();
  }
}

void XSqlTableModel::applyColumnRoles()
{
  QHashIterator<int, QPair<QVariant, int> > i(_columnRoles);
  while (i.hasNext()) {
    applyColumnRole(i.key(), i.value().second, i.value().first );
    i.next();
  }
}

void XSqlTableModel::applyColumnRoles(int row)
{
  QHashIterator<int, QPair<QVariant, int> > i(_columnRoles);
  while (i.hasNext()) {
    setData(index(row,i.key()), i.value().first, i.value().second);
    i.next();
  }
}

void XSqlTableModel::setColumnRole(int column, int role, const QVariant value)
{  
  QPair<QVariant, int> values;
  bool found = false;
  
  // Remove any previous value for this column/role pair
  QMultiHash<int, QPair<QVariant, int> >::iterator i = _columnRoles.find(column);
  while (i != _columnRoles.end() && i.key() == column && !found) {
     if (i.value().second == role) {
       values.first = i.value().first;
       values.second = i.value().second;
       _columnRoles.remove(column, values);
       found = true;
       break;
     }
     ++i;
  }
  
  // Insert new
  values.first = value;
  values.second = role;
  _columnRoles.insert(column, values);
  
  // Apply
  applyColumnRole(column, role, value);
}

void XSqlTableModel::setTable(const QString &tableName, int keyColumns)
{
  QSqlRelationalTableModel::setTable(tableName);
  setKeys(keyColumns);
}

void XSqlTableModel::setKeys(int keyColumns)
{
  if (keyColumns && tableName().length()) {
    QSqlRecord rec = database().record(tableName());
    QSqlIndex idx(tableName());
    for (int i = 0; i < keyColumns; i++)
      idx.append(rec.field(i));
    idx.setName(tableName() + "_pkey");
    QSqlRelationalTableModel::setPrimaryKey(idx);
  }
}

/* qAppendWhereClause() and XSqlTableModel::selectStatement() have been
   copied from qt-mac-commercial-src-4.4.3/src/sql/models/qsqlrelationaltablemodel.cpp
   and hacked to work around Qt bug 171541:
     It would be useful if the QRelationalSqlTableModel would display rows with
     unresolvavble foreign keys
   http://www.qtsoftware.com/developer/task-tracker/index_html?method=entry&id=171541

   Unfortunately the original Qt code is full of references to private data structures,
   so this really is a hack.
   */

static void qAppendWhereClause(QString &query, const QString &clause1, const QString &clause2)
{
    if (clause1.isEmpty() && clause2.isEmpty())
        return;
    if (clause1.isEmpty() || clause2.isEmpty())
        query.append(QLatin1String(" WHERE (")).append(clause1).append(clause2);
    else
        query.append(QLatin1String(" WHERE (")).append(clause1).append(
                        QLatin1String(") AND (")).append(clause2);
    query.append(QLatin1String(") "));
}

QString XSqlTableModel::selectStatement() const
{
    QString query;

    if (tableName().isEmpty())
        return query;

    QString tList;
    QString fList;
    QString where;

    QSqlRecord rec = record();
    QStringList tables;

    // Count how many times each field name occurs in the record
    int fkeycount = 0;
    QHash<QString, int> fieldNames;
    for (int i = 0; i < rec.count(); ++i) {
        QSqlRelation rel = relation(i);
        QString name;
        if (rel.isValid()) {
            // Count the display column name, not the original foreign key
            name = rel.displayColumn();
            fkeycount++;
        }
        else
            name = rec.fieldName(i);
        fieldNames.insert(name, fieldNames.value(name, 0) + 1);
    }
    if (fkeycount == 0)
      return QSqlRelationalTableModel::selectStatement();

    for (int i = 0; i < rec.count(); ++i) {
        QSqlRelation rel = relation(i);
        if (rel.isValid()) {
            QString relTableAlias = QString::fromLatin1("%1_%2")
                                              .arg(rel.tableName()).arg(i);
            fList.append(relTableAlias + "." + rel.displayColumn());
            
            // If there are duplicate field names they must be aliased
            if (fieldNames.value(rel.displayColumn()) > 1)
                fList.append(QString::fromLatin1(" AS %1_%2")
                               .arg(relTableAlias)
                               .arg(rel.displayColumn()));

            fList.append(QLatin1Char(','));
            if (!tables.contains(rel.tableName()))
                tables.append(QString(" LEFT OUTER JOIN %1 %2 ON (%3.%4 = %5.%6) ")
                                 .arg(rel.tableName()) .arg(relTableAlias)
                                 .arg(tableName())     .arg(rec.fieldName(i))
                                 .arg(relTableAlias)   .arg(rel.indexColumn()));
        } else {
            fList.append(tableName() + "." + rec.fieldName(i));
            fList.append(QLatin1Char(','));
        }
    }
    if (!tables.isEmpty())
        tList.append(tables.join(QLatin1String(" "))).append(QLatin1String(" "));
    if (fList.isEmpty())
        return query;
    tList.prepend(QLatin1Char(' ')).prepend(database().driver()->escapeIdentifier(tableName(),
                QSqlDriver::TableName));
    // truncate tailing comma
    tList.chop(1);
    fList.chop(1);
    query.append(QLatin1String("SELECT "));
    query.append(fList).append(QLatin1String(" FROM ")).append(tList);
    if (!where.isEmpty())
        where.chop(5);
    qAppendWhereClause(query, where, filter());

    QString orderBy = orderByClause();
    if (!orderBy.isEmpty())
        query.append(QLatin1Char(' ')).append(orderBy);

    return query;
}

QVariant XSqlTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
      return QVariant();
        
    switch (role) {
    case Qt::DisplayRole: {
      QVariant value = QSqlRelationalTableModel::data(index, Qt::DisplayRole);
      QVariant formatRole = data(index, FormatRole);
      if (formatRole.isValid())
        return formatValue(value, formatRole);
      else if (value.type() == QVariant::Bool)
        return value.toBool() ? tr("Yes") : tr("No");
      else
        return value;
    } break;
    case Qt::EditRole: { 
      return QSqlRelationalTableModel::data(index);
    } break;
    case Qt::TextAlignmentRole:
    case Qt::ForegroundRole:
    case FormatRole:
    case EditorRole:
    case MenuRole:
      QPair<QModelIndex, int> key;
      key.first = index;
      key.second = role;
      if (roles.contains(key))
        return roles.value(key);
    }
    
    return QVariant(); 
}

bool XSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
      return false;
  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole: {
    if (data(index, FormatRole).isValid())
      QSqlRelationalTableModel::setData(index, formatValue(value, data(index, FormatRole)), role);
    else 
      QSqlRelationalTableModel::setData(index, value, role);
  } break;
  case FormatRole: {
    QSqlRelationalTableModel::setData(index, formatValue(data(index), value));
  }
  case Qt::TextAlignmentRole:
  case Qt::ForegroundRole:
  case EditorRole:
  case MenuRole:
    QPair<QModelIndex, int> key;
    key.first = index;
    key.second = role;
    if (roles.contains(key)) {
      if (roles.value(key) == value)
        return true;
    }
    roles.insert(key, value);
    emit dataChanged ( index, index );
  }
  return true;
}

QVariant XSqlTableModel::formatValue(const QVariant &value, const QVariant &format) const
{
  int scale = decimalPlaces(_locales.at(format.toInt()));
  double fval = value.toDouble();
  if (format.toInt() == Percent)
    fval = fval * 100;
  return QVariant(QLocale().toString(fval, 'f', scale));
}
