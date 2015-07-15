/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxType.h"

#include <QVariant>
#include <QMessageBox>

taxType::taxType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

taxType::~taxType()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("taxtype_id", &valid);
  if (valid)
  {
    _taxtypeid = param.toInt();
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

      _name->setEnabled(false);
      _description->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void taxType::sCheck()
{
  XSqlQuery taxCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    taxCheck.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));" );
    taxCheck.bindValue(":taxtype_name", _name->text());
    taxCheck.exec();
    if (taxCheck.first())
    {
      _taxtypeid = taxCheck.value("taxtype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void taxType::sSave()
{
  XSqlQuery taxSave;
  if (_name->text().trimmed().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Tax Type before saving it."));
    _name->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    taxSave.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE ( (taxtype_id<>:taxtype_id)"
               " AND (UPPER(taxtype_name)=UPPER(:taxtype_name)) );");
    taxSave.bindValue(":taxtype_id", _taxtypeid);
    taxSave.bindValue(":taxtype_name", _name->text().trimmed());
    taxSave.exec();
    if (taxSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists."
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    taxSave.prepare( "UPDATE taxtype "
               "SET taxtype_name=:taxtype_name,"
               "    taxtype_descrip=:taxtype_descrip "
               "WHERE (taxtype_id=:taxtype_id);" );
    taxSave.bindValue(":taxtype_id", _taxtypeid);
    taxSave.bindValue(":taxtype_name", _name->text().trimmed());
    taxSave.bindValue(":taxtype_descrip", _description->text());
    taxSave.exec();
  }
  else if (_mode == cNew)
  {
    taxSave.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));");
    taxSave.bindValue(":taxtype_name", _name->text().trimmed());
    taxSave.exec();
    if (taxSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists.\n"
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    taxSave.exec("SELECT NEXTVAL('taxtype_taxtype_id_seq') AS taxtype_id;");
    if (taxSave.first())
      _taxtypeid = taxSave.value("taxtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    taxSave.prepare( "INSERT INTO taxtype "
               "( taxtype_id, taxtype_name, taxtype_descrip ) "
               "VALUES "
               "( :taxtype_id, :taxtype_name, :taxtype_descrip );" );
    taxSave.bindValue(":taxtype_id", _taxtypeid);
    taxSave.bindValue(":taxtype_name", _name->text().trimmed());
    taxSave.bindValue(":taxtype_descrip", _description->text());
    taxSave.exec();
  }

  done(_taxtypeid);
}

void taxType::populate()
{
  XSqlQuery taxpopulate;
  taxpopulate.prepare( "SELECT taxtype_name, taxtype_descrip, taxtype_sys "
             "FROM taxtype "
             "WHERE (taxtype_id=:taxtype_id);" );
  taxpopulate.bindValue(":taxtype_id", _taxtypeid);
  taxpopulate.exec();
  if (taxpopulate.first())
  {
    _name->setText(taxpopulate.value("taxtype_name").toString());
    if(taxpopulate.value("taxtype_sys").toBool())
      _name->setEnabled(false);
    _description->setText(taxpopulate.value("taxtype_descrip").toString());
  }
}

