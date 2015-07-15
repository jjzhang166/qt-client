/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESREP_H
#define SALESREP_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_salesRep.h"

class salesRep : public XDialog, public Ui::salesRep
{
    Q_OBJECT

public:
    salesRep(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~salesRep();

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCheck();
    virtual bool sPopulate();
    virtual void sSave();
    virtual void reject();

protected slots:
    virtual void languageChange();

    virtual bool save();
    virtual void sCrmaccount();

signals:
    void populated();
    void newId(int);
    void newMode(int);
    void saved(int);

private:
    int _crmacctid;
    int _empid;
    int _mode;
    int _NumberGen;
    int _salesrepid;
    QString _crmowner;

};

#endif // SALESREP_H
