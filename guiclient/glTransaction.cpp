/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glTransaction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "glcluster.h"
#include <openreports.h>
#include "errorReporter.h"

glTransaction::glTransaction(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Post"));
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sPost()));

    // This should all be generated as part of the UI but it was the only
    // way I could get the tab order to work exactly as it was supposed to.
    QWidget::setTabOrder(_amount, _distDate);
    QWidget::setTabOrder(_distDate, _docType);
    QWidget::setTabOrder(_docType, _docNumber);
    QWidget::setTabOrder(_docNumber, _debit);
    QWidget::setTabOrder(_debit, _credit);
    QWidget::setTabOrder(_credit, _notes);
    QWidget::setTabOrder(_notes, _buttonBox->button(QDialogButtonBox::Ok));
    QWidget::setTabOrder(_buttonBox->button(QDialogButtonBox::Ok), _buttonBox->button(QDialogButtonBox::Cancel));

    _captive = false;
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
  XDialog::set(pParams);
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

      _docType->setEnabled(false);
      _docType->setText("JE");
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _amount->setEnabled(false);
      _docNumber->setEnabled(false);
      _distDate->setEnabled(false);
      _docType->setEnabled(false);
      _debit->setEnabled(false);
      _credit->setEnabled(false);
      _notes->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void glTransaction::sPost()
{
  XSqlQuery glPost;
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
    { !_metrics->boolean("IgnoreCompany") &&
      _credit->companyId() != _debit->companyId(),
      tr("The Accounts must belong to the same Company to Post this transaciton." ), _credit },
    { _metrics->boolean("MandatoryGLEntryNotes") &&
      _notes->toPlainText().trimmed().isEmpty(),
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

  glPost.prepare( "SELECT insertGLTransaction( fetchJournalNumber('GL-MISC'), 'G/L', :docType, :docNumber, :notes,"
             "                            :creditAccntid, :debitAccntid, -1, :amount, :distDate, true, false ) AS result;" );
  glPost.bindValue(":distDate", _distDate->date());
  glPost.bindValue(":docType", _docType->text().trimmed());
  glPost.bindValue(":docNumber", _docNumber->text().trimmed());
  glPost.bindValue(":notes", _notes->toPlainText().trimmed());
  glPost.bindValue(":creditAccntid", _credit->id());
  glPost.bindValue(":debitAccntid", _debit->id());
  glPost.bindValue(":amount", _amount->baseValue());
  glPost.exec();
  if (glPost.first())
  {
//  Print on Post
    if (_print->isChecked())
    {
      ParameterList params;
      params.append("sequence", glPost.value("result").toInt());

      orReport report("GLSimple", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }

    if (_captive)
      done(glPost.value("result").toInt());
    else
    {
      clear();
      _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Cancel));
      _buttonBox->removeButton(_buttonBox->button(QDialogButtonBox::Close));
      QPushButton* button = _buttonBox->addButton(QDialogButtonBox::Close);
      button->setShortcut(QKeySequence::Close);
      button->setToolTip(button->text().append(" ").append(button->shortcut().toString(QKeySequence::NativeText)));
      _amount->setFocus();
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting G/L Transaction"),
                                glPost, __FILE__, __LINE__))
  {
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
