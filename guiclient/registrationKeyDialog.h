/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REGISTRATIONKEYDIALOG_H
#define REGISTRATIONKEYDIALOG_H

#include "guiclient.h"
#include <parameter.h>
#include "ui_registrationKeyDialog.h"

class registrationKeyDialog : public QDialog, public Ui::registrationKeyDialog
{
    Q_OBJECT

public:
    registrationKeyDialog(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~registrationKeyDialog();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sCheckKey();
    virtual void sSelect();
    virtual void sClose();

protected slots:
    virtual void languageChange();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    int _bankaccntid;
    QString _type;

};

#endif // REGISTRATIONKEYDIALOG_H
