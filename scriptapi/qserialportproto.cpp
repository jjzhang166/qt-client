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
#include "qserialportproto.h"

#define DEBUG false

#if QT_VERSION < 0x050100
void setupQSerialPortProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else

// qScriptRegisterMetaType handlers

QScriptValue BaudRatetoScriptValue(QScriptEngine *engine, const enum QSerialPort::BaudRate &p)
{
  return QScriptValue(engine, (int)p);
}

void BaudRatefromScriptValue(const QScriptValue &obj, enum QSerialPort::BaudRate &p)
{
  p = (enum QSerialPort::BaudRate)obj.toInt32();
}

QScriptValue DataBitstoScriptValue(QScriptEngine *engine, const enum QSerialPort::DataBits &p)
{
  return QScriptValue(engine, (int)p);
}

void DataBitsfromScriptValue(const QScriptValue &obj, enum QSerialPort::DataBits &p)
{
  p = (enum QSerialPort::DataBits)obj.toInt32();
}

QScriptValue DirectiontoScriptValue(QScriptEngine *engine, const enum QSerialPort::Direction &p)
{
  return QScriptValue(engine, (int)p);
}

void DirectionfromScriptValue(const QScriptValue &obj, enum QSerialPort::Direction &p)
{
  p = (enum QSerialPort::Direction)obj.toInt32();
}

QScriptValue DirectionFlagstoScriptValue(QScriptEngine *engine, const QSerialPort::Directions &p)
{
  return QScriptValue(engine, (int)p);
}

void DirectionFlagsfromScriptValue(const QScriptValue &obj, QSerialPort::Directions &p)
{
  p = (QSerialPort::Directions)obj.toInt32();
}

QScriptValue FlowControltoScriptValue(QScriptEngine *engine, const enum QSerialPort::FlowControl &p)
{
  return QScriptValue(engine, (int)p);
}

void FlowControlfromScriptValue(const QScriptValue &obj, enum QSerialPort::FlowControl &p)
{
  p = (enum QSerialPort::FlowControl)obj.toInt32();
}

QScriptValue ParitytoScriptValue(QScriptEngine *engine, const enum QSerialPort::Parity &p)
{
  return QScriptValue(engine, (int)p);
}

void ParityfromScriptValue(const QScriptValue &obj, enum QSerialPort::Parity &p)
{
  p = (enum QSerialPort::Parity)obj.toInt32();
}

QScriptValue PinoutSignaltoScriptValue(QScriptEngine *engine, const enum QSerialPort::PinoutSignal &p)
{
  return QScriptValue(engine, (int)p);
}

void PinoutSignalfromScriptValue(const QScriptValue &obj, enum QSerialPort::PinoutSignal &p)
{
  p = (enum QSerialPort::PinoutSignal)obj.toInt32();
}

QScriptValue PinoutSignalstoScriptValue(QScriptEngine *engine, const QSerialPort::PinoutSignals &p)
{
  return QScriptValue(engine, (int)p);
}

void PinoutSignalsfromScriptValue(const QScriptValue &obj, QSerialPort::PinoutSignals &p)
{
  p = (QSerialPort::PinoutSignals)obj.toInt32();
}

QScriptValue SerialPortErrortoScriptValue(QScriptEngine *engine, const enum QSerialPort::SerialPortError &p)
{
  return QScriptValue(engine, (int)p);
}

void SerialPortErrorfromScriptValue(const QScriptValue &obj, enum QSerialPort::SerialPortError &p)
{
  p = (enum QSerialPort::SerialPortError)obj.toInt32();
}

QScriptValue StopBitstoScriptValue(QScriptEngine *engine, const enum QSerialPort::StopBits &p)
{
  return QScriptValue(engine, (int)p);
}

void StopBitsfromScriptValue(const QScriptValue &obj, enum QSerialPort::StopBits &p)
{
  p = (enum QSerialPort::StopBits)obj.toInt32();
}
//

