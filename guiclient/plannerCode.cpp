/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannerCode.h"

#include <QVariant>
#include <QMessageBox>
#include "errorReporter.h"

plannerCode::plannerCode(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if (_metrics->value("Application") != "PostBooks")
  {
    QButtonGroup* _explosionGroupInt = new QButtonGroup(this);
    _explosionGroupInt->addButton(_singleLevel);
    _explosionGroupInt->addButton(_multipleLevel);
    _singleLevel->setChecked(true);
    _explosionGroup->setEnabled(false);
  }
  else
  {
    _mrpexcpResched->hide();
    _mrpexcpDelete->hide();
    _autoExplode->hide();
    _explosionGroup->hide();
  }
  
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

plannerCode::~plannerCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void plannerCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannerCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _plancodeid = param.toInt();
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
      _code->setEnabled(false);
      _description->setEnabled(false);
      _mrpexcpResched->setEnabled(false);
      _mrpexcpDelete->setEnabled(false);
      _autoExplode->setEnabled(false);
      _explosionGroup->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

bool plannerCode::sCheck()
{
  XSqlQuery plannerCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length() != 0))
  {
    plannerCheck.prepare( "SELECT plancode_id "
               "FROM plancode "
               "WHERE (UPPER(plancode_code)=UPPER(:plancode_code));" );
    plannerCheck.bindValue(":plancode_code", _code->text());
    plannerCheck.exec();
    if (plannerCheck.first())
    {
      _plancodeid = plannerCheck.value("plancode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
      return true;
    }
  }
  return false;
}

void plannerCode::sSave()
{
  XSqlQuery plannerSave;
  _code->setText(_code->text().trimmed().toUpper());
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Planner Code"),
                              tr("You must enter a valid Code for this Planner.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    if (sCheck())
    {
      QMessageBox::warning( this, tr("Cannot Save Planner Code"),
                            tr("This Planner code already exists.  You have been placed in edit mode.") );
      return;
    }

    plannerSave.exec("SELECT NEXTVAL('plancode_plancode_id_seq') AS plancode_id");
    if (plannerSave.first())
      _plancodeid = plannerSave.value("plancode_id").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Planner Code Information"),
                           plannerSave, __FILE__, __LINE__);
      return;
    }

    plannerSave.prepare( "INSERT INTO plancode "
               "( plancode_id, plancode_code, plancode_name,"
               "  plancode_mpsexplosion, plancode_consumefcst,"
               "  plancode_mrpexcp_resched, plancode_mrpexcp_delete ) "
               "VALUES "
               "( :plancode_id, :plancode_code, :plancode_name,"
               "  :plancode_mpsexplosion, :plancode_consumefcst,"
               "  :plancode_mrpexcp_resched, :plancode_mrpexcp_delete );" );
  }
  else if (_mode == cEdit)
    plannerSave.prepare("SELECT plancode_id"
              "  FROM plancode"
              " WHERE((plancode_id != :plancode_id)"
              " AND (plancode_code = :plancode_code));");

  plannerSave.bindValue(":plancode_id", _plancodeid); 
  plannerSave.bindValue(":plancode_code", _code->text());
  plannerSave.exec();
  if(plannerSave.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Planner Code"),
                          tr("You may not rename this Planner code with the entered name as it is in use by another Planner code.") );
    _code->setFocus();
    return;
  }

  plannerSave.prepare( "UPDATE plancode "
             "SET plancode_code=:plancode_code, plancode_name=:plancode_name,"
             "    plancode_mpsexplosion=:plancode_mpsexplosion,"
             "    plancode_consumefcst=:plancode_consumefcst,"
             "    plancode_mrpexcp_resched=:plancode_mrpexcp_resched, "
             "    plancode_mrpexcp_delete=:plancode_mrpexcp_delete "
             "WHERE (plancode_id=:plancode_id);" );

  plannerSave.bindValue(":plancode_id", _plancodeid);
  plannerSave.bindValue(":plancode_code", _code->text());
  plannerSave.bindValue(":plancode_name", _description->text().trimmed());
  plannerSave.bindValue(":plancode_consumefcst", false);
  plannerSave.bindValue(":plancode_mrpexcp_resched", QVariant(_mrpexcpResched->isChecked()));
  plannerSave.bindValue(":plancode_mrpexcp_delete", QVariant(_mrpexcpDelete->isChecked()));

  if (_autoExplode->isChecked())
  {
    if (_singleLevel->isChecked())
      plannerSave.bindValue(":plancode_mpsexplosion", "S");
    else
      plannerSave.bindValue(":plancode_mpsexplosion", "M");
  }
  else
    plannerSave.bindValue(":plancode_mpsexplosion", "N");

  plannerSave.exec();

  done(_plancodeid);
}

void plannerCode::populate()
{
  XSqlQuery plannerpopulate;
  plannerpopulate.prepare( "SELECT * "
             "FROM plancode "
             "WHERE (plancode_id=:plancode_id);" );
  plannerpopulate.bindValue(":plancode_id", _plancodeid);
  plannerpopulate.exec();
  if (plannerpopulate.first())
  {
    _code->setText(plannerpopulate.value("plancode_code"));
    _description->setText(plannerpopulate.value("plancode_name"));
    _mrpexcpResched->setChecked(plannerpopulate.value("plancode_mrpexcp_resched").toBool());
    _mrpexcpDelete->setChecked(plannerpopulate.value("plancode_mrpexcp_delete").toBool());

    if (plannerpopulate.value("plancode_mpsexplosion").toString() == "N")
      _autoExplode->setChecked(false);
    else
    {
      _autoExplode->setChecked(true);

      if (plannerpopulate.value("plancode_mpsexplosion").toString() == "S")
        _singleLevel->setChecked(true);
      else if (plannerpopulate.value("plancode_mpsexplosion").toString() == "M")
        _multipleLevel->setChecked(true);
    }
  }
} 

