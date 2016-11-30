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
#include "errorReporter.h"
#include "guiErrorCheck.h"

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

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_name->text().isEmpty(), _name,
                        tr("Country name is required."))
       <<GuiErrorCheck(_abbr->text().isEmpty(), _abbr,
                       tr("Country abbreviation is required."))
       <<GuiErrorCheck(_abbr->text().length() != 2, _abbr,
                      tr("Country abbreviation must be 2 characters long."))
       <<GuiErrorCheck(_currAbbr->text().length() != 3, _currAbbr,
                      tr("Currency abbreviations must be 3 characters long."))
        <<GuiErrorCheck(! _currNumber->text().isEmpty() && _currNumber->text().length() != 3, _currNumber,
                      tr("Currency numbers must be 3 digits long."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot save Country"),errors))
      return;
  
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

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Country Information"),
                                countrySave, __FILE__, __LINE__))
  {
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
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Country Information"),
                                countrypopulate, __FILE__, __LINE__))
  {
    return;
  }
}

void country::sToUpper()
{
  _abbr->setText(_abbr->text().toUpper());
  _currAbbr->setText(_currAbbr->text().toUpper());
}
