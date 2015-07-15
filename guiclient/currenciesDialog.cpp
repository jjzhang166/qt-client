/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currenciesDialog.h"
#include "currencies.h"

currenciesDialog::currenciesDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  if (name)
    setObjectName(name);

  this->setFixedSize(400, 400);
  _layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
  _currencies = new currencies(this, "", Qt::Widget);
  _msg = new QLabel(tr("Please select a base currency for this database."), this);
  _layout->addWidget(_msg);
  _layout->addWidget(_currencies);
  connect(_currencies->_close, SIGNAL(clicked()), this, SLOT(close()));
}

currenciesDialog::~currenciesDialog()
{
  // no need to delete child widgets, Qt does it all for us
}
