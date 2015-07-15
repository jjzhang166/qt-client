/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RECONCILEBANKACCOUNT_H
#define RECONCILEBANKACCOUNT_H

#include "guiclient.h"

#include "xwidget.h"

#include "ui_reconcileBankaccount.h"

class reconcileBankaccount : public XWidget, public Ui::reconcileBankaccount
{
    Q_OBJECT

public:
    reconcileBankaccount(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~reconcileBankaccount();

    Q_INVOKABLE static void   openReconcileBankaccount( int pBankaccntid, QWidget *parent = 0 );

public slots:
    virtual void populate();
    virtual void sAddAdjustment();
    virtual void sBankaccntChanged();
    virtual void sCancel();
    virtual void sChecksToggleCleared();
    virtual void sImport();
    virtual void sReceiptsToggleCleared();
    virtual void sReconcile();
    virtual bool sSave(bool = true);
    virtual void sDateChanged();

protected slots:
    virtual void languageChange();

private:
    int _bankrecid;
	int _bankaccntid;
    bool _datesAreOK;

};

#endif // RECONCILEBANKACCOUNT_H
