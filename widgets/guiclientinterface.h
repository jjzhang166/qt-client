/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef guiclientinterface_h
#define guiclientinterface_h

#include "parameter.h"

#include <QString>
#include <QAction>

class Metrics;
class Metricsenc;
class Preferences;
class Privileges;

class GuiClientInterface : public QObject
{
  Q_OBJECT

  protected:
    GuiClientInterface(QObject *pParent) : QObject(pParent) { };

  public:
    virtual ~GuiClientInterface() {}
    virtual QWidget *openWindow(const QString pname, ParameterList pparams, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0) = 0;
    virtual QAction *findAction(const QString pname) = 0;
    virtual void addDocumentWatch(QString path, int id) = 0;
    virtual void removeDocumentWatch(QString path) = 0;

    virtual bool hunspell_ready() = 0;
    virtual int hunspell_check(const QString word) = 0;
    virtual const QStringList hunspell_suggest(const QString word) = 0;
    virtual int hunspell_add(const QString word) = 0;
    virtual int hunspell_ignore(const QString word) = 0;

    virtual Metrics     *getMetrics()               = 0;
    virtual Metricsenc  *getMetricsenc()            = 0;
    virtual Preferences *getPreferences()           = 0;
    virtual Privileges  *getPrivileges()            = 0;

  signals:
    void dbConnectionLost();
};

#endif
