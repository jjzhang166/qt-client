/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "empGroup.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "empGroup.h"
#include "empcluster.h"
#include "storedProcErrorLookup.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns false
bool empGroup::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("empGroup::userHasPriv(%d)", pMode);
  bool retval = false;
  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewEmployeeGroups") ||
               _privileges->check("MaintainEmployeeGroups");
      break;
    case cNew:
    case cEdit:
      retval = _privileges->check("MaintainEmployeeGroups");
      break;
    default:
      retval = false;
      break;
  }
  if (DEBUG)
    qDebug("empGroup::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XDialog
void empGroup::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("empGroup::setVisible(%d) called with mode() == %d",
           visible, _mode);
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

empGroup::empGroup(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,  SIGNAL(clicked()), this, SLOT(reject()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_save,   SIGNAL(clicked()), this, SLOT(sSave()));

  _empgrpitem->addColumn(tr("Code"), _itemColumn, Qt::AlignLeft, true, "emp_code");
  _empgrpitem->addColumn(tr("Number"),        -1, Qt::AlignLeft, true, "emp_number");
}

empGroup::~empGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void empGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse empGroup::set(const ParameterList &pParams)
{
  XSqlQuery empet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("empgrp_id", &valid);
  if (valid)
  {
    _empgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _save->setEnabled(false);

      empet.exec("SELECT NEXTVAL('empgrp_empgrp_id_seq') AS empgrpid;");
      if (empet.first())
        _empgrpid = empet.value("empgrpid").toInt();
      else if (empet.lastError().type() != QSqlError::NoError)
      {
	systemError(this, empet.lastError().databaseText(), __FILE__, __LINE__);
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
    }
  }

  bool editing = (_mode == cNew || _mode == cEdit);
  if (editing)
  {
    connect(_empgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }
  else
  {
    _save->hide();
    _close->setText(tr("&Close"));
  }
  _name->setEnabled(_mode == cNew);
  _descrip->setEnabled(editing);
  _new->setEnabled(_mode == cEdit);

  return NoError;
}

void empGroup::sCheck()
{
  XSqlQuery empCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    empCheck.prepare( "SELECT empgrp_id "
               "FROM empgrp "
               "WHERE (UPPER(empgrp_name)=UPPER(:name));" );
    empCheck.bindValue(":name", _name->text());
    empCheck.exec();
    if (empCheck.first())
    {
      _empgrpid = empCheck.value("empgrp_id").toInt();
      _mode = cEdit;
      populate();
      _name->setEnabled(false);
    }
    else if (empCheck.lastError().type() != QSqlError::NoError)
    {
      systemError(this, empCheck.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  _save->setEnabled(true);
}

bool empGroup::close()
{
  XSqlQuery empclose;
  if (_mode == cNew && _empgrpid > 0)
  {
    empclose.prepare( "SELECT deleteEmpgrp(:grpid) AS result;");
    empclose.bindValue(":grpid", _empgrpid);
    empclose.exec();
    if (empclose.first())
    {
      int result = empclose.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteEmpgrp", result),
                    __FILE__, __LINE__);
        return false;
      }
    }
    else if (empclose.lastError().type() != QSqlError::NoError)
    {
      systemError(this, empclose.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return true;
}

void empGroup::sSave(const bool pClose)
{
  XSqlQuery empSave;
  _name->setText(_name->text().trimmed());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Employee Group.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    empSave.prepare( "INSERT INTO empgrp "
               "(empgrp_id, empgrp_name, empgrp_descrip) "
               "VALUES "
               "(:grpid, :empgrp_name, :empgrp_descrip);" );
  else if (_mode == cEdit)
    empSave.prepare( "UPDATE empgrp "
               "SET empgrp_name=:empgrp_name, empgrp_descrip=:empgrp_descrip "
               "WHERE (empgrp_id=:grpid);" );

  empSave.bindValue(":grpid",          _empgrpid);
  empSave.bindValue(":empgrp_name",    _name->text().trimmed());
  empSave.bindValue(":empgrp_descrip", _descrip->text().trimmed());
  empSave.exec();
  if (empSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, empSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (pClose)
  {
    int grpid = _empgrpid;        // don't delete groups in close()
    _empgrpid = -1;
    done(grpid);
  }
  else if (_mode == cNew)
    _mode = cEdit;
}

void empGroup::sDelete()
{
  XSqlQuery empDelete;
  empDelete.prepare( "DELETE FROM empgrpitem "
             "WHERE (empgrpitem_id=:empgrpitem_id);" );
  empDelete.bindValue(":empgrpitem_id", _empgrpitem->id());
  empDelete.exec();
  if (empDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, empDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void empGroup::sNew()
{
  XSqlQuery empNew;
  ParameterList params;

  int empid = EmpClusterLineEdit::idFromList(this);
  if (empid != XDialog::Rejected && empid != -1)
  {
    sSave(false);
    empNew.prepare( "SELECT empgrpitem_id "
               "FROM empgrpitem "
               "WHERE ((empgrpitem_empgrp_id=:empgrpitem_empgrp_id)"
               "   AND (empgrpitem_emp_id=:empgrpitem_emp_id) );" );
    empNew.bindValue(":empgrpitem_empgrp_id", _empgrpid);
    empNew.bindValue(":empgrpitem_emp_id", empid);
    empNew.exec();
    if (empNew.first())
      return;
    else if (empNew.lastError().type() != QSqlError::NoError)
    {
      systemError(this, empNew.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    empNew.prepare( "INSERT INTO empgrpitem "
               "(empgrpitem_empgrp_id, empgrpitem_emp_id) "
               "VALUES "
               "(:empgrpitem_empgrp_id, :empgrpitem_emp_id);" );
    empNew.bindValue(":empgrpitem_empgrp_id", _empgrpid);
    empNew.bindValue(":empgrpitem_emp_id", empid);
    empNew.exec();
    if (empNew.lastError().type() != QSqlError::NoError)
    {
      systemError(this, empNew.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void empGroup::sFillList()
{
  XSqlQuery empFillList;
  empFillList.prepare( "SELECT empgrpitem_id, emp_code, emp_number "
             "FROM empgrpitem, emp "
             "WHERE ((empgrpitem_emp_id=emp_id) "
             "   AND (empgrpitem_empgrp_id=:empgrp_id) ) "
             "ORDER BY emp_code;" );
  empFillList.bindValue(":empgrp_id", _empgrpid);
  empFillList.exec();
  _empgrpitem->populate(empFillList);
  if (empFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, empFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void empGroup::populate()
{
  XSqlQuery emppopulate;
  emppopulate.prepare( "SELECT empgrp_name, empgrp_descrip "
             "FROM empgrp "
             "WHERE (empgrp_id=:empgrp_id);" );
  emppopulate.bindValue(":empgrp_id", _empgrpid);
  emppopulate.exec();
  if (emppopulate.first())
  {
    _name->setText(emppopulate.value("empgrp_name").toString());
    _descrip->setText(emppopulate.value("empgrp_descrip").toString());
  }
  else if (emppopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, emppopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}
