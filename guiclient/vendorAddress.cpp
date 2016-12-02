/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorAddress.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <vendorcluster.h>
#include "errorReporter.h"

vendorAddress::vendorAddress(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_number, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

vendorAddress::~vendorAddress()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorAddress::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorAddress::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  XSqlQuery setVendor;
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendid = param.toInt();

  param = pParams.value("vendaddr_id", &valid);
  if (valid)
  {
    _vendaddrid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      setVendor.prepare("SELECT crmacct_id "
                "FROM vendinfo "
                "  JOIN crmacct ON (crmacct_vend_id=vend_id) "
                "WHERE (vend_id=:vend_id);");
      setVendor.bindValue(":vend_id", _vendid);
      setVendor.exec();
      if(setVendor.first())
      {
        _address->setSearchAcct(setVendor.value("crmacct_id").toInt());
        _contact->setSearchAcct(setVendor.value("crmacct_id").toInt());
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor Information"),
                                    setVendor, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(false);
      _name->setEnabled(false);
      _contact->setEnabled(false);
      _address->setEnabled(false);
      _notes->setReadOnly(true);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void vendorAddress::sSave()
{
  XSqlQuery vendorSave;
  if (! vendorSave.exec("BEGIN"))
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Vendor Information"),
                         vendorSave, __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple uses of this Vendor "
		       "Address.\nWhat would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: There was an error saving this address (%2).\n"
                            "Check the database server log for errors.")
                         .arg(windowTitle())
                         .arg(saveResult),__FILE__,__LINE__);
    _address->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    vendorSave.exec("SELECT NEXTVAL('vendaddr_vendaddr_id_seq') AS vendaddr_id;");
    if (vendorSave.first())
      _vendaddrid = vendorSave.value("vendaddr_id").toInt();
    else
    {
      rollback.exec();
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Vendor Information"),
                           vendorSave, __FILE__, __LINE__);
      return;
    }

    vendorSave.prepare( "INSERT INTO vendaddrinfo "
               "( vendaddr_id, vendaddr_vend_id,"
               "  vendaddr_code, vendaddr_name,"
               "  vendaddr_cntct_id, vendaddr_comments, vendaddr_addr_id ) "
               "VALUES "
               "( :vendaddr_id, :vendaddr_vend_id,"
               "  :vendaddr_code, :vendaddr_name,"
               "  :vendaddr_cntct_id, :vendaddr_comments, :vendaddr_addr_id );");
  }
  else if (_mode == cEdit)
    vendorSave.prepare( "UPDATE vendaddrinfo "
               "SET vendaddr_code=:vendaddr_code, vendaddr_name=:vendaddr_name,"
               "    vendaddr_cntct_id=:vendaddr_cntct_id,"
	       "    vendaddr_comments=:vendaddr_comments, "
               "    vendaddr_addr_id=:vendaddr_addr_id "
               "WHERE (vendaddr_id=:vendaddr_id);" );

  vendorSave.bindValue(":vendaddr_id", _vendaddrid);
  vendorSave.bindValue(":vendaddr_vend_id", _vendid);
  vendorSave.bindValue(":vendaddr_code", _number->text().trimmed());
  vendorSave.bindValue(":vendaddr_name", _name->text().trimmed());
  if (_contact->id() > 0)
    vendorSave.bindValue(":vendaddr_cntct_id", _contact->id());
  if (_address->id() > 0)
    vendorSave.bindValue(":vendaddr_addr_id", _address->id());
  vendorSave.bindValue(":vendaddr_comments", _notes->toPlainText().trimmed());
  vendorSave.exec();
  if (vendorSave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Vendor Information"),
                         vendorSave, __FILE__, __LINE__);
    return;
  }

  vendorSave.exec("COMMIT;");

  done(_vendaddrid);
}

void vendorAddress::sCheck()
{
  XSqlQuery vendorCheck;
  if ((_mode == cNew) && (_number->text().length()))
  {
    vendorCheck.prepare( "SELECT vendaddr_id "
               "FROM vendaddrinfo "
               "WHERE ( (vendaddr_vend_id=:vend_id)"
               " AND (UPPER(vendaddr_code)=UPPER(:vendaddr_code)) );" );
    vendorCheck.bindValue(":vend_id", _vendid);
    vendorCheck.bindValue(":vendaddr_code", _number->text().trimmed());
    vendorCheck.exec();
    if (vendorCheck.first())
    {
      _vendaddrid = vendorCheck.value("vendaddr_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(false);
    }
  }
}

void vendorAddress::populate()
{
  XSqlQuery vendorpopulate;
  vendorpopulate.prepare( "SELECT vendaddrinfo.*, crmacct_id "
             "FROM vendaddrinfo "
             " JOIN crmacct ON (vendaddr_vend_id=crmacct_vend_id) "
             "WHERE (vendaddr_id=:vendaddr_id);" );
  vendorpopulate.bindValue(":vendaddr_id", _vendaddrid);
  vendorpopulate.exec();
  if (vendorpopulate.first())
  {
    _vendid = vendorpopulate.value("vendaddr_vend_id").toInt();
    _number->setText(vendorpopulate.value("vendaddr_code"));
    _name->setText(vendorpopulate.value("vendaddr_name"));
    _contact->setId(vendorpopulate.value("vendaddr_cntct_id").toInt());
    _address->setId(vendorpopulate.value("vendaddr_addr_id").toInt());
    _notes->setText(vendorpopulate.value("vendaddr_comments").toString());
    _address->setSearchAcct(vendorpopulate.value("crmacct_id").toInt());
    _contact->setSearchAcct(vendorpopulate.value("crmacct_id").toInt());
  }
}
