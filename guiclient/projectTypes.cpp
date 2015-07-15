/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to y6ou under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projectTypes.h"

#include <parameter.h>
#include "projectType.h"
#include "guiclient.h"

projectTypes::projectTypes(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_projecttype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

  _projecttype->addColumn(tr("Code"),        -1, Qt::AlignLeft, true, "prjtype_code" );
  _projecttype->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "prjtype_descr" );
  _projecttype->addColumn(tr("Active"),      -1, Qt::AlignLeft, true, "prjtype_active" );
  
  if (_privileges->check("MaintainProjectTypes"))
  {
    connect(_projecttype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_projecttype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_projecttype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
  }

  connect(omfgThis, SIGNAL(itemGroupsUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

projectTypes::~projectTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

void projectTypes::languageChange()
{
  retranslateUi(this);
}

void projectTypes::sDelete()
{
  XSqlQuery typeDelete;
  typeDelete.prepare( "DELETE FROM prjtype "
             "WHERE (prjtype_id=:prjtype_id);" );
  typeDelete.bindValue(":prjtype_id", _projecttype->id());
  typeDelete.exec();

  sFillList();
}


void projectTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  projectType *newdlg = new projectType();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void projectTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtype_id", _projecttype->id());

  projectType *newdlg = new projectType();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void projectTypes::sFillList()
{
  _projecttype->populate( "SELECT * "
                      "FROM prjtype "
                      "ORDER BY prjtype_code;" );
}

