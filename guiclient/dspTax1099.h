/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPTAX1099_H
#define DSPTAX1099_H

#include "xwidget.h"

#include "ui_dspTax1099.h"

class dspTax1099 : public XWidget, public Ui::dspTax1099
{
    Q_OBJECT

public:
    dspTax1099(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~dspTax1099();

public slots:
    virtual void sFillList();
    virtual void setParams(ParameterList &);
    virtual void sChg1099();
    virtual void sSetAccount();
    virtual void sPrint();
    virtual void sPrint1099();
    virtual void sPrint1096();

protected slots:
    virtual void languageChange();

};

#endif // dspTax1099_H
