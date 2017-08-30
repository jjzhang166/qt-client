/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XSQLTABLEMODEL_H
#define XSQLTABLEMODEL_H

#include <QHash>
#include <QSize>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRelationalTableModel>
#include <QStringList>

#include "parameter.h"

class QScriptEngine;
class XSqlTableModel;

class XSqlTableNode : public QObject
{
public:
  XSqlTableNode(const QString tableName, ParameterList relations, XSqlTableNode *parent = 0);
  ~XSqlTableNode();

  int index() const;
  int count() const { return _children.count(); }
  void appendChild(XSqlTableNode *child) { _children.append(child); }
  void removeChild(int index) { _children.removeAt(index); }
  XSqlTableNode* appendChild(const QString &tableName, ParameterList &relations);

  QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* > modelMap() { return _modelMap; }
  QString tableName() const { return _tableName; }
  QList<XSqlTableNode *> children() const { return _children; }
  ParameterList relations() const { return _relations; }
  XSqlTableNode* child(int index) { return _children.at(index); }
  XSqlTableNode* child(const QString &tableName);
  XSqlTableNode* parent() const { return _parent; }
  XSqlTableModel* model(XSqlTableModel* parent = 0, int row = 0);

  void clear();
  void load(QPair<XSqlTableModel*, int> key);
  bool save();

private:
  ParameterList _relations;
  QMap<QPair<XSqlTableModel*, int>, XSqlTableModel* >_modelMap;
  QList<XSqlTableNode *> _children;
  QString _filter;
  QString _tableName;
  XSqlTableNode *_parent;
};

class XSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

  public:
    XSqlTableModel(QObject *parent = 0);
    ~XSqlTableModel();

    enum itemDataRole { FormatRole = (Qt::UserRole + 1),
                  EditorRole,
                  MenuRole, /* Other roles for xtreewidget?
                  RawRole,
                  IdRole,
                  RunningSetRole,
                  RunningInitRole,
                  TotalSetRole,
                  TotalInitRole,
                  IndentRole */
    };

    enum FormatFlags { Money, Qty, Curr, Percent, Cost, QtyPer,
      SalesPrice, PurchPrice, UOMRatio, ExtPrice, Weight
    };

    using QSqlRelationalTableModel::setTable;
    Q_INVOKABLE virtual void          setTable(const QString &tableName, int keyColumns);

    Q_INVOKABLE virtual void          clear();
    Q_INVOKABLE virtual QVariant      data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE virtual bool          insertRows(int row, int count, const QModelIndex &parent = QModelIndex());

    Q_INVOKABLE virtual QSqlError     lastError() const;
    Q_INVOKABLE virtual QSqlQuery     query() const;
    Q_INVOKABLE virtual bool          canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual int           columnCount(const QModelIndex &parent = QModelIndex())  const;
    Q_INVOKABLE virtual void          fetchMore(const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE virtual bool          insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE virtual bool          setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    Q_INVOKABLE virtual bool            dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    Q_INVOKABLE virtual QModelIndex     index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual QModelIndex     sibling(int row, int column, const QModelIndex &index) const;

    Q_INVOKABLE virtual QModelIndex     buddy(const QModelIndex &index) const;
    Q_INVOKABLE virtual bool            hasIndex(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    Q_INVOKABLE virtual bool            insertColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE virtual bool            insertRow(int row, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    Q_INVOKABLE virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const;
    Q_INVOKABLE virtual QMimeData      *mimeData(const QModelIndexList &indexes) const;
    Q_INVOKABLE virtual QStringList     mimeTypes()                              const;
    Q_INVOKABLE virtual bool            removeColumn(int column, const QModelIndex &parent = QModelIndex());
    Q_INVOKABLE virtual bool            setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles);
    Q_INVOKABLE virtual QSize           span(const QModelIndex &index) const;
    Q_INVOKABLE virtual Qt::DropActions supportedDragActions()         const;
    Q_INVOKABLE virtual Qt::DropActions supportedDropActions()         const;


    Q_INVOKABLE virtual QSqlRelation    relation(int column)                     const;
    Q_INVOKABLE virtual QSqlTableModel *relationModel(int column)                const;
    Q_INVOKABLE virtual void            setRelation(int column, const QSqlRelation &relation);



    Q_INVOKABLE void applyColumnRole(int column, int role, QVariant value);
    Q_INVOKABLE void applyColumnRoles();
    Q_INVOKABLE void applyColumnRoles(int row);
    Q_INVOKABLE void setColumnRole(int column, int role, QVariant value);
    Q_INVOKABLE QVariant formatValue(const QVariant &dataValue, const QVariant &formatValue) const;
    Q_INVOKABLE bool select();
    Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Q_INVOKABLE void setKeys(int keyColumns);

    Q_INVOKABLE virtual QString selectStatement() const;

    // Table node functions
    static::QString buildFilter(ParameterList &params);
    static::ParameterList buildParams(XSqlTableModel* parent, int row, ParameterList relations);

    Q_INVOKABLE virtual int            nodeCount() const;
    Q_INVOKABLE virtual void           appendChild(XSqlTableNode *child);
    Q_INVOKABLE virtual void           clearChildren();
    Q_INVOKABLE virtual void           removeChild(int index);
    Q_INVOKABLE virtual XSqlTableNode *appendChild(const QString &tableName, ParameterList &relations);

    Q_INVOKABLE virtual QList<XSqlTableNode *> children();
    Q_INVOKABLE virtual XSqlTableNode *child(int index);
    Q_INVOKABLE virtual XSqlTableNode *child(const QString &tableName);

    Q_INVOKABLE virtual void set(ParameterList params);
    Q_INVOKABLE virtual ParameterList parameters();

    Q_INVOKABLE virtual void load(int row);
    Q_INVOKABLE virtual void loadAll();
    Q_INVOKABLE virtual bool save();
    Q_INVOKABLE virtual QString toString() const;

  private:
    QHash<QPair<QModelIndex, int>, QVariant> roles;
    QMultiHash<int, QPair<QVariant, int> > _columnRoles;
    QList<QString> _locales;

    QList<XSqlTableNode *> _children;
    ParameterList _params;
};

void setupXSqlTableModel(QScriptEngine *engine);

#endif
