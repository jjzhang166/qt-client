/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSTATEMENTSBYCUSTOMERGROUP
#define PRINTSTATEMENTSBYCUSTOMERGROUP

#include "printSinglecopyDocument.h"
#include "ui_printStatementsByCustomerGroup.h"


class printStatementsByCustomerGroup : public printSinglecopyDocument,
                                       public Ui::printStatementsByCustomerGroup
{
  Q_OBJECT

  public:
    printStatementsByCustomerGroup(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printStatementsByCustomerGroup();

    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParams();

  public slots:
    virtual void clear();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();
};
#endif // PRINTSTATEMENTSBYCUSTOMERGROUP

