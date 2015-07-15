/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _PRINTQUOTE_H
#define _PRINTQUOTE_H

#include "printSinglecopyDocument.h"
#include "ui_printQuote.h"

class printQuote : public printSinglecopyDocument,
                   public Ui::printQuote
{
    Q_OBJECT

  public:
    printQuote(QWidget* parent = 0, const char* name = 0, bool modal = 0, Qt::WindowFlags fl = 0);
    ~printQuote();

    Q_INVOKABLE virtual ParameterList getParamsDocList();

  public slots:
    virtual void sFinishedWithAll();
    virtual void sHandleButtons();
    virtual void sHandleNewQuoteId();
    virtual void sPopulate(XSqlQuery *docq);

  protected slots:
    virtual void languageChange();
};

#endif // _PRINTQUOTE_H
