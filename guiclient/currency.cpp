/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currency.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"

currency::currency(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl),
    _currid(-1)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(sClose()));

  // avoid sConfirmBaseFlag() when calling populate for editing base currency
  // baseOrig gets set in set() for cNew and in populate() for cEdit and cView
  baseOrig = TRUE;
}

currency::~currency()
{
  // no need to delete child widgets, Qt does it all for us
}

void currency::languageChange()
{
  retranslateUi(this);
}

bool currency::isBaseSet()
{
  XSqlQuery baseq;
  bool hasBase = false;
  
  baseq.prepare("SELECT EXISTS(SELECT 1"
                "              FROM curr_symbol WHERE curr_base) AS hasbase;");
  baseq.exec();
  if (baseq.first())
  {
    hasBase = baseq.value("hasBase").toBool();
  }
  ErrorReporter::error(QtCriticalMsg, this, tr("Checking for Base"),
                       baseq, __FILE__, __LINE__);

  return hasBase;
}

enum SetResponse currency::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _currid = param.toInt();
    populate();
  }
  else
    baseOrig = FALSE; // see comments in constructor

   
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _currBase->setEnabled(! isBaseSet());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _currBase->setEnabled(! isBaseSet());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _currName->setEnabled(FALSE);
      _currSymbol->setEnabled(FALSE);
      _currAbbr->setEnabled(FALSE);
      _currBase->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void currency::sSave()
{
  XSqlQuery saveq;
  sConfirmBaseFlag();

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_currName->text().isEmpty(), _currName,
                          tr("Currency name is required."))
         << GuiErrorCheck(_currAbbr->text().isEmpty() && _currSymbol->text().isEmpty(),
                          _currSymbol,
                          tr("<p>Either the currency symbol or abbreviation must be "
                             "supplied. Both would be better."))
         << GuiErrorCheck(_currAbbr->text().length() > 3, _currAbbr,
                          tr("<p>The currency abbreviation must have 3 or fewer "
                             "characters. ISO abbreviations are 3 characters long."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Currency"), errors))
    return;
  
  if (_mode == cNew)
  {
    saveq.prepare("INSERT INTO curr_symbol ("
		  "   curr_name,   curr_symbol,  curr_abbr,  curr_base"
                  "   curr_number, curr_scale"
		  ") VALUES ("
		  "  :curr_name,  :curr_symbol, :curr_abbr, :curr_base"
                  "  :curr_number,:curr_scale"
                  ") RETURNING curr_id;" );
  }
  else if (_mode == cEdit)
  {
    saveq.prepare("UPDATE curr_symbol"
                  "   SET curr_name  =:curr_name,   curr_symbol=:curr_symbol,"
                  "       curr_abbr  =:curr_abbr,   curr_base  =:curr_base,"
                  "       curr_number=:curr_number, curr_scale =:curr_scale"
                  " WHERE (curr_id=:curr_id)"
                  " RETURNING curr_id;" );
    saveq.bindValue(":curr_id", _currid);
   }
  
  saveq.bindValue(":curr_name",   _currName->text());
  saveq.bindValue(":curr_symbol", _currSymbol->text());
  saveq.bindValue(":curr_abbr",   _currAbbr->text());
  saveq.bindValue(":curr_base",   QVariant(_currBase->isChecked()));
  saveq.bindValue(":curr_number", _number->text());
  saveq.bindValue(":curr_scale",  _minorUnit->value());
  saveq.exec();
  if (saveq.first())
    _currid = saveq.value("curr_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Currency"),
                                saveq, __FILE__, __LINE__))
    return;
  
  done(_currid);
}

void currency::populate()
{
  XSqlQuery getq;
  getq.prepare("SELECT *"
               "  FROM curr_symbol"
               " WHERE (curr_id=:curr_id);");
  getq.bindValue(":curr_id", _currid);
  getq.exec();
  if (getq.first())
  {
    _currName->setText(getq.value("curr_name").toString());
    _currSymbol->setText(getq.value("curr_symbol").toString());
    _currAbbr->setText(getq.value("curr_abbr").toString());
    _currBase->setChecked(getq.value("curr_base").toBool());
    _number->setText(getq.value("curr_number").toString());
    _minorUnit->setValue(getq.value("curr_scale").toInt());

    baseOrig = _currBase->isChecked();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Currency"),
                                getq, __FILE__, __LINE__))
    return;
}

void currency::sConfirmBaseFlag()
{
  if (_currBase->isChecked() && !baseOrig)
  {
    int response = QMessageBox::warning (this, tr("Set Base Currency?"),
                            tr("You cannot change the base currency "
                              "after it is set.  Are you sure you want "
                              "%1 to be the base currency?")
                              .arg(_currName->text()),
                            QMessageBox::Yes | QMessageBox::Escape,
                            QMessageBox::No | QMessageBox::Default);
    if (response != QMessageBox::Yes)
    {
      _currBase->setChecked(FALSE);
    }
  }
}

void currency::sClose()
{
  close();
}
