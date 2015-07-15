/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTPURCHASEORDER_H
#define PRINTPURCHASEORDER_H

#include "printMulticopyDocument.h"
#include "ui_printPurchaseOrder.h"

class printPurchaseOrder : public printMulticopyDocument,
                           public Ui::printPurchaseOrder
{
    Q_OBJECT

  public:
    printPurchaseOrder(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printPurchaseOrder();

    Q_INVOKABLE virtual ParameterList getParamsOneCopy(const int row,
                                                       XSqlQuery *qry);
    Q_INVOKABLE virtual bool isOkToPrint();

  protected slots:
    virtual void languageChange();
    virtual void sHandleDocUpdated(int docid);
    virtual void sHandlePopulated(XSqlQuery *docq);
    virtual void sFinishedWithAll();

};

#endif // PRINTPURCHASEORDER_H
