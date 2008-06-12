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

#include "taxCode.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a taxCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
taxCode::taxCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxCode::~taxCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxCode::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("tax_id", &valid);
  if (valid)
  {
    _taxid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _taxRateA->setEnabled(FALSE);
      _taxRateB->setEnabled(FALSE);
      _taxRateC->setEnabled(FALSE);
      _cumulative->setEnabled(FALSE);
      _accountA->setReadOnly(TRUE);
      _accountB->setReadOnly(TRUE);
      _accountC->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void taxCode::sSave()
{
  if(_code->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::warning( this, tr("No Tax Name Code"),
                          tr("You must specify a name code for this Tax.") );
    _code->setFocus();
    return;
  }

  double ratea = (_taxRateA->toDouble() / 100.0);
  double rateb = (_taxRateB->toDouble() / 100.0);
  double ratec = (_taxRateC->toDouble() / 100.0);
  int accounta = _accountA->id();
  int accountb = _accountB->id();
  int accountc = _accountC->id();

  if(ratea != 0.0)
  {
    if (!_accountA->isValid())
    {
      QMessageBox::warning( this, tr("Select G/L Accout"),
                            tr("You must select a G/L Account for this Tax.") );
      _accountA->setFocus();
      return;
    }
  }
  else
    accounta = -1;

  if(rateb != 0.0)
  {
    if (!_accountB->isValid())
    {
      QMessageBox::warning( this, tr("Select G/L Account"),
                            tr("You must select a G/L Account for this Tax.") );
      _accountB->setFocus();
      return;
    }
  }
  else
    accountb = -1;

  if(ratec != 0.0)
  {
    if (!_accountC->isValid())
    {
      QMessageBox::warning( this, tr("Select G/L Account"),
                            tr("You must select a G/L Account for this Tax.") );
      _accountC->setFocus();
      return;
    }
  }
  else
    accountc = -1;

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('tax_tax_id_seq') AS _tax_id;");
    if (q.first())
      _taxid = q.value("_tax_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO tax "
               "( tax_id, tax_code, tax_descrip, tax_ratea,"
               "  tax_sales_accnt_id, tax_cumulative,"
               "  tax_rateb, tax_salesb_accnt_id,"
               "  tax_ratec, tax_salesc_accnt_id ) "
               "VALUES "
               "( :tax_id, :tax_code, :tax_descrip, :tax_ratea,"
               "  :tax_sales_accnt_id, :tax_cumulative,"
               "  :tax_rateb, :tax_salesb_accnt_id,"
               "  :tax_ratec, :tax_Salesc_accnt_id );" );
  }
  else
    q.prepare( "UPDATE tax "
               "SET tax_code=:tax_code, tax_descrip=:tax_descrip,"
               "    tax_ratea=:tax_ratea,"
               "    tax_sales_accnt_id=:tax_sales_accnt_id,"
               "    tax_rateb=:tax_rateb,"
               "    tax_salesb_accnt_id=:tax_salesb_accnt_id,"
               "    tax_ratec=:tax_ratec,"
               "    tax_salesc_accnt_id=:tax_salesc_accnt_id,"
               "    tax_cumulative=:tax_cumulative "
               "WHERE (tax_id=:tax_id);" );

  q.bindValue(":tax_id", _taxid);
  q.bindValue(":tax_code", _code->text().stripWhiteSpace());
  q.bindValue(":tax_descrip", _description->text());
  q.bindValue(":tax_ratea", ratea);
  q.bindValue(":tax_rateb", rateb);
  q.bindValue(":tax_ratec", ratec);
  q.bindValue(":tax_sales_accnt_id", accounta);
  q.bindValue(":tax_salesb_accnt_id", accountb);
  q.bindValue(":tax_salesc_accnt_id", accountc);
  q.bindValue(":tax_cumulative", QVariant(_cumulative->isChecked(), 0));
  q.exec();

  done (_taxid);
}

void taxCode::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT tax_id "
               "FROM tax "
               "WHERE (UPPER(tax_code)=UPPER(:tax_code));" );
    q.bindValue(":tax_code", _code->text());
    q.exec();
    if (q.first())
    {
      _taxid = q.value("tax_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void taxCode::populate()
{
  q.prepare( "SELECT tax_code, tax_descrip,"
             "       formatScrap(tax_ratea) AS ratea,"
             "       tax_sales_accnt_id,"
             "       formatScrap(tax_rateb) AS rateb,"
             "       tax_salesb_accnt_id,"
             "       formatScrap(tax_ratec) AS ratec,"
             "       tax_salesc_accnt_id,"
             "       tax_cumulative "
             "FROM tax "
             "WHERE (tax_id=:tax_id);" );
  q.bindValue(":tax_id", _taxid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("tax_code").toString());
    _description->setText(q.value("tax_descrip").toString());
    _taxRateA->setText(q.value("ratea").toString());
    _accountA->setId(q.value("tax_sales_accnt_id").toInt());
    _taxRateB->setText(q.value("rateb").toString());
    _accountB->setId(q.value("tax_salesb_accnt_id").toInt());
    _taxRateC->setText(q.value("ratec").toString());
    _accountC->setId(q.value("tax_salesc_accnt_id").toInt());
    _cumulative->setChecked(q.value("tax_cumulative").toBool());
  }
}

