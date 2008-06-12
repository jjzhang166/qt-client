/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "country.h"

#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

country::country(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_abbr,	SIGNAL(lostFocus()),	this,	SLOT(sToUpper()));
  connect(_currAbbr,	SIGNAL(lostFocus()),	this,	SLOT(sToUpper()));
  connect(_save,	SIGNAL(clicked()),	this,	SLOT(sSave()));

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
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
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

      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  return NoError;
}

void country::sSave()
{
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
      q.prepare( "INSERT INTO country ("
		 " country_abbr,  country_name,  country_curr_abbr, "
		 " country_curr_name, country_curr_number, country_curr_symbol "
		 ") VALUES ("
		 " :country_abbr,  :country_name,  :country_curr_abbr, "
		 " :country_curr_name, :country_curr_number, :country_curr_symbol "
		 ");" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE country SET "
	       "   country_abbr=:country_abbr, country_name=:country_name, "
	       "   country_curr_abbr=:country_curr_abbr, "
	       "   country_curr_name=:country_curr_name, "
	       "   country_curr_number=:country_curr_number, "
	       "   country_curr_symbol=:country_curr_symbol "
               "WHERE (country_id=:country_id);" );
    q.bindValue(":country_id", _countryId);
  }
  
  q.bindValue(":country_abbr",	_abbr->text());
  q.bindValue(":country_name",	_name->text());
  q.bindValue(":country_curr_abbr",	_currAbbr->text());
  q.bindValue(":country_curr_name",	_currName->text());
  q.bindValue(":country_curr_number",	_currNumber->text());
  q.bindValue(":country_curr_symbol",	_currSymbol->text());

  q.exec();

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  done(_countryId);
}

void country::populate()
{
  q.prepare( "SELECT country_abbr,  country_name,  country_curr_abbr, "
	     "     country_curr_name, country_curr_number, country_curr_symbol "
             "FROM country "
             "WHERE (country_id=:country_id);" );
  q.bindValue(":country_id", _countryId);
  q.exec();
  if (q.first())
  {
    _abbr->setText(q.value("country_abbr").toString());
    _name->setText(q.value("country_name").toString());
    _currAbbr->setText(q.value("country_curr_abbr").toString());
    _currName->setText(q.value("country_curr_name").toString());
    _currNumber->setText(q.value("country_curr_number").toString());
    _currSymbol->setText(q.value("country_curr_symbol").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void country::sToUpper()
{
  _abbr->setText(_abbr->text().toUpper());
  _currAbbr->setText(_currAbbr->text().toUpper());
}
