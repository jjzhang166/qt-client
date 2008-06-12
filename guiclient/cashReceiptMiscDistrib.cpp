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

#include "cashReceiptMiscDistrib.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a cashReceiptMiscDistrib as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
cashReceiptMiscDistrib::cashReceiptMiscDistrib(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
cashReceiptMiscDistrib::~cashReceiptMiscDistrib()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void cashReceiptMiscDistrib::languageChange()
{
    retranslateUi(this);
}


void cashReceiptMiscDistrib::init()
{
}

enum SetResponse cashReceiptMiscDistrib::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
    _cashrcptid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("cashrcptmisc_id", &valid);
  if (valid)
  {
    _cashrcptmiscid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
  }

  return NoError;
}

void cashReceiptMiscDistrib::populate()
{
  q.prepare( "SELECT cashrcptmisc_accnt_id, cashrcptmisc_notes,"
             "       cashrcptmisc_amount, cashrcpt_curr_id, cashrcpt_distdate "
             "FROM cashrcptmisc JOIN cashrcpt ON (cashrcptmisc_cashrcpt_id = cashrcpt_id) "
             "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );
  q.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  q.exec();
  if (q.first())
  {
    _account->setId(q.value("cashrcptmisc_accnt_id").toInt());
    _amount->set(q.value("cashrcptmisc_amount").toDouble(),
    		 q.value("cashrcpt_curr_id").toInt(),
		 q.value("cashrcpt_distdate").toDate(), false);
    _notes->setText(q.value("cashrcptmisc_notes").toString());
  }
}

void cashReceiptMiscDistrib::sSave()
{
  if (!_account->isValid())
  {
    QMessageBox::warning( this, tr("Select Account"),
                          tr("You must select an Account to post this Miscellaneous Distribution to.") );
    _account->setFocus();
    return;
  }

  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Enter Amount"),
                          tr("You must enter an amount for this Miscellaneous Distribution.") );
    _amount->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cashrcptmisc_cashrcptmisc_id_seq') AS _cashrcptmisc_id;");
    if (q.first())
      _cashrcptmiscid = q.value("_cashrcptmisc_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO cashrcptmisc "
               "( cashrcptmisc_id, cashrcptmisc_cashrcpt_id,"
               "  cashrcptmisc_accnt_id, cashrcptmisc_amount,"
               "  cashrcptmisc_notes ) "
               "VALUES "
               "( :cashrcptmisc_id, :cashrcptmisc_cashrcpt_id,"
               "  :cashrcptmisc_accnt_id, :cashrcptmisc_amount,"
               "  :cashrcptmisc_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cashrcptmisc "
               "SET cashrcptmisc_accnt_id=:cashrcptmisc_accnt_id,"
               "    cashrcptmisc_amount=:cashrcptmisc_amount, cashrcptmisc_notes=:cashrcptmisc_notes "
               "WHERE (cashrcptmisc_id=:cashrcptmisc_id);" );

  q.bindValue(":cashrcptmisc_id", _cashrcptmiscid);
  q.bindValue(":cashrcptmisc_cashrcpt_id", _cashrcptid);
  q.bindValue(":cashrcptmisc_accnt_id", _account->id());
  q.bindValue(":cashrcptmisc_amount", _amount->localValue());
  q.bindValue(":cashrcptmisc_notes", _notes->text().stripWhiteSpace());
  q.exec();

  done(_cashrcptmiscid);
}

