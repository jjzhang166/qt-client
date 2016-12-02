/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslproto.h"
#include <QScriptValueIterator>

#if QT_VERSION < 0x050000
void setupQSslProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue AlternativeNameEntryTypeToScriptValue(QScriptEngine *engine, const QSsl::AlternativeNameEntryType &item)
{
  return engine->newVariant(item);
}
void AlternativeNameEntryTypeFromScriptValue(const QScriptValue &obj, QSsl::AlternativeNameEntryType &item)
{
  item = (QSsl::AlternativeNameEntryType)obj.toInt32();
}

QScriptValue EncodingFormatToScriptValue(QScriptEngine *engine, const QSsl::EncodingFormat &item)
{
  return engine->newVariant(item);
}
void EncodingFormatFromScriptValue(const QScriptValue &obj, QSsl::EncodingFormat &item)
{
  item = (QSsl::EncodingFormat)obj.toInt32();
}

QScriptValue KeyAlgorithmToScriptValue(QScriptEngine *engine, const QSsl::KeyAlgorithm &item)
{
  return engine->newVariant(item);
}
void KeyAlgorithmFromScriptValue(const QScriptValue &obj, QSsl::KeyAlgorithm &item)
{
  item = (QSsl::KeyAlgorithm)obj.toInt32();
}

QScriptValue KeyTypeToScriptValue(QScriptEngine *engine, const QSsl::KeyType &item)
{
  return engine->newVariant(item);
}
void KeyTypeFromScriptValue(const QScriptValue &obj, QSsl::KeyType &item)
{
  item = (QSsl::KeyType)obj.toInt32();
}

QScriptValue SslOptionToScriptValue(QScriptEngine *engine, const QSsl::SslOption &item)
{
  return engine->newVariant(item);
}
void SslOptionFromScriptValue(const QScriptValue &obj, QSsl::SslOption &item)
{
  item = (QSsl::SslOption)obj.toInt32();
}

QScriptValue SslProtocolToScriptValue(QScriptEngine *engine, const QSsl::SslProtocol &item)
{
  return engine->newVariant(item);
}
void SslProtocolFromScriptValue(const QScriptValue &obj, QSsl::SslProtocol &item)
{
  item = (QSsl::SslProtocol)obj.toInt32();
}

/**
 * Converts `QMultiMap<QSsl::AlternativeNameEntryType, QString>` to Javascritp friendly form:
 *
 *   var myMap = {
 *     0: ["bar@example.com", "baz@example.com"],   // `0` is for enum QSsl::EmailEntry.
 *     1: ["buzz.example.com", "woof.example.com"]  // `1` is for enum QSsl::DnsEntry.
 *   };
 *
 */
// TODO: Figure out how to convert QMultiMap.
/*
QScriptValue QMultiMapQSslAlternativeNameEntryTypeQStringToScriptValue(QScriptEngine *engine, const QMultiMap<QSsl::AlternativeNameEntryType, QString> &map)
{
  QMapIterator<QSsl::AlternativeNameEntryType, QString> it(map);
  QScriptValue newObject = engine->newObject();

  while (it.hasNext()) {
    it.next();
    QScriptValue childArray = engine->newArray();
    QMultiMap<QSsl::AlternativeNameEntryType, QString>::iterator childIt = map.find(it.key());
    int childCount = 0;

    while (childIt != map.end() && childIt.key() == it.key()) {
      childArray.setProperty(childCount, childIt.value());
      childCount++;
      childIt++;
    }
    newObject.setProperty((QSsl::AlternativeNameEntryType)it.key(), childArray);
  }
  return newObject;
}
*/
/**
 * Converts Javascritp friendly form to `QMultiMap<QSsl::AlternativeNameEntryType, QString>`.
 *
 *   var myMap = {
 *     0: ["bar@example.com", "baz@example.com"],   // `0` is for enum QSsl::EmailEntry.
 *     1: ["buzz.example.com", "woof.example.com"]  // `1` is for enum QSsl::DnsEntry.
 *   };
 *
 */
// TODO: Figure out how to convert QMultiMap.
/*
void QMultiMapQSslAlternativeNameEntryTypeQStringFromScriptValue(const QScriptValue &obj, QMultiMap<QSsl::AlternativeNameEntryType, QString> &map)
{
  map = QMultiMap<QSsl::AlternativeNameEntryType, QString>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration) {
      continue;
    }

    QScriptValueIterator childIt(it.value());
    while (childIt.hasNext()) {
      QString item = qscriptvalue_cast<QString>(childIt.value());
      QSsl::AlternativeNameEntryType enumKey = (QSsl::AlternativeNameEntryType)it.name().toInt();
      map.insert(enumKey, item);
    }
  }
}
*/

