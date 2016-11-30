/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsocketprotocolproto.h"

#if QT_VERSION >= 0x050000
QScriptValue CloseCodeToScriptValue(QScriptEngine *engine, const QWebSocketProtocol::CloseCode &item)
{
  return engine->newVariant(item);
}
void CloseCodeFromScriptValue(const QScriptValue &obj, QWebSocketProtocol::CloseCode &item)
{
  item = (QWebSocketProtocol::CloseCode)obj.toInt32();
}

QScriptValue VersionToScriptValue(QScriptEngine *engine, const QWebSocketProtocol::Version &item)
{
  return engine->newVariant(item);
}
void VersionFromScriptValue(const QScriptValue &obj, QWebSocketProtocol::Version &item)
{
  item = (QWebSocketProtocol::Version)obj.toInt32();
}
#endif

void setupQWebSocketProtocolProto(QScriptEngine *engine)
{
  QScriptValue obj = engine->newObject();
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

#if QT_VERSION >= 0x050000
  qScriptRegisterMetaType(engine, CloseCodeToScriptValue, CloseCodeFromScriptValue);
  qScriptRegisterMetaType(engine, VersionToScriptValue, VersionFromScriptValue);

  obj.setProperty("CloseCodeNormal",                 QScriptValue(engine, QWebSocketProtocol::CloseCodeNormal),                permanent);
  obj.setProperty("CloseCodeGoingAway",              QScriptValue(engine, QWebSocketProtocol::CloseCodeGoingAway),             permanent);
  obj.setProperty("CloseCodeProtocolError",          QScriptValue(engine, QWebSocketProtocol::CloseCodeProtocolError),         permanent);
  obj.setProperty("CloseCodeDatatypeNotSupported",   QScriptValue(engine, QWebSocketProtocol::CloseCodeDatatypeNotSupported),  permanent);
  obj.setProperty("CloseCodeReserved1004",           QScriptValue(engine, QWebSocketProtocol::CloseCodeReserved1004),          permanent);
  obj.setProperty("CloseCodeMissingStatusCode",      QScriptValue(engine, QWebSocketProtocol::CloseCodeMissingStatusCode),     permanent);
  obj.setProperty("CloseCodeAbnormalDisconnection",  QScriptValue(engine, QWebSocketProtocol::CloseCodeAbnormalDisconnection), permanent);
  obj.setProperty("CloseCodeWrongDatatype",          QScriptValue(engine, QWebSocketProtocol::CloseCodeWrongDatatype),         permanent);
  obj.setProperty("CloseCodePolicyViolated",         QScriptValue(engine, QWebSocketProtocol::CloseCodePolicyViolated),        permanent);
  obj.setProperty("CloseCodeTooMuchData",            QScriptValue(engine, QWebSocketProtocol::CloseCodeTooMuchData),           permanent);
  obj.setProperty("CloseCodeMissingExtension",       QScriptValue(engine, QWebSocketProtocol::CloseCodeMissingExtension),      permanent);
  obj.setProperty("CloseCodeBadOperation",           QScriptValue(engine, QWebSocketProtocol::CloseCodeBadOperation),          permanent);
  obj.setProperty("CloseCodeTlsHandshakeFailed",     QScriptValue(engine, QWebSocketProtocol::CloseCodeTlsHandshakeFailed),    permanent);

  obj.setProperty("VersionUnknown",  QScriptValue(engine, QWebSocketProtocol::VersionUnknown), permanent);
  obj.setProperty("Version0",        QScriptValue(engine, QWebSocketProtocol::Version0),       permanent);
  obj.setProperty("Version4",        QScriptValue(engine, QWebSocketProtocol::Version4),       permanent);
  obj.setProperty("Version5",        QScriptValue(engine, QWebSocketProtocol::Version5),       permanent);
  obj.setProperty("Version6",        QScriptValue(engine, QWebSocketProtocol::Version6),       permanent);
  obj.setProperty("Version7",        QScriptValue(engine, QWebSocketProtocol::Version7),       permanent);
  obj.setProperty("Version8",        QScriptValue(engine, QWebSocketProtocol::Version8),       permanent);
  obj.setProperty("Version13",       QScriptValue(engine, QWebSocketProtocol::Version13),      permanent);
  obj.setProperty("VersionLatest",   QScriptValue(engine, QWebSocketProtocol::VersionLatest),  permanent);
#endif

  engine->globalObject().setProperty("QWebSocketProtocol", obj, permanent);
}
