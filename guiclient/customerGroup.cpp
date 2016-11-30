/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerGroup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "customerGroup.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

customerGroup::customerGroup(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _custgrpitem->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "cust_number");
  _custgrpitem->addColumn(tr("Name"),   -1,          Qt::AlignLeft, true, "cust_name");
}

customerGroup::~customerGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse customerGroup::set(const ParameterList &pParams)
{
  XSqlQuery customeret;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custgrp_id", &valid);
  if (valid)
  {
    _custgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      customeret.exec("SELECT NEXTVAL('custgrp_custgrp_id_seq') AS _custgrp_id;");
      if (customeret.first())
        _custgrpid = customeret.value("_custgrp_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Group Information"),
                                    customeret, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      connect(_custgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      sFillList();

      connect(_custgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _new->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      sFillList();
    }
  }

  return NoError;
}

void customerGroup::sCheck()
{
  XSqlQuery customerCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    customerCheck.prepare( "SELECT custgrp_id "
               "FROM custgrp "
               "WHERE (UPPER(custgrp_name)=UPPER(:custgrp_name));" );
    customerCheck.bindValue(":custgrp_name", _name->text());
    customerCheck.exec();
    if (customerCheck.first())
    {
      _custgrpid = customerCheck.value("custgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Group Information"),
                                  customerCheck, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void customerGroup::sClose()
{
  XSqlQuery customerClose;
  if (_mode == cNew)
  {
    customerClose.prepare( "DELETE FROM custgrpitem "
               "WHERE (custgrpitem_custgrp_id=:custgrp_id);"
               "DELETE FROM custgrp "
               "WHERE (custgrp_id=:custgrp_id);" );
    customerClose.bindValue(":custgrp_id", _custgrpid);
    customerClose.exec();
    if (customerClose.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Rejecting Customer Group"),
                           customerClose, __FILE__, __LINE__);
    }
  }

  reject();
}

void customerGroup::sSave()
{
  XSqlQuery customerSave;

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                        tr("You cannot have an empty name."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Customer Group"),errors))
      return;

  customerSave.prepare("SELECT custgrp_id"
            "  FROM custgrp"
            " WHERE((custgrp_name=:custgrp_name)"
            "   AND (custgrp_id != :custgrp_id))");
  customerSave.bindValue(":custgrp_id", _custgrpid);
  customerSave.bindValue(":custgrp_name", _name->text());
  customerSave.exec();
  if(customerSave.first())
  {
    QMessageBox::warning(this, tr("Cannot Save Customer Group"),
      tr("You cannot have a duplicate name."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    customerSave.prepare( "INSERT INTO custgrp "
               "(custgrp_id, custgrp_name, custgrp_descrip) "
               "VALUES "
               "(:custgrp_id, :custgrp_name, :custgrp_descrip);" );
  else if (_mode == cEdit)
    customerSave.prepare( "UPDATE custgrp "
               "SET custgrp_name=:custgrp_name, custgrp_descrip=:custgrp_descrip "
               "WHERE (custgrp_id=:custgrp_id);" );

  customerSave.bindValue(":custgrp_id", _custgrpid);
  customerSave.bindValue(":custgrp_name", _name->text());
  customerSave.bindValue(":custgrp_descrip", _descrip->text().trimmed());
  customerSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Customer Group"),
                                customerSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_custgrpid);
}

void customerGroup::sDelete()
{
  XSqlQuery customerDelete;
  customerDelete.prepare( "DELETE FROM custgrpitem "
             "WHERE (custgrpitem_id=:custgrpitem_id);" );
  customerDelete.bindValue(":custgrpitem_id", _custgrpitem->id());
  customerDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Customer From Group"),
                                customerDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void customerGroup::sNew()
{
  XSqlQuery customerNew;
  ParameterList params;

  CRMAcctSearch *newdlg = new CRMAcctSearch(this);
  newdlg->setSubtype(CRMAcctLineEdit::Cust);

  int custid;
  if ((custid = newdlg->exec()) != XDialog::Rejected)
  {
    customerNew.prepare( "SELECT custgrpitem_id "
               "FROM custgrpitem "
               "WHERE ( (custgrpitem_custgrp_id=:custgrpitem_custgrp_id)"
               " AND (custgrpitem_cust_id=:custgrpitem_cust_id) );" );
    customerNew.bindValue(":custgrpitem_custgrp_id", _custgrpid);
    customerNew.bindValue(":custgrpitem_cust_id", custid);
    customerNew.exec();
    if (customerNew.first())
      return;
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Adding Customer To Group"),
                                  customerNew, __FILE__, __LINE__))
    {
      return;
    }

    customerNew.prepare( "INSERT INTO custgrpitem "
               "(custgrpitem_custgrp_id, custgrpitem_cust_id) "
               "VALUES "
               "(:custgrpitem_custgrp_id, :custgrpitem_cust_id);" );
    customerNew.bindValue(":custgrpitem_custgrp_id", _custgrpid);
    customerNew.bindValue(":custgrpitem_cust_id", custid);
    customerNew.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Adding Customer To Group"),
                                  customerNew, __FILE__, __LINE__))
    {
      return;
    }

    sFillList();
  }
}

void customerGroup::sFillList()
{
  XSqlQuery customerFillList;
  customerFillList.prepare( "SELECT custgrpitem_id, cust_number, cust_name "
             "FROM custgrpitem, custinfo "
             "WHERE ( (custgrpitem_cust_id=cust_id) "
             " AND (custgrpitem_custgrp_id=:custgrp_id) ) "
             "ORDER BY cust_number;" );
  customerFillList.bindValue(":custgrp_id", _custgrpid);
  customerFillList.exec();
  _custgrpitem->populate(customerFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Group Information"),
                                customerFillList, __FILE__, __LINE__))
  {
    return;
  }
}

void customerGroup::populate()
{
  XSqlQuery customerpopulate;
  customerpopulate.prepare( "SELECT custgrp_name, custgrp_descrip "
             "FROM custgrp "
             "WHERE (custgrp_id=:custgrp_id);" );
  customerpopulate.bindValue(":custgrp_id", _custgrpid);
  customerpopulate.exec();
  if (customerpopulate.first())
  {
    _name->setText(customerpopulate.value("custgrp_name").toString());
    _descrip->setText(customerpopulate.value("custgrp_descrip").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Group Information"),
                                customerpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
