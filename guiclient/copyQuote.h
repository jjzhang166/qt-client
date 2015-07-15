/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYQUOTE_H
#define COPYQUOTE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copyQuote.h"

class copyQuote : public XDialog, public Ui::copyQuote
{
    Q_OBJECT

public:
    copyQuote(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~copyQuote();

public slots:
    virtual SetResponse set(const ParameterList & pParams);
    virtual void setId(int);
    virtual void sCopy();

protected slots:
    virtual void languageChange();

signals:
    void newId(int);

private:
    int _quoteid;

};

#endif // COPYQUOTE_H
