/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxCodeRate.h"

#include <QMessageBox>
#include <QSqlError>
#include "errorReporter.h"

taxCodeRate::taxCodeRate(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  
  _percent->setValidator(omfgThis->negPercentVal());
}

taxCodeRate::~taxCodeRate()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxCodeRate::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxCodeRate::set( const ParameterList & pParams )
{
  XSqlQuery taxet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxrate_id", &valid);
  if (valid)
    _taxrateid = param.toInt();
  
  param = pParams.value("tax_id", &valid);
  if (valid)
    _taxId = param.toInt(); 

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
	  taxet.prepare(" (SELECT taxrate_id "
                " FROM taxrate "
                " WHERE taxrate_tax_id = :taxrate_tax_id); ");
	  taxet.bindValue(":taxrate_tax_id", _taxId);
	  taxet.exec();
	  if(taxet.first())
      {
	    XSqlQuery maxdate;
		maxdate.prepare(" (SELECT (MAX(taxrate_expires) + 1) AS max_expires"
                        " FROM taxrate "
                        " WHERE taxrate_tax_id = :taxrate_tax_id) ");
	    maxdate.bindValue(":taxrate_tax_id", _taxId);
	    maxdate.exec();
	    if(maxdate.first())
		{
		  _dates->setStartDate(maxdate.value("max_expires").toDate());
		}
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Rate Information"),
                                      maxdate, __FILE__, __LINE__))
        {
          return UndefinedError;
        }
	  }
      
      taxet.exec("SELECT NEXTVAL('taxrate_taxrate_id_seq') AS taxrate_id");
      if (taxet.first())
        _taxrateid = taxet.value("taxrate_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Rate Information"),
                                    taxet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
	}
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
	  sPopulate();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
	  sPopulate();

      _dates->setEnabled(false);
	  _percent->setEnabled(false);
	  _flat->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }
  
  return NoError;
}

void taxCodeRate::sSave()
{ 
  XSqlQuery taxSave;
  // Check if start date is greater than end date
  if( _dates->startDate() > _dates->endDate()) 
  {
	QMessageBox::critical(this, tr("Incorrect Date Entry"),
		tr("The start date should be earlier than the end date.") );
	_dates->setFocus();
	return;
  }
  // Check for overlapping dates
  taxSave.prepare("SELECT taxrate_id,taxrate_tax_id,taxrate_effective,taxrate_expires "
            "  FROM taxrate "
            " WHERE (taxrate_id != :taxrate_id) "
			"   AND (taxrate_tax_id = :taxrate_tax_id) "
			"   AND (taxrate_effective < :taxrate_expires) "
			"   AND (taxrate_expires > :taxrate_effective ) ");
  taxSave.bindValue(":taxrate_id", _taxrateid);
  taxSave.bindValue(":taxrate_tax_id", _taxId);
  taxSave.bindValue(":taxrate_effective", _dates->startDate());
  taxSave.bindValue(":taxrate_expires", _dates->endDate());
  taxSave.exec();
  if(taxSave.first())
  {
    QMessageBox::critical(this, tr("Invalid Date Range"),
      tr("A Tax Rate already exists within the specified Date Range.") );
    _dates->setFocus();
    return;
  }
  
  // Save the values in the database
  if (cNew == _mode) 
  {
    taxSave.prepare("INSERT INTO taxrate (taxrate_id, "
	      "    taxrate_tax_id, taxrate_percent, "
	      "    taxrate_curr_id, taxrate_amount, "
		  "    taxrate_effective, taxrate_expires) "
	      "    VALUES (:taxrate_id, :taxrate_tax_id, "
	      "    :taxrate_percent, :taxrate_curr_id, "
	      "    :taxrate_amount, :taxrate_effective, "
		  "    :taxrate_expires);");
  }
  else 
  {
    taxSave.prepare("UPDATE taxrate SET "
	      "    taxrate_tax_id=:taxrate_tax_id, "
	      "    taxrate_percent=:taxrate_percent, "
	      "    taxrate_curr_id=:taxrate_curr_id, "
		  "    taxrate_amount=:taxrate_amount, "
		  "    taxrate_effective=:taxrate_effective, "
		  "	   taxrate_expires=:taxrate_expires "
		  "WHERE (taxrate_id=:taxrate_id);");
  }
  taxSave.bindValue(":taxrate_id", _taxrateid);
  taxSave.bindValue(":taxrate_tax_id", _taxId);

  if(_percent->isValid())
    taxSave.bindValue(":taxrate_percent", (_percent->toDouble() / 100));
  else
    taxSave.bindValue(":taxrate_percent", 0.0);

  taxSave.bindValue(":taxrate_curr_id", _flat->id());
  if(_flat->isEmpty())
    taxSave.bindValue(":taxrate_amount", 0.0);
  else
    taxSave.bindValue(":taxrate_amount", _flat->localValue());
  taxSave.bindValue(":taxrate_effective", _dates->startDate());
  taxSave.bindValue(":taxrate_expires", _dates->endDate()); 

  taxSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Tax Rate Information"),
                                taxSave, __FILE__, __LINE__))
  {
    return;
  }

  done (_taxrateid);
}

void taxCodeRate::sPopulate()
{
  XSqlQuery taxPopulate;
  taxPopulate.prepare("SELECT * FROM taxrate WHERE (taxrate_id=:taxrate_id);");
  taxPopulate.bindValue(":taxrate_id", _taxrateid);
  taxPopulate.exec();
  if (taxPopulate.first())
  { 
    _taxrateid	= taxPopulate.value("taxrate_id").toInt();
	_dates->setStartDate(taxPopulate.value("taxrate_effective").toDate()); 
	_dates->setEndDate(taxPopulate.value("taxrate_expires").toDate());
	_percent->setText(taxPopulate.value("taxrate_percent").toDouble() * 100);
	_flat->setId(taxPopulate.value("taxrate_curr_id").toInt());
	_flat->setLocalValue(taxPopulate.value("taxrate_amount").toDouble());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Tax Rate Information"),
                                taxPopulate, __FILE__, __LINE__))
  {
    return;
  }
}
