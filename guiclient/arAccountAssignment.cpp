/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "arAccountAssignment.h"

#include <QVariant>
#include <QMessageBox>

arAccountAssignment::arAccountAssignment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _araccntid = -1;

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_selectedCustomerType, SIGNAL(toggled(bool)), _customerTypes, SLOT(setEnabled(bool)));
  connect(_customerTypePattern, SIGNAL(toggled(bool)), _customerType, SLOT(setEnabled(bool)));

  _customerTypes->setType(XComboBox::CustomerTypes);

  if(!_metrics->boolean("EnableCustomerDeposits"))
  {
    _deferred->hide();
    _deferredLit->hide();
  }

  _ar->setType(GLCluster::cAsset);
  _prepaid->setType(GLCluster::cRevenue);
  _freight->setType(GLCluster::cRevenue | GLCluster::cLiability);
  _deferred->setType(GLCluster::cLiability);
  _discount->setType(GLCluster::cRevenue | GLCluster::cExpense);
}

arAccountAssignment::~arAccountAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

void arAccountAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arAccountAssignment::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("araccnt_id", &valid);
  if (valid)
  {
    _araccntid = param.toInt();
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

      _customerTypeGroup->setEnabled(false);
      _ar->setReadOnly(true);
      _prepaid->setReadOnly(true);
      _freight->setReadOnly(true);
      _deferred->setReadOnly(true);
      _discount->setReadOnly(true);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void arAccountAssignment::sSave()
{
  XSqlQuery arSave;
  QList<GuiErrorCheck> errors;

  if (_metrics->boolean("InterfaceARToGL"))
  {
    errors << GuiErrorCheck(!_ar->isValid(), _ar,
                            tr("<p>You must select a A/R Account before saving this A/R Account Assignment."))
           << GuiErrorCheck(!_prepaid->isValid(), _prepaid,
                            tr("<p>You must select a Prepaid Receivables Account before saving this A/R Account Assignment."))
           << GuiErrorCheck(!_freight->isValid(), _freight,
                            tr("<p>You must select a Freight Account before saving this A/R Account Assignment."))
           << GuiErrorCheck(_metrics->boolean("EnableCustomerDeposits") && !_deferred->isValid(), _deferred,
                            tr("<p>You must select a Deferred Revenue Account before saving this A/R Account Assignment."))
           << GuiErrorCheck(!_discount->isValid(), _discount,
                            tr("<p>You must select a Discount Account before saving this A/R Account Assignment."))
           ;
  }

  arSave.prepare("SELECT araccnt_id"
            "  FROM araccnt"
            " WHERE((araccnt_custtype_id=:araccnt_custtype_id)"
            "   AND (araccnt_custtype=:araccnt_custtype)"
            "   AND (araccnt_id != :araccnt_id))");
  arSave.bindValue(":araccnt_id", _araccntid);
  if (_selectedCustomerType->isChecked())
  {
    arSave.bindValue(":araccnt_custtype_id", _customerTypes->id());
    arSave.bindValue(":araccnt_custtype", "^[a-zA-Z0-9_]");
  }
  else if (_customerTypePattern->isChecked())
  {
    arSave.bindValue(":araccnt_custtype_id", -1);
    arSave.bindValue(":araccnt_custtype", _customerType->text());
  }
  arSave.exec();
  if(arSave.first())
  {
    errors << GuiErrorCheck(true, _customerTypePattern,
                           tr("<p>You may not save this A/R Account Assignment as it already exists."));
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save A/R Account Assignment"), errors))
    return;

  if (_mode == cNew)
  {
    arSave.exec("SELECT NEXTVAL('araccnt_araccnt_id_seq') AS _araccntid;");
    if (arSave.first())
      _araccntid = arSave.value("_araccntid").toInt();
//  ToDo

    arSave.prepare( "INSERT INTO araccnt "
               "( araccnt_id, araccnt_custtype_id, araccnt_custtype,"
               "  araccnt_ar_accnt_id, araccnt_prepaid_accnt_id, araccnt_freight_accnt_id,"
               "  araccnt_deferred_accnt_id, araccnt_discount_accnt_id ) "
               "VALUES "
               "( :araccnt_id, :araccnt_custtype_id, :araccnt_custtype,"
               "  :araccnt_ar_accnt_id, :araccnt_prepaid_accnt_id, :araccnt_freight_accnt_id,"
               "  :araccnt_deferred_accnt_id, :araccnt_discount_accnt_id );" );
  }
  else if (_mode == cEdit)
    arSave.prepare( "UPDATE araccnt "
               "SET araccnt_custtype_id=:araccnt_custtype_id, araccnt_custtype=:araccnt_custtype,"
               "    araccnt_ar_accnt_id=:araccnt_ar_accnt_id,"
               "    araccnt_prepaid_accnt_id=:araccnt_prepaid_accnt_id,"
               "    araccnt_freight_accnt_id=:araccnt_freight_accnt_id,"
               "    araccnt_deferred_accnt_id=:araccnt_deferred_accnt_id,"
			   "    araccnt_discount_accnt_id=:araccnt_discount_accnt_id "
               "WHERE (araccnt_id=:araccnt_id);" );

  arSave.bindValue(":araccnt_id", _araccntid);

  if (_selectedCustomerType->isChecked())
  {
    arSave.bindValue(":araccnt_custtype_id", _customerTypes->id());
    arSave.bindValue(":araccnt_custtype", "^[a-zA-Z0-9_]");
  }
  else if (_customerTypePattern->isChecked())
  {
    arSave.bindValue(":araccnt_custtype_id", -1);
    arSave.bindValue(":araccnt_custtype", _customerType->text());
  }

  arSave.bindValue(":araccnt_ar_accnt_id", _ar->id());
  arSave.bindValue(":araccnt_prepaid_accnt_id", _prepaid->id());
  arSave.bindValue(":araccnt_freight_accnt_id", _freight->id());
  arSave.bindValue(":araccnt_deferred_accnt_id", _deferred->id());
  arSave.bindValue(":araccnt_discount_accnt_id", _discount->id());

  arSave.exec();

  done(_araccntid);
}

void arAccountAssignment::populate()
{
  XSqlQuery arpopulate;
  arpopulate.prepare( "SELECT araccnt_custtype_id, araccnt_custtype,"
             "       araccnt_ar_accnt_id, araccnt_prepaid_accnt_id,"
             "       araccnt_freight_accnt_id, araccnt_deferred_accnt_id, "
			 "       araccnt_discount_accnt_id "
             "FROM araccnt "
             "WHERE (araccnt_id=:araccnt_id);" );
  arpopulate.bindValue(":araccnt_id", _araccntid);
  arpopulate.exec();
  if (arpopulate.first())
  {
    if (arpopulate.value("araccnt_custtype_id").toInt() == -1)
    {
      _customerTypePattern->setChecked(true);
      _customerType->setText(arpopulate.value("araccnt_custtype").toString());
    }
    else
    {
      _selectedCustomerType->setChecked(true);
      _customerTypes->setId(arpopulate.value("araccnt_custtype_id").toInt());
    }

    _ar->setId(arpopulate.value("araccnt_ar_accnt_id").toInt());
    _prepaid->setId(arpopulate.value("araccnt_prepaid_accnt_id").toInt());
    _freight->setId(arpopulate.value("araccnt_freight_accnt_id").toInt());
    _deferred->setId(arpopulate.value("araccnt_deferred_accnt_id").toInt());
    _discount->setId(arpopulate.value("araccnt_discount_accnt_id").toInt());
  }
}
