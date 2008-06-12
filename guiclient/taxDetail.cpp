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

#include "taxDetail.h"

#include <QSqlError>
#include <QVariant>

taxDetail::taxDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_amountA,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_amountB,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_amountC,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_calculate, SIGNAL(clicked()),	this, SLOT(sCalculateTax()));
    connect(_cancel,	SIGNAL(clicked()),	this, SLOT(sCancel()));
    connect(_taxCode,	SIGNAL(newID(int)),	this, SLOT(sCalculateTax()));
    connect(_taxCode,	SIGNAL(newID(int)),	this, SLOT(sPopulate()));

    _blankDetailDescriptions = false;
    clear();
    _taxCodeInitialized = false;
    
    sCalculateTotal();
}

taxDetail::~taxDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

void taxDetail::languageChange()
{
    retranslateUi(this);
}

enum SetResponse taxDetail::set(const ParameterList & pParams )
{
  QVariant param;
  bool     valid;
  
  _readonly = pParams.inList("readOnly") ||
	(!pParams.inList("readOnly") && !_privileges->check("OverrideTax"));

  // tax_id first so signal/slot cascade doesn't overwrite passed-in A,B,C
  param = pParams.value("tax_id", &valid);
  _blankDetailDescriptions = !valid;
  if (valid)
    _taxCode->setId(param.toInt());
  else
    clear();
  _taxCodeInitialized = true;

  param = pParams.value("curr_id", &valid);
  if (valid)
    _total->setId(param.toInt());

  param = pParams.value("date", &valid);
  if (valid)
    _total->setEffective(param.toDate());

  param = pParams.value("valueA", &valid);
  if (valid)
  {
    _aCache = param.toDouble();
    _amountA->setLocalValue(_aCache);
  }

  param = pParams.value("valueB", &valid);
  if (valid)
  {
    _bCache = param.toDouble();
    _amountB->setLocalValue(_bCache);
  }

  param = pParams.value("valueC", &valid);
  if (valid)
  {
    _cCache = param.toDouble();
    _amountC->setLocalValue(_cCache);
  }

  param = pParams.value("pctA", &valid);
  if (valid)
  {
    _aPctCache = param.toDouble();
    _prcntA->setText(QString::number(_aPctCache));
  }

  param = pParams.value("pctB", &valid);
  if (valid)
  {
    _bPctCache = param.toDouble();
    _prcntB->setText(QString::number(_bPctCache));
  }

  param = pParams.value("pctC", &valid);
  if (valid)
  {
    _cPctCache = param.toDouble();
    _prcntC->setText(QString::number(_cPctCache));
  }
  
  param = pParams.value("subtotal", &valid);
  if (valid)
    _subtotal->setLocalValue(param.toDouble());

  sCalculateTotal();
  
  if (_taxCode->id() == -1 || _subtotal->isZero())
  {
    _calculate->setEnabled(FALSE);
    _calculate->hide();
  }

  if (_readonly)
  {
    _taxCode->setEnabled(FALSE);
    _calculate->setEnabled(FALSE);
    _calculate->hide();
    _save->hide();
    _cancel->setText(tr("&Close"));
  }

  return NoError;
}

void taxDetail::sCancel()
{
    _amountA->setLocalValue(_aCache);
    _amountB->setLocalValue(_bCache);
    _amountC->setLocalValue(_cCache);
    _prcntA->setText(QString::number(_aPctCache));
    _prcntB->setText(QString::number(_bPctCache));
    _prcntC->setText(QString::number(_cPctCache));
    reject();
}

void taxDetail::sCalculateTotal()
{
  _total->setLocalValue(_amountA->localValue() + _amountB->localValue() + _amountC->localValue());
}

double taxDetail::amountA() const
{
  return _amountA->localValue();
}

double taxDetail::amountB() const
{
  return _amountB->localValue();
}

double taxDetail::amountC() const
{
  return _amountC->localValue();
}

double taxDetail::pctA() const
{
  return _prcntA->text().toDouble();
}

double taxDetail::pctB() const
{
  return _prcntB->text().toDouble();
}

double taxDetail::pctC() const
{
  return _prcntC->text().toDouble();
}

