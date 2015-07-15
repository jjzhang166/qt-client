/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef TOGGLEBANKRECCLEARED_H
#define TOGGLEBANKRECCLEARED_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_toggleBankrecCleared.h"

class toggleBankrecCleared : public XDialog, public Ui::toggleBankrecCleared
{
    Q_OBJECT

public:
    toggleBankrecCleared(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~toggleBankrecCleared();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams );
    virtual void sSave();
    virtual void populate();
    virtual void populateReceipt();
    virtual void populateCheck();

protected slots:
    virtual void languageChange();

private:
    int _bankaccntid;
    int _bankrecid;
    int _sourceid;
    double _baseamount;
    QString _source;
    QString _transtype;

};

#endif // TOGGLEBANKRECCLEARED_H
