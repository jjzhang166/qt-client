/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UIFORMCHOOSER_H
#define UIFORMCHOOSER_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_uiformchooser.h"

class uiformchooser : public XDialog, public Ui::uiformchooser
{
  Q_OBJECT

  public:
    explicit uiformchooser(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);

  public slots:
    void populate(const XSqlQuery pQuery);
    void sSelect();
    void sCancel();

  protected slots:
    virtual void languageChange();
};

#endif // UIFORMCHOOSER_H
