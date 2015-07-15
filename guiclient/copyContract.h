/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COPYCONTRACT_H
#define COPYCONTRACT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_copyContract.h"

class copyContract : public XDialog, public Ui::copyContract
{
    Q_OBJECT

public:
    copyContract(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~copyContract();

public slots:
    virtual SetResponse set(const ParameterList & pParams);
    virtual void sPopulateContractInfo();
    virtual void sCopy();

protected slots:
    virtual void languageChange();

private:
    bool _captive;
    int _contrctid;
    int _vendid;

};

#endif // COPYCONTRACT_H
