/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHARACTERISTIC_H
#define CHARACTERISTIC_H

#include <QSqlTableModel>

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_characteristic.h"

class characteristicPrivate;

class characteristic : public XDialog, public Ui::characteristic
{
    Q_OBJECT

public:
    characteristic(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~characteristic();

    enum Type { Text, List, Date };

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void populate();
    virtual void sFillList();
    virtual void sNew();
    virtual void sDelete();
    virtual void sCharoptClicked(QModelIndex idx);

protected slots:
    virtual void languageChange();

    virtual void sSave();
    virtual void sCheck();


private:
    characteristicPrivate *_d;
};

#endif // CHARACTERISTIC_H
