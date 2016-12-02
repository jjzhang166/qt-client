/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QCOREAPPLICATIONPROTO_H__
#define __QCOREAPPLICATIONPROTO_H__

#include <QCoreApplication>
#include <QObject>
#include <QtScript>

class QAbstractEventDispatcher;
class QEvent;
class QString;

Q_DECLARE_METATYPE(QCoreApplication*)

void setupQCoreApplicationProto(QScriptEngine *engine);

class QCoreApplicationProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QCoreApplicationProto(QObject *parent = 0);

    Q_INVOKABLE void addLibraryPath(const QString &path);
    Q_INVOKABLE QString applicationDirPath();
    Q_INVOKABLE QString applicationFilePath();
    Q_INVOKABLE qint64 applicationPid();
    Q_INVOKABLE QString applicationVersion();
    Q_INVOKABLE QStringList arguments();
    Q_INVOKABLE bool closingDown();
    Q_INVOKABLE int exec();
    Q_INVOKABLE void exit(int returnCode);
    Q_INVOKABLE void flush();
    Q_INVOKABLE void installTranslator(QTranslator *translationFile);
    Q_INVOKABLE QCoreApplication *instance();
    Q_INVOKABLE QStringList libraryPaths();
    Q_INVOKABLE void processEvents(QEventLoop::ProcessEventsFlags flags);
    Q_INVOKABLE void processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime);
    Q_INVOKABLE void quit();
    Q_INVOKABLE void removeLibraryPath(const QString &path);
    Q_INVOKABLE void removePostedEvents(QObject *receiver);
    Q_INVOKABLE void removePostedEvents(QObject *receiver, int eventType);
    Q_INVOKABLE void removeTranslator(QTranslator *translationFile);
    Q_INVOKABLE bool sendEvent(QObject *receiver, QEvent *event);
    Q_INVOKABLE void setApplicationName(const QString &application);
    Q_INVOKABLE void setApplicationVersion(const QString &version);
    Q_INVOKABLE void setAttribute(Qt::ApplicationAttribute attribute, bool on);
    Q_INVOKABLE void setLibraryPaths(const QStringList &paths);
    Q_INVOKABLE void setOrganizationDomain(const QString &orgDomain);
    Q_INVOKABLE void setOrganizationName(const QString &orgName);
    Q_INVOKABLE bool startingUp();
    Q_INVOKABLE bool testAttribute(Qt::ApplicationAttribute attribute);

#if QT_VERSION < 0x050000
    Q_INVOKABLE bool hasPendingEvents();
    Q_INVOKABLE void postEvent(QObject *receiver, QEvent *event);
    Q_INVOKABLE void postEvent(QObject *receiver, QEvent *event, int priority);
    Q_INVOKABLE void sendPostedEvents(QObject *receiver, int event_type);
    Q_INVOKABLE void sendPostedEvents();
    Q_INVOKABLE QString translate(const char *context, const char *sourceText, const char *disambiguation, QCoreApplication::Encoding encoding, int n);
    Q_INVOKABLE QString translate(const char *context, const char *sourceText, const char *disambiguation, QCoreApplication::Encoding encoding);
#else
    Q_INVOKABLE QAbstractEventDispatcher *eventDispatcher();
    Q_INVOKABLE bool isQuitLockEnabled();
    Q_INVOKABLE bool isSetuidAllowed();
    Q_INVOKABLE void postEvent(QObject *receiver, QEvent *event, int priority = Qt::NormalEventPriority);
    Q_INVOKABLE void sendPostedEvents(QObject *receiver = 0, int event_type = 0);
    Q_INVOKABLE void setQuitLockEnabled(bool enabled);
    Q_INVOKABLE void setSetuidAllowed(bool allow);
    Q_INVOKABLE QString translate(const char *context, const char *sourceText, const char *disambiguation = 0, int n = -1);
#endif
    Q_INVOKABLE QString toString();

};

#endif
