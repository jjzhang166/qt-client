/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemPricingSchedules.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <metasql.h>
#include <parameter.h>
#include "itemPricingSchedule.h"
#include "errorReporter.h"
#include "mqlutil.h"

itemPricingSchedules::itemPricingSchedules(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteExpired, SIGNAL(clicked()), this, SLOT(sDeleteExpired()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_ipshead, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));

  _ipshead->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,   true,  "ipshead_name"   );
  _ipshead->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "ipshead_descrip"   );
  _ipshead->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter, true,  "ipshead_effective" );
  _ipshead->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter, true,  "ipshead_expires" );

  if (_privileges->check("MaintainPricingSchedules"))
  {
    connect(_ipshead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipshead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_ipshead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _listpricesched = false;
}

itemPricingSchedules::~itemPricingSchedules()
{
  // no need to delete child widgets, Qt does it all for us
}

void itemPricingSchedules::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingSchedules::set(const ParameterList &pParams)
{
  bool     valid;
  QVariant param;
  
  param = pParams.value("listpricesched", &valid);
  if (valid)
  {
    _listpricesched = true;
    setWindowTitle(tr("List Pricing Schedules"));
  }
  
  sFillList();
  
  return NoError;
}

void itemPricingSchedules::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_listpricesched)
    params.append("listpricesched", true);

  itemPricingSchedule newdlg(this, "", true);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    sFillList(result);
}

void itemPricingSchedules::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ipshead_id", _ipshead->id());
  if (_listpricesched)
    params.append("listpricesched", true);

  itemPricingSchedule newdlg(this, "", true);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    sFillList(result);
}

void itemPricingSchedules::sCopy()
{
  XSqlQuery itemCopy;
  itemCopy.prepare("SELECT copypricingschedule(:ipshead_id) AS result;");
  itemCopy.bindValue(":ipshead_id", _ipshead->id());
  itemCopy.exec();
  if (itemCopy.first())
  {
    ParameterList params;
    params.append("mode", "copy");
    params.append("ipshead_id", itemCopy.value("result").toInt());

    itemPricingSchedule newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();

    sFillList();
  }
  else if (itemCopy.lastError().type() != QSqlError::NoError)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying Item Pricing Schedule"),
                       itemCopy, __FILE__, __LINE__);
}

void itemPricingSchedules::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("ipshead_id", _ipshead->id());

  itemPricingSchedule newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void itemPricingSchedules::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare( "SELECT ipsass_id "
             "FROM ipsass "
             "WHERE (ipsass_ipshead_id=:ipshead_id) "
             "LIMIT 1;" );
  itemDelete.bindValue(":ipshead_id", _ipshead->id());
  itemDelete.exec();
  if (itemDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Pricing Schedule"),
                           tr( "The selected Pricing Schedule cannot be deleted as it has been assign to one or more Customers.\n"
                               "You must delete these assignments before you may delete the selected Pricing Schedule." ) );
    return;
  }

  itemDelete.prepare( "SELECT sale_id "
             "FROM sale "
             "WHERE (sale_ipshead_id=:ipshead_id) "
             "LIMIT 1;" );
  itemDelete.bindValue(":ipshead_id", _ipshead->id());
  itemDelete.exec();
  if (itemDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Pricing Schedule"),
                           tr( "The selected Pricing Schedule cannot be deleted as it has been assign to one or more Sales.\n"
                               "You must delete these assignments before you may delete the selected Pricing Schedule." ) );
    return;
  }

  itemDelete.prepare( "DELETE FROM ipsiteminfo "
             "WHERE (ipsitem_ipshead_id=:ipshead_id); "
             "DELETE FROM ipsfreight "
             "WHERE (ipsfreight_ipshead_id=:ipshead_id); "
             "DELETE FROM ipshead "
             "WHERE (ipshead_id=:ipshead_id);" );
  itemDelete.bindValue(":ipshead_id", _ipshead->id());
  itemDelete.exec();

  sFillList();
}

void itemPricingSchedules::sDeleteExpired()
{
  XSqlQuery itemDeleteExpired;
  int answer = QMessageBox::question(this, tr("Delete Expired?"),
                          tr("<p>This will permanently delete all "
						     "expired Pricing Schedules. <p> "
                             "OK to continue? "),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No);
  if (answer == QMessageBox::No)
    return;

  itemDeleteExpired.prepare("SELECT deleteexpiredips() AS result;");
  itemDeleteExpired.exec();
  if (itemDeleteExpired.lastError().type() != QSqlError::NoError)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Item Pricing Schedule"),
                       itemDeleteExpired, __FILE__, __LINE__);
	
  sFillList();
}

void itemPricingSchedules::sFillList()
{
  sFillList(-1);
}

void itemPricingSchedules::sFillList(int pIpsheadid)
{
  ParameterList params;
  
  if (!_showExpired->isChecked())
    params.append("showUnexpired", true);
  if (!_showFuture->isChecked())
    params.append("showUneffective", true);
  if (_listpricesched)
    params.append("listpricesched", true);

  MetaSQLQuery mql = mqlLoad("pricingSchedules", "detail");
  XSqlQuery qry = mql.toQuery(params);
  if (pIpsheadid == -1)
    _ipshead->populate(qry);
  else
    _ipshead->populate(qry, pIpsheadid);
}

void itemPricingSchedules::sSearch( const QString & pTarget)
{
  _ipshead->clearSelection();
  int i;
  for (i = 0; i < _ipshead->topLevelItemCount(); i++)
  {
    if (_ipshead->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _ipshead->topLevelItemCount())
  {
    _ipshead->setCurrentItem(_ipshead->topLevelItem(i));
    _ipshead->scrollToItem(_ipshead->topLevelItem(i));
  }
}
