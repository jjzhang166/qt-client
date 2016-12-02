/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shipTo.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "addresscluster.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

shipTo::shipTo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_shipToNumber, SIGNAL(editingFinished()), this, SLOT(sPopulateNumber()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_salesRep, SIGNAL(newID(int)), this, SLOT(sPopulateCommission(int)));
  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _shipZone->populate( "SELECT shipzone_id, shipzone_name "
                       "FROM shipzone "
                       "ORDER BY shipzone_name;" );

  _commission->setValidator(omfgThis->percentVal());

  _shiptoid = -1;
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _sellingWarehouseLit->hide();
    _sellingWarehouse->hide();
  }
  else
    _sellingWarehouse->setId(_preferences->value("PreferredWarehouse").toInt());
}

shipTo::~shipTo()
{
    // no need to delete child widgets, Qt does it all for us
}

void shipTo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse shipTo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("shipto_id", &valid);
  if (valid)
  {
    _shiptoid = param.toInt();
    _documents->setId(_shiptoid);
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _salesRep->setType(XComboBox::SalesRepsActive);

      XSqlQuery cust;
      cust.prepare( "SELECT cust_number, cust_name, cust_taxzone_id, "
                 "       cust_salesrep_id, cust_shipform_id, cust_shipvia, cust_shipchrg_id, "
                 "       crmacct_id "
                 "FROM custinfo "
                 "  JOIN crmacct ON (cust_id=crmacct_cust_id) "
                 "WHERE (cust_id=:cust_id);" );
      cust.bindValue(":cust_id", _custid);
      cust.exec();
      if (cust.first())
      {
        _custNum->setText(cust.value("cust_number").toString());
        _custName->setText(cust.value("cust_name").toString());
        _salesRep->setId(cust.value("cust_salesrep_id").toInt());
        _shipform->setId(cust.value("cust_shipform_id").toInt());
        _taxzone->setId(cust.value("cust_taxzone_id").toInt());
        _contact->setSearchAcct(cust.value("crmacct_id").toInt());
        _address->setSearchAcct(cust.value("crmacct_id").toInt());
        _shipchrg->setId(cust.value("cust_shipchrg_id").toInt());

	//  Handle the free-form Ship Via
        _shipVia->setId(-1);
        QString shipvia = cust.value("cust_shipvia").toString().trimmed();
        if (shipvia.length())
        {
          for (int counter = 0; counter < _shipVia->count(); counter++)
            if (_shipVia->itemText(counter) == shipvia)
              _shipVia->setCurrentIndex(counter);

          if (_shipVia->id() == -1)
          {
            _shipVia->addItem(shipvia);
            _shipVia->setCurrentIndex(_shipVia->count() - 1);
          }
        }
      }
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                    cust, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
      sPopulateNumber();
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _shipToNumber->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _shipToNumber->setEnabled(false);
      _active->setEnabled(false);
      _default->setEnabled(false);
      _name->setEnabled(false);
      _contact->setEnabled(false);
      _address->setEnabled(false);
      _salesRep->setEnabled(false);
      _commission->setEnabled(false);
      _shipZone->setEnabled(false);
      _taxzone->setEnabled(false);
      _shipVia->setEnabled(false);
      _shipform->setEnabled(false);
      _shipchrg->setEnabled(false);
      _sellingWarehouse->setEnabled(false);
      _comments->setEnabled(false);
      _shippingComments->setEnabled(false);
      _documents->setReadOnly(true);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

int shipTo::id() const
{
  return _shiptoid;
}

int shipTo::mode() const
{
  return _mode;
}

void shipTo::sSave()
{
  XSqlQuery shipSave;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_name->text().length() == 0, _name,
                          tr("You must enter a valid Name."))
  ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Ship To"), errors))
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! shipSave.exec("BEGIN"))
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Ship To Information"),
                         shipSave, __FILE__, __LINE__);
    return;
  }

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple uses of this Ship-To "
		       "Address. What would you like to do?"),
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
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Ship To Information (%1). ")
                         .arg(saveResult),shipSave, __FILE__, __LINE__);
    rollback.exec();
    _address->setFocus();
    return;
  }

  XSqlQuery saveq;
  saveq.prepare( "UPDATE shiptoinfo "
                 "SET shipto_active=:shipto_active, shipto_default=:shipto_default,"
                 "    shipto_num=:shipto_num, shipto_name=:shipto_name,"
                 "    shipto_cntct_id=:shipto_cntct_id, shipto_commission=:shipto_commission,"
                 "    shipto_comments=:shipto_comments, shipto_shipcomments=:shipto_shipcomments,"
                 "    shipto_taxzone_id=:shipto_taxzone_id, shipto_salesrep_id=:shipto_salesrep_id,"
                 "    shipto_shipzone_id=:shipto_shipzone_id,"
                 "    shipto_shipvia=:shipto_shipvia, shipto_shipform_id=:shipto_shipform_id,"
                 "    shipto_shipchrg_id=:shipto_shipchrg_id,"
                 "    shipto_preferred_warehous_id=:shipto_preferred_warehous_id,"
                 "    shipto_addr_id=:shipto_addr_id "
                 "WHERE (shipto_id=:shipto_id);" );

  saveq.bindValue(":shipto_id", _shiptoid);
  saveq.bindValue(":shipto_active", QVariant(_active->isChecked()));
  saveq.bindValue(":shipto_default", QVariant(_default->isChecked()));
  saveq.bindValue(":shipto_cust_id", _custid);
  saveq.bindValue(":shipto_num", _shipToNumber->text().trimmed());
  saveq.bindValue(":shipto_name", _name->text());
  if (_contact->id() > 0)
    saveq.bindValue(":shipto_cntct_id", _contact->id());
  if (_address->id() > 0)
    saveq.bindValue(":shipto_addr_id", _address->id());
  saveq.bindValue(":shipto_commission", (_commission->toDouble() / 100));
  saveq.bindValue(":shipto_comments", _comments->toPlainText());
  saveq.bindValue(":shipto_shipcomments", _shippingComments->toPlainText());
  saveq.bindValue(":shipto_shipvia", _shipVia->currentText());
  if (_taxzone->isValid())
    saveq.bindValue(":shipto_taxzone_id",  _taxzone->id());
  if (_salesRep->id() != -1)
    saveq.bindValue(":shipto_salesrep_id", _salesRep->id());
  if (_shipZone->isValid())
    saveq.bindValue(":shipto_shipzone_id", _shipZone->id());
  if (_shipform->id() != -1)
    saveq.bindValue(":shipto_shipform_id", _shipform->id());
  if (_shipchrg->id() != -1)
    saveq.bindValue(":shipto_shipchrg_id", _shipchrg->id());
  saveq.bindValue(":shipto_preferred_warehous_id", _sellingWarehouse->id());
  saveq.exec();
  if (saveq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Ship To Information"),
                         saveq, __FILE__, __LINE__);
    return;
  }

  shipSave.exec("COMMIT;");

  if (_mode == cNew)
    emit newId(_shiptoid);
  _mode = 0;

  done(_shiptoid);
}

