/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotSerialRegistration.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

#include "storedProcErrorLookup.h"

lotSerialRegistration::lotSerialRegistration(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _lsregid = -1;

  connect(_buttonBox, SIGNAL(accepted()),            this, SLOT(sSave()));
  connect(_soldDate,  SIGNAL(newDate(const QDate&)), this, SLOT(sDateUpdated()));
  connect(_crmacct,   SIGNAL(newId(int)),            this, SLOT(sSetSoCustId()));
  connect(_so,        SIGNAL(newId(int,QString)),    this, SLOT(sSetSoId()));
  connect(_lotSerial, SIGNAL(newId(int)),        _charass, SLOT(setId(int)));
  
  _charass->setType("LS"); // use LSR??? for now read from LS and disallow edits
  _charass->setReadOnly(true);
 
  _lotSerial->setStrict(true);
  _shipment->setStrict(true);

  _so->setAllowedTypes(OrderLineEdit::Sales);

  _qty->setValidator(omfgThis->qtyVal());
  
  adjustSize();
}

lotSerialRegistration::~lotSerialRegistration()
{
  // no need to delete child widgets, Qt does it all for us
}

void lotSerialRegistration::languageChange()
{
  retranslateUi(this);
}

enum SetResponse lotSerialRegistration::set(const ParameterList &pParams)
{
  XSqlQuery lotet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacct->setId(param.toInt());
  
  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());
  
  param = pParams.value("ls_id", &valid);
  if (valid)
  {
    _lotSerial->setId(param.toInt());
    _charass->setId(_lotSerial->id());
  }
  
  param = pParams.value("lsreg_id", &valid);
  if (valid)
  {
    _lsregid=param.toInt();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      _mode = cNew;
      lotet.exec ("SELECT fetchlsregnumber() AS number;");
      if (lotet.first())
	_regNumber->setText(lotet.value("number").toString());
      else if(lotet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, lotet.lastError().databaseText(), __FILE__, __LINE__);
        reject();
      }
      lotet.exec("SELECT NEXTVAL('lsreg_lsreg_id_seq') AS _lsreg_id;");
      if (lotet.first())
      {
        _lsregid = lotet.value("_lsreg_id").toInt();
      }
      else if (lotet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, lotet.lastError().databaseText(), __FILE__, __LINE__);
        reject();
      }
      _qty->setText("1");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      populate();
      _itemGroup->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _regDate->setEnabled(false);
      _soldDate->setEnabled(false);
      _expireDate->setEnabled(false);
      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _type->setEnabled(false);
      _item->setReadOnly(true);
      _lotSerial->setEnabled(false);
      _charass->setReadOnly(true);
      _notes->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }
  return NoError;
}

void lotSerialRegistration::closeEvent(QCloseEvent *pEvent)
{
  XSqlQuery lotcloseEvent;
  if (_mode == cNew)
  {
    lotcloseEvent.prepare("SELECT releaseLsRegNumber(:regNumber);" );
    lotcloseEvent.bindValue(":regNumber", _regNumber->text());
    lotcloseEvent.exec();
    if (lotcloseEvent.lastError().type() != QSqlError::NoError)
      systemError(this, lotcloseEvent.lastError().databaseText(), __FILE__, __LINE__);
  }

  pEvent->accept();
}

