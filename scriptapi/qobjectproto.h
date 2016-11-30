/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QOBJECTPROTO_H__
#define __QOBJECTPROTO_H__

#include <QObject>
#include <QtScript>

Q_DECLARE_METATYPE(QObject*)

void setupQObjectProto(QScriptEngine *engine);
QScriptValue constructQObject(QScriptContext *context, QScriptEngine *engine);

class QObjectProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY (QString objectName            READ objectName     WRITE setObjectName)

  public:
    QObjectProto(QObject *parent);
    virtual ~QObjectProto();

    Q_INVOKABLE bool                        blockSignals(bool block);
    Q_INVOKABLE const QObjectList&          children() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE QMetaObject::Connection     connect(const QObject * sender, const char * signal, const char * method, Qt::ConnectionType type = Qt::AutoConnection) const;
#endif
    Q_INVOKABLE bool                        disconnect(const char * signal = 0, const QObject * receiver = 0, const char * method = 0) const;
    Q_INVOKABLE bool                        disconnect(const QObject * receiver, const char * method = 0) const;
    Q_INVOKABLE void                        dumpObjectInfo();
    Q_INVOKABLE void                        dumpObjectTree();
    Q_INVOKABLE QList<QByteArray>           dynamicPropertyNames() const;
    Q_INVOKABLE bool                        event(QEvent * e);
    Q_INVOKABLE bool                        eventFilter(QObject * watched, QEvent * event);
    // TODO: Does not work. `T` does not have a type
    /*
    Q_INVOKABLE T                           findChild(const QString & name = QString(), Qt::FindChildOptions options = Qt::FindChildrenRecursively) const;
    Q_INVOKABLE QList<T>                    findChildren(const QString & name = QString(), Qt::FindChildOptions options = Qt::FindChildrenRecursively) const;
    Q_INVOKABLE QList<T>                    findChildren(const QRegExp & regExp, Qt::FindChildOptions options = Qt::FindChildrenRecursively) const;
    Q_INVOKABLE QList<T>                    findChildren(const QRegularExpression & re, Qt::FindChildOptions options = Qt::FindChildrenRecursively) const;
    */
    Q_INVOKABLE bool                        inherits(const char * className) const;
    Q_INVOKABLE void                        installEventFilter(QObject * filterObj);
    Q_INVOKABLE bool                        isWidgetType() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE bool                        isWindowType() const;
#endif
    Q_INVOKABLE void                        killTimer(int id);
    // TODO: Does not work. `virtual const QMetaObject* QObjectProto::metaObject() const` cannot be overloaded
    //Q_INVOKABLE const QMetaObject*          metaObject() const;
    Q_INVOKABLE void                        moveToThread(QThread * targetThread);
    Q_INVOKABLE QString                     objectName() const;
    Q_INVOKABLE QObject*                    parent() const;
    Q_INVOKABLE QVariant                    property(const char * name) const;
    Q_INVOKABLE void                        removeEventFilter(QObject * obj);
    Q_INVOKABLE void                        setObjectName(const QString & name);
    Q_INVOKABLE void                        setParent(QObject * parent);
    Q_INVOKABLE bool                        setProperty(const char * name, const QVariant & value);
    Q_INVOKABLE bool                        signalsBlocked() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE int                         startTimer(int interval, Qt::TimerType timerType = Qt::CoarseTimer);
#endif
    Q_INVOKABLE QThread*                    thread() const;

    Q_INVOKABLE QString                     toString() const;

  public slots:
    Q_INVOKABLE void                        deleteLater();

  signals:
    void                                    destroyed(QObject * obj = 0);
    void                                    objectNameChanged(const QString & objectName);

};

Q_SCRIPT_DECLARE_QMETAOBJECT(QObjectProto, QObject*)

#endif
