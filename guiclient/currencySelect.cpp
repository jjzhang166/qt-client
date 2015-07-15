/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currencySelect.h"

#include <QSqlError>

currencySelect::currencySelect(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,	SIGNAL(clicked()),	this,	SLOT(sClose()));
  connect(_country,	SIGNAL(newID(int)),	this,	SLOT(sPopulate()));
  connect(_select,	SIGNAL(clicked()),	this,	SLOT(sSelect()));

  _country->clear();
  XSqlQuery country;
  country.prepare("SELECT country_id, country_name FROM country ORDER BY country_name;");
  country.exec();
  _country->populate(country, 0);
}

currencySelect::~currencySelect()
{
    // no need to delete child widgets, Qt does it all for us
}

void currencySelect::languageChange()
{
    retranslateUi(this);
}

void currencySelect::sSelect()
{
  done(_country->id());
}

void currencySelect::sClose()
{
  done(-1);
}

void currencySelect::sPopulate()
{
  XSqlQuery currencyPopulate;
  currencyPopulate.prepare("SELECT * FROM country WHERE country_id = :country_id;");
  currencyPopulate.bindValue(":country_id", _country->id());
  currencyPopulate.exec();
  if (currencyPopulate.first())
  {
    _currName->setText(currencyPopulate.value("country_curr_name").toString());
    _currAbbr->setText(currencyPopulate.value("country_curr_abbr").toString());
    _currSymbol->setText(currencyPopulate.value("country_curr_symbol").toString());
  }
  else if (currencyPopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, currencyPopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
