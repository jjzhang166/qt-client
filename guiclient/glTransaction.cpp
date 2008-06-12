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

#include "glTransaction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "glcluster.h"

glTransaction::glTransaction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

    _captive = FALSE;
}

glTransaction::~glTransaction()
{
    // no need to delete child widgets, Qt does it all for us
}

void glTransaction::languageChange()
{
    retranslateUi(this);
}

enum SetResponse glTransaction::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("glsequence", &valid);
  if (valid)
  {
    _glsequence = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      _distDate->setDate(omfgThis->dbDate(), true);

      _docType->setEnabled(FALSE);
      _docType->setText("JE");
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _amount->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _distDate->setEnabled(FALSE);
      _docType->setEnabled(FALSE);
      _debit->setEnabled(FALSE);
      _credit->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _post->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void glTransaction::sPost()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _amount->isZero(), tr("<p>You must enter an amount for this G/L "
			    "Transaction before you may Post it."), _amount },

    { ! _debit->isValid(), tr("<p>You must select a Debit Account for this G/L "
			    "Transaction before you may Post it." ), _debit },
    { ! _credit->isValid(), tr("<p>You must select a Credit Account for this G/L "
			     "Transaction before you may Post it." ), _credit },
    { _metrics->boolean("MandatoryGLEntryNotes") &&
      _notes->text().stripWhiteSpace().isEmpty(),
      tr("<p>You must enter some Notes to describe this transaction."), _notes},
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Post G/L Journal Entry"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  if (! _amount->isBase() &&
      QMessageBox::question(this, tr("G/L Transaction Not In Base Currency"),
		          tr("G/L transactions are recorded in the base currency.\n"
			  "Do you wish to convert %1 %2 at the rate effective on %3?")
			  .arg(_amount->localValue()).arg(_amount->currAbbr())
			  .arg(_distDate->date().toString(Qt::LocalDate)),
			  QMessageBox::Yes|QMessageBox::Escape,
			  QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
	_amount->setFocus();
	return;
  }

  q.prepare( "SELECT insertGLTransaction( 'G/L', :docType, :docNumber, :notes,"
             "                            :creditAccntid, :debitAccntid, -1, :amount, :distDate ) AS result;" );
  q.bindValue(":distDate", _distDate->date());
  q.bindValue(":docType", _docType->text().stripWhiteSpace());
  q.bindValue(":docNumber", _docNumber->text().stripWhiteSpace());
  q.bindValue(":notes", _notes->text().stripWhiteSpace());
  q.bindValue(":creditAccntid", _credit->id());
  q.bindValue(":debitAccntid", _debit->id());
  q.bindValue(":amount", _amount->baseValue());
  q.exec();
  if (q.first())
  {
    if (_captive)
      done(q.value("result").toInt());
    else
    {
      clear();
      _close->setText(tr("&Close"));
      _amount->setFocus();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void glTransaction::clear()
{
  _amount->clear();
  _docNumber->clear();
  _debit->setId(-1);
  _credit->setId(-1);
  _notes->clear();
  _mode = cNew;

  _amount->setFocus();
}

void glTransaction::populate()
{
}
