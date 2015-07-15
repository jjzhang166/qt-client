/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentCategories.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "incidentCategory.h"

incidentCategories::incidentCategories(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_close,  SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_incidentCategories, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_incidentCategories, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new,   SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,  SIGNAL(clicked()), this, SLOT(sView()));

  _incidentCategories->addColumn(tr("Order"),  _seqColumn, Qt::AlignRight, true, "incdtcat_order");
  _incidentCategories->addColumn(tr("Category"),      100, Qt::AlignLeft, true, "incdtcat_name");
  _incidentCategories->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "incdtcat_descrip");

  if (_privileges->check("MaintainIncidentCategories"))
  {
    connect(_incidentCategories, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_incidentCategories, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_incidentCategories, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_incidentCategories, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

incidentCategories::~incidentCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentCategories::languageChange()
{
    retranslateUi(this);
}

void incidentCategories::sFillList()
{
  XSqlQuery incidentFillList;
  incidentFillList.prepare( "SELECT incdtcat_id, incdtcat_order, "
	     "       incdtcat_name, firstLine(incdtcat_descrip) AS incdtcat_descrip "
             "FROM incdtcat "
             "ORDER BY incdtcat_order, incdtcat_name;" );
  incidentFillList.exec();
  if (incidentFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, incidentFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _incidentCategories->populate(incidentFillList);
}

void incidentCategories::sDelete()
{
  XSqlQuery incidentDelete;
  incidentDelete.prepare( "DELETE FROM incdtcat "
             "WHERE (incdtcat_id=:incdtcat_id);" );
  incidentDelete.bindValue(":incdtcat_id", _incidentCategories->id());
  incidentDelete.exec();
  if (incidentDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, incidentDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void incidentCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incidentCategory newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdtcat_id", _incidentCategories->id());

  incidentCategory newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void incidentCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdtcat_id", _incidentCategories->id());

  incidentCategory newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void incidentCategories::sPopulateMenu( QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentCategories"));

  pMenu->addAction(tr("View..."), this, SLOT(sView()));

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainIncidentCategories"));
}

void incidentCategories::sPrint()
{
  orReport report("IncidentCategoriesList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
