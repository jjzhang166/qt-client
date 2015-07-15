/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTCREDITMEMO_H
#define PRINTCREDITMEMO_H

#include "printMulticopyDocument.h"
#include "ui_printCreditMemo.h"

class printCreditMemo : public printMulticopyDocument,
                        public Ui::printCreditMemo
{
    Q_OBJECT

  public:
    printCreditMemo(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printCreditMemo();

  protected slots:
    virtual void languageChange();
    virtual void sHandleDocUpdated(int docid);
    virtual void sHandlePopulated(XSqlQuery *qry);

  protected:

};

#endif // PRINTCREDITMEMO_H
