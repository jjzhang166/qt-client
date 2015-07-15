/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSHIPPINGFORM_H
#define PRINTSHIPPINGFORM_H

#include "printMulticopyDocument.h"
#include "ui_printShippingForm.h"

class printShippingForm : public printMulticopyDocument,
                          public Ui::printShippingForm
{
    Q_OBJECT

  public:
    printShippingForm(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags fl = 0);
    ~printShippingForm();

    Q_INVOKABLE virtual void clear();
    Q_INVOKABLE virtual ParameterList getParamsDocList();
    Q_INVOKABLE virtual ParameterList getParamsOneCopy(const int row, XSqlQuery *qry);
    Q_INVOKABLE virtual bool isOkToPrint();
    Q_INVOKABLE virtual void populate();

  public slots:
    virtual void sHandleShipment();
    virtual void sHandleOrder();
    virtual void sHandleTo();
    virtual void sHandleSo();

  protected slots:
    virtual void languageChange();

  protected:
    virtual void depopulate();

};

#endif // PRINTSHIPPINGFORM_H
