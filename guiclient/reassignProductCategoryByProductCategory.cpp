/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reassignProductCategoryByProductCategory.h"

#include "guiErrorCheck.h"
#include "metasql.h"

reassignProductCategoryByProductCategory::reassignProductCategoryByProductCategory(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_productCategoryPattern, SIGNAL(toggled(bool)), _productCategory, SLOT(setEnabled(bool)));
  connect(_selectedProductCategory, SIGNAL(toggled(bool)), _productCategories, SLOT(setEnabled(bool)));
  connect(_reassign, SIGNAL(clicked()), this, SLOT(sReassign()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _productCategories->setType(XComboBox::ProductCategories);
  _newProductCategory->setType(XComboBox::ProductCategories);
}

reassignProductCategoryByProductCategory::~reassignProductCategoryByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void reassignProductCategoryByProductCategory::languageChange()
{
  retranslateUi(this);
}

void reassignProductCategoryByProductCategory::sReassign()
{
  QList<GuiErrorCheck> errs;
  errs << GuiErrorCheck(_productCategoryPattern->isChecked() &&
                        _productCategory->text().trimmed().isEmpty(),
                        _productCategory, tr("You must enter a Product Category Pattern."));
  GuiErrorCheck::reportErrors(this, tr("Missing Product Category Pattern"), errs);

  MetaSQLQuery mql("UPDATE item"
                   "   SET item_prodcat_id=<? value('new_prodcat_id') ?>"
                   "<? if exists('old_prodcat_id') ?>"
                   " WHERE item_prodcat_id = <? value('old_prodcat_id') ?>"
                   "<? elseif exists('old_prodcat_code') ?>"
                   "  FROM prodcat"
                   " WHERE (item_prodcat_id = prodcat_id)"
                   "   AND (prodcat_code ~ <? value('old_prodcat_code') ?>)"
                   "<? endif ?>"
                   ";");
  ParameterList params;
  params.append("new_prodcat_id", _newProductCategory->id());
  if (_selectedProductCategory->isChecked())
    params.append("old_prodcat_id", _productCategories->id());
  else if (_productCategoryPattern->isChecked())
    params.append("old_prodcat_code", _productCategory->text());

  XSqlQuery reassign = mql.toQuery(params);

  accept();
}
