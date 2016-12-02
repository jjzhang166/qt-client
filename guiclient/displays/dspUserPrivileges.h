/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DSPUSERPRIVILEGES_H__
#define __DSPUSERPRIVILEGES_H__

#include "guiclient.h"
#include "display.h"

class dspUserPrivileges : public display
{
    Q_OBJECT

public:
    dspUserPrivileges(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

    virtual bool setParams(ParameterList & params);

public slots:
    virtual void sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int pColumn);
    virtual void sPreview();
    virtual void sPrint();
    virtual void sEditUser();
    virtual void sEditRole();

private:
    bool     _printing;
    QString  _grpSql;

};

#endif // __DSPUSERPRIVILEGES_H__
