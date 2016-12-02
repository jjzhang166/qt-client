/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "quuidproto.h"

#if QT_VERSION < 0x050000
void setupQUuidProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue VariantToScriptValue(QScriptEngine *engine, const QUuid::Variant &item)
{
  return engine->newVariant(item);
}
void VariantFromScriptValue(const QScriptValue &obj, QUuid::Variant &item)
{
  item = (QUuid::Variant)obj.toInt32();
}

QScriptValue VersionToScriptValue(QScriptEngine *engine, const QUuid::Version &item)
{
  return engine->newVariant(item);
}
void VersionFromScriptValue(const QScriptValue &obj, QUuid::Version &item)
{
  item = (QUuid::Version)obj.toInt32();
}

QScriptValue createUuidForJS(QScriptContext* /*context*/, QScriptEngine* engine)
{
  return engine->toScriptValue(QUuid::createUuid());
}

QScriptValue createUuidV3ForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2) {
    QUuid ns = qscriptvalue_cast<QUuid>(context->argument(0));
    if (context->argument(1).isString()) {
      return engine->toScriptValue(QUuid::createUuidV3(ns, context->argument(1).toString()));
    } else {
      return engine->toScriptValue(QUuid::createUuidV3(ns, context->argument(1).toVariant().toByteArray()));
    }
  } else {
    return engine->undefinedValue();
  }
}

QScriptValue createUuidV5ForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2) {
    QUuid ns = qscriptvalue_cast<QUuid>(context->argument(0));
    if (context->argument(1).isString()) {
      return engine->toScriptValue(QUuid::createUuidV5(ns, context->argument(1).toString()));
    } else {
      return engine->toScriptValue(QUuid::createUuidV5(ns, context->argument(1).toVariant().toByteArray()));
    }
  } else {
    return engine->undefinedValue();
  }
}

QScriptValue fromRfc4122ForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    return engine->toScriptValue(QUuid::fromRfc4122(context->argument(0).toVariant().toByteArray()));
  } else {
    return engine->undefinedValue();
  }
}

void setupQUuidProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QUuidProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QUuid*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QUuid>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQUuid, proto);
  engine->globalObject().setProperty("QUuid", constructor);

  qScriptRegisterMetaType(engine, VariantToScriptValue, VariantFromScriptValue);
  constructor.setProperty("VarUnknown", QScriptValue(engine, QUuid::VarUnknown), permanent);
  constructor.setProperty("NCS", QScriptValue(engine, QUuid::NCS), permanent);
  constructor.setProperty("DCE", QScriptValue(engine, QUuid::DCE), permanent);
  constructor.setProperty("Microsoft", QScriptValue(engine, QUuid::Microsoft), permanent);
  constructor.setProperty("Reserved", QScriptValue(engine, QUuid::Reserved), permanent);

  qScriptRegisterMetaType(engine, VersionToScriptValue, VersionFromScriptValue);
  constructor.setProperty("VerUnknown", QScriptValue(engine, QUuid::VerUnknown), permanent);
  constructor.setProperty("Time", QScriptValue(engine, QUuid::Time), permanent);
  constructor.setProperty("EmbeddedPOSIX", QScriptValue(engine, QUuid::EmbeddedPOSIX), permanent);
  constructor.setProperty("Name", QScriptValue(engine, QUuid::Name), permanent);
  constructor.setProperty("Md5", QScriptValue(engine, QUuid::Md5), permanent);
  constructor.setProperty("Random", QScriptValue(engine, QUuid::Random), permanent);
  constructor.setProperty("Sha1", QScriptValue(engine, QUuid::Sha1), permanent);

  QScriptValue createUuid = engine->newFunction(createUuidForJS);
  constructor.setProperty("createUuid", createUuid);
  QScriptValue createUuidV3 = engine->newFunction(createUuidV3ForJS);
  constructor.setProperty("createUuidV3", createUuidV3);
  QScriptValue createUuidV5 = engine->newFunction(createUuidV5ForJS);
  constructor.setProperty("createUuidV5", createUuidV5);
  QScriptValue fromRfc4122 = engine->newFunction(fromRfc4122ForJS);
  constructor.setProperty("fromRfc4122", fromRfc4122);
}

QScriptValue constructQUuid(QScriptContext *context, QScriptEngine  *engine)
{
  QUuid *obj = 0;
  if (context->argumentCount() == 11) {
    // TODO: How to do these conversions?
    /*
    uint l = context->argument(0).toUInt32();
    ushort w1 = context->argument(1).toUInt16();
    ushort w2 = context->argument(2).toUInt16();
    // QScriptValue does not have a .toUInt8() `unsigned char`, so...
    uchar b1;
    memcpy(b1, context->argument(3).toString().toStdString().c_str(), context->argument(3).toString().size());
    uchar b2;
    memcpy(b2, context->argument(4).toString().toStdString().c_str(), context->argument(4).toString().size());
    uchar b3;
    memcpy(b3, context->argument(5).toString().toStdString().c_str(), context->argument(5).toString().size());
    uchar b4;
    memcpy(b4, context->argument(6).toString().toStdString().c_str(), context->argument(6).toString().size());
    uchar b5;
    memcpy(b5, context->argument(7).toString().toStdString().c_str(), context->argument(7).toString().size());
    uchar b6;
    memcpy(b6, context->argument(8).toString().toStdString().c_str(), context->argument(8).toString().size());
    uchar b7;
    memcpy(b7, context->argument(9).toString().toStdString().c_str(), context->argument(9).toString().size());
    uchar b8;
    memcpy(b8, context->argument(10).toString().toStdString().c_str(), context->argument(10).toString().size());

    obj = new QUuid(l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8);
    */
  } else if (context->argumentCount() == 1) {
    if (context->argument(0).isString()) {
      obj = new QUuid(context->argument(0).toString());
    } else {
      QByteArray text = qscriptvalue_cast<QByteArray>(context->argument(0));
      obj = new QUuid(text);
    }
  } else {
    obj = new QUuid();
  }

  return engine->toScriptValue(obj);
}

QUuidProto::QUuidProto(QObject *parent) : QObject(parent)
{
}
QUuidProto::~QUuidProto()
{
}

bool QUuidProto::isNull() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QByteArray QUuidProto::toByteArray() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toByteArray();
  return QByteArray();
}

QByteArray QUuidProto::toRfc4122() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toRfc4122();
  return QByteArray();
}

QString QUuidProto::toString() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->toString();
  return QString();
}

QUuid::Variant QUuidProto::variant() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->variant();
  return QUuid::Variant();
}

QUuid::Version QUuidProto::version() const
{
  QUuid *item = qscriptvalue_cast<QUuid*>(thisObject());
  if (item)
    return item->version();
  return QUuid::Version();
}

#endif
