/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updatePrices.h"

#include <QCloseEvent>
#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <metasql.h>
#include <parameter.h>
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "mqlutil.h"
#include "guiclient.h"
#include "xdoublevalidator.h"

updatePrices::updatePrices(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  XSqlQuery updateupdatePrices;
  setupUi(this);


  // signals and slots connections
  connect(_byItem,             SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_byItemGroup,        SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_byProductCategory,  SIGNAL(toggled(bool)), this, SLOT(sHandleBy(bool)));
  connect(_close,              SIGNAL(clicked()),     this, SLOT(close()));
  connect(_add,                SIGNAL(clicked()),     this, SLOT(sAdd()));
  connect(_addAll,             SIGNAL(clicked()),     this, SLOT(sAddAll()));
  connect(_remove,             SIGNAL(clicked()),     this, SLOT(sRemove()));
  connect(_removeAll,          SIGNAL(clicked()),     this, SLOT(sRemoveAll()));
  connect(_update,             SIGNAL(clicked()),     this, SLOT(sUpdate()));
  connect(_showEffective,      SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_showExpired,        SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_showCurrent,        SIGNAL(clicked()),     this, SLOT(populate()));
  connect(_value,              SIGNAL(clicked()),     this, SLOT(sHandleCharPrice()));
  connect(_percent,            SIGNAL(clicked()),     this, SLOT(sHandleCharPrice()));
  connect(_avail,              SIGNAL(itemSelected(int)), _add,    SLOT(animateClick()));
  connect(_sel,                SIGNAL(itemSelected(int)), _remove, SLOT(animateClick()));

  _updateBy->setValidator(new XDoubleValidator(-100, 9999, decimalPlaces("curr"), _updateBy));

  MetaSQLQuery mql = mqlLoad("updateprices", "createselsched");
  ParameterList params;
  updateupdatePrices = mql.toQuery(params);
  if (updateupdatePrices.lastError().type() != QSqlError::NoError)
    systemError(this, updateupdatePrices.lastError().databaseText(), __FILE__, __LINE__);

  _avail->addColumn(tr("Schedule"),      -1,          Qt::AlignLeft,  true,  "ipshead_name");
  _avail->addColumn(tr("Description"),   -1,          Qt::AlignLeft,  true,  "ipshead_descrip");
  _avail->addColumn(tr("Effective"),     -1,          Qt::AlignLeft,  true,  "ipshead_effective");
  _avail->addColumn(tr("Expires"),       -1,          Qt::AlignLeft,  true,  "ipshead_expires");
  _avail->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _sel->addColumn(tr("Schedule"),        -1,          Qt::AlignLeft,  true,  "ipshead_name");
  _sel->addColumn(tr("Description"),     -1,          Qt::AlignLeft,  true,  "ipshead_descrip");

  _group->hide();
  //	_value->setChecked(true);

  populate();
}

updatePrices::~updatePrices()
{
  // no need to delete child widgets, Qt does it all for us
}

void updatePrices::languageChange()
{
  retranslateUi(this);
}

void updatePrices::closeEvent(QCloseEvent * /*pEvent*/)
{
  XSqlQuery updatecloseEvent;
  MetaSQLQuery mql = mqlLoad("updateprices", "dropselsched");
  ParameterList params;
  updatecloseEvent = mql.toQuery(params);
  if (updatecloseEvent.lastError().type() != QSqlError::NoError)
    systemError(this, updatecloseEvent.lastError().databaseText(), __FILE__, __LINE__);
}