void setupQSerialPortProto(QScriptEngine *engine)
{
  if (DEBUG) qDebug("setupQSerialPortProto entered");

  QScriptValue serialproto = engine->newQObject(new QSerialPortProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QSerialPort*>(), serialproto);

  QScriptValue serialPortConstructor = engine->newFunction(constructQSerialPort, serialproto);
  engine->globalObject().setProperty("QSerialPort", serialPortConstructor);

  // enum QSerialPort::BaudRate
  qScriptRegisterMetaType(engine, BaudRatetoScriptValue, BaudRatefromScriptValue);
  serialPortConstructor.setProperty("Baud1200",    QScriptValue(engine, QSerialPort::Baud1200),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud2400",    QScriptValue(engine, QSerialPort::Baud2400),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud4800",    QScriptValue(engine, QSerialPort::Baud4800),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud9600",    QScriptValue(engine, QSerialPort::Baud9600),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud19200",   QScriptValue(engine, QSerialPort::Baud19200),   ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud38400",   QScriptValue(engine, QSerialPort::Baud38400),   ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud57600",   QScriptValue(engine, QSerialPort::Baud57600),   ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Baud115200",  QScriptValue(engine, QSerialPort::Baud115200),  ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownBaud", QScriptValue(engine, QSerialPort::UnknownBaud), ENUMPROPFLAGS);

  // enum QSerialPort::DataBits
  qScriptRegisterMetaType(engine, DataBitstoScriptValue, DataBitsfromScriptValue);
  serialPortConstructor.setProperty("Data5",           QScriptValue(engine, QSerialPort::Data5),           ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Data6",           QScriptValue(engine, QSerialPort::Data6),           ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Data7",           QScriptValue(engine, QSerialPort::Data7),           ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Data8",           QScriptValue(engine, QSerialPort::Data8),           ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownDataBits", QScriptValue(engine, QSerialPort::UnknownDataBits), ENUMPROPFLAGS);
 
   // enum QSerialPort::Direction
  qScriptRegisterMetaType(engine, DirectiontoScriptValue, DirectionfromScriptValue);
  qScriptRegisterMetaType(engine, DirectionFlagstoScriptValue, DirectionFlagsfromScriptValue);
  serialPortConstructor.setProperty("Input",         QScriptValue(engine, QSerialPort::Input),         ENUMPROPFLAGS);
  serialPortConstructor.setProperty("Output",        QScriptValue(engine, QSerialPort::Output),        ENUMPROPFLAGS);
  serialPortConstructor.setProperty("AllDirections", QScriptValue(engine, QSerialPort::AllDirections), ENUMPROPFLAGS);
  
  // enum QSerialPort::FlowControl
  qScriptRegisterMetaType(engine, FlowControltoScriptValue, FlowControlfromScriptValue);
  serialPortConstructor.setProperty("NoFlowControl",      QScriptValue(engine, QSerialPort::NoFlowControl),      ENUMPROPFLAGS);
  serialPortConstructor.setProperty("HardwareControl",    QScriptValue(engine, QSerialPort::HardwareControl),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("SoftwareControl",    QScriptValue(engine, QSerialPort::SoftwareControl),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownFlowControl", QScriptValue(engine, QSerialPort::UnknownFlowControl), ENUMPROPFLAGS);
  
  // enum QSerialPort::Parity
  qScriptRegisterMetaType(engine, ParitytoScriptValue, ParityfromScriptValue);
  serialPortConstructor.setProperty("NoParity",      QScriptValue(engine,   QSerialPort::NoParity),      ENUMPROPFLAGS);
  serialPortConstructor.setProperty("EvenParity",    QScriptValue(engine,   QSerialPort::EvenParity),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("OddParity",     QScriptValue(engine,   QSerialPort::OddParity),     ENUMPROPFLAGS);
  serialPortConstructor.setProperty("SpaceParity",   QScriptValue(engine,   QSerialPort::SpaceParity),   ENUMPROPFLAGS);
  serialPortConstructor.setProperty("MarkParity",    QScriptValue(engine,   QSerialPort::MarkParity),    ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownParity", QScriptValue(engine,   QSerialPort::UnknownParity), ENUMPROPFLAGS);

  // enum QSerialPort::PinoutSignal
  qScriptRegisterMetaType(engine, PinoutSignaltoScriptValue, PinoutSignalfromScriptValue);
  qScriptRegisterMetaType(engine, PinoutSignalstoScriptValue, PinoutSignalsfromScriptValue);
  serialPortConstructor.setProperty("NoSignal",                       QScriptValue(engine,       QSerialPort::NoSignal),                       ENUMPROPFLAGS);
  serialPortConstructor.setProperty("TransmittedDataSignal",          QScriptValue(engine,       QSerialPort::TransmittedDataSignal),          ENUMPROPFLAGS);
  serialPortConstructor.setProperty("ReceivedDataSignal",             QScriptValue(engine,       QSerialPort::ReceivedDataSignal),             ENUMPROPFLAGS);
  serialPortConstructor.setProperty("DataTerminalReadySignal",        QScriptValue(engine,       QSerialPort::DataTerminalReadySignal),        ENUMPROPFLAGS);
  serialPortConstructor.setProperty("DataCarrierDetectSignal",        QScriptValue(engine,       QSerialPort::DataCarrierDetectSignal),        ENUMPROPFLAGS);
  serialPortConstructor.setProperty("DataSetReadySignal",             QScriptValue(engine,       QSerialPort::DataSetReadySignal),             ENUMPROPFLAGS);
  serialPortConstructor.setProperty("RingIndicatorSignal",            QScriptValue(engine,       QSerialPort::RingIndicatorSignal),            ENUMPROPFLAGS);
  serialPortConstructor.setProperty("RequestToSendSignal",            QScriptValue(engine,       QSerialPort::RequestToSendSignal),            ENUMPROPFLAGS);
  serialPortConstructor.setProperty("ClearToSendSignal",              QScriptValue(engine,       QSerialPort::ClearToSendSignal),              ENUMPROPFLAGS);
  serialPortConstructor.setProperty("SecondaryTransmittedDataSignal", QScriptValue(engine,       QSerialPort::SecondaryTransmittedDataSignal), ENUMPROPFLAGS);
  serialPortConstructor.setProperty("SecondaryReceivedDataSignal",    QScriptValue(engine,       QSerialPort::SecondaryReceivedDataSignal),    ENUMPROPFLAGS);

  // enum QSerialPort::SerialPortError
  qScriptRegisterMetaType(engine,                       SerialPortErrortoScriptValue, SerialPortErrorfromScriptValue);
  serialPortConstructor.setProperty("NoError",                    QScriptValue(engine,          QSerialPort::NoError),                   ENUMPROPFLAGS);
  serialPortConstructor.setProperty("DeviceNotFoundError",        QScriptValue(engine,          QSerialPort::DeviceNotFoundError),       ENUMPROPFLAGS);
  serialPortConstructor.setProperty("PermissionError",            QScriptValue(engine,          QSerialPort::PermissionError),           ENUMPROPFLAGS);
  serialPortConstructor.setProperty("OpenError",                  QScriptValue(engine,          QSerialPort::OpenError),                 ENUMPROPFLAGS);
  serialPortConstructor.setProperty("NotOpenError",               QScriptValue(engine,          QSerialPort::NotOpenError),              ENUMPROPFLAGS);
  serialPortConstructor.setProperty("ParityError",                QScriptValue(engine,          QSerialPort::ParityError),               ENUMPROPFLAGS);
  serialPortConstructor.setProperty("FramingError",               QScriptValue(engine,          QSerialPort::FramingError),              ENUMPROPFLAGS);
  serialPortConstructor.setProperty("BreakConditionError",        QScriptValue(engine,          QSerialPort::BreakConditionError),       ENUMPROPFLAGS);
  serialPortConstructor.setProperty("WriteError",                 QScriptValue(engine,          QSerialPort::WriteError),                ENUMPROPFLAGS);
  serialPortConstructor.setProperty("ReadError",                  QScriptValue(engine,          QSerialPort::ReadError),                 ENUMPROPFLAGS);
  serialPortConstructor.setProperty("ResourceError",              QScriptValue(engine,          QSerialPort::ResourceError),             ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnsupportedOperationError",  QScriptValue(engine,          QSerialPort::UnsupportedOperationError), ENUMPROPFLAGS);
  serialPortConstructor.setProperty("TimeoutError",               QScriptValue(engine,          QSerialPort::TimeoutError),              ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownError",               QScriptValue(engine,          QSerialPort::UnknownError),              ENUMPROPFLAGS);

  // enum QSerialPort::StopBits
  qScriptRegisterMetaType(engine,            StopBitstoScriptValue, StopBitsfromScriptValue);
  serialPortConstructor.setProperty("OneStop",         QScriptValue(engine,   QSerialPort::OneStop),         ENUMPROPFLAGS);
  serialPortConstructor.setProperty("OneAndHalfStop",  QScriptValue(engine,   QSerialPort::OneAndHalfStop),  ENUMPROPFLAGS);
  serialPortConstructor.setProperty("TwoStop",         QScriptValue(engine,   QSerialPort::TwoStop),         ENUMPROPFLAGS);
  serialPortConstructor.setProperty("UnknownStopBits", QScriptValue(engine,   QSerialPort::UnknownStopBits), ENUMPROPFLAGS);

}

QScriptValue constructQSerialPort(QScriptContext *context, QScriptEngine *engine)
{
    if (DEBUG) qDebug("constructQSerialPort called");
    
    QSerialPort *object = 0;

    if (context->argumentCount() == 2 && context->argument(0).isString())
    {
      if (DEBUG) qDebug("qserialport(2 args, string/qobject)");
      object = new QSerialPort(context->argument(0).toString(), context->argument(1).toQObject());
    }
    else if (context->argumentCount() == 2 && !context->argument(0).isString())
    {
      if (DEBUG) qDebug("qserialport(2args, qserialportinfo)");
      context->throwError(QScriptContext::UnknownError, "QSerialPort(QSerialPortInfo &info, QObject *parent) constructor is not yet supported");
    }
    else if (context->argumentCount() == 1)
    {
      object = new QSerialPort(context->argument(0).toQObject());
    }
    else
    {
        if (DEBUG) qDebug("qserialport unknown");
        context->throwError(QScriptContext::UnknownError, "Uknown Constructor");
    }

    return engine->toScriptValue(object);
}

QSerialPortProto::QSerialPortProto(QObject *parent) 
    : QIODeviceProto(parent)
{
}

qint32 QSerialPortProto::baudRate(QSerialPort::Directions directions) const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->baudRate(directions);

  return qint32();
}

bool QSerialPortProto::clear(QSerialPort::Directions directions)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->clear(directions);

  return false;
}

void QSerialPortProto::clearError()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    item->clearError();
}

QSerialPort::DataBits QSerialPortProto::dataBits() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->dataBits();

  return QSerialPort::Data8;
}

QSerialPort::SerialPortError QSerialPortProto::error() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->error();

  return QSerialPort::SerialPortError();
}

QSerialPort::FlowControl QSerialPortProto::flowControl() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->flowControl();

  return QSerialPort::FlowControl();
}

