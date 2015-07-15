/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSTATEMENTBYCUSTOMER_H
#define PRINTSTATEMENTBYCUSTOMER_H

#include "printSinglecopyDocument.h"
#include "ui_printStatementByCustomer.h"

class printStatementByCustomer : public printSinglecopyDocument,
                                 public Ui::printStatementByCustomer
{
    Q_OBJECT

  public:
    printStatementByCustomer(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printStatementByCustomer();

    Q_INVOKABLE virtual void          clear();
    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParams();
    Q_INVOKABLE virtual bool          isOkToPrint();

  public slots:
    virtual enum SetResponse set(const ParameterList &pParams);

    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();
};

#endif // PRINTSTATEMENTBYCUSTOMER_H
