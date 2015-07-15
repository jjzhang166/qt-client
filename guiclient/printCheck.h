/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTCHECK_H
#define PRINTCHECK_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_printCheck.h"

class QCloseEvent;

class printCheck : public XWidget, public Ui::printCheck
{
    Q_OBJECT

public:
    printCheck(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0);
    ~printCheck();

    static QString eftFileDir;

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void done(int r);
    virtual void sCreateEFT();
    virtual void sEnableCreateEFT();
    virtual void sHandleBankAccount( int pBankaccntid );
    virtual void sPrint();
    virtual void sPrintedAlready();
    virtual void populate( int pcheckid );

protected slots:
    virtual void languageChange();

protected:
    virtual void storeEftFileDir();

private:
    bool _captive;
    int  _setCheckNumber;

    virtual void markCheckAsPrinted(const int);
    virtual void sPrintImpl(bool printedAlready);

};

#endif // PRINTCHECK_H
