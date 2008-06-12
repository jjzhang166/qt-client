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

#include "currencyConversion.h"

#include <qvariant.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include <qimage.h>
#include <qpixmap.h>

#include "xcombobox.h"

/*
 *  Constructs a currencyConversion as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
currencyConversion::currencyConversion(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(_sClose()));
    connect(_save, SIGNAL(clicked()), this, SLOT(_sSave()));
    connect(_rate, SIGNAL(lostFocus()), this, SLOT(sFixRate()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
currencyConversion::~currencyConversion()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void currencyConversion::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>
// perhaps this should be a generalized XDoubleValidator, but for now
// it should match the definition of the curr_rate column in curr_rate table
// with one difference - the number of decimal places should be one less than
// the scale of curr_rate.curr_rate (see Mantis bug 3901).

class RateValidator : public QDoubleValidator {
    public:
	RateValidator(QObject*, const char* = 0);
	void fixup(QString& ) const;
};

RateValidator::RateValidator(QObject* parent, const char* name) :
    QDoubleValidator(0.00001, 99999.99999, 5, parent, name)
{
}

void RateValidator::fixup ( QString & input ) const
{
    if (input.isEmpty())
	return;

    double rateDouble = input.toDouble();
    if (rateDouble < bottom())
	rateDouble = bottom();
    else if (rateDouble > top())
	rateDouble = top();

    input.setNum(rateDouble, 'f', decimals());
}

void currencyConversion::init()
{
    _currency->setType(XComboBox::CurrenciesNotBase);
    _rate->setValidator(new RateValidator (_rate) );
}

enum SetResponse currencyConversion::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("curr_rate_id", &valid);
  if (valid)
  {
      _curr_rate_id = param.toInt();
  }
  else
      _curr_rate_id = 0;
  
  param = pParams.value("curr_id", &valid);
  if (valid)
  {
      _curr_id = param.toInt();
  }
  else
  {
      _curr_id = 0;
  }
  
  populate();
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
      if (param.toString() == "new")
      {
	  _mode = cNew;
	  _currency->setFocus();
      }
    else if (param.toString() == "edit")
    {
	_mode = cEdit;
	_currency->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _currency->setEnabled(FALSE);
      _rate->setEnabled(FALSE);
      _dateCluster->setEnabled(FALSE);
      
      _close->setText(tr("&Close"));
      _save->hide();
      
      _close->setFocus();
    }
  }

  return NoError;
}

void currencyConversion::_sClose()
{
    done(_curr_rate_id);
}

void currencyConversion::_sSave()
{
    if (! _currency->isValid())
    {
	QMessageBox::warning(this, tr("Missing Currency"),
			     tr("Please specify a currency for this exchange rate."));
	_currency->setFocus();
	return;
    }
    
    if (!_dateCluster->startDate().isValid())
    {
	QMessageBox::warning( this, tr("Missing Start Date"),
			      tr("Please specify a Start Date for this exchange rate."));
	_dateCluster->setFocus();
	return;
    }

    if (!_dateCluster->endDate().isValid())
    {
	QMessageBox::warning( this, tr("Missing End Date"),
			      tr("Please specify an End Date for this exchange rate. "));
	_dateCluster->setFocus();
	return;
    }

    if (_dateCluster->startDate() > _dateCluster->endDate())
    {
	QMessageBox::warning(this, tr("Invalid Date Order"),
			    tr("The Start Date for this exchange rate is "
			       "later than the End Date.\n"
			       "Please check the values of these dates."));
	_dateCluster->setFocus();
	return;
    }

    QString inverter("");
    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
	inverter = "1 / ";

    QString sql;
    if (_mode == cNew)
	sql = QString("INSERT INTO curr_rate "
		      "(curr_id, curr_rate, curr_effective, curr_expires) "
		      "VALUES "
		      "(:curr_id, %1 NUMERIC :curr_rate, "
		      " :curr_effective, :curr_expires)")
		    .arg(inverter);
    else if (_mode == cEdit)
	sql = QString("UPDATE curr_rate SET "
		      "curr_id = :curr_id, "
		      "curr_rate = %1 NUMERIC :curr_rate, "
		      "curr_effective = :curr_effective, "
		      "curr_expires = :curr_expires "
		      "WHERE curr_rate_id = :curr_rate_id")
		      .arg(inverter);


    q.prepare(sql);
    q.bindValue(":curr_rate_id", _curr_rate_id);
    q.bindValue(":curr_id", _currency->id());
    q.bindValue(":curr_rate", _rate->text());
    q.bindValue(":curr_effective", _dateCluster->startDate());
    q.bindValue(":curr_expires", _dateCluster->endDate());
    
    q.exec();

    if (q.lastError().type() != QSqlError::NoError)
    {
	QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      q.lastError().databaseText());
	return;
    }

    done(_curr_rate_id);
}

void currencyConversion::populate()
{
    QString rateString;

    if (_curr_rate_id)
    {
	QString inverter("");
	if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
	    inverter = "1 / ";
	QString sql = QString("SELECT curr_id, %1 curr_rate AS curr_rate, "
			      "curr_effective, curr_expires "
			      "FROM curr_rate "
			      "WHERE curr_rate_id = :curr_rate_id;")
			      .arg(inverter);
	q.prepare(sql);
	q.bindValue(":curr_rate_id", _curr_rate_id);
	q.exec();
	{
	    if (q.first())
	    {
		_currency->setId(q.value("curr_id").toInt());
		_dateCluster->setStartDate(q.value("curr_effective").toDate());
		_dateCluster->setEndDate(q.value("curr_expires").toDate());
		_rate->setText(rateString.setNum(q.value("curr_rate").toDouble(), 'f', 5));
	    }
	}
    }
    if (_curr_id)
    {
	_currency->setId(_curr_id);
    }
}

void currencyConversion::sFixRate()
{
    QString rateStr = _rate->text();
    RateValidator* rateValidator = new RateValidator(_rate);
    int dummy = 0;

    if (rateValidator->validate(rateStr, dummy) == QValidator::Intermediate)
    {
	rateValidator->fixup(rateStr);
	_rate->setText(rateStr);
    }
}