void shipTo::populate()
{
  XSqlQuery popq;
  popq.prepare( "SELECT cust_number, cust_name, shipto_active, shipto_default,"
                "       shipto_cust_id,"
                "       shipto_num, shipto_name, shipto_cntct_id,"
                "       shipto_shipvia, shipto_commission,"
                "       shipto_comments, shipto_shipcomments,"
                "       shipto_taxzone_id, shipto_shipchrg_id,"
                "       COALESCE(shipto_salesrep_id,-1) AS shipto_salesrep_id,"
                "       COALESCE(shipto_shipzone_id,-1) AS shipto_shipzone_id,"
                "       COALESCE(shipto_shipform_id,-1) AS shipto_shipform_id,"
                "       shipto_preferred_warehous_id, shipto_addr_id,"
                "       crmacct_id "
                "FROM shiptoinfo "
                "  LEFT OUTER JOIN custinfo ON (shipto_cust_id=cust_id) "
                "  LEFT OUTER JOIN crmacct ON (cust_id=crmacct_cust_id) "
                "WHERE (shipto_id=:shipto_id);" );
  popq.bindValue(":shipto_id", _shiptoid);
  popq.exec();
  if (popq.first())
  {
    double commission = popq.value("shipto_commission").toDouble();
    _custid = popq.value("shipto_cust_id").toInt();
    _custNum->setText(popq.value("cust_number").toString());
    _custName->setText(popq.value("cust_name").toString());
    _active->setChecked(popq.value("shipto_active").toBool());
    _default->setChecked(popq.value("shipto_default").toBool());
    _shipToNumber->setText(popq.value("shipto_num"));
    _name->setText(popq.value("shipto_name"));
    _contact->setId(popq.value("shipto_cntct_id").toInt());
    _contact->setSearchAcct(popq.value("crmacct_id").toInt());
    _address->setSearchAcct(popq.value("crmacct_id").toInt());
    _comments->setText(popq.value("shipto_comments").toString());
    _shippingComments->setText(popq.value("shipto_shipcomments").toString());
    _taxzone->setId(popq.value("shipto_taxzone_id").toInt());
    _shipZone->setId(popq.value("shipto_shipzone_id").toInt());
    _shipform->setId(popq.value("shipto_shipform_id").toInt());
    _shipchrg->setId(popq.value("shipto_shipchrg_id").toInt());
    _sellingWarehouse->setId(popq.value("shipto_preferred_warehous_id").toInt());
    _address->setId(popq.value("shipto_addr_id").toInt());

    //  Handle the free-form Ship Via
    _shipVia->setCurrentIndex(-1);
    QString shipvia = popq.value("shipto_shipvia").toString();
    if (shipvia.trimmed().length() != 0)
    {
      for (int counter = 0; counter < _shipVia->count(); counter++)
        if (_shipVia->itemText(counter) == shipvia)
          _shipVia->setCurrentIndex(counter);

      if (_shipVia->id() == -1)
      {
        _shipVia->addItem(shipvia);
        _shipVia->setCurrentIndex(_shipVia->count() - 1);
      }
    }

    _salesRep->setId(popq.value("shipto_salesrep_id").toInt());
    _commission->setDouble(commission * 100);

    emit newId(_shiptoid);
    emit populated();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Ship To Information"),
                                popq, __FILE__, __LINE__))
  {
    return;
  }
}

