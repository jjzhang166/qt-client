/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uomConv.h"

#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"
#include "guiErrorCheck.h"

uomConv::uomConv(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _uomidFrom = -1;
  _uomconvid = -1;
  _ignoreSignals = false;

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sFromChanged()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sToChanged()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _fromValue->setValidator(omfgThis->ratioVal());
  _toValue->setValidator(omfgThis->ratioVal());
}

uomConv::~uomConv()
{
  // no need to delete child widgets, Qt does it all for us
}

void uomConv::languageChange()
{
  retranslateUi(this);
}

enum SetResponse uomConv::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("from_uom_id", &valid);
  if (valid)
  {
    _uomidFrom = param.toInt();
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }

  param = pParams.value("uomconv_id", &valid);
  if (valid)
  {
    _uomconvid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _fromValue->setEnabled(false);
      _toValue->setEnabled(false);
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _fractional->setEnabled(false);
      _cancel->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void uomConv::sSave()
{
  XSqlQuery uomSave;
  bool valid;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_uomFrom->isValid(), _uomFrom,
                          tr("You must select a From UOM."))
         << GuiErrorCheck(!_uomTo->isValid(), _uomTo,
                          tr("You must select a To UOM."))
         << GuiErrorCheck(_fromValue->toDouble(&valid) == 0, _fromValue,
                          tr("You must enter a valid Ratio before saving this UOM Conversion."))
         << GuiErrorCheck(_toValue->toDouble(&valid) == 0, _toValue,
                          tr("You must enter a valid Ratio before saving this UOM Conversion."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save UOM Conversion"), errors))
    return;

  if (_mode == cEdit)
    uomSave.prepare( "UPDATE uomconv "
               "   SET uomconv_from_value=:uomconv_from_value,"
               "       uomconv_to_value=:uomconv_to_value,"
               "       uomconv_fractional=:uomconv_fractional "
               " WHERE(uomconv_id=:uomconv_id);" );
  else if (_mode == cNew)
  {
    uomSave.exec("SELECT NEXTVAL('uomconv_uomconv_id_seq') AS uomconv_id");
    if (uomSave.first())
      _uomconvid = uomSave.value("uomconv_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    uomSave.prepare( "INSERT INTO uomconv "
               "( uomconv_id, uomconv_from_uom_id, uomconv_from_value, uomconv_to_uom_id, uomconv_to_value, uomconv_fractional ) "
               "VALUES "
               "( :uomconv_id, :uomconv_from_uom_id, :uomconv_from_value, :uomconv_to_uom_id, :uomconv_to_value, :uomconv_fractional );" );
  }

  uomSave.bindValue(":uomconv_id", _uomconvid);
  uomSave.bindValue(":uomconv_from_uom_id", _uomFrom->id());
  uomSave.bindValue(":uomconv_to_uom_id", _uomTo->id());
  uomSave.bindValue(":uomconv_from_value", _fromValue->toDouble());
  uomSave.bindValue(":uomconv_to_value", _toValue->toDouble());
  uomSave.bindValue(":uomconv_fractional", QVariant(_fractional->isChecked()));
  uomSave.exec();

  accept();
}

void uomConv::sFromChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sToChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sCheck()
{
  XSqlQuery uomCheck;
  if ( (_mode == cNew) )
  {
    uomCheck.prepare( "SELECT uomconv_id"
               "  FROM uomconv"
               " WHERE((uomconv_from_uom_id=:from AND uomconv_to_uom_id=:to)"
               "    OR (uomconv_from_uom_id=:to AND uomconv_to_uom_id=:from));" );
    uomCheck.bindValue(":from", _uomFrom->id());
    uomCheck.bindValue(":to", _uomTo->id());
    uomCheck.exec();
    if (uomCheck.first())
    {
      _uomconvid = uomCheck.value("uomconv_id").toInt();
      _mode = cEdit;
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      populate();
    }
  }
}

void uomConv::populate()
{
  XSqlQuery uompopulate;
  uompopulate.prepare( "SELECT uomconv_from_uom_id,"
             "       uomconv_from_value,"
             "       uomconv_to_uom_id,"
             "       uomconv_to_value,"
             "       uomconv_fractional "
             "  FROM uomconv"
             " WHERE(uomconv_id=:uomconv_id);" );
  uompopulate.bindValue(":uomconv_id", _uomconvid);
  uompopulate.exec();
  if (uompopulate.first())
  {
    _uomidFrom = uompopulate.value("uomconv_from_uom_id").toInt();
    _uomFrom->setId(_uomidFrom);
    _uomTo->setId(uompopulate.value("uomconv_to_uom_id").toInt());
    _fromValue->setText(uompopulate.value("uomconv_from_value").toDouble());
    _toValue->setText(uompopulate.value("uomconv_to_value").toDouble());
    _fractional->setChecked(uompopulate.value("uomconv_fractional").toBool());
  }
}

