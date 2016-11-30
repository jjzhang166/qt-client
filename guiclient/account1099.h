/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ACCOUNT1099_H
#define ACCOUNT1099_H

#include "xwidget.h"

#include "ui_account1099.h"

class account1099 : public XWidget, public Ui::account1099
{
    Q_OBJECT

public:
    account1099(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~account1099();

public slots:
    virtual void sFillList();
    virtual void setParams(ParameterList &);
    virtual void sAddMisc();
    virtual void sAddRent();
    virtual void sRemove();

protected slots:
    virtual void languageChange();

};

#endif // account1099_H
