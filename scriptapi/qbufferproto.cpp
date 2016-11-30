/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qbufferproto.h"

void setupQBufferProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QBufferProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QBuffer*>(), proto);
  // Not allowed. Is Q_DISABLE_COPY() in qbuffer.h
  //engine->setDefaultPrototype(qMetaTypeId<QBuffer>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQBuffer, proto);
  engine->globalObject().setProperty("QBuffer",  constructor);
}

QScriptValue constructQBuffer(QScriptContext *context, QScriptEngine  *engine)
{
  QBuffer *obj = 0;
  QByteArray *byteArray = 0;
  if (context->argumentCount() == 2) {
    byteArray = new QByteArray(context->argument(0).toString().toLocal8Bit());
    QObject *parent = context->argument(1).toQObject();
    obj = new QBuffer(byteArray, parent);
  } else if (context->argumentCount() == 1) {
    byteArray = new QByteArray(context->argument(0).toString().toLocal8Bit());
    obj = new QBuffer(byteArray);
  } else {
    obj = new QBuffer();
  }
  return engine->toScriptValue(obj);
}

QBufferProto::QBufferProto(QObject *parent) : QIODeviceProto(parent)
{
}
QBufferProto::~QBufferProto()
{
}

QByteArray & QBufferProto::buffer()
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->buffer();
  return *(new QByteArray());
}

const QByteArray & QBufferProto::data() const
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->data();
  return *(new QByteArray());
}

void QBufferProto::setBuffer(QByteArray * byteArray)
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    item->setBuffer(byteArray);
}

void QBufferProto::setData(const QByteArray & data)
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    item->setData(data);
}

void QBufferProto::setData(const char * data, int size)
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    item->setData(data, size);
}

bool QBufferProto::atEnd() const
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->atEnd();
  return false;
}

bool QBufferProto::canReadLine() const
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->canReadLine();
  return false;
}

void QBufferProto::close()
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    item->close();
}

bool QBufferProto::open(QIODevice::OpenMode flags)
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->open(flags);
  return false;
}

qint64 QBufferProto::pos() const
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->pos();
  return 0;
}

bool QBufferProto::seek(qint64 pos)
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->seek(pos);
  return false;
}

qint64 QBufferProto::size() const
{
  QBuffer *item = qscriptvalue_cast<QBuffer*>(thisObject());
  if (item)
    return item->size();
  return 0;
}
