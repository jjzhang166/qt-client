/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "country.h"

#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

country::country(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_abbr,	SIGNAL(editingFinished()),	this,	SLOT(sToUpper()));
  connect(_currAbbr,	SIGNAL(editingFinished()),	this,	SLOT(sToUpper()));
  connect(_buttonBox,	SIGNAL(accepted()),	this,	SLOT(sSave()));

  _currNumber->setValidator(new QRegExpValidator(QRegExp("[0-9][0-9][0-9]"), this));
}

country::~country()
{
  // no need to delete child widgets, Qt does it all for us
}

void country::languageChange()
{
  retranslateUi(this);
}

enum SetResponse country::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("country_id", &valid);
  if (valid)
  {
    _countryId = param.toInt();
    populate();
  }
   
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _name->setEnabled(false);
      _abbr->setEnabled(false);
      _currAbbr->setEnabled(false);
      _currName->setEnabled(false);
      _currSymbol->setEnabled(false);
      _currNumber->setEnabled(false);

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void country::sSave()
{
  XSqlQuery countrySave;
  sToUpper();

  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _name->text().isEmpty(), tr("Country name is required."),         _name },
    { _abbr->text().isEmpty(), tr("Country abbreviation is required."), _abbr },
    { _abbr->text().length() != 2, tr("Country abbreviation must be 2 "
				      "characters long."),              _abbr },
    { _currAbbr->text().length() != 3, tr("Currency abbreviations must be "
					 "3 characters long."), _currAbbr},
    { ! _currNumber->text().isEmpty() && _currNumber->text().length() != 3,
		   tr("Currency numbers must be 3 digits long."), _currNumber },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot save Country"), error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }
  
  if (_mode == cNew)
  {
      countrySave.prepare( "INSERT INTO country ("
		 " country_abbr,  country_name,  country_curr_abbr, "
		 " country_curr_name, country_curr_number, country_curr_symbol "
		 ") VALUES ("
		 " :country_abbr,  :country_name,  :country_curr_abbr, "
		 " :country_curr_name, :country_curr_number, :country_curr_symbol "
		 ");" );
  }
  else if (_mode == cEdit)
  {
    countrySave.prepare( "UPDATE country SET "
	       "   country_abbr=:country_abbr, country_name=:country_name, "
	       "   country_curr_abbr=:country_curr_abbr, "
	       "   country_curr_name=:country_curr_name, "
	       "   country_curr_number=:country_curr_number, "
	       "   country_curr_symbol=:country_curr_symbol "
               "WHERE (country_id=:country_id);" );
    countrySave.bindValue(":country_id", _countryId);
  }
  
  countrySave.bindValue(":country_abbr",	_abbr->text());
  countrySave.bindValue(":country_name",	_name->text());
  countrySave.bindValue(":country_curr_abbr",	_currAbbr->text());
  countrySave.bindValue(":country_curr_name",	_currName->text());
  countrySave.bindValue(":country_curr_number",	_currNumber->text());
  countrySave.bindValue(":country_curr_symbol",	_currSymbol->text());

  countrySave.exec();

  if (countrySave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, countrySave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  done(_countryId);
}

void country::populate()
{
  XSqlQuery countrypopulate;
  countrypopulate.prepare( "SELECT country_abbr,  country_name,  country_curr_abbr, "
	     "     country_curr_name, country_curr_number, country_curr_symbol "
             "FROM country "
             "WHERE (country_id=:country_id);" );
  countrypopulate.bindValue(":country_id", _countryId);
  countrypopulate.exec();
  if (countrypopulate.first())
  {
    _abbr->setText(countrypopulate.value("country_abbr").toString());
    _name->setText(countrypopulate.value("country_name").toString());
    _currAbbr->setText(countrypopulate.value("country_curr_abbr").toString());
    _currName->setText(countrypopulate.value("country_curr_name").toString());
    _currNumber->setText(countrypopulate.value("country_curr_number").toString());
    _currSymbol->setText(countrypopulate.value("country_curr_symbol").toString());
  }
  else if (countrypopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, countrypopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void country::sToUpper()
{
  _abbr->setText(_abbr->text().toUpper());
  _currAbbr->setText(_currAbbr->text().toUpper());
}