bool QSerialPortProto::flush()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->flush();

  return false;
}

bool QSerialPortProto::isDataTerminalReady()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->isDataTerminalReady();

  return false;
}

bool QSerialPortProto::isRequestToSend()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->isRequestToSend();

  return false;
}

bool QSerialPortProto::open(QIODevice::OpenModeFlag mode)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->open(mode);

  return false;
}

QSerialPort::Parity QSerialPortProto::parity() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->parity();

  return QSerialPort::Parity();
}

QSerialPort::PinoutSignals QSerialPortProto::pinoutSignals()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->pinoutSignals();

  return QSerialPort::PinoutSignals();
}

QString QSerialPortProto::portName() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->portName();

  return QString();
}

qint64 QSerialPortProto::readBufferSize() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->readBufferSize();

  return 0;
}

bool QSerialPortProto::sendBreak(int duration)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->sendBreak(duration);

  return false;
}

bool QSerialPortProto::setBaudRate(qint32 baudRate, QSerialPort::Directions directions)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setBaudRate(baudRate, directions);

  return false;
}

bool QSerialPortProto::setBreakEnabled(bool set)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setBreakEnabled(set);

  return false;
}

bool QSerialPortProto::setDataBits(QSerialPort::DataBits dataBits)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setDataBits(dataBits);

  return false;
}

