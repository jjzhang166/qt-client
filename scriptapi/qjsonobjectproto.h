/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QJSONOBJECTPROTO_H__
#define __QJSONOBJECTPROTO_H__

#include <QtScript>
void setupQJsonObjectProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QJsonValue>
#include <QJsonValueRef>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantHash>
#include <QVariantMap>

// uncomment the following when/if we need to iterate; better to just use JS
// #define Use_QJsonObjectIterators

Q_DECLARE_METATYPE(QJsonObject*)

#ifdef Use_QJsonObjectIterators
Q_DECLARE_METATYPE(QJsonObject::iterator)
Q_DECLARE_METATYPE(QJsonObject::const_iterator)
#endif

QScriptValue constructQJsonObject(QScriptContext *context, QScriptEngine *engine);
QScriptValue QJsonObjectToScriptValue(QScriptEngine *engine, QJsonObject* const &in);
void QJsonObjectFromScriptValue(const QScriptValue &obj, QJsonObject* &out);

class QJsonObjectProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QJsonObjectProto(QObject *parent = 0);
    ~QJsonObjectProto();

#ifdef Use_QJsonObjectIterators
    Q_INVOKABLE QJsonObject::iterator         begin();
    Q_INVOKABLE QJsonObject::const_iterator   begin() const;
    Q_INVOKABLE QJsonObject::const_iterator   constBegin() const;
    Q_INVOKABLE QJsonObject::const_iterator   constEnd() const;
    Q_INVOKABLE QJsonObject::const_iterator   constFind(const QString & key) const;
#endif
    Q_INVOKABLE bool                          contains(const QString & key) const;
    Q_INVOKABLE int                           count() const;
    Q_INVOKABLE bool                          empty() const;
#ifdef Use_QJsonObjectIterators
    Q_INVOKABLE QJsonObject::iterator         end();
    Q_INVOKABLE QJsonObject::const_iterator   end() const;
    Q_INVOKABLE QJsonObject::iterator         erase(QJsonObject::iterator it);
    Q_INVOKABLE QJsonObject::iterator         find(const QString & key);
    Q_INVOKABLE QJsonObject::const_iterator   find(const QString & key) const;
    Q_INVOKABLE QJsonObject::iterator         insert(const QString & key, const QJsonValue & value);
#endif
    Q_INVOKABLE bool                          isEmpty() const;
    Q_INVOKABLE QStringList                   keys() const;
    Q_INVOKABLE int                           length() const;
    Q_INVOKABLE void                          remove(const QString & key);
    Q_INVOKABLE int                           size() const;
    Q_INVOKABLE QJsonValue                    take(const QString & key);
    Q_INVOKABLE QVariantHash                  toVariantHash() const;
    Q_INVOKABLE QVariantMap                   toVariantMap() const;
    Q_INVOKABLE QJsonValue                    value(const QString & key) const;
    Q_INVOKABLE bool                          operator!=(const QJsonObject & other) const;
    Q_INVOKABLE QJsonObject                  &operator=(const QJsonObject & other);
    Q_INVOKABLE bool                          operator==(const QJsonObject & other) const;
    Q_INVOKABLE QJsonValue                    operator[](const QString & key) const;
    Q_INVOKABLE QJsonValueRef                 operator[](const QString & key);
};

#endif
#endif
