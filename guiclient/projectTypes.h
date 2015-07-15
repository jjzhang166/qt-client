/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PROJECTTYPES_H
#define PROJECTTYPES_H

#include "xwidget.h"
#include "ui_projectTypes.h"

class projectTypes : public XWidget, public Ui::projectTypes
{
    Q_OBJECT

public:
    projectTypes(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);
    ~projectTypes();

public slots:
    virtual void sDelete();
    virtual void sNew();
    virtual void sEdit();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

};

#endif // PROJECTTYPES_H