void shipTo::sPopulateNumber()
{
  if (_shipToNumber->text().trimmed().length() == 0)
  {
    XSqlQuery nextnumq;
    nextnumq.prepare( "SELECT (COALESCE(MAX(CAST(shipto_num AS INTEGER)), 0) + 1) AS n_shipto_num "
                      "  FROM shiptoinfo "
                      " WHERE ((shipto_cust_id=:cust_id)"
                      "   AND  (shipto_num~'^[0-9]*$') )" );
    nextnumq.bindValue(":cust_id", _custid);
    nextnumq.exec();
    if (nextnumq.first())
      _shipToNumber->setText(nextnumq.value("n_shipto_num"));
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Ship To Information"),
                                  nextnumq, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    XSqlQuery dupnumq;
    dupnumq.prepare( "SELECT shipto_id "
                     "FROM shiptoinfo "
                     "WHERE ( (shipto_cust_id=:cust_id)"
                     " AND (UPPER(shipto_num)=UPPER(:shipto_num))"
                     " AND (shipto_id != :shipto_id));" );
    dupnumq.bindValue(":cust_id", _custid);
    dupnumq.bindValue(":shipto_num", _shipToNumber->text().trimmed());
    dupnumq.bindValue(":shipto_id", _shiptoid);
    dupnumq.exec();
    if (dupnumq.first())
    {
      if (_mode == cNew && _shiptoid != -1)
      {
        XSqlQuery delnumq;
        delnumq.prepare( "DELETE FROM shiptoinfo "
                         "WHERE (shipto_id=:shipto_id);" );
        delnumq.bindValue(":shipto_id", _shiptoid);
        delnumq.exec();
      }
      _mode = cEdit;
      _shiptoid = dupnumq.value("shipto_id").toInt();
      populate();

      _shipToNumber->setEnabled(false);
      _name->setFocus();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Ship To Information"),
                                  dupnumq, __FILE__, __LINE__))
    {
      return;
    }
  }

  if (_mode == cNew && _shiptoid == -1)
  {
    XSqlQuery newnumq;
    newnumq.prepare( "INSERT INTO shiptoinfo "
                     "( shipto_cust_id, shipto_active, shipto_num, shipto_commission ) "
                     "VALUES "
                     "( :shipto_cust_id, :shipto_active, :shipto_num, :shipto_commission ) "
                     "RETURNING shipto_id;");

    newnumq.bindValue(":shipto_active", QVariant(_active->isChecked()));
    newnumq.bindValue(":shipto_cust_id", _custid);
    newnumq.bindValue(":shipto_num", _shipToNumber->text().trimmed());
    newnumq.bindValue(":shipto_commission", (_commission->toDouble() / 100));
    newnumq.exec();
    if (newnumq.first())
      _shiptoid = newnumq.value("shipto_id").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Ship To Information"),
                                  newnumq, __FILE__, __LINE__))
    {
      return;
    }
  }
  _documents->setId(_shiptoid);
  _save->setEnabled(true);
}

void shipTo::sPopulateCommission(int pSalesrepid)
{
  if (_mode != cView)
  {
    XSqlQuery commq;
    commq.prepare( "SELECT salesrep_commission "
                   "FROM salesrep "
                   "WHERE (salesrep_id=:salesrep_id);" );
    commq.bindValue(":salesrep_id", pSalesrepid);
    commq.exec();
    if (commq.first())
      _commission->setDouble(commq.value("salesrep_commission").toDouble() * 100);
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Sales Rep Information"),
                                  commq, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void shipTo::closeEvent(QCloseEvent * /*pEvent*/)
{
  if (DEBUG)
    qDebug("%s::closeEvent() _mode = %d", qPrintable(objectName()), _mode);
  if (_mode == cNew)
  {
    XSqlQuery delnumq;
    delnumq.prepare( "DELETE FROM shiptoinfo "
                     "WHERE (shipto_id=:shipto_id);" );
    delnumq.bindValue(":shipto_id", _shiptoid);
    delnumq.exec();
  }
}

