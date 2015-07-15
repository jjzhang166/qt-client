/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotSerialSequence.h"

#include <QVariant>
#include <QMessageBox>

lotSerialSequence::lotSerialSequence(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  
  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_number, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_prefix, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_nextValue, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_length, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));
  connect(_suffix, SIGNAL(editingFinished()), this, SLOT(sHandleExample()));


  _nextValue->setValidator(omfgThis->orderVal());
}

lotSerialSequence::~lotSerialSequence()
{
  // no need to delete child widgets, Qt does it all for us
}

void lotSerialSequence::languageChange()
{
  retranslateUi(this);
}

enum SetResponse lotSerialSequence::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("lsseq_id", &valid);
  if (valid)
  {
    _lsseqid = param.toInt();
    populate();
  }

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
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _number->setEnabled(false);
      _description->setEnabled(false);
      _prefix->setEnabled(false);
      _nextValue->setEnabled(false);
      _length->setEnabled(false);
      _suffix->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

bool lotSerialSequence::sCheck()
{
  XSqlQuery lotCheck;
  _number->setText(_number->text().trimmed());
  if ((_mode == cNew) && (_number->text().length() != 0))
  {
    lotCheck.prepare( "SELECT lsseq_id "
               "FROM lsseq "
               "WHERE (UPPER(lsseq_number)=UPPER(:lsseq_number));" );
    lotCheck.bindValue(":lsseq_number", _number->text());
    lotCheck.exec();
    if (lotCheck.first())
    {
      _lsseqid = lotCheck.value("lsseq_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(false);
      return true;
    }
  }
  return false;
}

void lotSerialSequence::sSave()
{
  XSqlQuery lotSave;
  _number->setText(_number->text().trimmed().toUpper());
  if (_number->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Sequence"),
                              tr("You must enter a valid Code for this Sequence.") );
    _number->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Sequence"),
                            tr("This Sequence number already exists.  You have been placed in edit mode.") );
      return;
    }

    lotSave.exec("SELECT NEXTVAL('lsseq_lsseq_id_seq') AS lsseq_id");
    if (lotSave.first())
      _lsseqid = lotSave.value("lsseq_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    lotSave.prepare( "INSERT INTO lsseq "
               "( lsseq_id, lsseq_number) "
               "VALUES "
               "( :lsseq_id, :lsseq_number);");
  }
  else if (_mode == cEdit)
    lotSave.prepare("SELECT lsseq_id"
              "  FROM lsseq"
              " WHERE((lsseq_id != :lsseq_id)"
              " AND (lsseq_number = :lsseq_number));");

  lotSave.bindValue(":lsseq_id", _lsseqid);
  lotSave.bindValue(":lsseq_number", _number->text());
  lotSave.exec();
  if(lotSave.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Sequence Number"),
                          tr("You may not rename this Sequence number with the entered name as it is in use by another Planner code.") );
    _number->setFocus();
    return;
  }

  lotSave.prepare( "UPDATE lsseq "
             "SET lsseq_number=:lsseq_number, lsseq_descrip=:lsseq_descrip,"
             "    lsseq_prefix=:lsseq_prefix,lsseq_seqlen=:lsseq_seqlen, "
             "    lsseq_suffix=:lsseq_suffix "
             "WHERE (lsseq_id=:lsseq_id);"
             "SELECT setval(:sequence, :next_value - 1);");

  lotSave.bindValue(":lsseq_id", _lsseqid);
  lotSave.bindValue(":lsseq_number", _number->text());
  lotSave.bindValue(":lsseq_descrip", _description->text().trimmed());
  lotSave.bindValue(":lsseq_prefix", _prefix->text());
  lotSave.bindValue(":lsseq_seqlen", _length->value());
  lotSave.bindValue(":lsseq_suffix", _suffix->text());
  lotSave.bindValue(":next_value", _nextValue->text());
  lotSave.bindValue(":sequence", QString("lsseq_number_seq_%1").arg(_lsseqid));
  lotSave.exec();

  done(_lsseqid);
}

void lotSerialSequence::populate()
{
  XSqlQuery lotpopulate;
  lotpopulate.prepare( "SELECT * "
             "FROM lsseq "
             "WHERE (lsseq_id=:lsseq_id);" );
  lotpopulate.bindValue(":lsseq_id", _lsseqid);
  lotpopulate.exec();
  if (lotpopulate.first())
  {
    _number->setText(lotpopulate.value("lsseq_number"));
    _description->setText(lotpopulate.value("lsseq_descrip"));
    _prefix->setText(lotpopulate.value("lsseq_prefix"));
    _length->setValue(lotpopulate.value("lsseq_seqlen").toInt());
    _suffix->setText(lotpopulate.value("lsseq_suffix"));

    lotpopulate.prepare("SELECT last_value + 1 AS next_number "
              "FROM lsseq_number_seq_:lsseq_id;");
    lotpopulate.bindValue(":lsseq_id", _lsseqid);
    lotpopulate.exec();
    if (lotpopulate.first())
      _nextValue->setText(lotpopulate.value("next_number"));
    sHandleExample();
  }
}

void lotSerialSequence::sHandleExample()
{
  _example->setText(_prefix->text() + _nextValue->text().rightJustified(_length->value(),QChar('0')) + _suffix->text());
} 

