/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTPACKINGLIST_H
#define PRINTPACKINGLIST_H

#include "printSinglecopyDocument.h"
#include "ui_printPackingList.h"

class printPackingListPrivate;

class printPackingList : public printSinglecopyDocument,
                         public Ui::printPackingList
{
    Q_OBJECT

  public:
    printPackingList(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~printPackingList();

    Q_INVOKABLE virtual void          clear();
                virtual QString       doctype();
    Q_INVOKABLE virtual ParameterList getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList getParamsDocList();
    Q_INVOKABLE virtual bool          isOkToPrint();
                virtual QString       reportKey();

  public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sHandleShipment();
    virtual void sHandleReprint();
    virtual void sPopulate();

  protected slots:
    virtual void languageChange();

  protected:
    printPackingListPrivate *_pldata;
};

#endif // PRINTPACKINGLIST_H