bool QSerialPortProto::setDataTerminalReady(bool set)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setDataTerminalReady(set);

  return false;
}

bool QSerialPortProto::setFlowControl(QSerialPort::FlowControl flowControl)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setFlowControl(flowControl);

  return false;
}

bool QSerialPortProto::setParity(QSerialPort::Parity parity)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setParity(parity);

  return false;
}

void QSerialPortProto::setPort(const QSerialPortInfo & serialPortInfo)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    item->setPort(serialPortInfo);
}

void QSerialPortProto::setPortName(const QString & name)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    item->setPortName(name);
}

void QSerialPortProto::setReadBufferSize(qint64 size)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    item->setReadBufferSize(size);
}

bool QSerialPortProto::setRequestToSend(bool set)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setRequestToSend(set);

  return false;
}

bool QSerialPortProto::setStopBits(QSerialPort::StopBits stopBits)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->setStopBits(stopBits);

  return false;
}

QSerialPort::StopBits QSerialPortProto::stopBits() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->stopBits();

  return QSerialPort::StopBits();
}

bool QSerialPortProto::atEnd() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->atEnd();

  return false;
}

qint64 QSerialPortProto::bytesAvailable() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->bytesAvailable();

  return 0;
}

qint64 QSerialPortProto::bytesToWrite() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->bytesToWrite();

  return 0;
}

bool QSerialPortProto::canReadLine() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->canReadLine();

  return false;
}

void QSerialPortProto::close()
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    item->close();
}

bool QSerialPortProto::isSequential() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->isSequential();

  return false;
}

bool QSerialPortProto::waitForBytesWritten(int msecs)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->waitForBytesWritten(msecs);

  return false;
}

bool QSerialPortProto::waitForReadyRead(int msecs)
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return item->waitForReadyRead(msecs);

  return false;
}

QString QSerialPortProto::toString() const
{
  QSerialPort *item = qscriptvalue_cast<QSerialPort*>(thisObject());
  if (item)
    return QString("QSerialPort(%1)").arg(item->portName());
  return QString("QSerialPort(unknown)");
}
#endif
