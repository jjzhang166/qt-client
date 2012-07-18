/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CURRENCYCONVERSION_H
#define CURRENCYCONVERSION_H

#include "guiclient.h"
#include "xdialog.h"
#include "parameter.h"

#include "ui_currencyConversion.h"

class currencyConversion : public XDialog, public Ui::currencyConversion
{
    Q_OBJECT

public:
    currencyConversion(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~currencyConversion();

    virtual SetResponse set(const ParameterList & pParams);

public slots:
    virtual void sClose();
    virtual void sHandleInactive();
    virtual void sSave();
    virtual void sPopulate();

protected slots:
    virtual void languageChange();

private:
    int _curr_id;
    int _curr_rate_id;
    int _mode;

};

#endif // CURRENCYCONVERSION_H
