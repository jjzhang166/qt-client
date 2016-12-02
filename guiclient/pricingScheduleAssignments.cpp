/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "pricingScheduleAssignments.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "errorReporter.h"
#include "mqlutil.h"
#include "pricingScheduleAssignment.h"

/*
 *  Constructs a pricingScheduleAssignments as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
pricingScheduleAssignments::pricingScheduleAssignments(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_ipsass, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

//  statusBar()->hide();

  _ipsass->addColumn(tr("Ship-To"),          _itemColumn, Qt::AlignCenter, true, "shiptonum" );
  _ipsass->addColumn(tr("Customer #"),       _itemColumn, Qt::AlignCenter, true, "custnumber" );
  _ipsass->addColumn(tr("Cust. Name"),       150,         Qt::AlignCenter, true, "custname" );
  _ipsass->addColumn(tr("Cust. Type"),       _itemColumn, Qt::AlignCenter, true, "custtype" );
  _ipsass->addColumn(tr("Ship Zone"),       _itemColumn, Qt::AlignCenter, true, "shipzone" );
  _ipsass->addColumn(tr("Sale Type"),       _itemColumn, Qt::AlignCenter, true, "saletype" );
  _ipsass->addColumn(tr("Pricing Schedule"), -1,          Qt::AlignCenter, true, "ipshead_name" );

  if (_privileges->check("AssignPricingSchedules"))
  {
    connect(_ipsass, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipsass, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipsass, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_ipsass, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  _listpricesched = false;
}

/*
 *  Destroys the object and frees any allocated resources
 */
pricingScheduleAssignments::~pricingScheduleAssignments()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void pricingScheduleAssignments::languageChange()
{
    retranslateUi(this);
}

enum SetResponse pricingScheduleAssignments::set(const ParameterList &pParams)
{
  bool     valid;
  QVariant param;
  
  param = pParams.value("listpricesched", &valid);
  if (valid)
  {
    _listpricesched = true;
    setWindowTitle(tr("List Pricing Schedule Assignments"));
  }
  
  sFillList();
  
  return NoError;
}

void pricingScheduleAssignments::sPrint()
{
  orReport report("PricingScheduleAssignments");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void pricingScheduleAssignments::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_listpricesched)
    params.append("listpricesched", true);

  pricingScheduleAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void pricingScheduleAssignments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipsass_id", _ipsass->id());
  if (_listpricesched)
    params.append("listpricesched", true);

  pricingScheduleAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void pricingScheduleAssignments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ipsass_id", _ipsass->id());

  pricingScheduleAssignment newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void pricingScheduleAssignments::sDelete()
{
  XSqlQuery pricingDelete;
  pricingDelete.prepare( "DELETE FROM ipsass "
             "WHERE (ipsass_id=:ipsass_id);" );
  pricingDelete.bindValue(":ipsass_id", _ipsass->id());
  pricingDelete.exec();

  sFillList();
}

void pricingScheduleAssignments::sFillList()
{
  MetaSQLQuery mql = mqlLoad("pricingScheduleAssignment", "detail");

  ParameterList params;
  if (_listpricesched)
    params.append("listpricesched", true);
  XSqlQuery ps = mql.toQuery(params);
  if(!ErrorReporter::error(QtCriticalMsg, this, tr("Pricing Schedule Assignments "),
                         ps.lastError(), __FILE__, __LINE__))
  {
    _ipsass->populate(ps, true);
  }
}
