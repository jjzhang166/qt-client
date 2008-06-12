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

#include "terms.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a terms as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
terms::terms(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_proximo, SIGNAL(toggled(bool)), _cutOffDay, SLOT(setEnabled(bool)));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_days, SIGNAL(toggled(bool)), this, SLOT(sTypeChanged()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
terms::~terms()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void terms::languageChange()
{
    retranslateUi(this);
}


void terms::init()
{
  _discountPercent->setValidator(omfgThis->percentVal());
}

enum SetResponse terms::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("terms_id", &valid);
  if (valid)
  {
    _termsid = param.toInt();
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
      _typeGroup->setEnabled(FALSE);
      _ap->setEnabled(FALSE);
      _ar->setEnabled(FALSE);
      _dueDays->setEnabled(FALSE);
      _discountDays->setEnabled(FALSE);
      _discountPercent->setEnabled(FALSE);
      _cutOffDay->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void terms::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    q.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE (UPPER(terms_code)=UPPER(:terms_code));" );
    q.bindValue(":terms_code", _code->text());
    q.exec();
    if (q.first())
    {
      _termsid = q.value("terms_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void terms::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('terms_terms_id_seq') AS _terms_id");
    if (q.first())
      _termsid = q.value("_terms_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO terms "
               "( terms_id, terms_code, terms_descrip, terms_type,"
               "  terms_ap, terms_ar,"
               "  terms_duedays, terms_discdays, terms_discprcnt, terms_cutoffday ) "
               "VALUES "
               "( :terms_id, :terms_code, :terms_descrip, :terms_type,"
               "  :terms_ap, :terms_ar,"
               "  :terms_duedays, :terms_discdays, :terms_discprcnt, :terms_cutoffday );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT terms_id "
               "FROM terms "
               "WHERE ( (UPPER(terms_code)=UPPER(:terms_code))"
               " AND (terms_id<>:terms_id) );" );
    q.bindValue(":terms_id", _termsid);
    q.bindValue(":terms_code", _code->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Terms"),
                            tr("You may not rename this Terms code with the entered name as it is in use by another Terms code.") );
      return;
    }

    q.prepare( "UPDATE terms "
               "SET terms_code=:terms_code, terms_descrip=:terms_descrip, terms_type=:terms_type,"
               "    terms_ap=:terms_ap, terms_ar=:terms_ar,"
               "    terms_duedays=:terms_duedays, terms_discdays=:terms_discdays,"
               "    terms_discprcnt=:terms_discprcnt, terms_cutoffday=:terms_cutoffday "
               "WHERE (terms_id=:terms_id);" );
  }

  if (_days->isChecked())
    q.bindValue(":terms_type", "D");
  else
    q.bindValue(":terms_type", "P");

  q.bindValue(":terms_id", _termsid);
  q.bindValue(":terms_code", _code->text());
  q.bindValue(":terms_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":terms_ap", QVariant(_ap->isChecked(), 0));
  q.bindValue(":terms_ar", QVariant(_ar->isChecked(), 0));
  q.bindValue(":terms_duedays", _dueDays->value());
  q.bindValue(":terms_discdays", _discountDays->value());
  q.bindValue(":terms_discprcnt", (_discountPercent->toDouble() / 100.0));
  q.bindValue(":terms_cutoffday", _cutOffDay->value());
  q.exec();

  done(_termsid);
}

void terms::sTypeChanged()
{
  if (_days->isChecked())
  {
    _dueDaysLit->setText(tr("Due Days:"));
    _discountDaysLit->setText(tr("Discnt. Days:"));

    _dueDays->setMinValue(0);
    _dueDays->setMaxValue(999);
    _discountDays->setMinValue(0);
    _discountDays->setMaxValue(999);
  }
  else
  {
    _dueDaysLit->setText(tr("Due Day:"));
    _discountDaysLit->setText(tr("Discnt. Day:"));

    _dueDays->setMinValue(1);
    _dueDays->setMaxValue(31);
    _discountDays->setMinValue(1);
    _discountDays->setMaxValue(31);
    _cutOffDay->setMaxValue(31);
  }
}

void terms::populate()
{
  q.prepare( "SELECT terms_code, terms_descrip, terms_type,"
             "       terms_ap, terms_ar,"
             "       terms_duedays, terms_discdays, terms_cutoffday,"
             "       formatScrap(terms_discprcnt) AS f_discount "
             "FROM terms "
             "WHERE (terms_id=:terms_id);" );
  q.bindValue(":terms_id", _termsid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("terms_code").toString());
    _description->setText(q.value("terms_descrip").toString());
    _ap->setChecked(q.value("terms_ap").toBool());
    _ar->setChecked(q.value("terms_ar").toBool());
    _dueDays->setValue(q.value("terms_duedays").toInt());
    _discountPercent->setText(q.value("f_discount").toString());
    _discountDays->setValue(q.value("terms_discdays").toInt());

    if (q.value("terms_type").toString() == "D")
    {
      _days->setChecked(TRUE);
      if (_mode == cEdit)
        _cutOffDay->setEnabled(FALSE);
    }
    else if (q.value("terms_type").toString() == "P")
    {
      _proximo->setChecked(TRUE);
      _cutOffDay->setValue(q.value("terms_cutoffday").toInt());
    }
  }
}
