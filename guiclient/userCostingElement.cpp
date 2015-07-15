/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "userCostingElement.h"

#include <QVariant>
#include <QMessageBox>

userCostingElement::userCostingElement(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_acceptPO, SIGNAL(toggled(bool)), _useCostItem, SLOT(setDisabled(bool)));
  connect(_acceptPO, SIGNAL(toggled(bool)), _expense, SLOT(setDisabled(bool)));

  _item->setType(ItemLineEdit::cCosting);
  _expense->setType(GLCluster::cExpense);
}

userCostingElement::~userCostingElement()
{
  // no need to delete child widgets, Qt does it all for us
}

void userCostingElement::languageChange()
{
  retranslateUi(this);
}

enum SetResponse userCostingElement::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("costelem_id", &valid);
  if (valid)
  {
    _costelemid = param.toInt();
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
      _active->setEnabled(false);
      _acceptPO->setEnabled(false);
      _expense->setEnabled((false));
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void userCostingElement::sSave()
{
  XSqlQuery userSave;
  if (_name->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                           tr( "You must enter a Name for this Costing Element." ) );
    _name->setFocus();
    return;
  }
    
  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Costing Element"),
                            tr("This Costing Element already exists.  You have been placed in edit mode.") );
      _name->setFocus();
      return;
    }

    userSave.exec("SELECT NEXTVAL('costelem_costelem_id_seq') AS _costelem_id");
    if (userSave.first())
      _costelemid = userSave.value("_costelem_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    userSave.prepare( "INSERT INTO costelem "
               "( costelem_id, costelem_type, costelem_active,"
               "  costelem_sys, costelem_po, costelem_cost_item_id, costelem_exp_accnt_id ) "
               "VALUES "
               "( :costelem_id, :costelem_type, :costelem_active,"
               "  false, :costelem_po, :costelem_cost_item_id, :costelem_exp_accnt_id );" );
  }
  else if (_mode == cEdit)
  {
    userSave.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE ( (costelem_id <> :costelem_id)"
               "AND (costelem_type=:costelem_type) );" );
    userSave.bindValue(":costelem_id", _costelemid);
    userSave.bindValue(":costelem_type", _name->text().trimmed());
    userSave.exec();
    if (userSave.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "A Costing Elements with the entered code already exists.\n"
                                 "You may not create a Costing Element with this code." ) );
      _name->setFocus();
      return;
    }

    userSave.prepare( "UPDATE costelem "
               "SET costelem_type=:costelem_type,"
               "    costelem_active=:costelem_active, costelem_po=:costelem_po,"
               "    costelem_cost_item_id=:costelem_cost_item_id,"
               "    costelem_exp_accnt_id=:costelem_exp_accnt_id "
               "WHERE (costelem_id=:costelem_id);" );
  }

  userSave.bindValue(":costelem_id", _costelemid);
  userSave.bindValue(":costelem_type", _name->text().trimmed());
  userSave.bindValue(":costelem_active", QVariant(_active->isChecked()));
  userSave.bindValue(":costelem_po", QVariant(_acceptPO->isChecked()));
  if (_expense->isEnabled() && _expense->isValid())
    userSave.bindValue(":costelem_exp_accnt_id",_expense->id());

  if (_useCostItem->isChecked())
    userSave.bindValue(":costelem_cost_item_id", _item->id());
  else
    userSave.bindValue(":costelem_cost_item_id", -1);

  userSave.exec();

  done(_costelemid);
}

void userCostingElement::populate()
{
  XSqlQuery userpopulate;
  userpopulate.prepare( "SELECT costelem_type, costelem_active,"
             "       costelem_po, costelem_cost_item_id, costelem_exp_accnt_id "
             "FROM costelem "
             "WHERE (costelem_id=:costelem_id);" );
  userpopulate.bindValue(":costelem_id", _costelemid);
  userpopulate.exec();
  if (userpopulate.first())
  {
    _name->setText(userpopulate.value("costelem_type").toString());
    _expense->setId(userpopulate.value("costelem_exp_accnt_id").toInt());
    _active->setChecked(userpopulate.value("costelem_active").toBool());

    if (userpopulate.value("costelem_po").toBool())
      _acceptPO->setChecked(true);
    else
    {
      if (userpopulate.value("costelem_cost_item_id").toInt() != -1)
      {
        _useCostItem->setChecked(true);
        _item->setId(userpopulate.value("costelem_cost_item_id").toInt());
      }
    }
  }
}

bool userCostingElement::sCheck()
{
  XSqlQuery userCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    userCheck.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE (UPPER(costelem_type)= UPPER(:costelem_type));" );
    userCheck.bindValue(":costelem_type", _name->text());
    userCheck.exec();
    if (userCheck.first())
    {
      _costelemid = userCheck.value("costelem_id").toInt();
      _mode = cEdit;
      populate();
      return true;
    }
  }
  return false;
}

