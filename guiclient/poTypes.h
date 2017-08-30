/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef POTYPES_H
#define POTYPES_H

#include "xwidget.h"
#include "ui_poTypes.h"

class poTypes : public XWidget, public Ui::poTypes
{
    Q_OBJECT

public:
    poTypes(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~poTypes();

public slots:
    virtual void sDelete();
    virtual void sDeleteUnused();
    virtual void sNew();
    virtual void sEdit();
    virtual void sPrint();
    virtual void sFillList();
    virtual void sNotified(const QString &notice);

protected slots:
    virtual void languageChange();

};

#endif // POTYPES_H
