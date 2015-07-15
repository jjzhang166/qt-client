/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSHIPPINGFORMS_H
#define PRINTSHIPPINGFORMS_H

#include "printMulticopyDocument.h"
#include "ui_printShippingForms.h"

class printShippingForms : public printMulticopyDocument,
                           public Ui::printShippingForms
{
    Q_OBJECT

  public:
    printShippingForms(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printShippingForms();

  public slots:
    Q_INVOKABLE virtual ParameterList getParamsDocList();
    Q_INVOKABLE virtual ParameterList getParamsOneCopy(const int row, XSqlQuery *qry);
    Q_INVOKABLE virtual bool isOkToPrint();

  protected slots:
    virtual void languageChange();

};

#endif // PRINTSHIPPINGFORMS_H
