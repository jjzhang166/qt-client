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

#include "bankAccount.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a bankAccount as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
bankAccount::bankAccount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_ap, SIGNAL(toggled(bool)), _nextCheckNumLit, SLOT(setEnabled(bool)));
    connect(_ap, SIGNAL(toggled(bool)), _nextCheckNum, SLOT(setEnabled(bool)));
    connect(_ap, SIGNAL(toggled(bool)), _checkFormatLit, SLOT(setEnabled(bool)));
    connect(_ap, SIGNAL(toggled(bool)), _form, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
bankAccount::~bankAccount()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bankAccount::languageChange()
{
    retranslateUi(this);
}


void bankAccount::init()
{
  _nextCheckNum->setValidator(omfgThis->orderVal());

  _assetAccount->setType(GLCluster::cAsset);
  _currency->setType(XComboBox::Currencies);
  _currency->setLabel(_currencyLit);

  _form->setAllowNull(TRUE);
  _form->populate( "SELECT form_id, form_name "
                   "FROM form "
                   "WHERE form_key='Chck' "
                   "ORDER BY form_name;" );
}

enum SetResponse bankAccount::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
  {
    _bankaccntid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _bankName->setEnabled(FALSE);
      _accountNumber->setEnabled(FALSE);
      _type->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _ap->setEnabled(FALSE);
      _nextCheckNum->setEnabled(FALSE);
      _form->setEnabled(FALSE);
      _ar->setEnabled(FALSE);
      _assetAccount->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void bankAccount::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.exec( "SELECT bankaccnt_id "
            "FROM bankaccnt "
            "WHERE (UPPER(bankaccnt_name)=UPPER(:bankaccnt_name));" );
    q.bindValue(":bankaccnt_name", _name->text());
    q.exec();
    if (q.first())
    {
      _bankaccntid = q.value("bankaccnt_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void bankAccount::sSave()
{
  if (!_assetAccount->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Bank Account"),
                           tr("You must select an G/L Account to assign to this Bank Account before you may save it.") );
    _assetAccount->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('bankaccnt_bankaccnt_id_seq') AS _bankaccnt_id");
    if (q.first())
      _bankaccntid = q.value("_bankaccnt_id").toInt();

    q.prepare( "INSERT INTO bankaccnt "
               "( bankaccnt_id, bankaccnt_name, bankaccnt_descrip,"
               "  bankaccnt_bankname, bankaccnt_accntnumber,"
               "  bankaccnt_type, bankaccnt_ap, bankaccnt_ar, bankaccnt_accnt_id,"
               "  bankaccnt_nextchknum, bankaccnt_check_form_id, "
	       "  bankaccnt_curr_id )"
               "VALUES "
               "( :bankaccnt_id, :bankaccnt_name, :bankaccnt_descrip,"
               "  :bankaccnt_bankname, :bankaccnt_accntnumber,"
               "  :bankaccnt_type, :bankaccnt_ap, :bankaccnt_ar, :bankaccnt_accnt_id,"
               "  :bankaccnt_nextchknum, :bankaccnt_check_form_id, "
	       "  :bankaccnt_curr_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE bankaccnt "
               "SET bankaccnt_name=:bankaccnt_name, bankaccnt_descrip=:bankaccnt_descrip,"
               "    bankaccnt_bankname=:bankaccnt_bankname, bankaccnt_accntnumber=:bankaccnt_accntnumber,"
               "    bankaccnt_type=:bankaccnt_type, bankaccnt_ap=:bankaccnt_ap, bankaccnt_ar=:bankaccnt_ar, bankaccnt_accnt_id=:bankaccnt_accnt_id,"
               "    bankaccnt_nextchknum=:bankaccnt_nextchknum, "
	       "    bankaccnt_check_form_id=:bankaccnt_check_form_id, "
	       "    bankaccnt_curr_id=:bankaccnt_curr_id "
               "WHERE (bankaccnt_id=:bankaccnt_id);" );

  q.bindValue(":bankaccnt_id", _bankaccntid);
  q.bindValue(":bankaccnt_name", _name->text());
  q.bindValue(":bankaccnt_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":bankaccnt_bankname", _bankName->text());
  q.bindValue(":bankaccnt_accntnumber", _accountNumber->text());
  q.bindValue(":bankaccnt_ap", QVariant(_ap->isChecked(), 0));
  q.bindValue(":bankaccnt_ar", QVariant(_ar->isChecked(), 0));
  q.bindValue(":bankaccnt_accnt_id", _assetAccount->id());
  q.bindValue(":bankaccnt_curr_id", _currency->id());

  q.bindValue(":bankaccnt_nextchknum", _nextCheckNum->text().toInt());
  q.bindValue(":bankaccnt_check_form_id", _form->id());

  if (_type->currentItem() == 0)
    q.bindValue(":bankaccnt_type", "K");
  else if (_type->currentItem() == 1)
    q.bindValue(":bankaccnt_type", "C");

  q.exec();

  done(_bankaccntid);
}

void bankAccount::populate()
{
  q.prepare( "SELECT bankaccnt_name, bankaccnt_descrip, bankaccnt_bankname, bankaccnt_accntnumber,"
             "       bankaccnt_type, bankaccnt_ap, bankaccnt_ar, bankaccnt_accnt_id,"
             "       bankaccnt_nextchknum, bankaccnt_check_form_id, "
	     "       bankaccnt_curr_id "
             "FROM bankaccnt "
             "WHERE (bankaccnt_id=:bankaccnt_id);" );
  q.bindValue(":bankaccnt_id", _bankaccntid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("bankaccnt_name"));
    _description->setText(q.value("bankaccnt_descrip"));
    _bankName->setText(q.value("bankaccnt_bankname"));
    _accountNumber->setText(q.value("bankaccnt_accntnumber"));
    _ap->setChecked(q.value("bankaccnt_ap").toBool());
    _ar->setChecked(q.value("bankaccnt_ar").toBool());
    _nextCheckNum->setText(q.value("bankaccnt_nextchknum"));
    _form->setId(q.value("bankaccnt_check_form_id").toInt());

    _assetAccount->setId(q.value("bankaccnt_accnt_id").toInt());
    _currency->setId(q.value("bankaccnt_curr_id").toInt());

    if (q.value("bankaccnt_type").toString() == "K")
      _type->setCurrentItem(0);
    else if (q.value("bankaccnt_type").toString() == "C")
      _type->setCurrentItem(1);
  }
}

