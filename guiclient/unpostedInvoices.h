/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef UNPOSTEDINVOICES_H
#define UNPOSTEDINVOICES_H

#include "display.h"

#include "ui_unpostedInvoices.h"

class unpostedInvoices : public display, public Ui::unpostedInvoices
{
    Q_OBJECT

public:
    unpostedInvoices(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    virtual bool checkSitePrivs(int invcid);

public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sPrint();
    virtual void sPost();
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem *, int);
    virtual void sFillList();

};

#endif // UNPOSTEDINVOICES_H
