/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configurescales.h"

#include <metasql.h>
#include <parameter.h>

#include "errorReporter.h"

class configureScalesPrivate {
  public:
    configureScalesPrivate()
    {
    }
};

configureScales::configureScales(QWidget *parent, const char *name, bool modal, Qt::WFlags fl)
  : XAbstractConfigure(parent, fl),
    _private(0)
{
  Q_UNUSED(modal);
  setupUi(this);

  if (name)
    setObjectName(name);

  connect(_costScale,     SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_moneyScale,    SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_percentScale,  SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_purchpScale,   SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyScale,      SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_qtyperScale,   SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_salepScale,    SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_uomratioScale, SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));
  connect(_weightScale,   SIGNAL(valueChanged(int)), this, SLOT(sUpdateSamples()));

  sPopulate();
}

configureScales::~configureScales()
{
  if (_private)
  {
    delete _private;
    _private = 0;
  }
}

void configureScales::languageChange()
{
  retranslateUi(this);
}

bool configureScales::sSave()
{
  emit saving();

  XSqlQuery saveq;
  saveq.prepare("SELECT setMetric('SCALE_COST',     CAST(:cost     AS TEXT)),"
                "       setMetric('SCALE_MONEY',    CAST(:money    AS TEXT)),"
                "       setMetric('SCALE_PERCENT',  CAST(:percent  AS TEXT)),"
                "       setMetric('SCALE_PURCHP',   CAST(:purchp   AS TEXT)),"
                "       setMetric('SCALE_QTY',      CAST(:qty      AS TEXT)),"
                "       setMetric('SCALE_QTYPER',   CAST(:qtyper   AS TEXT)),"
                "       setMetric('SCALE_SALEP',    CAST(:salep    AS TEXT)),"
                "       setMetric('SCALE_UOMRATIO', CAST(:uomratio AS TEXT)),"
                "       setMetric('SCALE_WEIGHT',   CAST(:weight   AS TEXT))"
                ";");
  saveq.bindValue(":cost",     _costScale->value());
  saveq.bindValue(":money",    _moneyScale->value());
  saveq.bindValue(":percent",  _percentScale->value());
  saveq.bindValue(":purchp",   _purchpScale->value());
  saveq.bindValue(":qty",      _qtyScale->value());
  saveq.bindValue(":qtyper",   _qtyperScale->value());
  saveq.bindValue(":salep",    _salepScale->value());
  saveq.bindValue(":uomratio", _uomratioScale->value());
  saveq.bindValue(":weight",   _weightScale->value());
  saveq.exec();
  // TODO: update runtime env?
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Setting Search Path"),
                           saveq, __FILE__, __LINE__))
    return false;

  return true;
}

void configureScales::sPopulate()
{
  XSqlQuery getq("SELECT fetchMetricValue('SCALE_COST')         AS cost,"
                 "       fetchMetricValue('SCALE_MONEY')        AS money,"
                 "       fetchMetricValue('SCALE_PERCENT')      AS percent,"
                 "       fetchMetricValue('SCALE_PURCHP')       AS purchp,"
                 "       fetchMetricValue('SCALE_QTY')          AS qty,"
                 "       fetchMetricValue('SCALE_QTYPER')       AS qtyper,"
                 "       fetchMetricValue('SCALE_SALEP')        AS salep,"
                 "       fetchMetricValue('SCALE_UOMRATIO')     AS uomratio,"
                 "       fetchMetricValue('SCALE_WEIGHT')       AS weight"
                 ";");
  if (getq.first())
  {
    _costScale->setValue(getq.value("cost").toInt());
    _moneyScale->setValue(getq.value("money").toInt());
    _percentScale->setValue(getq.value("percent").toInt());
    _purchpScale->setValue(getq.value("purchp").toInt());
    _qtyScale->setValue(getq.value("qty").toInt());
    _qtyperScale->setValue(getq.value("qtyper").toInt());
    _salepScale->setValue(getq.value("salep").toInt());
    _uomratioScale->setValue(getq.value("uomratio").toInt());
    _weightScale->setValue(getq.value("weight").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Scales"),
                                getq, __FILE__, __LINE__))
    return;
}

void configureScales::sUpdateSamples()
{
  double tmpVal = 1234567890.9876543210;

  _costSample->setText(QLocale().toString(tmpVal,     'f', _moneyScale->value() +
                                                           _costScale->value()));
  _moneySample->setText(QLocale().toString(tmpVal,    'f', _moneyScale->value()));
  _percentSample->setText(QLocale().toString(tmpVal,  'f', _percentScale->value()));
  _purchpSample->setText(QLocale().toString(tmpVal,   'f', _moneyScale->value() +
                                                           _purchpScale->value()));
  _qtySample->setText(QLocale().toString(tmpVal,      'f', _qtyScale->value()));
  _qtyperSample->setText(QLocale().toString(tmpVal,   'f', _qtyperScale->value()));
  _salepSample->setText(QLocale().toString(tmpVal,    'f', _moneyScale->value() +
                                                           _salepScale->value()));
  _uomratioSample->setText(QLocale().toString(tmpVal, 'f', _uomratioScale->value()));
  _weightSample->setText(QLocale().toString(tmpVal,   'f', _weightScale->value()));
}