void setupQSslProto(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  qScriptRegisterMetaType(engine, AlternativeNameEntryTypeToScriptValue, AlternativeNameEntryTypeFromScriptValue);
  obj.setProperty("EmailEntry", QScriptValue(engine, QSsl::EmailEntry), permanent);
  obj.setProperty("DnsEntry",   QScriptValue(engine, QSsl::DnsEntry), permanent);

  qScriptRegisterMetaType(engine, EncodingFormatToScriptValue, EncodingFormatFromScriptValue);
  obj.setProperty("Pem",  QScriptValue(engine, QSsl::Pem), permanent);
  obj.setProperty("Der",  QScriptValue(engine, QSsl::Der), permanent);

  qScriptRegisterMetaType(engine, KeyAlgorithmToScriptValue, KeyAlgorithmFromScriptValue);
  obj.setProperty("Rsa",    QScriptValue(engine, QSsl::Rsa), permanent);
  obj.setProperty("Dsa",    QScriptValue(engine, QSsl::Dsa), permanent);
  obj.setProperty("Ec",     QScriptValue(engine, QSsl::Ec), permanent);
  obj.setProperty("Opaque", QScriptValue(engine, QSsl::Opaque), permanent);

  qScriptRegisterMetaType(engine, KeyTypeToScriptValue, KeyTypeFromScriptValue);
  obj.setProperty("PrivateKey", QScriptValue(engine, QSsl::PrivateKey), permanent);
  obj.setProperty("PublicKey",  QScriptValue(engine, QSsl::PublicKey), permanent);

  qScriptRegisterMetaType(engine, SslOptionToScriptValue, SslOptionFromScriptValue);
  obj.setProperty("SslOptionDisableEmptyFragments",       QScriptValue(engine, QSsl::SslOptionDisableEmptyFragments), permanent);
  obj.setProperty("SslOptionDisableSessionTickets",       QScriptValue(engine, QSsl::SslOptionDisableSessionTickets), permanent);
  obj.setProperty("SslOptionDisableCompression",          QScriptValue(engine, QSsl::SslOptionDisableCompression), permanent);
  obj.setProperty("SslOptionDisableServerNameIndication", QScriptValue(engine, QSsl::SslOptionDisableServerNameIndication), permanent);
  obj.setProperty("SslOptionDisableLegacyRenegotiation",  QScriptValue(engine, QSsl::SslOptionDisableLegacyRenegotiation), permanent);
  obj.setProperty("SslOptionDisableSessionSharing",       QScriptValue(engine, QSsl::SslOptionDisableSessionSharing), permanent);
  obj.setProperty("SslOptionDisableSessionPersistence",   QScriptValue(engine, QSsl::SslOptionDisableSessionPersistence), permanent);

  qScriptRegisterMetaType(engine, SslProtocolToScriptValue, SslProtocolFromScriptValue);
  obj.setProperty("SslV3",            QScriptValue(engine, QSsl::SslV3), permanent);
  obj.setProperty("SslV2",            QScriptValue(engine, QSsl::SslV2), permanent);
  obj.setProperty("TlsV1_0",          QScriptValue(engine, QSsl::TlsV1_0), permanent);
  obj.setProperty("TlsV1_0OrLater",   QScriptValue(engine, QSsl::TlsV1_0OrLater), permanent);
  obj.setProperty("TlsV1_1",          QScriptValue(engine, QSsl::TlsV1_1), permanent);
  obj.setProperty("TlsV1_1OrLater",   QScriptValue(engine, QSsl::TlsV1_1OrLater), permanent);
  obj.setProperty("TlsV1_2",          QScriptValue(engine, QSsl::TlsV1_2), permanent);
  obj.setProperty("TlsV1_2OrLater",   QScriptValue(engine, QSsl::TlsV1_2OrLater), permanent);
  obj.setProperty("UnknownProtocol",  QScriptValue(engine, QSsl::UnknownProtocol), permanent);
  obj.setProperty("AnyProtocol",      QScriptValue(engine, QSsl::AnyProtocol), permanent);
  obj.setProperty("TlsV1SslV3",       QScriptValue(engine, QSsl::TlsV1SslV3), permanent);
  obj.setProperty("SecureProtocols",  QScriptValue(engine, QSsl::SecureProtocols), permanent);

  // TODO: Figure out how to convert QMultiMap.
  //qScriptRegisterMetaType(engine, QMultiMapQSslAlternativeNameEntryTypeQStringToScriptValue, QMultiMapQSslAlternativeNameEntryTypeQStringFromScriptValue);

  engine->globalObject().setProperty("QSsl", obj, permanent);
}

#endif
