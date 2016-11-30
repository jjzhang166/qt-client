/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PROJECT_H
#define PROJECT_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>
#include "ui_project.h"

class project : public XDialog, public Ui::project
{
    Q_OBJECT

public:
    project(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~project();

    static bool userHasPriv(const int = cView, const int = 0);

public slots:
    virtual SetResponse set( const ParameterList & pParams );
    virtual void populate();
    virtual void sPopulateMenu( QMenu * pMenu, QTreeWidgetItem * selected );
    virtual void sAssignedToChanged(const int);
    virtual void sStatusChanged(const int);
    virtual void sCRMAcctChanged(const int);
    virtual void sClose();
    virtual bool sSave(bool partial = false);
    virtual void sPrintTasks();
    virtual void sPrintOrders();
    virtual void sNewTask();
    virtual void sEditTask();
    virtual void sViewTask();
    virtual void sEditOrder();
    virtual void sViewOrder();
    virtual void sDeleteTask();
    virtual void sFillTaskList();
    virtual void sNewIncident();
    virtual void sNewQuotation();
    virtual void sNewSalesOrder();
    virtual void sNewPurchaseOrder();
    virtual void sNewWorkOrder();
    virtual void sNumberChanged();
    virtual void sHandleButtons(bool valid = false);

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _prjid;
    bool _saved;

signals:
    void saved(int);
    void populated(int);
    void deletedTask();

};

#endif // PROJECT_H
