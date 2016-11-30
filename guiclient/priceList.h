/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRICELIST_H
#define PRICELIST_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_priceList.h"

class priceList : public XDialog, public Ui::priceList
{
    Q_OBJECT

public:
    priceList(QWidget* parent = 0, const char * = 0, Qt::WindowFlags fl = 0);
    ~priceList();

    double _selectedPrice;
    QString _selectedMethod;
    QString _selectedType;
    QString _selectedSale;
    QString _selectedSchedule;
    double _selectedBasis;
    double _selectedModifierPct;
    double _selectedModifierAmt;
    double _selectedQtyBreak;
    int _curr_id;

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void sSelect();
    virtual void sFillList();

protected:
    int _saletypeid;
    int _shiptoid;
    int _shipzoneid;
    QString _shiptonum;
    int _prodcatid;
    int _custtypeid;
    double _iteminvpricerat;
    QString _custtypecode;
    QString _listpriceschedule;
    QDate _effective;
    QDate _asOf;

protected slots:
    virtual void languageChange();
    virtual void sNewCust();
    virtual void sNewShipto();
    virtual void sNewItem();

};

#endif // PRICELIST_H
