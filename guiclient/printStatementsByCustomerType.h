/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSTATEMENTSBYCUSTOMERTYPE_H
#define PRINTSTATEMENTSBYCUSTOMERTYPE_H

#include "printSinglecopyDocument.h"
#include "ui_printStatementsByCustomerType.h"

class printStatementsByCustomerType : public printSinglecopyDocument,
                                      public Ui::printStatementsByCustomerType
{
    Q_OBJECT

  public:
    printStatementsByCustomerType(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printStatementsByCustomerType();

    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParamsDocList();

  protected slots:
    virtual void languageChange();

};

#endif // PRINTSTATEMENTSBYCUSTOMERTYPE_H
