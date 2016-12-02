/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qcoreapplicationproto.h"

QScriptValue QCoreApplicationToScriptValue(QScriptEngine *engine,
                                           QCoreApplication *const &item)
{
  return engine->newQObject(item);
}

void QCoreApplicationFromScriptValue(const QScriptValue &obj, QCoreApplication *&item)
{
  item = qobject_cast<QCoreApplication*>(obj.toQObject());
}

void setupQCoreApplicationProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QCoreApplicationToScriptValue, QCoreApplicationFromScriptValue);

  QScriptValue proto = engine->newQObject(new QCoreApplicationProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QCoreApplication*>(), proto);
  engine->globalObject().setProperty("QCoreApplication",
                                     engine->newQObject(QCoreApplication::instance()));
}

QCoreApplicationProto::QCoreApplicationProto(QObject *parent)
  : QObject(parent)
{
}

void QCoreApplicationProto::quit()
{
  QCoreApplication *item = qscriptvalue_cast<QCoreApplication*>(thisObject());
  if (item)
    item->quit();
}

void QCoreApplicationProto::addLibraryPath(const QString &path)
{
  QCoreApplication::addLibraryPath(path);
}

QString QCoreApplicationProto::applicationDirPath()
{
  return QCoreApplication::applicationDirPath();
}

QString QCoreApplicationProto::applicationFilePath()
{
  return QCoreApplication::applicationFilePath();
}

qint64 QCoreApplicationProto::applicationPid()
{
  return QCoreApplication::applicationPid();
}

QString QCoreApplicationProto::applicationVersion()
{
  return QCoreApplication::applicationVersion();
}

QStringList QCoreApplicationProto::arguments()
{
  return QCoreApplication::arguments();
}

bool QCoreApplicationProto::closingDown()
{
  return QCoreApplication::closingDown();
}

#if QT_VERSION >= 0x050000
QAbstractEventDispatcher *QCoreApplicationProto::eventDispatcher()
{
  return QCoreApplication::eventDispatcher();
}
#endif

int QCoreApplicationProto::exec()
{
  return QCoreApplication::exec();
}

void QCoreApplicationProto::exit(int returnCode)
{
  QCoreApplication::exit(returnCode);
}

void QCoreApplicationProto::flush()
{
  QCoreApplication::flush();
}

#if QT_VERSION < 0x050000
bool QCoreApplicationProto::hasPendingEvents()
{
  return QCoreApplication::hasPendingEvents();
}
#endif

void QCoreApplicationProto::installTranslator(QTranslator *translationFile)
{
  QCoreApplication::installTranslator(translationFile);
}

QCoreApplication *QCoreApplicationProto::instance()
{
  return QCoreApplication::instance();
}

#if QT_VERSION >= 0x050000
bool QCoreApplicationProto::isQuitLockEnabled()
{
  return QCoreApplication::isQuitLockEnabled();
}

bool QCoreApplicationProto::isSetuidAllowed()
{
  return QCoreApplication::isSetuidAllowed();
}
#endif

QStringList QCoreApplicationProto::libraryPaths()
{
  return QCoreApplication::libraryPaths();
}

#if QT_VERSION < 0x050000
void QCoreApplicationProto::postEvent(QObject *receiver, QEvent *event)
{
  QCoreApplication::postEvent(receiver, event);
}
#endif

void QCoreApplicationProto::postEvent(QObject *receiver, QEvent *event, int priority)
{
  QCoreApplication::postEvent(receiver, event, priority);
}

void QCoreApplicationProto::processEvents(QEventLoop::ProcessEventsFlags flags)
{
  QCoreApplication::processEvents(flags);
}

void QCoreApplicationProto::processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime)
{
  QCoreApplication::processEvents(flags, maxtime);
}

void QCoreApplicationProto::removeLibraryPath(const QString &path)
{
  QCoreApplication::removeLibraryPath(path);
}

void QCoreApplicationProto::removePostedEvents(QObject *receiver)
{
  QCoreApplication::removePostedEvents(receiver);
}

void QCoreApplicationProto::removePostedEvents(QObject *receiver, int eventType)
{
  QCoreApplication::removePostedEvents(receiver, eventType);
}

void QCoreApplicationProto::removeTranslator(QTranslator *translationFile)
{
  QCoreApplication::removeTranslator(translationFile);
}

bool QCoreApplicationProto::sendEvent(QObject *receiver, QEvent *event)
{
  return QCoreApplication::sendEvent(receiver, event);
}

void QCoreApplicationProto::sendPostedEvents(QObject *receiver, int event_type)
{
  QCoreApplication::sendPostedEvents(receiver, event_type);
}

#if QT_VERSION < 0x050000
void QCoreApplicationProto::sendPostedEvents()
{
  QCoreApplication::sendPostedEvents();
}
#endif

void QCoreApplicationProto::setApplicationName(const QString &application)
{
  QCoreApplication::setApplicationName(application);
}

void QCoreApplicationProto::setApplicationVersion(const QString &version)
{
  QCoreApplication::setApplicationVersion(version);
}

void QCoreApplicationProto::setAttribute(Qt::ApplicationAttribute attribute, bool on)
{
  QCoreApplication::setAttribute(attribute, on);
}

void QCoreApplicationProto::setLibraryPaths(const QStringList &paths)
{
  QCoreApplication::setLibraryPaths(paths);
}

void QCoreApplicationProto::setOrganizationDomain(const QString &orgDomain)
{
  QCoreApplication::setOrganizationDomain(orgDomain);
}

void QCoreApplicationProto::setOrganizationName(const QString &orgName)
{
  QCoreApplication::setOrganizationName(orgName);
}

#if QT_VERSION >= 0x050000
void QCoreApplicationProto::setQuitLockEnabled(bool enabled)
{
  QCoreApplication::setQuitLockEnabled(enabled);
}

void QCoreApplicationProto::setSetuidAllowed(bool allow)
{
  QCoreApplication::setSetuidAllowed(allow);
}
#endif

bool QCoreApplicationProto::startingUp()
{
  return QCoreApplication::startingUp();
}

bool QCoreApplicationProto::testAttribute(Qt::ApplicationAttribute attribute)
{
  return QCoreApplication::testAttribute(attribute);
}

#if QT_VERSION < 0x050000
QString QCoreApplicationProto::translate(const char *context, const char *sourceText, const char *disambiguation, QCoreApplication::Encoding encoding, int n)
{
  return QCoreApplication::translate(context, sourceText, disambiguation, encoding, n);
}

QString QCoreApplicationProto::translate(const char *context, const char *sourceText, const char *disambiguation, QCoreApplication::Encoding encoding)
{
  return QCoreApplication::translate(context, sourceText, disambiguation, encoding);
}
#else
QString QCoreApplicationProto::translate(const char *context, const char *sourceText, const char *disambiguation, int n)
{
  return QCoreApplication::translate(context, sourceText, disambiguation, n);
}
#endif

QString QCoreApplicationProto::toString()
{
  return QCoreApplication::applicationName() + " "
       + QCoreApplication::arguments().join(" ");
}
