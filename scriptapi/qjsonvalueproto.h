/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QJSONVALUEPROTO_H__
#define __QJSONVALUEPROTO_H__

#include <QtScript>
void setupQJsonValueProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QVariant>

Q_DECLARE_METATYPE(QJsonValue)
Q_DECLARE_METATYPE(QJsonValue*)
Q_DECLARE_METATYPE(enum QJsonValue::Type)

QScriptValue constructQJsonValue(QScriptContext *context, QScriptEngine *engine);

class QJsonValueProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QJsonValueProto(QObject *parent = 0);
    ~QJsonValueProto();

    Q_INVOKABLE bool              isArray() const;
    Q_INVOKABLE bool              isBool() const;
    Q_INVOKABLE bool              isDouble() const;
    Q_INVOKABLE bool              isNull() const;
    Q_INVOKABLE bool              isObject() const;
    Q_INVOKABLE bool              isString() const;
    Q_INVOKABLE bool              isUndefined() const;
    Q_INVOKABLE QJsonArray        toArray(const QJsonArray & defaultValue) const;
    Q_INVOKABLE QJsonArray        toArray() const;
    Q_INVOKABLE bool              toBool(bool defaultValue = false) const;
    Q_INVOKABLE double            toDouble(double defaultValue = 0) const;
    Q_INVOKABLE int               toInt(int defaultValue = 0) const;
    Q_INVOKABLE QJsonObject       toObject(const QJsonObject & defaultValue) const;
    Q_INVOKABLE QJsonObject       toObject() const;
    Q_INVOKABLE QString           toString(const QString & defaultValue = QString()) const;
    Q_INVOKABLE QVariant          toVariant() const;
    Q_INVOKABLE QJsonValue::Type  type() const;
    Q_INVOKABLE bool              operator!=(const QJsonValue & other) const;
    Q_INVOKABLE QJsonValue       &operator=(const QJsonValue & other);
    Q_INVOKABLE bool              operator==(const QJsonValue & other) const;

    Q_INVOKABLE QJsonValue        fromVariant(const QVariant & variant);
};

#endif
#endif