void updatePrices::sUpdate()
{
  XSqlQuery updateUpdate;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_byItem->isChecked() && !_item->isValid(), _item,
                          tr("You must select an Item to continue."))
         << GuiErrorCheck(!_sel->topLevelItemCount(), _sel,
                          tr("You must select a Pricing Schedule to continue."))
         << GuiErrorCheck(!_nominal->isChecked() && !_discount->isChecked() && !_markup->isChecked(), _nominal,
                          tr("You must select a least one Price Type to continue."))
         << GuiErrorCheck(_updateBy->toDouble() == 0.0, _updateBy,
                          tr("You must provide a Value to continue."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Update Prices"), errors))
    return;
  
  ParameterList params;
  
  if (_byItem->isChecked())
    params.append("item_id", _item->id());
  else
    _paramGroup->appendValue(params);
  if (_nominal->isChecked())
    params.append("nominal", true);
  if (_discount->isChecked())
    params.append("discount", true);
  if (_markup->isChecked())
    params.append("markup", true);
  params.append("updateBy", _updateBy->toDouble());
  if (_value->isChecked())
    params.append("updateByValue", true);
  else
    params.append("updateByPercent", true);

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  updateUpdate.exec("BEGIN;");

  MetaSQLQuery mql = mqlLoad("updateprices", "update");
  updateUpdate = mql.toQuery(params);
  if (updateUpdate.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, updateUpdate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_updateCharPrices->isChecked())
  {
    MetaSQLQuery mql2 = mqlLoad("updateprices", "updatechar");
    updateUpdate = mql2.toQuery(params);
    if (updateUpdate.lastError().type() != QSqlError::NoError)
    {
      rollback.exec();
      systemError(this, updateUpdate.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  updateUpdate.exec("COMMIT;");
  
  QMessageBox::information( this, tr("Success"),
                            tr("Update Completed.") );
  _updateBy->clear();
}

void updatePrices::populate()
{
  XSqlQuery updatepopulate;
  ParameterList params;
  if (_showEffective->isChecked())
    params.append("showEffective", true);
  if (_showExpired->isChecked())
    params.append("showExpired", true);
  if (_showCurrent->isChecked())
    params.append("showCurrent", true);

  MetaSQLQuery mql = mqlLoad("updateprices", "availsched");
  updatepopulate = mql.toQuery(params);
  if (updatepopulate.lastError().type() != QSqlError::NoError)
    systemError(this, updatepopulate.lastError().databaseText(), __FILE__, __LINE__);
  _avail->populate(updatepopulate);

  MetaSQLQuery mql2 = mqlLoad("updateprices", "selsched");
  updatepopulate = mql2.toQuery(params);
  if (updatepopulate.lastError().type() != QSqlError::NoError)
    systemError(this, updatepopulate.lastError().databaseText(), __FILE__, __LINE__);
  _sel->populate(updatepopulate);
}

void updatePrices::sAdd()
{
  XSqlQuery updateAdd;
  QList<XTreeWidgetItem*> selected = _avail->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    MetaSQLQuery mql = mqlLoad("updateprices", "add");
    ParameterList params;
    params.append("ipshead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    updateAdd = mql.toQuery(params);
    if (updateAdd.lastError().type() != QSqlError::NoError)
      systemError(this, updateAdd.lastError().databaseText(), __FILE__, __LINE__);
  }
  populate();
}

void updatePrices::sAddAll()
{
  XSqlQuery updateAddAll;
  ParameterList params;
  if (_showEffective->isChecked())
    params.append("showEffective", true);
  if (_showExpired->isChecked())
    params.append("showExpired", true);
  if (_showCurrent->isChecked())
    params.append("showCurrent", true);

  MetaSQLQuery mql = mqlLoad("updateprices", "add");
  updateAddAll = mql.toQuery(params);
  if (updateAddAll.lastError().type() != QSqlError::NoError)
    systemError(this, updateAddAll.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sRemove()
{
  XSqlQuery updateRemove;
  MetaSQLQuery mql = mqlLoad("updateprices", "remove");
  ParameterList params;
  params.append("ipshead_id", _sel->id());
  updateRemove = mql.toQuery(params);
  if (updateRemove.lastError().type() != QSqlError::NoError)
    systemError(this, updateRemove.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sRemoveAll()
{
  XSqlQuery updateRemoveAll;
  MetaSQLQuery mql = mqlLoad("updateprices", "remove");
  ParameterList params;
  updateRemoveAll = mql.toQuery(params);
  if (updateRemoveAll.lastError().type() != QSqlError::NoError)
    systemError(this, updateRemoveAll.lastError().databaseText(), __FILE__, __LINE__);
  populate();
}

void updatePrices::sHandleBy(bool toggled)
{
  if (!toggled)
    return;
  if (_byItem->isChecked())
  {
    _paramGroup->hide();
    _group->show();
  }
  else
  {
    _group->hide();
    _paramGroup->show();
    if (_byItemGroup->isChecked())
      _paramGroup->setType(ParameterGroup::ItemGroup);
    else
      _paramGroup->setType(ParameterGroup::ProductCategory);
  }
}

void updatePrices::sHandleCharPrice()
{
  // Only enable update char prices for percentage updates.
  _updateCharPrices->setEnabled( _percent->isChecked() );
}
