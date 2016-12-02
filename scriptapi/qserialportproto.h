/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QSERIALPORTPROTO_H__
#define __QSERIALPORTPROTO_H__
#include <QObject>
#include <QString>
#include <QtScript>
#include <QIODevice>
void setupQSerialPortProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050100
#include <QSerialPort>
#include <QSerialPortInfo>
#include "qiodeviceproto.h"

QScriptValue constructQSerialPort(QScriptContext *context, QScriptEngine *engine);

Q_DECLARE_METATYPE(QSerialPort*)
Q_DECLARE_METATYPE(enum QSerialPort::BaudRate)
Q_DECLARE_METATYPE(enum QSerialPort::DataBits)
Q_DECLARE_METATYPE(enum QSerialPort::Direction)
Q_DECLARE_METATYPE(QSerialPort::Directions)
Q_DECLARE_METATYPE(enum QSerialPort::FlowControl)
Q_DECLARE_METATYPE(enum QSerialPort::Parity)
Q_DECLARE_METATYPE(enum QSerialPort::PinoutSignal)
Q_DECLARE_METATYPE(QSerialPort::PinoutSignals)
Q_DECLARE_METATYPE(enum QSerialPort::SerialPortError)
Q_DECLARE_METATYPE(enum QSerialPort::StopBits)

class QSerialPortProto : public QIODeviceProto
{
  Q_OBJECT

  Q_PROPERTY(qint32                        baudRate          READ baudRate              WRITE setBaudRate)
  Q_PROPERTY(QSerialPort::DataBits         dataBits          READ dataBits              WRITE setDataBits)
  Q_PROPERTY(bool                          dataTerminalReady READ isDataTerminalReady   WRITE setDataTerminalReady)
  Q_PROPERTY(QSerialPort::SerialPortError  error             READ error                 )
  Q_PROPERTY(QSerialPort::FlowControl      flowControl       READ flowControl           WRITE setFlowControl)
  Q_PROPERTY(QSerialPort::Parity           parity            READ parity                WRITE setParity)
  Q_PROPERTY(bool                          requestToSend     READ isRequestToSend       WRITE setRequestToSend)
  Q_PROPERTY(QSerialPort::StopBits         stopBits          READ stopBits              WRITE setStopBits)

  public:
    QSerialPortProto(QObject *parent);
    // public functions
    Q_INVOKABLE qint32                       baudRate(QSerialPort::Directions directions = QSerialPort::AllDirections) const;
    Q_INVOKABLE bool                         clear(QSerialPort::Directions directions = QSerialPort::AllDirections);
    Q_INVOKABLE void                         clearError();
    Q_INVOKABLE QSerialPort::DataBits        dataBits() const;
    Q_INVOKABLE QSerialPort::SerialPortError error() const;
    Q_INVOKABLE QSerialPort::FlowControl     flowControl() const;
    Q_INVOKABLE bool                         flush();
    Q_INVOKABLE bool                         isDataTerminalReady();
    Q_INVOKABLE bool                         isRequestToSend();
    Q_INVOKABLE QSerialPort::Parity          parity() const;
    Q_INVOKABLE QSerialPort::PinoutSignals   pinoutSignals();
    Q_INVOKABLE QString                      portName() const;
    Q_INVOKABLE qint64                       readBufferSize() const;
    Q_INVOKABLE bool                         sendBreak(int duration = 0);
    Q_INVOKABLE bool                         setBaudRate(qint32 baudRate, QSerialPort::Directions directions = QSerialPort::AllDirections);
    Q_INVOKABLE bool                         setBreakEnabled(bool set = true);
    Q_INVOKABLE bool                         setDataBits(QSerialPort::DataBits dataBits);
    Q_INVOKABLE bool                         setDataTerminalReady(bool set);
    Q_INVOKABLE bool                         setFlowControl(QSerialPort::FlowControl flowControl);
    Q_INVOKABLE bool                         setParity(QSerialPort::Parity parity);
    Q_INVOKABLE void                         setPort(const QSerialPortInfo &serialPortInfo);
    Q_INVOKABLE void                         setPortName(const QString & name);
    Q_INVOKABLE void                         setReadBufferSize(qint64 size);
    Q_INVOKABLE bool                         setRequestToSend(bool set);
    Q_INVOKABLE bool                         setStopBits(QSerialPort::StopBits stopBits);
    Q_INVOKABLE QSerialPort::StopBits        stopBits() const;
    // reimplemented public functions
    Q_INVOKABLE bool                         atEnd() const;
    Q_INVOKABLE qint64                       bytesAvailable() const;
    Q_INVOKABLE qint64                       bytesToWrite() const;
    Q_INVOKABLE bool                         canReadLine() const;
    Q_INVOKABLE void                         close();
    Q_INVOKABLE bool                         isSequential() const;
    Q_INVOKABLE bool                         open(QIODevice::OpenModeFlag mode);
    Q_INVOKABLE bool                         waitForBytesWritten(int msecs);
    Q_INVOKABLE bool                         waitForReadyRead(int msecs);
    // added custom
    Q_INVOKABLE QString                      toString() const;

  signals:
    void baudRateChanged(qint32 baudRate, QSerialPort::Directions directions);
    void breakEnabledChanged(bool set);
    void dataBitsChanged(int dataBits);
    void dataTerminalReadyChanged(bool set);
    void error(QSerialPort::SerialPortError error);
    void flowControlChanged(QSerialPort::FlowControl flow);
    void parityChanged(QSerialPort::Parity parity);
    void requestToSendChanged(bool set);
    void stopBitsChanged(int stopBits);

};
#endif
#endif
