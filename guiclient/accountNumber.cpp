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

#include "accountNumber.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <parameter.h>

accountNumber::accountNumber(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_type, SIGNAL(activated(int)), this, SLOT(populateSubTypes()));

  _currency->setLabel(_currencyLit);

  // Until we find out what use there is for an account-level currency, hide it
  _currencyLit->hide();
  _currency->hide();
  
  _subType->setAllowNull(TRUE);
  populateSubTypes();
  
  if (_metrics->value("GLCompanySize").toInt() == 0)
  {
    _company->hide();
    _sep1Lit->hide();
  }

  if (_metrics->value("GLProfitSize").toInt() == 0)
  {
    _profit->hide();
    _sep2Lit->hide();
  }

  if (_metrics->value("GLSubaccountSize").toInt() == 0)
  {
    _sub->hide();
    _sep3Lit->hide();
  }
}

accountNumber::~accountNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountNumber::languageChange()
{
  retranslateUi(this);
}

enum SetResponse accountNumber::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _accntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setEnabled(FALSE);
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _company->setEnabled(FALSE);
      _profit->setEnabled(FALSE);
      _number->setEnabled(FALSE);
      _sub->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _extReference->setEnabled(FALSE);
      _postIntoClosed->setEnabled(FALSE);
      _forwardUpdate->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _save->setEnabled(FALSE);
      _close->setText(tr("&Cancel"));
      _close->setDefault(TRUE);

      _close->setFocus();
    }
  }

  return NoError;
}

void accountNumber::sSave()
{
  QString sql("SELECT accnt_id "
              "FROM accnt "
              "WHERE ( (accnt_number=<? value(\"accnt_number\") ?>)"
	      "<? if exists(\"accnt_company\") ?>"
	      " AND (accnt_company=<? value(\"accnt_company\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_profit\") ?>"
	      " AND (accnt_profit=<? value(\"accnt_profit\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_sub\") ?>"
	      " AND (accnt_sub=<? value(\"accnt_sub\") ?>)"
	      "<? endif ?>"
	      "<? if exists(\"accnt_id\") ?>"
	      " AND (accnt_id<><? value(\"accnt_id\") ?>)"
	      "<? endif ?>"
	      " );" );

  ParameterList params;
  params.append("accnt_number", _number->text().stripWhiteSpace());

  if (_metrics->value("GLCompanySize").toInt())
    params.append("accnt_company", _company->currentText());

  if (_metrics->value("GLProfitSize").toInt())
    params.append("accnt_profit", _profit->currentText());

  if (_metrics->value("GLSubaccountSize").toInt())
    params.append("accnt_sub", _sub->currentText());

  if (_mode == cEdit)
    params.append("accnt_id", _accntid);

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Account"),
                          tr("<p>This Account cannot be saved as an Account "
			     "with the same number already exists.") );
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    if(_number->text().stripWhiteSpace().isEmpty())
    {
      QMessageBox::warning(this, tr("No Account Number"),
			   tr("<p>You must specify an account number before "
			      "you may save this record."));
      return;
    }

    q.exec("SELECT NEXTVAL('accnt_accnt_id_seq') AS _accnt_id;");
    if (q.first())
      _accntid = q.value("_accnt_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO accnt "
               "( accnt_id,"
               "  accnt_company, accnt_profit, accnt_number, accnt_sub,"
               "  accnt_closedpost, accnt_forwardupdate,"
               "  accnt_type, accnt_descrip, accnt_extref, accnt_comments, "
	       "  accnt_subaccnttype_code, accnt_curr_id ) "
               "VALUES "
               "( :accnt_id,"
               "  :accnt_company, :accnt_profit, :accnt_number, :accnt_sub,"
               "  :accnt_closedpost, :accnt_forwardupdate,"
               "  :accnt_type, :accnt_descrip, :accnt_extref, :accnt_comments,"
               "  (SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id), "
	       "  :accnt_curr_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE accnt "
               "SET accnt_company=:accnt_company, accnt_profit=:accnt_profit,"
               "    accnt_number=:accnt_number, accnt_sub=:accnt_sub,"
               "    accnt_closedpost=:accnt_closedpost, accnt_forwardupdate=:accnt_forwardupdate,"
               "    accnt_type=:accnt_type, accnt_descrip=:accnt_descrip, accnt_extref=:accnt_extref,"
               "    accnt_comments=:accnt_comments,"
               "    accnt_subaccnttype_code=(SELECT subaccnttype_code FROM subaccnttype WHERE subaccnttype_id=:accnt_subaccnttype_id),"
	       "    accnt_curr_id=:accnt_curr_id "
               "WHERE (accnt_id=:accnt_id);" );

  q.bindValue(":accnt_id", _accntid);
  q.bindValue(":accnt_company", _company->currentText());
  q.bindValue(":accnt_profit", _profit->currentText());
  q.bindValue(":accnt_number", _number->text());
  q.bindValue(":accnt_sub", _sub->currentText());
  q.bindValue(":accnt_descrip", _description->text());
  q.bindValue(":accnt_extref", _extReference->text());
  q.bindValue(":accnt_closedpost", QVariant(_postIntoClosed->isChecked(), 0));
  q.bindValue(":accnt_forwardupdate", QVariant(_forwardUpdate->isChecked(), 0));
  q.bindValue(":accnt_comments", _comments->text());
  q.bindValue(":accnt_curr_id", _currency->id());
  q.bindValue(":accnt_subaccnttype_id", _subType->id());

  if (_type->currentItem() == 0)
    q.bindValue(":accnt_type", "A");
  else if (_type->currentItem() == 1)
    q.bindValue(":accnt_type", "L");
  else if (_type->currentItem() == 2)
    q.bindValue(":accnt_type", "E");
  else if (_type->currentItem() == 3)
    q.bindValue(":accnt_type", "R");
  else if (_type->currentItem() == 4)
    q.bindValue(":accnt_type", "Q");

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_accntid);
}

