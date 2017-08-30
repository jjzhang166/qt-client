/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QDATAWIDGETMAPPERPROTO_H__
#define __QDATAWIDGETMAPPERPROTO_H__

#include <QObject>
#include <QDataWidgetMapper>
#include <QtScript>

class QDataWidgetMapperProto : public QObject, public QScriptable
{
    Q_OBJECT

  public:
    QDataWidgetMapperProto(QObject *parent = 0);

    Q_INVOKABLE void                   addMapping(QWidget *widget, int section);
    Q_INVOKABLE void                   addMapping(QWidget *widget, int section, const QByteArray &propertyName);
    Q_INVOKABLE void                   clearMapping();
    Q_INVOKABLE QAbstractItemDelegate *itemDelegate() const;
    Q_INVOKABLE QByteArray             mappedPropertyName(QWidget *widget) const;
    Q_INVOKABLE int                    mappedSection(QWidget *widget) const;
    Q_INVOKABLE QWidget               *mappedWidgetAt(int section) const;
    Q_INVOKABLE QAbstractItemModel    *model() const;
    Q_INVOKABLE void                   removeMapping(QWidget *widget);
    Q_INVOKABLE void                   setItemDelegate(QAbstractItemDelegate *delegate);
    Q_INVOKABLE void                   setModel(QAbstractItemModel *model);
    Q_INVOKABLE void                   setRootIndex(const QModelIndex &index);
    Q_INVOKABLE QModelIndex            rootIndex() const;
    Q_INVOKABLE QString                toString() const;

};

#if QT_VERSION < 0x050000
Q_DECLARE_METATYPE(QDataWidgetMapper*)
#endif
Q_DECLARE_METATYPE(enum QDataWidgetMapper::SubmitPolicy)

void setupQDataWidgetMapperProto(QScriptEngine *engine);

#endif // __QDATAWIDGETMAPPERPROTO_H__
