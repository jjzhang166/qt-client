/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef SALETYPES_H
#define SALETYPES_H

#include "xwidget.h"
#include "ui_saleTypes.h"

class saleTypes : public XWidget, public Ui::saleTypes
{
    Q_OBJECT

public:
    saleTypes(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~saleTypes();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sFillList( int pId );
    virtual void sPrint();

protected slots:
    virtual void languageChange();

};

#endif // SALETYPES_H
