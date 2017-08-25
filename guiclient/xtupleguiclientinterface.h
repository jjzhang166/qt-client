/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guiclientinterface.h"

class xTupleGuiClientInterface : public GuiClientInterface
{
  Q_OBJECT 

  public:
    xTupleGuiClientInterface(QObject *pParent);

    virtual QWidget          *openWindow(const QString pname, ParameterList pparams, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0);
    virtual QAction          *findAction(const QString pname);
    virtual void              addDocumentWatch(QString path, int id);
    virtual void              removeDocumentWatch(QString path);
    virtual bool              hunspell_ready();
    virtual int               hunspell_check(const QString word);
    virtual const QStringList hunspell_suggest(const QString word);
    virtual int               hunspell_add(const QString word);
    virtual int               hunspell_ignore(const QString word);

    virtual Metrics     *getMetrics();
    virtual Metricsenc  *getMetricsenc();
    virtual Preferences *getPreferences();
    virtual Privileges  *getPrivileges();
};
