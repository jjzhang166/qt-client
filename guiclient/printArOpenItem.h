/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTAROPENITEM_H
#define PRINTAROPENITEM_H

#include "printSinglecopyDocument.h"
#include "ui_printArOpenItem.h"

class printArOpenItem : public printSinglecopyDocument,
                        public Ui::printArOpenItem
{
    Q_OBJECT

  public:
    printArOpenItem(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printArOpenItem();

    Q_INVOKABLE virtual QString       doctype();
    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);

  public slots:
    virtual void sPopulate(XSqlQuery *qry);

  protected slots:
    virtual void languageChange();
};

#endif // PRINTAROPENITEM_H