int taxDetail::tax() const
{
  return _taxCode->id();
}

void taxDetail::sCalculateTax()
{
  XSqlQuery calcq;
  calcq.prepare("SELECT calculateTax(:tax_id, :subtotal, 0, 'A') AS ratea,"
            "       calculateTax(:tax_id, :subtotal, 0, 'B') AS rateb,"
            "       calculateTax(:tax_id, :subtotal, 0, 'C') AS ratec;" );
  calcq.bindValue(":tax_id", _taxCode->id());
  calcq.bindValue(":subtotal", _subtotal->localValue());
  calcq.exec();
  if(calcq.first())
  {
    _amountA->setLocalValue(calcq.value("ratea").toDouble());
    _amountB->setLocalValue(calcq.value("rateb").toDouble());
    _amountC->setLocalValue(calcq.value("ratec").toDouble());
    sCalculateTotal();
  }
  else if (calcq.lastError().type() != QSqlError::None)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxDetail::clear()
{
  if (_blankDetailDescriptions)
  {
    _description->setText("");
    _cumulative->setText("");
    _accntA->setText("");
    _accntB->setText("");
    _accntC->setText("");
    _taxCode->setNullStr("");
  }
  else
  {
    _description->setText(tr("Unspecified"));
    _cumulative->setText(tr("Unspecified"));
    _accntA->setText(tr("unassigned"));
    _accntB->setText(tr("unassigned"));
    _accntC->setText(tr("unassigned"));
    _taxCode->setNullStr(tr("Unspecified"));
  }

  _aPctCache = 0;
  _bPctCache = 0;
  _cPctCache = 0;
  _prcntA->clear();
  _prcntB->clear();
  _prcntC->clear();
  _amountA->setEnabled(false);
  _amountB->setEnabled(false);
  _amountC->setEnabled(false);
}

void taxDetail::sPopulate()
{
  clear();
  XSqlQuery popq;
  popq.prepare("SELECT tax_descrip,"
	    "       formatBoolYN(tax_cumulative) AS f_cumulative,"
	    "       formatScrap(tax_ratea) AS ratea,"
	    "       formatGLAccountLong(tax_sales_accnt_id) AS accnta,"
	    "       COALESCE(tax_sales_accnt_id,-1) AS accnta_id,"
	    "       formatScrap(tax_rateb) AS rateb,"
	    "       formatGLAccountLong(tax_salesb_accnt_id) AS accntb,"
	    "       COALESCE(tax_salesb_accnt_id,-1) AS accntb_id,"
	    "       formatScrap(tax_ratec) AS ratec,"
	    "       formatGLAccountLong(tax_salesc_accnt_id) AS accntc,"
	    "       COALESCE(tax_salesc_accnt_id,-1) AS accntc_id"
	    "  FROM tax"
	    " WHERE (tax_id=:tax_id); ");
  popq.bindValue(":tax_id", _taxCode->id());
  popq.exec();
  if(popq.first())
  {
    _description->setText(popq.value("tax_descrip").toString());
    _cumulative->setText(popq.value("f_cumulative").toString());

    if (! _taxCodeInitialized)
    {
      _aPctCache = popq.value("ratea").toDouble();
      _bPctCache = popq.value("rateb").toDouble();
      _cPctCache = popq.value("ratec").toDouble();
    }

    if(popq.value("accnta_id").toInt() != -1)
    { 
      _prcntA->setText(popq.value("ratea").toString());
      _accntA->setText(popq.value("accnta").toString());
      _amountA->setEnabled(! _readonly);
    }

    if(popq.value("accntb_id").toInt() != -1)
    { 
      _prcntB->setText(popq.value("rateb").toString());
      _accntB->setText(popq.value("accntb").toString());
      _amountB->setEnabled(! _readonly);
    }

    if(popq.value("accntc_id").toInt() != -1)
    { 
      _prcntC->setText(popq.value("ratec").toString());
      _accntC->setText(popq.value("accntc").toString());
      _amountC->setEnabled(! _readonly);
    }
  }
  else if (popq.lastError().type() != QSqlError::None)
  {
    systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
