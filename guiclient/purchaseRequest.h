/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PURCHASEREQUEST_H
#define PURCHASEREQUEST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_purchaseRequest.h"

class purchaseRequest : public XDialog, public Ui::purchaseRequest
{
    Q_OBJECT

public:
    purchaseRequest(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~purchaseRequest();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sClose();
    virtual void sSave();
    virtual void populate();
    virtual void populateNumber();
    virtual void closeEvent( QCloseEvent * pEvent );
    virtual void sNumberChanged();
    virtual void sReleaseNumber();

protected slots:
    virtual void languageChange();

    virtual void sCheckWarehouse(int);


private:
    int _mode;
    bool _captive;
    int _prid;
    int _planordid;
    int _lastWarehousid;
    int _NumberGen;

};

#endif // PURCHASEREQUEST_H
