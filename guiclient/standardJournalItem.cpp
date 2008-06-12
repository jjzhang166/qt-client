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

#include "standardJournalItem.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a standardJournalItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
standardJournalItem::standardJournalItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
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
standardJournalItem::~standardJournalItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardJournalItem::languageChange()
{
    retranslateUi(this);
}


void standardJournalItem::init()
{
}

enum SetResponse standardJournalItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
    _stdjrnlid = param.toInt();

  param = pParams.value("stdjrnlitem_id", &valid);
  if (valid)
  {
    _stdjrnlitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _account->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _account->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _senseGroup->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournalItem::sSave()
{
  double amount = _amount->baseValue();
  if (_debit->isChecked())
    amount *= -1;

  if (_amount->isZero())
  {
    QMessageBox::warning( this, tr("Incomplete Data"),
      tr("You must enter an amount value.") );
    _amount->setFocus();
    return;
  }

  if(!_account->isValid())
  {
    QMessageBox::warning( this, tr("Incomplete Data"),
      tr("You must enter an account.") );
    return;
  }

  if (! _amount->isBase() &&
      QMessageBox::question(this, tr("G/L Transaction Not In Base Currency"),
                          tr("G/L transactions are recorded in the base currency.\n"
                          "Do you wish to convert %1 %2 at the current rate?")
                          .arg(_amount->localValue()).arg(_amount->currAbbr()),
                          QMessageBox::Yes|QMessageBox::Escape,
                          QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
        _amount->setFocus();
        return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('stdjrnlitem_stdjrnlitem_id_seq') AS stdjrnlitem_id;");
    if (q.first())
      _stdjrnlitemid = q.value("stdjrnlitem_id").toInt();
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );

    q.prepare( "INSERT INTO stdjrnlitem "
               "( stdjrnlitem_id, stdjrnlitem_stdjrnl_id, stdjrnlitem_accnt_id,"
               "  stdjrnlitem_amount, stdjrnlitem_notes ) "
               "VALUES "
               "( :stdjrnlitem_id, :stdjrnlitem_stdjrnl_id, :stdjrnlitem_accnt_id,"
               "  :stdjrnlitem_amount, :stdjrnlitem_notes );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdjrnlitem "
               "SET stdjrnlitem_accnt_id=:stdjrnlitem_accnt_id,"
               "    stdjrnlitem_amount=:stdjrnlitem_amount,"
               "    stdjrnlitem_notes=:stdjrnlitem_notes "
               "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
 
  q.bindValue(":stdjrnlitem_id", _stdjrnlitemid);
  q.bindValue(":stdjrnlitem_stdjrnl_id", _stdjrnlid);
  q.bindValue(":stdjrnlitem_accnt_id", _account->id());
  q.bindValue(":stdjrnlitem_amount", amount);
  q.bindValue(":stdjrnlitem_notes", _notes->text().stripWhiteSpace());
  q.exec();

  done(_stdjrnlitemid);
}

void standardJournalItem::populate()
{
  q.prepare( "SELECT stdjrnlitem_accnt_id, stdjrnlitem_notes,"
             "       ABS(stdjrnlitem_amount) AS amount,"
             "       (stdjrnlitem_amount < 0) AS debit "
             "FROM stdjrnlitem "
             "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
  q.bindValue(":stdjrnlitem_id", _stdjrnlitemid);
  q.exec();
  if (q.first())
  {
    _amount->setBaseValue(q.value("amount").toDouble());
    if (q.value("debit").toBool())
      _debit->setChecked(TRUE);
    else
      _credit->setChecked(TRUE);

    _account->setId(q.value("stdjrnlitem_accnt_id").toInt());
    _notes->setText(q.value("stdjrnlitem_notes").toString());
  }
}

