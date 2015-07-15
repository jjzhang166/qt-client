/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTINVOICE_H
#define PRINTINVOICE_H

#include "printMulticopyDocument.h"
#include "ui_printInvoice.h"

class printInvoice : public printMulticopyDocument,
                     public Ui::printInvoice
{
    Q_OBJECT

  public:
    printInvoice(QWidget *parent = 0, const char *name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printInvoice();

  protected slots:
    virtual void languageChange();
    virtual void sHandlePopulated(XSqlQuery *qry);
    virtual void sHandleDocUpdated(int docid);

  protected:

};

#endif // PRINTINVOICE_H
