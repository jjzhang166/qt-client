/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QJSONDOCUMENTPROTO_H__
#define __QJSONDOCUMENTPROTO_H__

#include <QtScript>

void setupQJsonDocumentProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QVariant>

Q_DECLARE_METATYPE(QJsonDocument*)

Q_DECLARE_METATYPE(enum QJsonDocument::JsonFormat)
Q_DECLARE_METATYPE(enum QJsonDocument::DataValidation)

QScriptValue constructQJsonDocument(QScriptContext *context, QScriptEngine *engine);

class QJsonDocumentProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QJsonDocumentProto(QObject *parent = 0);
    ~QJsonDocumentProto();

    Q_INVOKABLE QJsonArray            array() const;
    Q_INVOKABLE bool                  isArray() const;
    Q_INVOKABLE bool                  isEmpty() const;
    Q_INVOKABLE bool                  isNull() const;
    Q_INVOKABLE bool                  isObject() const;
    Q_INVOKABLE QJsonObject           object() const;
    Q_INVOKABLE const char           *rawData(int * size) const;
    Q_INVOKABLE void                  setArray(const QJsonArray & array);
    Q_INVOKABLE void                  setObject(const QJsonObject & object);
    Q_INVOKABLE QByteArray            toBinaryData() const;
    Q_INVOKABLE QByteArray            toJson(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const;
    Q_INVOKABLE QVariant              toVariant() const;
    Q_INVOKABLE bool                  operator!=(const QJsonDocument & other) const;
    Q_INVOKABLE QJsonDocument &       operator=(const QJsonDocument & other);
    Q_INVOKABLE bool                  operator==(const QJsonDocument & other) const;

    Q_INVOKABLE QJsonDocument  fromBinaryData(const QByteArray & data, QJsonDocument::DataValidation validation = QJsonDocument::Validate);
    Q_INVOKABLE QJsonDocument  fromJson(const QByteArray & json, QJsonParseError * error = 0);
    Q_INVOKABLE QJsonDocument  fromRawData(const char * data, int size, QJsonDocument::DataValidation validation = QJsonDocument::Validate);
    Q_INVOKABLE QJsonDocument  fromVariant(const QVariant & variant);
};

#endif
#endif
