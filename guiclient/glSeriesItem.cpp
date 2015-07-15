/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "glSeriesItem.h"

#include <math.h>

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

glSeriesItem::glSeriesItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _glseriesid = -1;
    _glsequence = -1;

    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
}

glSeriesItem::~glSeriesItem()
{
    // no need to delete child widgets, Qt does it all for us
}

void glSeriesItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse glSeriesItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("glseries_id", &valid);
  if (valid)
  {
    _glseriesid = param.toInt();
    populate();
  }

  param = pParams.value("doctype", &valid);
  if (valid)
    _doctype = param.toString();

  param = pParams.value("docnumber", &valid);
  if (valid)
    _docnumber = param.toString();

  param = pParams.value("glSequence", &valid);
  if (valid)
    _glsequence = param.toInt();

  param = pParams.value("distDate", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
  }

  param = pParams.value("postStandardJournal", &valid);
  if (valid)
    _doctype = "ST";

  return NoError;
}

void glSeriesItem::sSave()
{
  XSqlQuery glSave;
  if (! _amount->isBase() &&
      QMessageBox::question(this, tr("G/L Transaction Not In Base Currency"),
		          tr("G/L transactions are recorded in the base currency.\n"
			  "Do you wish to convert %1 %2 at the rate effective on %3?")
			  .arg(_amount->localValue()).arg(_amount->currAbbr())
			  .arg(_amount->effective().toString(Qt::LocalDate)),
			  QMessageBox::Yes|QMessageBox::Escape,
			  QMessageBox::No |QMessageBox::Default) != QMessageBox::Yes)
  {
	_amount->setFocus();
	return;
  }

  if (!_metrics->boolean("IgnoreCompany") && _metrics->value("GLCompanySize").toInt())
  {
    XSqlQuery co;
    co.prepare("SELECT company_id "
               "FROM glseries "
               " JOIN accnt ON (accnt_id=glseries_accnt_id) "
               " JOIN company ON (accnt_company=company_number) "
               "WHERE ((glseries_sequence=:glsequence) "
               " AND (glseries_id != :glseries_id) "
               " AND (company_id != :company_id));");
    co.bindValue(":glsequence", _glsequence);
    co.bindValue(":glseries_id", _glseriesid);
    co.bindValue(":company_id", _account->companyId());
    co.exec();
    if (co.first())
    {
      QMessageBox::critical(this, tr("Can not Save Series Item"),
                            tr("The Company of this Account does not match the "
                               "Companies for other Accounts on this series.  This "
                               "entry can not be saved."));
      _account->setFocus();
      return;
    }
  }

  double amount = _amount->baseValue();
  if (_debit->isChecked())
    amount *= -1;

  if (_mode == cNew)
    glSave.prepare("SELECT insertIntoGLSeries(:glsequence, 'G/L', :doctype, :docnumber, :accnt_id, :amount, :distdate) AS result;");
  else if (_mode == cEdit)
    glSave.prepare( "UPDATE glseries "
               "SET glseries_accnt_id=:accnt_id,"
	       "    glseries_amount=:amount "
               "WHERE (glseries_id=:glseries_id);" );

  glSave.bindValue(":glseries_id",	_glseriesid);
  glSave.bindValue(":glsequence",	_glsequence);
  glSave.bindValue(":doctype",       _doctype);
  glSave.bindValue(":docnumber",     _docnumber);
  glSave.bindValue(":accnt_id",	_account->id());
  glSave.bindValue(":amount",	amount);
  glSave.bindValue(":distdate",	_amount->effective());
  glSave.exec();
  if (glSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, glSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void glSeriesItem::populate()
{
  XSqlQuery glpopulate;
  glpopulate.prepare( "SELECT glseries_amount, glseries_accnt_id "
             "FROM glseries "
             "WHERE (glseries_id=:glseries_id);" );
  glpopulate.bindValue(":glseries_id", _glseriesid);
  glpopulate.exec();
  if (glpopulate.first())
  {
    if (glpopulate.value("glseries_amount").toDouble() < 0)
    {
      _debit->setChecked(true);
      _amount->setBaseValue(fabs(glpopulate.value("glseries_amount").toDouble()));
    }
    else
    {
      _credit->setChecked(true);
      _amount->setBaseValue(glpopulate.value("glseries_amount").toDouble());
    }

    _account->setId(glpopulate.value("glseries_accnt_id").toInt());
  }
  else if (glpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, glpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
