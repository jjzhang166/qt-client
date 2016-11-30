/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QBUFFERPROTO_H__
#define __QBUFFERPROTO_H__

#include <QtScript>

void setupQBufferProto(QScriptEngine *engine);

#include <QBuffer>
#include <QIODevice>
#include "qiodeviceproto.h"

Q_DECLARE_METATYPE(QBuffer*)
//Q_DECLARE_METATYPE(QBuffer) // Is Q_DISABLE_COPY() in qbuffer.h

QScriptValue constructQBuffer(QScriptContext *context, QScriptEngine *engine);

class QBufferProto : public QIODeviceProto
{
  Q_OBJECT

  public:
    QBufferProto(QObject *parent);
    virtual ~QBufferProto();

    Q_INVOKABLE QByteArray &        buffer();
    Q_INVOKABLE const QByteArray &  data() const;
    Q_INVOKABLE void                setBuffer(QByteArray * byteArray);
    Q_INVOKABLE void                setData(const QByteArray & data);
    Q_INVOKABLE void                setData(const char * data, int size);

  // Reimplemented Public Functions.
    Q_INVOKABLE bool                atEnd() const;
    Q_INVOKABLE bool                canReadLine() const;
    Q_INVOKABLE void                close();
    Q_INVOKABLE bool                open(QIODevice::OpenMode flags);
    Q_INVOKABLE qint64              pos() const;
    Q_INVOKABLE bool                seek(qint64 pos);
    Q_INVOKABLE qint64              size() const;
};

#endif
