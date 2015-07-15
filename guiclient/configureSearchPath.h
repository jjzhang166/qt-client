/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONFIGURESEARCHPATH_H
#define CONFIGURESEARCHPATH_H

#include "xabstractconfigure.h"

#include "ui_configureSearchPath.h"

class configureSearchPathPrivate;

class configureSearchPath
  : public XAbstractConfigure, public Ui::configureSearchPath
{
  Q_OBJECT

  public:
    configureSearchPath(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~configureSearchPath();

  public slots:
    virtual bool sSave();

  protected slots:
    virtual void languageChange();

    virtual void sAdd();
    virtual void sAddAll();
    virtual void sHandleButtons();
    virtual void sMoveDown();
    virtual void sMoveUp();
    virtual void sPopulate();
    virtual void sRemove();
    virtual void sRemoveAll();

  signals:
    void saving();

  private:
    configureSearchPathPrivate *_private;
};

#endif
