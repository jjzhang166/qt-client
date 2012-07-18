/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "currencyConversion.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "xcombobox.h"

#define DEBUG true

currencyConversion::currencyConversion(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl),
    _curr_id(0),
    _curr_rate_id(0)
{
  setupUi(this);

  connect(_buttonBox,    SIGNAL(rejected()),    this, SLOT(sClose()));
  connect(_buttonBox,    SIGNAL(accepted()),    this, SLOT(sSave()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sHandleInactive()));

  _rate->setValidator(omfgThis->ratioVal());
}

currencyConversion::~currencyConversion()
{
  // no need to delete child widgets, Qt does it all for us
}

void currencyConversion::languageChange()
{
  retranslateUi(this);
}

enum SetResponse currencyConversion::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("curr_rate_id", &valid);
  if (valid)
  {
    _curr_rate_id = param.toInt();
  }

  param = pParams.value("curr_id", &valid);
  if (valid)
  {
    _curr_id = param.toInt();
  }

  sPopulate();

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
      
      _currency->setEnabled(FALSE);
      _rate->setEnabled(FALSE);
      _dateCluster->setEnabled(FALSE);
      
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void currencyConversion::sClose()
{
  done(_curr_rate_id);
}

void currencyConversion::sHandleInactive()
{
  if (_showInactive->isChecked())
  {
    _currency->setType(XComboBox::Adhoc);
    _currency->populate("SELECT curr_id, currConcat(curr_abbr, curr_symbol), curr_abbr "
                        "  FROM curr_symbol"
                        " WHERE NOT curr_base"
                        " ORDER BY curr_abbr;");
  }
  else
    _currency->setType(XComboBox::CurrenciesNotBase);
}

void currencyConversion::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _currency->isValid(), _currency,
                          tr("Please specify a currency for this exchange rate."))
         << GuiErrorCheck(_rate->toDouble() == 0, _rate,
                          tr("You must specify a Rate that is not zero."))
         << GuiErrorCheck(! _dateCluster->startDate().isValid(), _dateCluster,
                          tr("Please specify a Start Date for this exchange rate."))
         << GuiErrorCheck(! _dateCluster->endDate().isValid(), _dateCluster,
                          tr("Please specify an End Date for this exchange rate."))
         << GuiErrorCheck(_dateCluster->startDate() > _dateCluster->endDate(), _dateCluster,
                          tr("<p>The Start Date for this exchange rate is "
                              "later than the End Date. "
                              "Please check the values of these dates."));
  ;

  XSqlQuery saveq;
  saveq.prepare("SELECT EXISTS(SELECT 1"
                "         FROM curr_rate "
                "         WHERE curr_id = :curr_id"
                "           AND curr_rate_id != :curr_rate_id"
                "           AND ((curr_effective BETWEEN :curr_effective AND :curr_expires OR"
                "                 curr_expires BETWEEN :curr_effective AND :curr_expires)"
                "            OR  (curr_effective <= :curr_effective AND"
                "                 curr_expires   >= :curr_expires))) AS overlap;" );
  saveq.bindValue(":curr_rate_id",   _curr_rate_id);
  saveq.bindValue(":curr_id",        _currency->id());
  saveq.bindValue(":curr_effective", _dateCluster->startDate());
  saveq.bindValue(":curr_expires",   _dateCluster->endDate());
  saveq.exec();
  if (saveq.first())
  {
    errors << GuiErrorCheck(saveq.value("overlap").toBool(), _dateCluster,
                            tr("<p>The date range overlaps with another date "
                               "range for this currency. "
                               "Please check the values of these dates."));
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Exchange Rate"), errors))
      return;

  QString inverter("");
  if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
      inverter = "1 / ";

  QString sql;
  if (_mode == cNew)
      sql = QString("INSERT INTO curr_rate "
                    "(curr_id, curr_rate, curr_effective, curr_expires) "
                    "VALUES "
                    "(:curr_id, %1 CAST(:curr_rate AS NUMERIC), "
                    " :curr_effective, :curr_expires)")
                  .arg(inverter);
  else if (_mode == cEdit)
      sql = QString("UPDATE curr_rate SET "
                    "curr_id = :curr_id, "
                    "curr_rate = %1 CAST(:curr_rate AS NUMERIC), "
                    "curr_effective = :curr_effective, "
                    "curr_expires = :curr_expires "
                    "WHERE curr_rate_id = :curr_rate_id")
                    .arg(inverter);


  saveq.prepare(sql);
  saveq.bindValue(":curr_rate_id", _curr_rate_id);
  saveq.bindValue(":curr_id", _currency->id());
  saveq.bindValue(":curr_rate", _rate->toDouble());
  saveq.bindValue(":curr_effective", _dateCluster->startDate());
  saveq.bindValue(":curr_expires", _dateCluster->endDate());
  
  saveq.exec();

  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Exchange Rate"),
                           saveq, __FILE__, __LINE__))
      return;

  done(_curr_rate_id);
}

void currencyConversion::sPopulate()
{
  XSqlQuery currencypopulate;

  if (_curr_rate_id)
  {
    QString inverter("");
    if (_metrics->value("CurrencyExchangeSense").toInt() == 1)
        inverter = "1 / ";
    QString sql = QString("SELECT curr_id, %1 curr_rate AS curr_rate, "
                          "curr_effective, curr_expires "
                          "FROM curr_rate "
                          "WHERE curr_rate_id = :curr_rate_id;")
                          .arg(inverter);
    currencypopulate.prepare(sql);
    currencypopulate.bindValue(":curr_rate_id", _curr_rate_id);
    currencypopulate.exec();
    if (currencypopulate.first())
    {
      _currency->setId(currencypopulate.value("curr_id").toInt());
      _dateCluster->setStartDate(currencypopulate.value("curr_effective").toDate());
      _dateCluster->setEndDate(currencypopulate.value("curr_expires").toDate());
      _rate->setDouble(currencypopulate.value("curr_rate").toDouble());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Exchange Rate"),
                                  currencypopulate, __FILE__, __LINE__))
      return;
  }
  if (_curr_id)
  {
    _currency->setId(_curr_id);
  }
}
