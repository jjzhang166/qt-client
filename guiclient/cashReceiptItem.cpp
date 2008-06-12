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

#include "cashReceiptItem.h"

#include <QVariant>
#include <QValidator>
#include <QMessageBox>

/*
 *  Constructs a cashReceiptItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
cashReceiptItem::cashReceiptItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_openAmount, SIGNAL(idChanged(int)), _amountToApply, SLOT(setId(int)));
  connect(_openAmount, SIGNAL(effectiveChanged(const QDate&)), _amountToApply, SLOT(setEffective(const QDate&)));

  _cust->setReadOnly(TRUE);
}

/*
 *  Destroys the object and frees any allocated resources
 */
cashReceiptItem::~cashReceiptItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void cashReceiptItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse cashReceiptItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

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

      _save->setFocus();
    }
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
    _openAmount->setId(param.toInt());

  param = pParams.value("cashrcpt_id", &valid);
  if (valid)
    _cashrcptid = param.toInt();
  else
    _cashrcptid = -1;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _aropenid = param.toInt();
    populate();
  }
  else
    _aropenid = -1;

  param = pParams.value("cashrcptitem_id", &valid);
  if (valid)
  {
    _cashrcptitemid = param.toInt();
    populate();
  }
  else
    _cashrcptitemid = -1;

  return NoError;
}

void cashReceiptItem::sSave()
{
  if (_amountToApply->localValue() > _openAmount->localValue())
  {
    QMessageBox::warning( this, tr("Cannot Apply"),
      tr("You may not apply more than the balance of this item.") );
    _amountToApply->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    XSqlQuery cashrcptitemid("SELECT NEXTVAL('cashrcptitem_cashrcptitem_id_seq') AS _cashrcptitem_id;");
    if (cashrcptitemid.first())
      _cashrcptitemid = cashrcptitemid.value("_cashrcptitem_id").toInt();
//  ToDo

    XSqlQuery newReceipt;
    newReceipt.prepare( "INSERT INTO cashrcptitem "
                        "( cashrcptitem_id, cashrcptitem_cashrcpt_id,"
                        "  cashrcptitem_aropen_id, cashrcptitem_amount ) "
                        "VALUES "
                        "( :cashrcptitem_id, :cashrcptitem_cashrcpt_id,"
                        "  :cashrcptitem_aropen_id, :cashrcptitem_amount );" );
    newReceipt.bindValue(":cashrcptitem_id", _cashrcptitemid);
    newReceipt.bindValue(":cashrcptitem_cashrcpt_id", _cashrcptid);
    newReceipt.bindValue(":cashrcptitem_aropen_id", _aropenid);
    newReceipt.bindValue(":cashrcptitem_amount", _amountToApply->localValue());
    newReceipt.exec();
  }
  else if (_mode == cEdit)
  {
    XSqlQuery updateReceipt;
    updateReceipt.prepare( "UPDATE cashrcptitem "
                           "SET cashrcptitem_amount=:cashrcptitem_amount "
                           "WHERE (cashrcptitem_id=:cashrcptitem_id);" );
    updateReceipt.bindValue(":cashrcptitem_id", _cashrcptitemid);
    updateReceipt.bindValue(":cashrcptitem_amount", _amountToApply->localValue());
    updateReceipt.exec();
  }

  done(_cashrcptitemid);
}

void cashReceiptItem::populate()
{
  XSqlQuery query;

  if (_mode == cNew)
  {
    query.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype,"
                   "       aropen_docdate, aropen_duedate,"
                   "       currToCurr(aropen_curr_id, :curr_id, "
		   "		      aropen_amount - aropen_paid, aropen_docdate) AS f_amount "
                   "FROM aropen "
                   "WHERE (aropen_id=:aropen_id);" );
    query.bindValue(":aropen_id", _aropenid);
    query.bindValue(":curr_id", _openAmount->id());
    query.exec();
    if (query.first())
    {
      _cust->setId(query.value("aropen_cust_id").toInt());
      _docNumber->setText(query.value("aropen_docnumber").toString());
      _docType->setText(query.value("aropen_doctype").toString());
      _docDate->setDate(query.value("aropen_docdate").toDate(), true);
      _dueDate->setDate(query.value("aropen_duedate").toDate());
      _openAmount->set(query.value("f_amount").toDouble(),
		       _openAmount->id(),
		       query.value("aropen_docdate").toDate(), false);
    }
//  ToDo
  }
  else if (_mode == cEdit)
  {
    query.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype,"
                   "       aropen_docdate, aropen_duedate,"
                   "       currToCurr(aropen_curr_id, cashrcpt_curr_id, "
		   "		      aropen_amount - aropen_paid, aropen_docdate) AS balance,"
                   "       cashrcptitem_amount, cashrcpt_curr_id "
                   "FROM cashrcptitem, cashrcpt, aropen "
                   "WHERE ( (cashrcptitem_cashrcpt_id=cashrcpt_id)"
                   " AND (cashrcptitem_aropen_id=aropen_id)"
                   " AND (cashrcptitem_id=:cashrcptitem_id) );" );
    query.bindValue(":cashrcptitem_id", _cashrcptitemid);
    query.exec();
    if (query.first())
    {
      _cust->setId(query.value("aropen_cust_id").toInt());
      _docNumber->setText(query.value("aropen_docnumber").toString());
      _docType->setText(query.value("aropen_doctype").toString());
      _docDate->setDate(query.value("aropen_docdate").toDate(), true);
      _dueDate->setDate(query.value("aropen_duedate").toDate());
      _openAmount->set(query.value("balance").toDouble(),
		       query.value("cashrcpt_curr_id").toInt(),
		       query.value("aropen_docdate").toDate(), false);
      _amountToApply->setLocalValue(query.value("cashrcptitem_amount").toDouble());
    }
//  ToDo
  }
}

