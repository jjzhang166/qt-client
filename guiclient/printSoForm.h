/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSOFORM_H
#define PRINTSOFORM_H

#include "printSinglecopyDocument.h"
#include "ui_printSoForm.h"
#include "guiclient.h"

class printSoForm : public printSinglecopyDocument,
                    public Ui::printSoForm
{
    Q_OBJECT

  public:
    printSoForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printSoForm();

    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParamsDocList();

public slots:
    virtual enum SetResponse set( const ParameterList & pParams);
    virtual void sFinishedWithAll();
    virtual void sHandleButtons();
    virtual void sHandleNewOrderId();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();

};

#endif // PRINTSOFORM_H
