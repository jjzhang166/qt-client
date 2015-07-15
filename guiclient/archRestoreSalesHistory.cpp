/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "archRestoreSalesHistory.h"

#include <QVariant>

#include "metasql.h"

#include "errorReporter.h"

#define cArchive 0x01
#define cRestore  0x02

archRestoreSalesHistory::archRestoreSalesHistory(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSelect()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  _customerType->setType(ParameterGroup::CustomerType);
  _productCategory->setType(ParameterGroup::ProductCategory);
}

archRestoreSalesHistory::~archRestoreSalesHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void archRestoreSalesHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse archRestoreSalesHistory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("archieve", &valid);
  if (valid)
  {
    _mode = cArchive;

    setWindowTitle(tr("Archive Sales History"));
    _miscItems->setText(tr("Archive Freight, Sales Tax and Misc. Items"));
  }

  param = pParams.value("restore", &valid);
  if (valid)
  {
    _mode = cRestore;

    setWindowTitle(tr("Restore Sales History"));
    _miscItems->setText(tr("Restore Freight, Sales Tax and Misc. Items"));
  }

  return NoError;
}

void archRestoreSalesHistory::sSelect()
{
  // TODO: can we reduce the diffs between archive and restore cases?
  // TODO: can we reduce the diffs between miscitems and else cases?
  MetaSQLQuery mql("<? if exists('archive') ?>"
                   "SELECT archiveSalesHistory(cohist_id) "     
                   "  FROM custinfo"
                   "  JOIN custtype ON (cust_custtype_id=custtype_id)"
                   "  JOIN cohist   ON (cohist_cust_id=cust_id)"
                   "  LEFT OUTER JOIN itemsite ON (cohist_itemsite_id=itemsite_id)"
                   "  LEFT OUTER JOIN item     ON (itemsite_item_id=item_id)"
                   "  LEFT OUTER JOIN prodcat  ON (item_prodcat_id=prodcat_id)"
                   " WHERE (cohist_invcdate BETWEEN <? value('startDate') ?> AND <? value('endDate') ?>)"
                   "<? elseif exists('restore') ?>"
                   "SELECT restoreSalesHistory(asohist_id) "
                   "  FROM custinfo"
                   "  JOIN custtype ON (cust_custtype_id=custtype_id)"
                   "  JOIN asohist  ON (asohist_cust_id=cust_id)"
                   "  LEFT OUTER JOIN itemsite ON (asohist_itemsite_id=itemsite_id) "
                   "  LEFT OUTER JOIN item     ON (itemsite_item_id=item_id)"
                   "  LEFT OUTER JOIN prodcat  ON (item_prodcat_id=prodcat_id)"
                   "WHERE (asohist_invcdate BETWEEN <? value('startDate') ?> AND <? value('endDate') ?>)"
                   "<? endif ?>"
                   "<? if exists('miscitems') ?>"
                   "  <? if exists('warehous_id') ?>"
                   " AND ( (itemsite_id IS NULL) OR (itemsite_warehous_id=:warehous_id) )"
                   "  <? endif ?>"
                   "  <? if exists('prodcat_id') ?>"
                   " AND ( (itemsite_id IS NULL) OR (prodcat_id=<? value('prodcat_id') ?>) )"
                   "  <? elseif exists('prodcat_pattern') ?>"
                   " AND ( (itemsite_id IS NULL) OR (prodcat_code ~ <? value('prodcat_pattern') ?>) )"
                   "<? else ?>"
                   " AND (itemsite_id IS NOT NULL)"
                   "  <? if exists('warehous_id') ?>"
                   " AND (itemsite_warehous_id=:warehous_id)"
                   "  <? endif ?>"
                   "  <? if exists('prodcat_id') ?>"
                   " AND (prodcat_id=<? value('prodcat_id') ?>)"
                   "  <? elseif exists('prodcat_pattern') ?>"
                   " AND (prodcat_code ~ <? value('prodcat_pattern') ?>)"
                   "  <? endif ?>"
                   "<? endif ?>"
                   "<? if exists('custtype_id') ?>"
                   " AND (custtype_id=<? value('custtype_id') ?>)"
                   "<? elseif exists('custtype_pattern') ?>"
                   " AND (custtype_code ~ <? value('custtype_pattern') ?>)"
                   " );");
  ParameterList params;
  if (_mode == cArchive)
    params.append("archive");
  else if (_mode == cRestore)
    params.append("restore");
  
  if (_miscItems->isChecked())
    params.append("miscitems");

  _warehouse->appendValue(params);
  _customerType->appendValue(params);
  _productCategory->appendValue(params);
  _dates->appendValue(params);

  XSqlQuery qry = mql.toQuery(params);
  qry.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Processing Error"),
                           qry, __FILE__, __LINE__))
    reject();

  accept();
}

