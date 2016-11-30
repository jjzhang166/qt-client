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
#include "qnetworkinterfaceproto.h"

#define DEBUG false

#if QT_VERSION < 0x050000
void setupQNetworkInterfaceProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

static QScriptValue qnetworkinterface_allAddresses(QScriptContext * /*context*/, QScriptEngine *engine)
{
  QStringList list;
  foreach (const QHostAddress &host, QNetworkInterface::allAddresses()) {
       list << host.toString();
  }

  return engine->toScriptValue(list);
}

static QScriptValue qnetworkinterface_allInterfaces(QScriptContext * /*context*/, QScriptEngine *engine)
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QScriptValue scriptlist = engine->newObject();
    for (int i = 0; i < interfaces.size(); i += 1) {
        QScriptValue proplist = engine->newObject();
        proplist.setProperty("name", interfaces.at(i).name());
        proplist.setProperty("humanReadableName", interfaces.at(i).humanReadableName());
        proplist.setProperty("hardwareAddress", interfaces.at(i).hardwareAddress());
        QList<QNetworkAddressEntry> addresses = interfaces.at(i).addressEntries();
        for (int j = 0; j < addresses.size(); j += 1) {
            QScriptValue addressPropList = engine->newObject();
            addressPropList.setProperty("broadcast", addresses.at(j).broadcast().toString());
            addressPropList.setProperty("ip", addresses.at(j).ip().toString());
            addressPropList.setProperty("netmask", addresses.at(j).netmask().toString());
            addressPropList.setProperty("prefixLength", addresses.at(j).prefixLength());
            proplist.setProperty("addresses", addressPropList);
        }
        scriptlist.setProperty(interfaces.at(i).humanReadableName(), proplist);
    }
    return scriptlist;
}

QScriptValue QNetworkInterfacetoScriptValue(QScriptEngine *engine, QNetworkInterface* const &item)
{
  if (DEBUG) qDebug("QNetworkInterfacetoScriptValue(%p, %p) called", engine, item);
  QScriptValue result = engine->newObject();
  result.setData(engine->newVariant(qVariantFromValue(item)));
  return result;
}

void QNetworkInterfacefromScriptValue(const QScriptValue &obj, QNetworkInterface* &item)
{
  item = obj.data().toVariant().value<QNetworkInterface*>();
}

QScriptValue InterfaceFlagtoScriptValue(QScriptEngine *engine, const enum QNetworkInterface::InterfaceFlag &p)
{
  return QScriptValue(engine, (int)p);
}

void InterfaceFlagfromScriptValue(const QScriptValue &obj, enum QNetworkInterface::InterfaceFlag &p)
{
  p = (enum QNetworkInterface::InterfaceFlag)obj.toInt32();
}

QScriptValue InterfaceFlagstoScriptValue(QScriptEngine *engine, const QNetworkInterface::InterfaceFlags &p)
{
  return QScriptValue(engine, (int)p);
}

void InterfaceFlagsfromScriptValue(const QScriptValue &obj, QNetworkInterface::InterfaceFlags &p)
{
  p = (QNetworkInterface::InterfaceFlags)obj.toInt32();
}

void setupQNetworkInterfaceProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QNetworkInterfacetoScriptValue, QNetworkInterfacefromScriptValue);

  QScriptValue proto = engine->newQObject(new QNetworkInterfaceProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QNetworkInterface*>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQNetworkInterface, proto);
  engine->globalObject().setProperty("QNetworkInterface",  constructor);

  // enum QNetworkInterface::InterfaceFlag
  qScriptRegisterMetaType(engine,     InterfaceFlagtoScriptValue,  InterfaceFlagfromScriptValue);
  qScriptRegisterMetaType(engine,     InterfaceFlagstoScriptValue, InterfaceFlagsfromScriptValue);
  proto.setProperty("IsUp",           QScriptValue(engine,         QNetworkInterface::IsUp),           ENUMPROPFLAGS);
  proto.setProperty("IsRunning",      QScriptValue(engine,         QNetworkInterface::IsRunning),      ENUMPROPFLAGS);
  proto.setProperty("CanBroadcast",   QScriptValue(engine,         QNetworkInterface::CanBroadcast),   ENUMPROPFLAGS);
  proto.setProperty("IsLoopBack",     QScriptValue(engine,         QNetworkInterface::IsLoopBack),     ENUMPROPFLAGS);
  proto.setProperty("IsPointToPoint", QScriptValue(engine,         QNetworkInterface::IsPointToPoint), ENUMPROPFLAGS);
  proto.setProperty("CanMulticast",   QScriptValue(engine,         QNetworkInterface::CanMulticast),   ENUMPROPFLAGS);
  // static methods
  constructor.setProperty("allAddresses",  engine->newFunction(qnetworkinterface_allAddresses),  STATICPROPFLAGS);
  constructor.setProperty("allInterfaces", engine->newFunction(qnetworkinterface_allInterfaces), STATICPROPFLAGS);
}

QScriptValue constructQNetworkInterface(QScriptContext * /*context*/, QScriptEngine  *engine)
{
  QNetworkInterface *obj = 0;
  if (DEBUG) qDebug("constructQNetworkInterface(): calling QNetworkInterface()");
  
  obj = new QNetworkInterface();
  return engine->toScriptValue(obj);
}

QNetworkInterfaceProto::QNetworkInterfaceProto(QObject *parent) : QObject(parent)
{
}

QList<QNetworkAddressEntry> QNetworkInterfaceProto::addressEntries() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->addressEntries();
  return QList<QNetworkAddressEntry>();
}

QNetworkInterface::InterfaceFlags QNetworkInterfaceProto::flags() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->flags();
  return 0;
}

QString QNetworkInterfaceProto::hardwareAddress() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->hardwareAddress();
  return QString();
}

QString QNetworkInterfaceProto::humanReadableName() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->humanReadableName();
  return QString();
}

int QNetworkInterfaceProto::index() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->index();
  return 0;
}

bool QNetworkInterfaceProto::isValid() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QString QNetworkInterfaceProto::name() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

void QNetworkInterfaceProto::swap(QNetworkInterface & other)
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    item->swap(other);
}


QString QNetworkInterfaceProto::toString() const
{
  QNetworkInterface *item = qscriptvalue_cast<QNetworkInterface*>(thisObject());
  if (item)
    return QString("QNetworkInterface(%1)").arg(item->humanReadableName());
  return QString("QNetworkInterface(unknown)");
}
#endif
