/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDOR_H
#define VENDOR_H

#include "applock.h"
#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "contactcluster.h"
#include "ui_vendor.h"

class ParameterList;

class dspCheckRegister;
class dspPOsByVendor;
class dspPoItemReceivingsByVendor;
class dspVendorAPHistory;
class unappliedAPCreditMemos;
class selectPayments;

class vendor : public XWidget, public Ui::vendor
{
    Q_OBJECT

public:
    vendor(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~vendor();

    Q_INVOKABLE virtual int id() const;
    Q_INVOKABLE virtual int mode() const;

public slots:
    virtual SetResponse set(const ParameterList & pParams );
    virtual void setId(int);
    virtual bool sSave();
    virtual void sSaveClicked();
    virtual void sCheck();
    virtual bool sCheckRequired();
    virtual bool sPopulate();
    virtual void sPrintAddresses();
    virtual void sNewAddress();
    virtual void sEditAddress();
    virtual void setViewMode();
    virtual void sViewAddress();
    virtual void sDeleteAddress();
    virtual void sFillAddressList();
    virtual void sFillTaxregList();
    virtual void sNewTaxreg();
    virtual void sEditTaxreg();
    virtual void sViewTaxreg();
    virtual void sDeleteTaxreg();
    virtual void sHandleButtons();
    virtual void sNumberEdited();
    virtual void sSaveAndNewComment();

    virtual void sNext();
    virtual void sPrevious();

    virtual void clear();
    virtual void close();
    virtual void sNumberEditable(bool);
    virtual void sPrepare();

signals:
    void populated();
    void newId(int);
    void newMode(int);
    void saved(int);

protected slots:
    virtual void languageChange();
    virtual bool sCheckSave();
    virtual void sCrmAccount();
    virtual void sLoadCrmAcct(int);

protected:
    virtual void closeEvent(QCloseEvent*);
    QValidator                  *_accountValidator;
    QValidator                  *_routingValidator;
    unappliedAPCreditMemos      *_credits;
    selectPayments              *_payables;
    dspPOsByVendor              *_po;
    dspPoItemReceivingsByVendor *_receipts;
    dspVendorAPHistory          *_history;
    dspCheckRegister            *_checks;

private:
    int _mode;
    int _tempMode;
    int _vendid;
    int _crmacctid;
    int _NumberGen;
    AppLock _lock;
    QString _cachedNumber;
    bool _captive;
    bool _notice;
    QString _crmowner;

};

#endif // VENDOR_H
