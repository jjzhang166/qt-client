/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CONTRACT_H
#define CONTRACT_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_contract.h"

class contract : public XWidget, public Ui::contract
{
    Q_OBJECT

public:
//    contract(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
	contract(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~contract();

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool sSave();
    virtual void sSaveClicked();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * pSelected );
    virtual void sNewItemSrc();
    virtual void sEditItemSrc();
    virtual void sNewPo();
    virtual void sEditPo();
    virtual void sViewPo();
    virtual void sDeletePo();
    virtual void sReleasePo();
    virtual void sNewRcpt();
    virtual void sNewRtrn();
    virtual void sPrint();
	virtual void sHandleButtons(XTreeWidgetItem *pItem, int pCol);
	virtual void sFillList();
    virtual void populate();
    virtual void sRejected();

protected slots:
    virtual void languageChange();

private:
    int  _mode;
    int  _contrctid;
    bool _captive;
    bool _new;
 
};

#endif // CONTRACT_H
