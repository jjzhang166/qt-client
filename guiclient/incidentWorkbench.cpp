/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "incidentWorkbench.h"

#include <QSqlError>

#include <openreports.h>
#include <metasql.h>

#include "guiclient.h"
#include "incident.h"
#include "mqlutil.h"

incidentWorkbench::incidentWorkbench(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
	
	QSqlDatabase db = QSqlDatabase::database();

  QString qryStatus = QString("SELECT status_seq, "
                              " CASE WHEN status_code = 'N' THEN '%1' "
                              " WHEN status_code = 'F' THEN '%2' "
                              " WHEN status_code = 'C' THEN '%3' "
                              " WHEN status_code = 'A' THEN '%4' "
                              " WHEN status_code = 'R' THEN '%5' "
                              " WHEN status_code = 'L' THEN '%6' "
                              " END AS name, status_code AS code "
                              "FROM status; ")
      .arg(tr("New"))
      .arg(tr("Feedback"))
      .arg(tr("Confirmed"))
      .arg(tr("Assigned"))
      .arg(tr("Resolved"))
      .arg(tr("Closed"));

  parameterWidget->setType(tr("Owner"), "owner_username", ParameterWidget::User);
  parameterWidget->setType(tr("Assigned User"), "assigned_username", ParameterWidget::User, db.userName());
  parameterWidget->setType(tr("Assigned Pattern"), "assigned_usr_pattern", ParameterWidget::Text);
  parameterWidget->setType(tr("Owner Pattern"), "owner_usr_pattern", ParameterWidget::Text);
  parameterWidget->setType(tr("Start Date"), "startDate", ParameterWidget::Date);
  parameterWidget->setType(tr("End Date"), "endDate", ParameterWidget::Date);
  parameterWidget->setType(tr("CRM Account"), "crmAccountId", ParameterWidget::Crmacct);
  parameterWidget->setType(tr("Contact"),"cntct_id", ParameterWidget::Contact);
  parameterWidget->setXComboBoxType(tr("Severity"), "severity_id", XComboBox::IncidentSeverity);
  //parameterWidget->setXComboBoxType(tr("Category"), "category_id", XComboBox::IncidentCategory);
  parameterWidget->setType(tr("Category"), "categorylist",
                           ParameterWidget::Multiselect, 0,
                           "SELECT incdtcat_id, incdtcat_name"
                           "  FROM incdtcat"
                           " ORDER BY incdtcat_name;");
	parameterWidget->setType(tr("Status"), "statuslist",
                           ParameterWidget::Multiselect, 0,
                           qryStatus);
  //parameterWidget->setXComboBoxType(tr("Status is"), "status_equal", qryStatus);
  parameterWidget->setXComboBoxType(tr("Status Above"), "status_above", qryStatus, 4);

  _closeAct = new QAction(tr("Close"), this);
  _closeAct->setShortcut(QKeySequence::Close);
  _close->addAction(_closeAct);
  
  _queryAct = new QAction(tr("Query"), this);

  _printAct = new QAction(tr("Print"), this);
  _printAct->setShortcut(QKeySequence::Print);
  _print->addAction(_printAct);

  _newAct = new QAction(tr("New"), this);
  _newAct->setShortcut(QKeySequence::New);
  _new->addAction(_newAct);

  parameterWidget->applyDefaultFilterSet();

  connect(_autoUpdate,	SIGNAL(toggled(bool)),	this,	SLOT(sHandleAutoUpdate(bool)));
  connect(_close,       SIGNAL(clicked()),      this,   SLOT(close()));
  connect(_closeAct,    SIGNAL(triggered()),    this,   SLOT(close()));
  connect(_new,         SIGNAL(clicked()),      this,   SLOT(sNew()));
  connect(_newAct,	SIGNAL(triggered()),	this,	SLOT(sNew()));
  connect(_print,       SIGNAL(clicked()),      this,   SLOT(sPrint()));
  connect(_printAct,	SIGNAL(triggered()),	this,	SLOT(sPrint()));
  connect(_queryAct,    SIGNAL(triggered()),    this,   SLOT(sFillList()));
  connect(_query,	SIGNAL(clicked()),	this,	SLOT(sFillList()));
  connect(_incdt,       SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  connect(_incdt,       SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateShiptoMenu(QMenu*)));

  _incdt->addColumn(tr("Number"),      _orderColumn,Qt::AlignLeft, true, "incdt_number" );
  _incdt->addColumn(tr("Created"),     _dateColumn, Qt::AlignLeft, true, "incdt_timestamp" );
  _incdt->addColumn(tr("Account"),     _itemColumn, Qt::AlignLeft, true, "crmacct_name" );
  _incdt->addColumn(tr("Status"),      _itemColumn, Qt::AlignLeft, true, "incdt_status" );
  _incdt->addColumn(tr("Updated"),     _dateColumn, Qt::AlignLeft, true, "incdt_updated" );
  _incdt->addColumn(tr("Assigned To"), _userColumn, Qt::AlignLeft, true, "incdt_assigned_username" ); 
  _incdt->addColumn(tr("Owner"),       _userColumn, Qt::AlignLeft, true, "incdt_owner_username" );
  _incdt->addColumn(tr("Summary"),     -1,          Qt::AlignLeft, true, "incdt_summary" );
  _incdt->addColumn(tr("Category"),    _userColumn, Qt::AlignLeft, false, "incdtcat_name");
  _incdt->addColumn(tr("Severity"),    _userColumn, Qt::AlignLeft, false, "incdtseverity_name");
  _incdt->addColumn(tr("Priority"),    _userColumn, Qt::AlignLeft, false, "incdtpriority_name");
  _incdt->addColumn(tr("Contact"),     _userColumn, Qt::AlignLeft, false, "cntct_name");

  sFillList();
}

incidentWorkbench::~incidentWorkbench()
{
    // no need to delete child widgets, Qt does it all for us
}

void incidentWorkbench::languageChange()
{
    retranslateUi(this);
}

void incidentWorkbench::sPopulateShiptoMenu(QMenu *menu)
{
  menu->addAction(tr("Edit"), this, SLOT(sEdit()));
  menu->addAction(tr("View"), this, SLOT(sView()));
}

void incidentWorkbench::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void incidentWorkbench::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _incdt->id());

  incident newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void incidentWorkbench::setParams(ParameterList & params)
{
  params.append("new",		tr("New"));
  params.append("feedback",	tr("Feedback"));
  params.append("confirmed",	tr("Confirmed"));
  params.append("assigned",	tr("Assigned"));
  params.append("resolved",	tr("Resolved"));
  params.append("closed",	tr("Closed"));

  params.append("startDate", omfgThis->startOfTime());
  params.append("endDate", omfgThis->endOfTime());

  parameterWidget->appendValue(params);

  params.append("pattern", _search->text());
}

void incidentWorkbench::sFillList()
{
  MetaSQLQuery mql = mqlLoad("incidents", "detail");
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);

  _incdt->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void incidentWorkbench::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("IncidentWorkbenchList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void incidentWorkbench::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

