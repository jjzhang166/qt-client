/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scriptapi_internal.h"
#include "qserialportinfoproto.h"

#define DEBUG false

#if QT_VERSION < 0x050100
void setupQSerialPortInfoProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

static QScriptValue qserialportinfo_availablePorts(QScriptContext * /*context*/, QScriptEngine *engine)
{
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    QScriptValue scriptlist = engine->newArray(ports.size());
    for (int i = 0; i < ports.size(); i += 1) {
        QScriptValue proplist = engine->newObject();
        proplist.setProperty("description",            ports.at(i).description());
        proplist.setProperty("hasProductIdentifier",   ports.at(i).hasProductIdentifier());
        proplist.setProperty("hasVendorIdentifier",    ports.at(i).hasVendorIdentifier());
        proplist.setProperty("isBusy",                 ports.at(i).isBusy());
        proplist.setProperty("isNull",                 ports.at(i).isNull());
        proplist.setProperty("manufacturer",           ports.at(i).manufacturer());
        proplist.setProperty("portName",               ports.at(i).portName());
        proplist.setProperty("productIdentifier",      ports.at(i).productIdentifier());
        proplist.setProperty("serialNumber",           ports.at(i).serialNumber());
        proplist.setProperty("systemLocation",         ports.at(i).systemLocation());
        proplist.setProperty("vendorIdentifier",       ports.at(i).vendorIdentifier());
        scriptlist.setProperty(i, proplist);
    }
    return scriptlist;
}

static QScriptValue qserialportinfo_standardBaudRates(QScriptContext * /*context*/, QScriptEngine *engine)
{
  QStringList list;
  foreach (const qint32 &baud, QSerialPortInfo::standardBaudRates()) {
        QString s = QString::number(baud);
        list << s;
  }

  return engine->toScriptValue(list);
}

void setupQSerialPortInfoProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQSerialPortInfoProto entered");

  QScriptValue serialinfoproto = engine->newQObject(new QSerialPortInfoProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSerialPortInfo>(), serialinfoproto);
  engine->setDefaultPrototype(qMetaTypeId<QSerialPortInfo*>(), serialinfoproto);

  QScriptValue serialPortInfoConstructor = engine->newFunction(constructQSerialPortInfo, serialinfoproto);
  engine->globalObject().setProperty("QSerialPortInfo", serialPortInfoConstructor);
  // static methods
  serialPortInfoConstructor.setProperty("availablePorts",    engine->newFunction(qserialportinfo_availablePorts),    STATICPROPFLAGS);
  serialPortInfoConstructor.setProperty("standardBaudRates", engine->newFunction(qserialportinfo_standardBaudRates), STATICPROPFLAGS);
}

QScriptValue constructQSerialPortInfo(QScriptContext *context, QScriptEngine *engine)
{
    if (DEBUG) qDebug("constructQSerialPortInfo called");
    
    QSerialPortInfo *object = 0;

    if (context->argumentCount() == 0)
    {
        if (DEBUG) qDebug("qserialportinfo(2 args, string/qobject)");
        object = new QSerialPortInfo();
    }
    else if (context->argumentCount() == 1 && context->argument(0).isString())
    {
      if (DEBUG) qDebug("qserialportinfo(1 arg, string)");
      object = new QSerialPortInfo(context->argument(0).toString());
    }
    else
    {
      if (DEBUG) qDebug("qserialportinfo unknown");
      context->throwError(QScriptContext::UnknownError, "Unknown Constructor");
    }

    return engine->toScriptValue(object);
}

QSerialPortInfoProto::QSerialPortInfoProto(QObject *parent) 
    : QObject(parent)
{
}

QString QSerialPortInfoProto::description() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->description();
  return QString();
}

bool QSerialPortInfoProto::hasProductIdentifier() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->hasProductIdentifier();
  return false;
}

bool QSerialPortInfoProto::hasVendorIdentifier() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->hasVendorIdentifier();
  return false;
}

bool QSerialPortInfoProto::isBusy() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->isBusy();
  return false;
}

bool QSerialPortInfoProto::isNull() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QString QSerialPortInfoProto::manufacturer() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->manufacturer();
  return QString();
}

QString QSerialPortInfoProto::portName() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->portName();
  return QString();
}

quint16 QSerialPortInfoProto::productIdentifier() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->productIdentifier();
  return quint16();
}

QString QSerialPortInfoProto::serialNumber() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->serialNumber();
  return QString();
}

void QSerialPortInfoProto::swap(QSerialPortInfo & other)
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    item->swap(other);
}

QString QSerialPortInfoProto::systemLocation() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->systemLocation();
  return QString();
}

quint16 QSerialPortInfoProto::vendorIdentifier() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return item->vendorIdentifier();
  return quint16();
}

QString QSerialPortInfoProto::toString() const
{
  QSerialPortInfo *item = qscriptvalue_cast<QSerialPortInfo*>(thisObject());
  if (item)
    return QString("QSerialPortInfo()");
  return QString("QSerialPortInfo(unknown)");
}

#endif
