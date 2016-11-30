/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALESORDERSIMPLE_H
#define SALESORDERSIMPLE_H

#include "applock.h"
#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>
#include "ui_salesOrderSimple.h"

class salesOrderSimple : public XWidget, public Ui::salesOrderSimple
{
  Q_OBJECT

  public: salesOrderSimple(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags fl = Qt::Window);
    ~salesOrderSimple();

    Q_INVOKABLE virtual bool  sSave();
    Q_INVOKABLE virtual bool  save( bool partial );
    Q_INVOKABLE static void   newSalesOrder();
//  Q_INVOKABLE static void   newSalesOrder( int pCustid, QWidget *parent = 0 );
    Q_INVOKABLE virtual int   id() { return _soheadid; }

  public slots:
    virtual SetResponse set(const ParameterList &pParams );
    virtual void        sChangeState( int pState );
    virtual void        sHoldClicked();
    virtual void        sCompleteOrder();
    virtual void        sSaveLine();
    virtual void        sPopulateMenu(QMenu *pMenu);
    virtual void        populateOrderNumber();
    virtual void        sSetUserEnteredOrderNumber();
    virtual void        sHandleOrderNumber();
    virtual void        sPopulateCustomerInfo( int pCustid );
    virtual void        sPopulateShiptoInfo();
    virtual void        sPopulateWhsInfo();
    virtual void        sPopulateTaxZone();
    virtual void        sPopulateItemInfo();
    virtual void        sHandleRequiredFields();
    virtual void        setItemExtraClause();
    virtual void        populate();
    virtual void        sFillItemList();
    virtual void        sCalculateTotal();
    virtual void        prepare();
    virtual void        prepareLine();
    virtual void        sClose();
    virtual void        closeEvent( QCloseEvent *pEvent );
    virtual void        sTaxDetail();
    virtual void        populateCCInfo();
    virtual void        sNewCreditCard();
    virtual void        sViewCreditCard();
    virtual void        sFillCcardList();
    virtual void        sChargeCC();
    virtual void        sCreditAllocate();
    virtual void        sAllocateCreditMemos();
    virtual bool        sIssueLineBalance();
    virtual bool        sShipInvoice();
    virtual void        sHandleFundsType();
    virtual void        sEnterCashPayment();
    virtual void        sCalculateTax();
    virtual void        sRecalculatePrice();
    virtual void        sViewItemWorkbench();
    virtual void        sDelete();
    virtual void        sEdit();

  protected slots:
    virtual void  languageChange();
    virtual bool  okToProcessCC();

  signals:
    void populated();
    void newId(int);
    void saved(int);

  private:
    bool    deleteForCancel();
    bool    _saved;
    bool    _closeThis;
    int     _orderNumberGen;
    double  _authCC;
    double  _amountOutstanding;
    double  _amountAllocated;
    double  _taxableSubtotal;
    double  _creditlmt;
    bool    _userEnteredOrderNumber;
    bool    _ignoreSignals;
    bool    _blanketPos;
    bool    _usesPos;
    bool    _captive;
    int     _soheadid;
    int     _soitemid;
    int     _lineMode;
    AppLock _lock;
    int     _mode;
    int     _numSelected;
    int     _crmacctid;
};

#endif  // SALESORDERSIMPLE_H