void lotSerialRegistration::populate()
{
  XSqlQuery lotpopulate;
  lotpopulate.prepare("SELECT *"
            "  FROM lsreg LEFT OUTER JOIN ls ON (lsreg_ls_id=ls_id)"
            " WHERE(lsreg_id=:lsreg_id);");
  lotpopulate.bindValue(":lsreg_id", _lsregid);
  lotpopulate.exec();
  if(lotpopulate.first())
  {
    _lsregid = lotpopulate.value("lsreg_id").toInt();
    _regNumber->setText(lotpopulate.value("lsreg_number").toString());
    _type->setId(lotpopulate.value("lsreg_regtype_id").toInt());
    _qty->setDouble(lotpopulate.value("lsreg_qty").toDouble());
    _item->setId(lotpopulate.value("ls_item_id").toInt());
    _lotSerial->setId(lotpopulate.value("lsreg_ls_id").toInt());
    _regDate->setDate(lotpopulate.value("lsreg_regdate").toDate());
    _soldDate->setDate(lotpopulate.value("lsreg_solddate").toDate());
    _expireDate->setDate(lotpopulate.value("lsreg_expiredate").toDate());
    _crmacct->setId(lotpopulate.value("lsreg_crmacct_id").toInt());
    _cntct->setId(lotpopulate.value("lsreg_cntct_id").toInt());
    _notes->setText(lotpopulate.value("lsreg_notes").toString());
    if(!lotpopulate.value("lsreg_cohead_id").isNull())
      _so->setId(lotpopulate.value("lsreg_cohead_id").toInt());
    if(!lotpopulate.value("lsreg_shiphead_id").isNull())
      _shipment->setId(lotpopulate.value("lsreg_shiphead_id").toInt());
    _charass->setId(lotpopulate.value("lsreg_ls_id").toInt());
  }
  else if(lotpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, lotpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void lotSerialRegistration::sSave()
{
  XSqlQuery lotSave;
  if(cView == _mode)
    return;

  if (!_crmacct->isValid())
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a Account."));
    _crmacct->setFocus();
    return;
  }
  
  if (!_regDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a registration date."));
    _regDate->setFocus();
    return;
  }
  
  if (!_soldDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a sold date."));
    _soldDate->setFocus();
    return;
  }

  if (!_expireDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a expiration date."));
    _expireDate->setFocus();
    return;
  }
  
  if (_lotSerial->id() == -1)
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a lot/serial number."));
    _lotSerial->setFocus();
    return;
  }
  
  if (!(_qty->toDouble() > 0))
  {
    QMessageBox::warning(this, windowTitle(), tr("You must provide a quantity greater than zero."));
    _qty->setFocus();
    return;
  }

  if (_cntct->id() == -1)
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a contact.");
    _cntct->setFocus();
    return;
  }

  if(cNew == _mode)
  {
    lotSave.prepare("INSERT INTO lsreg"
              "      (lsreg_id, lsreg_number, lsreg_regtype_id,"
              "       lsreg_ls_id, lsreg_qty, lsreg_regdate, lsreg_solddate,"
              "       lsreg_expiredate, lsreg_crmacct_id, lsreg_cntct_id,"
              "       lsreg_notes, lsreg_cohead_id, lsreg_shiphead_id) "
              "VALUES(:lsreg_id, :lsreg_number, :lsreg_regtype_id,"
              "       :lsreg_ls_id, :lsreg_qty, :lsreg_regdate, :lsreg_solddate,"
              "       :lsreg_expiredate, :lsreg_crmacct_id, :lsreg_cntct_id,"
              "       :lsreg_notes, :lsreg_cohead_id, :lsreg_shiphead_id);");
  }
  else if(cEdit == _mode)
    lotSave.prepare("UPDATE lsreg"
              "   SET lsreg_number=:lsreg_number,"
              "       lsreg_regtype_id=:lsreg_regtype_id,"
              "       lsreg_ls_id=:lsreg_ls_id,"
              "       lsreg_qty=:lsreg_qty,"
              "       lsreg_regdate=:lsreg_regdate,"
              "       lsreg_solddate=:lsreg_solddate,"
              "       lsreg_expiredate=:lsreg_expiredate,"
              "       lsreg_crmacct_id=:lsreg_crmacct_id,"
              "       lsreg_cntct_id=:lsreg_cntct_id,"
              "       lsreg_notes=:lsreg_notes,"
              "       lsreg_cohead_id=:lsreg_cohead_id,"
              "       lsreg_shiphead_id=:lsreg_shiphead_id"
              " WHERE(lsreg_id=:lsreg_id);");
  
  lotSave.bindValue(":lsreg_id", _lsregid);
  lotSave.bindValue(":lsreg_number", _regNumber->text().trimmed());
  lotSave.bindValue(":lsreg_regtype_id", _type->id());
  lotSave.bindValue(":lsreg_ls_id", _lotSerial->id());
  lotSave.bindValue(":lsreg_qty", _qty->toDouble());
  lotSave.bindValue(":lsreg_regdate", _regDate->date());
  lotSave.bindValue(":lsreg_solddate", _soldDate->date());
  lotSave.bindValue(":lsreg_expiredate", _expireDate->date());
  lotSave.bindValue(":lsreg_crmacct_id", _crmacct->id());
  lotSave.bindValue(":lsreg_cntct_id", _cntct->id());
  lotSave.bindValue(":lsreg_notes",    _notes->toPlainText());
  if(_so->id() != -1)
    lotSave.bindValue(":lsreg_cohead_id", _so->id());
  if(_shipment->id() != -1)
    lotSave.bindValue(":lsreg_shiphead_id", _shipment->id());

  lotSave.exec();
  if (lotSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, lotSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _mode = 0;
  close();
}

void lotSerialRegistration::sDateUpdated()
{
  if(cView == _mode)
    return;

  QDate date = _soldDate->date();
  if(date.isNull())
    return;

  XSqlQuery dq;
  dq.prepare("SELECT item_warrdays"
             "  FROM item"
             " WHERE(item_id=:item_id);");
  dq.bindValue(":item_id", _item->id());
  dq.exec();
  if(dq.first())
  {
    date.addDays(dq.value("item_warrdays").toInt());
    _expireDate->setDate(date);
  }
}

void lotSerialRegistration::sSetSoCustId()
{
  if (_crmacct->id() != -1)
  {
    XSqlQuery cq;
    cq.prepare("SELECT crmacct_cust_id "
               "FROM crmacct "
	       "WHERE (crmacct_id=:crmacct_id);");
    cq.bindValue(":crmacct_id", _crmacct->id());
    cq.exec();
    if (cq.first())
    {
      _so->setCustId(cq.value("crmacct_cust_id").toInt());
      _shipment->setId(-1);
    }
    else if(cq.lastError().type() != QSqlError::NoError)
    { 
      systemError(this, cq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }     
  }
  else
  {
    _shipment->setId(-1);
    _so->setCustId(-1);
  }
}

void lotSerialRegistration::sSetSoId()
{
  if (_so->id() != -1)
    _shipment->limitToOrder(_so->id());
  else
    _shipment->removeOrderLimit();
}
