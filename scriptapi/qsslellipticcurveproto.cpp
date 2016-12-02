/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qsslellipticcurveproto.h"

#if QT_VERSION < 0x050000
void setupQSslEllipticCurveProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
// TODO: Figure out how to expose QVector
/*
QScriptValue QVectorQSslEllipticCurveToScriptValue(QScriptEngine *engine, const QVector<QSslEllipticCurve> &list)
{
  QScriptValue newArray = engine->newArray();
  for (int i = 0; i < list.size(); i += 1) {
    newArray.setProperty(i, engine->toScriptValue(list.at(i)));
  }
  return newArray;
}
void QVectorQSslEllipticCurveFromScriptValue(const QScriptValue &obj, QVector<QSslEllipticCurve> &list)
{
  list = QVector<QSslEllipticCurve>();
  QScriptValueIterator it(obj);

  while (it.hasNext()) {
    it.next();
    if (it.flags() & QScriptValue::SkipInEnumeration)
      continue;
    QSslEllipticCurve item = qscriptvalue_cast<QSslEllipticCurve>(it.value());
    list.insert(it.name().toInt32, item);
  }
}
*/

QScriptValue fromLongNameForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString name = context->argument(0).toString();
    QSslEllipticCurve fromLongName = QSslEllipticCurve::fromLongName(name);
    return engine->toScriptValue(fromLongName);
  } else {
    return engine->undefinedValue();
  }
}

QScriptValue fromShortNameForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString name = context->argument(0).toString();
    QSslEllipticCurve fromShortName = QSslEllipticCurve::fromShortName(name);
    return engine->toScriptValue(fromShortName);
  } else {
    return engine->undefinedValue();
  }
}

void setupQSslEllipticCurveProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QSslEllipticCurveProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSslEllipticCurve*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QSslEllipticCurve>(), proto);

  QScriptValue constructor = engine->newFunction(constructQSslEllipticCurve, proto);
  engine->globalObject().setProperty("QSslEllipticCurve",  constructor);

  // TODO: Figure out how to expose QVector
  //qScriptRegisterMetaType(engine, QVectorQSslEllipticCurveToScriptValue, QVectorQSslEllipticCurveFromScriptValue);

  QScriptValue fromLongName = engine->newFunction(fromLongNameForJS);
  constructor.setProperty("fromLongName", fromLongName);
  QScriptValue fromShortName = engine->newFunction(fromShortNameForJS);
  constructor.setProperty("fromShortName", fromShortName);
}

QScriptValue constructQSslEllipticCurve(QScriptContext * /*context*/, QScriptEngine  *engine)
{
  QSslEllipticCurve *obj = 0;
  obj = new QSslEllipticCurve();
  return engine->toScriptValue(obj);
}

QSslEllipticCurveProto::QSslEllipticCurveProto(QObject *parent)
    : QObject(parent)
{
}

bool QSslEllipticCurveProto::isTlsNamedCurve() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->isTlsNamedCurve();
  return false;
}

bool QSslEllipticCurveProto::isValid() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->isValid();
  return false;
}

QString QSslEllipticCurveProto::longName() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->longName();
  return QString();
}

QString QSslEllipticCurveProto::shortName() const
{
  QSslEllipticCurve *item = qscriptvalue_cast<QSslEllipticCurve*>(thisObject());
  if (item)
    return item->shortName();
  return QString();
}

#endif
