/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contracts.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "contract.h"
#include "copyContract.h"
#include "guiclient.h"
#include "parameterwidget.h"
#include "errorReporter.h"

contracts::contracts(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "contracts", fl)
{
  setWindowTitle(tr("Contracts"));
  setReportName("Contracts");
  setMetaSQLOptions("contracts", "detail");
  setUseAltId(false);
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);

  parameterWidget()->append(tr("Vendor"), "vend_id", ParameterWidget::Vendor);
  parameterWidget()->append(tr("Effective Start"), "effectiveStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Effective End"), "effectiveEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Expires Start"), "expireStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Expires End"), "expireEndDate", ParameterWidget::Date);

  list()->addColumn(tr("Vendor Number"),      -1,          Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Vendor Name"),        -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Contract Number"),    _itemColumn, Qt::AlignLeft,   true,  "contrct_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "contrct_descrip"   );
  list()->addColumn(tr("Effective"),          _dateColumn, Qt::AlignCenter, true,  "contrct_effective" );
  list()->addColumn(tr("Expires"),            _dateColumn, Qt::AlignLeft,   true,  "contrct_expires"   );
  list()->addColumn(tr("Item Count"),         _itemColumn, Qt::AlignLeft,   true,  "item_count"   );

  connect(omfgThis, SIGNAL(contractsUpdated(int, bool)), this, SLOT(sFillList()));

  if (_privileges->check("MaintainItemSources"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

bool contracts::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;
  params.append("always", tr("Always"));
  params.append("never", tr("Never"));

  return true;
}

void contracts::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));

  menuItem = menuThis->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));
}

void contracts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  contract *newdlg = new contract();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

//  contract newdlg(this, "", false);
//  newdlg.set(params);

//  newdlg.exec();
  sFillList();
}

void contracts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("contrct_id", list()->id());

  contract *newdlg = new contract();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

//  contract newdlg(this, "", true);
//  newdlg.set(params);

//  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contracts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("contrct_id", list()->id());

  contract *newdlg = new contract();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);

//  contract newdlg(this, "", true);
//  newdlg.set(params);
//  newdlg.exec();
}

void contracts::sCopy()
{
  ParameterList params;
  params.append("contrct_id", list()->id());

  copyContract newdlg(this, "", true);
  newdlg.set(params);
//  if (newdlg.exec() != XDialog::Rejected)
//    sFillList();
  newdlg.exec();
}

void contracts::sDelete()
{
  XSqlQuery itemDelete;
  if (QMessageBox::question(this, tr("Delete Contract"),
                            tr( "Are you sure that you want to delete the selected Contract?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::Yes)
  {
    if (QMessageBox::question(this, tr("Delete Contract"),
                              tr( "Do you want to deactivate the associated Item Sources?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
    {
      // itemsrcp deleted on cascade
      itemDelete.prepare( "UPDATE itemsrc SET itemsrc_active=false, "
                          "                   itemsrc_contrct_id=NULL "
                          "WHERE (itemsrc_contrct_id=:contrct_id);");
      itemDelete.bindValue(":contrct_id", list()->id());
      itemDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Contract"),
                                    itemDelete, __FILE__, __LINE__))
      {
        return;
      }
    }
    else
    {
      itemDelete.prepare( "UPDATE itemsrc SET itemsrc_contrct_id=NULL "
                          "WHERE (itemsrc_contrct_id=:contrct_id);");
      itemDelete.bindValue(":contrct_id", list()->id());
      itemDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Contract"),
                                    itemDelete, __FILE__, __LINE__))
      {
        return;
      }
    }

    itemDelete.prepare( "DELETE FROM contrct "
                        "WHERE (contrct_id=:contrct_id);");
    itemDelete.bindValue(":contrct_id", list()->id());
    itemDelete.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Contract"),
                                  itemDelete, __FILE__, __LINE__))
    {
      return;
    }

    sFillList();
  }
}
