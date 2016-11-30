/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTTOFORM_H
#define PRINTTOFORM_H

#include "printSinglecopyDocument.h"
#include "ui_printToForm.h"

class printToForm : public printSinglecopyDocument,
                    public Ui::printToForm
{
    Q_OBJECT

  public:
    printToForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printToForm();

    Q_INVOKABLE virtual ParameterList getParamsDocList();

  public slots:
    virtual void sFinishedWithAll();
    virtual void sHandleButtons();
    virtual void sHandleNewOrderId();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();

};

#endif // printToForm_H
