/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOM_H
#define BOM_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_bom.h"

class BOM : public XWidget, public Ui::BOM
{
    Q_OBJECT

public:
    BOM(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~BOM();
    virtual bool save( bool partial );

    Q_INVOKABLE virtual int id()   const;
    Q_INVOKABLE virtual int mode() const;
  
public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual bool setParams(ParameterList &pParams);
    virtual bool sSave();
    virtual void sPrint();
    virtual void sPopulateMenu( QMenu * menuThis );
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sExpire();
    virtual void sDelete();
    virtual void sReplace();
    virtual void sMoveUp();
    virtual void sMoveDown();
    virtual void sFillList();
    virtual void sFillList( int pItemid, bool );
    virtual void sClose();
    virtual bool sCheckRequiredQtyPer();

protected slots:
    virtual void languageChange();

  signals:
    void populated();
    void newId(int);
    void newMode(int);
    void saved(int);
  
private:
    int _mode;
    int _bomheadid;
    int _revid;
    double _totalQtyPerCache;

};

#endif // BOM_H
