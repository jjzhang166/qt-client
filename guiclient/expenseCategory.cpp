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

#include "expenseCategory.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a expenseCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
expenseCategory::expenseCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
expenseCategory::~expenseCategory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void expenseCategory::languageChange()
{
    retranslateUi(this);
}


void expenseCategory::init()
{
}

enum SetResponse expenseCategory::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("expcat_id", &valid);
  if (valid)
  {
    _expcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _category->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _category->clear();
      _category->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _expense->setReadOnly(TRUE);
      _purchasePrice->setReadOnly(TRUE);
      _liability->setReadOnly(TRUE);
      _freight->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void expenseCategory::sCheck()
{
  _category->setText(_category->text().stripWhiteSpace().upper());
  if ((_mode == cNew) && (_category->text().length() != 0))
  {
    q.prepare( "SELECT expcat_id "
               "FROM expcat "
               "WHERE (UPPER(expcat_code)=:expcat_code);" );
    q.bindValue(":expcat_code", _category->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      _expcatid = q.value("expcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
  }
}

void expenseCategory::sSave()
{
  if (!_expense->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a Expense Account Number for this Expense Category before you may save it.") );
    _expense->setFocus();
    return;
  }

  if (!_liability->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a P/O Liability Clearing Account Number for this Expense Category before you may save it.") );
    _expense->setFocus();
    return;
  }

  if (!_freight->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save Expense Category"),
                          tr("You must select a Freight Receiving Account Number for this Expense Category before you may save it.") );
    _freight->setFocus();
    return;
  }

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    q.exec("SELECT NEXTVAL('expcat_expcat_id_seq') AS expcat_id");
    if (q.first())
      _expcatid = q.value("expcat_id").toInt();

    q.prepare( "INSERT INTO expcat"
               "( expcat_id, expcat_code, expcat_active, expcat_descrip,"
               "  expcat_exp_accnt_id, expcat_purchprice_accnt_id,"
               "  expcat_liability_accnt_id, expcat_freight_accnt_id ) "
               "VALUES "
               "( :expcat_id, :expcat_code, :expcat_active, :expcat_descrip,"
               "  :expcat_exp_accnt_id, :expcat_purchprice_accnt_id,"
               "  :expcat_liability_accnt_id, :expcat_freight_accnt_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE expcat "
               "SET expcat_code=:expcat_code, expcat_active=:expcat_active,"
               "    expcat_descrip=:expcat_descrip,"
               "    expcat_exp_accnt_id=:expcat_exp_accnt_id,"
               "    expcat_purchprice_accnt_id=:expcat_purchprice_accnt_id,"
               "    expcat_liability_accnt_id=:expcat_liability_accnt_id,"
               "    expcat_freight_accnt_id=:expcat_freight_accnt_id "
               "WHERE (expcat_id=:expcat_id);" );

  q.bindValue(":expcat_id", _expcatid);
  q.bindValue(":expcat_code", _category->text().stripWhiteSpace());
  q.bindValue(":expcat_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":expcat_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":expcat_exp_accnt_id", _expense->id());
  q.bindValue(":expcat_purchprice_accnt_id", _purchasePrice->id());
  q.bindValue(":expcat_liability_accnt_id", _liability->id());
  q.bindValue(":expcat_freight_accnt_id", _freight->id());
  q.exec();

  done(_expcatid);
}

void expenseCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM expcat "
             "WHERE (expcat_id=:expcat_id);" );
  q.bindValue(":expcat_id", _expcatid);
  q.exec();
  if (q.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(q.value("expcat_code").toString());
      _description->setText(q.value("expcat_descrip").toString());
      _active->setChecked(q.value("expcat_active").toBool());
    }
    else
      _active->setChecked(TRUE);

    _expense->setId(q.value("expcat_exp_accnt_id").toInt());
    _purchasePrice->setId(q.value("expcat_purchprice_accnt_id").toInt());
    _liability->setId(q.value("expcat_liability_accnt_id").toInt());
    _freight->setId(q.value("expcat_freight_accnt_id").toInt());
  }
}
 
