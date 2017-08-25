/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSQLTABLEMODELPROTO_H
#define __QSQLTABLEMODELPROTO_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlField>
#include <QSqlIndex>
#include <QSqlRecord>
#include <QSqlTableModel>
#include <QtScript>

void setupQSqlTableModelProto(QScriptEngine *engine);

class QSqlTableModelProto : public QObject, public QScriptable
{
    Q_OBJECT

  public:
    QSqlTableModelProto(QObject *parent);

    Q_INVOKABLE void          clear();
    Q_INVOKABLE QVariant      data(const QModelIndex &idx, int role = Qt::DisplayRole) const;
    Q_INVOKABLE QSqlDatabase  database() const;
    Q_INVOKABLE QSqlTableModel::EditStrategy editStrategy() const;
    Q_INVOKABLE int           fieldIndex(const QString &fieldName) const;
    Q_INVOKABLE QString       filter() const;
    Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex &index) const;
    Q_INVOKABLE QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Q_INVOKABLE bool          insertRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE bool          insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool          isDirty() const;
    Q_INVOKABLE bool          isDirty(const QModelIndex &index) const;
    Q_INVOKABLE QSqlIndex     primaryKey() const;
    Q_INVOKABLE QSqlRecord    record() const;
    Q_INVOKABLE QSqlRecord    record(int row) const;
    Q_INVOKABLE bool          removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE bool          removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE void          revertRow(int row);
    Q_INVOKABLE int           rowCount(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE bool          setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE void          setEditStrategy(QSqlTableModel::EditStrategy strategy);
    Q_INVOKABLE void          setFilter(const QString &filter);
    Q_INVOKABLE bool          setRecord(int row, const QSqlRecord &record);
    Q_INVOKABLE void          setSort(int column, Qt::SortOrder order);
    Q_INVOKABLE void          setTable(const QString &tableName);
    Q_INVOKABLE void          sort(int column, Qt::SortOrder order);
    Q_INVOKABLE QString       tableName() const;

    Q_INVOKABLE QString toString() const;

};

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QSqlTableModel*)
#endif
Q_DECLARE_METATYPE(enum QSqlTableModel::EditStrategy)

#endif // __QSQLTABLEMODELPROTO_H
