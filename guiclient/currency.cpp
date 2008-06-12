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

#include "currency.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "currencySelect.h"

currency::currency(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_currBase, SIGNAL(toggled(bool)), this, SLOT(sConfirmBaseFlag()));
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_select,	SIGNAL(clicked()),	this, SLOT(sSelect()));

    // avoid sConfirmBaseFlag() when calling populate for editing base currency
    // baseOrig gets set in set() for cNew and in populate() for cEdit and cView
    baseOrig = TRUE;
}

currency::~currency()
{
    // no need to delete child widgets, Qt does it all for us
}

void currency::languageChange()
{
    retranslateUi(this);
}

bool currency::isBaseSet()
{
    int numSet = 0;
    
    q.prepare("SELECT count(*) AS numSet "
              "FROM curr_symbol WHERE curr_base = TRUE");
    q.exec();
    if (q.first())
    {
	numSet = q.value("numSet").toInt();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
    return numSet != 0;
}

enum SetResponse currency::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _currid = param.toInt();
    populate();
  }
  else
    baseOrig = FALSE; // see comments in constructor

   
  param = pParams.value("mode", &valid);
  if (valid)
  {
      if (param.toString() == "new")
      {
	  _mode = cNew;
	  _currName->setFocus();
	_currBase->setEnabled(! isBaseSet());
      }
    else if (param.toString() == "edit")
    {
	_mode = cEdit;
	_currBase->setEnabled(! isBaseSet());
	_currName->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _currName->setEnabled(FALSE);
      _currSymbol->setEnabled(FALSE);
      _currAbbr->setEnabled(FALSE);
      _currBase->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  return NoError;
}

void currency::sSave()
{
  sConfirmBaseFlag();

  if (_currName->text().isEmpty())
  {
    QMessageBox::critical(this, tr("Name Required")
                      .arg(__FILE__)
                      .arg(__LINE__),
		      tr("Currency name is required."));
    _currName->setFocus();
    return;
  }

  if (_currAbbr->text().isEmpty() && _currSymbol->text().isEmpty())
  {
    QMessageBox::critical(this, tr("Symbol or Abbreviation Required")
                      .arg(__FILE__)
                      .arg(__LINE__),
		      tr("Either the currency symbol or abbreviation must be "
		         "supplied.\n(Both would be better.)")
		      );
    _currSymbol->setFocus();
    return;
  }
  
  if (_currAbbr->text().length() > 3)
  {
    QMessageBox::critical(this, tr("Abbreviation Too Long")
                      .arg(__FILE__)
                      .arg(__LINE__),
		      tr("The currency abbreviation must have "
		         "3 or fewer characters.\n"
			 "ISO abbreviations are exactly 3 characters long.")
		      );
    return;
  }

  
  if (_mode == cNew)
  {
      q.prepare( "INSERT INTO curr_symbol "
		 "(  curr_name,  curr_symbol,  curr_abbr,  curr_base ) "
		 "VALUES "
		 "( :curr_name, :curr_symbol, :curr_abbr, :curr_base );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE curr_symbol "
               "SET curr_name=:curr_name, curr_symbol=:curr_symbol, "
	       "    curr_abbr=:curr_abbr, curr_base = :curr_base "
               "WHERE (curr_id=:curr_id);" );
    q.bindValue(":curr_id", _currid);
   }
  
  q.bindValue(":curr_name", _currName->text());
  q.bindValue(":curr_symbol", _currSymbol->text());
  q.bindValue(":curr_abbr", _currAbbr->text());
  q.bindValue(":curr_base", QVariant(_currBase->isChecked(), 0));
  q.exec();

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  done(_currid);
}

void currency::populate()
{
  q.prepare( "SELECT curr_name, curr_symbol, curr_base, curr_abbr "
             "FROM curr_symbol "
             "WHERE (curr_id=:curr_id);" );
  q.bindValue(":curr_id", _currid);
  q.exec();
  if (q.first())
  {
    _currName->setText(q.value("curr_name").toString());
    _currSymbol->setText(q.value("curr_symbol").toString());
    _currBase->setChecked(q.value("curr_base").toBool());
    _currAbbr->setText(q.value("curr_abbr").toString());

    baseOrig = _currBase->isChecked();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void currency::sConfirmBaseFlag()
{
    if (_currBase->isChecked() && !baseOrig)
    {
	int response = QMessageBox::warning (this, tr("Set Base Currency?"),
				tr("You cannot change the base currency "
				  "after it is set.  Are you sure you want "
				  "%1 to be the base currency?")
				  .arg(_currName->text()),
				QMessageBox::Yes | QMessageBox::Escape,
				QMessageBox::No | QMessageBox::Default);
	if (response != QMessageBox::Yes)
	{
	    _currBase->setChecked(FALSE);
	}
    }
}

void currency::sClose()
{
  close();
}

void currency::sSelect()
{
  currencySelect *newdlg = new currencySelect(this, "", true);
  int country_id = newdlg->exec();
  if (country_id > 0)
  {
    q.prepare("SELECT * FROM country WHERE country_id = :country_id;");
    q.bindValue(":country_id", country_id);
    q.exec();
    if (q.first())
    {
      _currName->setText(q.value("country_curr_name").toString());
      _currAbbr->setText(q.value("country_curr_abbr").toString());
      _currSymbol->setText(q.value("country_curr_symbol").toString());
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