void accountNumber::populate()
{
  q.prepare( "SELECT accnt_company, accnt_profit, accnt_number, accnt_sub,"
             "       accnt_closedpost, accnt_forwardupdate,"
             "       accnt_type, accnt_descrip, accnt_extref, accnt_comments, subaccnttype_id, "
	     "       accnt_curr_id "
             "FROM accnt LEFT OUTER JOIN subaccnttype ON (subaccnttype_code=accnt_subaccnttype_code) "
             "WHERE (accnt_id=:accnt_id);" );
  q.bindValue(":accnt_id", _accntid);
  q.exec();
  if (q.first())
  {
    if (_metrics->value("GLCompanySize").toInt())
      _company->setText(q.value("accnt_company"));

    if (_metrics->value("GLProfitSize").toInt())
      _profit->setText(q.value("accnt_profit"));

    if (_metrics->value("GLSubaccountSize").toInt())
      _sub->setText(q.value("accnt_sub"));

    _number->setText(q.value("accnt_number"));
    _description->setText(q.value("accnt_descrip"));
    _extReference->setText(q.value("accnt_extref"));
    _postIntoClosed->setChecked(q.value("accnt_closedpost").toBool());
    _forwardUpdate->setChecked(q.value("accnt_forwardupdate").toBool());
    _comments->setText(q.value("accnt_comments").toString());
    _currency->setId(q.value("accnt_curr_id").toInt());

    if (q.value("accnt_type").toString() == "A")
      _type->setCurrentItem(0);
    else if (q.value("accnt_type").toString() == "L")
      _type->setCurrentItem(1);
    else if (q.value("accnt_type").toString() == "E")
      _type->setCurrentItem(2);
    else if (q.value("accnt_type").toString() == "R")
      _type->setCurrentItem(3);
    else if (q.value("accnt_type").toString() == "Q")
      _type->setCurrentItem(4);

    populateSubTypes();
    _subType->setId(q.value("subaccnttype_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void accountNumber::populateSubTypes()
{
  XSqlQuery sub;
  sub.prepare("SELECT subaccnttype_id, (subaccnttype_code||'-'||subaccnttype_descrip) "
              "FROM subaccnttype "
              "WHERE (subaccnttype_accnt_type=:subaccnttype_accnt_type) "
              "ORDER BY subaccnttype_code; ");
  if (_type->currentItem() == 0)
    sub.bindValue(":subaccnttype_accnt_type", "A");
  else if (_type->currentItem() == 1)
    sub.bindValue(":subaccnttype_accnt_type", "L");
  else if (_type->currentItem() == 2)
    sub.bindValue(":subaccnttype_accnt_type", "E");
  else if (_type->currentItem() == 3)
    sub.bindValue(":subaccnttype_accnt_type", "R");
  else if (_type->currentItem() == 4)
    sub.bindValue(":subaccnttype_accnt_type", "Q");
  sub.exec();
  _subType->populate(sub);
  if (sub.lastError().type() != QSqlError::None)
  {
    systemError(this, sub.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
}

