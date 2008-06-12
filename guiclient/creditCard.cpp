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

#include "creditCard.h"

#include <QMessageBox>
#include <QRegExp>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "storedProcErrorLookup.h"

#define DEBUG false

creditCard::creditCard(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

creditCard::~creditCard()
{
    // no need to delete child widgets, Qt does it all for us
}

void creditCard::languageChange()
{
    retranslateUi(this);
}

const char *_fundsTypes2[] = { "M", "V", "A", "D" };
const int _fundsTypeCount2 = 4;

enum SetResponse creditCard::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("ccard_id", &valid);
  if (valid)
  {
    _ccardid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_number, cust_name "
               "FROM cust "
               "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", _custid);
    cust.exec();
    if (cust.first())
    {
      _custNum->setText(cust.value("cust_number").toString());
      _custName->setText(cust.value("cust_name").toString());

    }
    else if (cust.lastError().type() != QSqlError::None)
      systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);

    if (param.toString() == "new")
    {
      _mode = cNew;
      _address->setCountry("United States");

      cust.prepare( "SELECT cust_number, cust_name,"
                    "       addr_line1, addr_line2, addr_line3,"
                    "       addr_city, addr_state, addr_country,"
                    "       addr_postalcode "
                    "  FROM custinfo, cntct, addr "
                    " WHERE((cust_id=:cust_id)"
                    "   AND (cust_cntct_id=cntct_id)"
                    "   AND (cntct_addr_id=addr_id));" );
      cust.bindValue(":cust_id", _custid);
      cust.exec();
      if (cust.first())
      {
        _address->setLine1(cust.value("addr_line1").toString());
        _address->setLine3(cust.value("addr_line3").toString());
        _address->setLine2(cust.value("addr_line2").toString());
        _address->setCity(cust.value("addr_city").toString());
        _address->setState(cust.value("addr_state").toString());
        _address->setPostalCode(cust.value("addr_postalcode").toString());
        _address->setCountry(cust.value("addr_country").toString());
      }
      _fundsType2->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _fundsType2->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _fundsType2->setEnabled(FALSE);
      _creditCardNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _expireMonth->setEnabled(FALSE);
      _expireYear->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void creditCard::sSave()
{
  bool everythingOK;
  everythingOK = true;

  QString ccname	= _name->text().stripWhiteSpace();
  QString ccAddress1	= _address->line1().stripWhiteSpace();
  QString ccCity	= _address->city().stripWhiteSpace();
  QString ccState	= _address->state().stripWhiteSpace();
  QString ccZip		= _address->postalCode().stripWhiteSpace();
  QString ccCountry	= _address->country().stripWhiteSpace();
  QString ccExpireMonth	= _expireMonth->text().stripWhiteSpace();
  QString ccExpireYear	= _expireYear->text().stripWhiteSpace();

  if (ccExpireMonth.length() == 1)
      ccExpireMonth = "0" + ccExpireMonth;

  int mymonth = ccExpireMonth.toInt();
  int myyear = ccExpireYear.toInt();

  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { ccname.length() == 0 || ccname.isEmpty(),
      tr("The name of the card holder must be entered"), _name },
    { ccAddress1.length() == 0 || ccAddress1.isEmpty(),
      tr("The first address line must be entered"), _address },
    { ccCity.length() == 0 || ccCity.isEmpty(),
      tr("The city must be entered"), _address },
    { ccState.length() == 0 || ccState.isEmpty(),
      tr("The state must be entered"), _address },
    { ccZip.length() == 0 || ccZip.isEmpty(),
      tr("The zip code must be entered"), _address },
    { ccCountry.length() == 0 || ccCountry.isEmpty(),
      tr("The country must be entered"), _address },
    { ccExpireMonth.length() == 0 || ccExpireMonth.isEmpty() || ccExpireMonth.length() > 2,
      tr("The Expiration Month must be entered"), _expireMonth },
    { mymonth < 1 || mymonth > 12,
      tr("Valid Expiration Months are 01 through 12"), _expireMonth },
    { ccExpireYear.length() != 4,
      tr("Valid Expiration Years are CCYY in format"), _expireYear },
    { myyear < 1970 || myyear > 2100,
      tr("Valid Expiration Years are 1970 through 2100"), _expireYear },
    { true, "", NULL }
  };

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Credit Card Information"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  if (! _address->line3().isEmpty())
  {
    int answer = QMessageBox::question(this, tr("Third Address Line Ignored"),
				       tr("<p>The third line of the street "
				          "address will not be saved as part "
					  "of the credit card address. The "
					  "OpenMFG credit card interface "
					  "supports only 2 lines of street "
					  "address.</p><p>Continue processing?"),
					QMessageBox::Yes,
					QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::No)
    {
      _address->setFocus();
      return;
    }
  }

  QString key;
  key = omfgThis->_key;

  QString ccnum = _creditCardNumber->text().stripWhiteSpace().remove(QRegExp("[-\\s]"));
  bool allNumeric = QRegExp( "[0-9]{13,16}" ).exactMatch(ccnum);
  bool hasBeenFormatted = QRegExp( "\\**([0-9]){4}" ).exactMatch(ccnum); // tricky - repeated *s

  if (DEBUG)
    qDebug("creditCard::sSave() %s allNumeric %d, hasBeenFormatted %d",
           qPrintable(ccnum.left(4)),
           allNumeric, hasBeenFormatted);

  if (_mode == cNew ||
      (_mode == cEdit && ! hasBeenFormatted))
  {
    if (!allNumeric || ccnum.length() == 14)
    {
      QMessageBox::warning( this, tr("Invalid Credit Card Number"),
                          tr("The credit card number must be all numeric and must be 13, 15 or 16 characters in length") );
      _creditCardNumber->setFocus();
      return;
    }
  }

  if (_mode == cNew)
  {
    QString cctype;
    int cceditreturn = 0;
    cctype = QString(*(_fundsTypes2 + _fundsType2->currentItem()));

    q.prepare("SELECT editccnumber(text(:ccnum), text(:cctype)) AS cc_back;");
    q.bindValue(":ccnum", ccnum);
    q.bindValue(":cctype", cctype);
    q.exec();
    if (q.first())
      cceditreturn = q.value("cc_back").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (cceditreturn == -10)
    { // don't combine into (-10 && !CCtest): continue on -10 if not strict
      if (!_metrics->boolean("CCTest"))
      {
       QMessageBox::warning( this, tr("Invalid Credit Card Number"),
			   storedProcErrorLookup("editccnumber", cceditreturn));
       _creditCardNumber->setFocus();
       return;
      }
    }
    else if (cceditreturn < 0)
    {
      QMessageBox::warning(this, tr("Invalid Credit Card Information"),
			   storedProcErrorLookup("editccnumber", cceditreturn));
      _creditCardNumber->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('ccard_ccard_id_seq') AS ccard_id;");
    if (q.first())
      _ccardid = q.value("ccard_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

     q.prepare("SELECT COALESCE(MAX(ccard_seq), 0) + 10 AS ccard_seq FROM ccard WHERE ccard_cust_id =:ccard_cust_id;");
    q.bindValue(":ccard_cust_id", _custid);
    q.exec();
    if (q.first())
      _seq = q.value("ccard_seq").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO ccard "
               "( ccard_id, ccard_seq, ccard_cust_id, "
               "  ccard_active, ccard_name, ccard_address1,"
               "  ccard_address2,"
               "  ccard_city, ccard_state, ccard_zip,"
               "  ccard_country, ccard_number,"
               "  ccard_month_expired, ccard_year_expired, ccard_type)"
               "VALUES "
               "( :ccard_id,"
	  "  :ccard_seq,"
	  "  :ccard_cust_id,"
               "  :ccard_active, "
               "  encrypt(setbytea(:ccard_name), setbytea(:key), 'bf'),"
	  " encrypt(setbytea(:ccard_address1), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_address2), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_city), setbytea(:key), 'bf'),"
	  "  encrypt(setbytea(:ccard_state), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_zip), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_country), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_number), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_month_expired), setbytea(:key), 'bf'),"
               "  encrypt(setbytea(:ccard_year_expired), setbytea(:key), 'bf'),"
               "  :ccard_type );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE ccard "
               "SET ccard_active=:ccard_active, "
               "    ccard_name=encrypt(setbytea(:ccard_name), setbytea(:key), 'bf'),"
	  "    ccard_address1=encrypt(setbytea(:ccard_address1), setbytea(:key), 'bf'),"
               "    ccard_address2=encrypt(setbytea(:ccard_address2), setbytea(:key), 'bf'), "
               "    ccard_city=encrypt(setbytea(:ccard_city), setbytea(:key), 'bf'),"
	  "    ccard_state=encrypt(setbytea(:ccard_state), setbytea(:key), 'bf'),"
	  "    ccard_zip=encrypt(setbytea(:ccard_zip), setbytea(:key), 'bf'),"
               "    ccard_country=encrypt(setbytea(:ccard_country), setbytea(:key), 'bf'),"
               "    ccard_month_expired=encrypt(setbytea(:ccard_month_expired), setbytea(:key), 'bf'),"
               "    ccard_year_expired=encrypt(setbytea(:ccard_year_expired), setbytea(:key), 'bf'),"
	  "    ccard_type=:ccard_type "
               "WHERE (ccard_id=:ccard_id);" );

  q.bindValue(":ccard_id", _ccardid);
  q.bindValue(":ccard_seq", _seq);
  q.bindValue(":ccard_cust_id", _custid);
  q.bindValue(":ccard_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":ccard_number", ccnum);
  q.bindValue(":ccard_name", _name->text());
  q.bindValue(":ccard_address1", _address->line1());
  q.bindValue(":ccard_address2", _address->line2());
  q.bindValue(":ccard_city",	 _address->city());
  q.bindValue(":ccard_state",	 _address->state());
  q.bindValue(":ccard_zip",	 _address->postalCode());
  q.bindValue(":ccard_country",	 _address->country());
  q.bindValue(":ccard_month_expired",_expireMonth->text());
  q.bindValue(":ccard_year_expired",_expireYear->text());
  q.bindValue(":key", key);
  q.bindValue(":ccard_type", QString(*(_fundsTypes2 + _fundsType2->currentItem())));
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  // TODO: combine with UPDATE above?
  if (_mode == cEdit && ! hasBeenFormatted)     // user wants to change the cc number
  {
    q.prepare("UPDATE ccard SET ccard_number=encrypt(setbytea(:ccard_number), setbytea(:key), 'bf') "
              "WHERE (ccard_id=:ccard_id);" );
    q.bindValue(":ccard_number", ccnum);
    q.bindValue(":key",          key);
    q.bindValue(":ccard_id",     _ccardid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  done(_ccardid);
}

void creditCard::populate()
{
  QString key;
  key = omfgThis->_key;

  q.prepare( "SELECT ccard_active,"
	"       formatbytea(decrypt(setbytea(ccard_name), setbytea(:key), 'bf')) AS ccard_name,"
	"       formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key), 'bf')) AS ccard_address1,"
	"       formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key), 'bf')) AS ccard_address2,"
	"       formatbytea(decrypt(setbytea(ccard_city), setbytea(:key), 'bf')) AS ccard_city,"
	"       formatbytea(decrypt(setbytea(ccard_state), setbytea(:key), 'bf')) AS ccard_state,"
             "       formatbytea(decrypt(setbytea(ccard_zip), setbytea(:key), 'bf')) AS ccard_zip,"
	"       formatbytea(decrypt(setbytea(ccard_country), setbytea(:key), 'bf')) AS ccard_country,"
	"       formatccnumber(decrypt(setbytea(ccard_number), setbytea(:key), 'bf')) AS ccard_number,"
	"       ccard_debit,"
	"       formatbytea(decrypt(setbytea(ccard_month_expired), setbytea(:key), 'bf')) AS ccard_month_expired,"
	"       formatbytea(decrypt(setbytea(ccard_year_expired), setbytea(:key), 'bf')) AS ccard_year_expired,"
             "       ccard_type "
             "FROM ccard "
             "WHERE (ccard_id=:ccard_id);" );
  q.bindValue(":ccard_id", _ccardid);
  q.bindValue(":key",key);
  q.exec();
  if (q.first())
  {
    // _custNum->setText(q.value("cust_number").toString());
    // _custName->setText(q.value("cust_name").toString());
    _active->setChecked(q.value("ccard_active").toBool());
    _creditCardNumber->setText(q.value("ccard_number"));
    _name->setText(q.value("ccard_name"));
    _address->setLine1(q.value("ccard_address1").toString());
    _address->setLine2(q.value("ccard_address2").toString());
    _address->setCity(q.value("ccard_city").toString());
    _address->setState(q.value("ccard_state").toString());
    _address->setPostalCode(q.value("ccard_zip").toString());
    _address->setCountry(q.value("ccard_country").toString());
    _expireMonth->setText(q.value("ccard_month_expired").toString());
    _expireYear->setText(q.value("ccard_year_expired").toString());

    for (int counter = 0; counter < _fundsType2->count(); counter++)
      if (QString(q.value("ccard_type").toString()[0]) == _fundsTypes2[counter])
        _fundsType2->setCurrentItem(counter);

  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
