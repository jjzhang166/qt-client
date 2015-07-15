/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutSpecial.h"

#include <QVariant>
#include <QMessageBox>

financialLayoutSpecial::financialLayoutSpecial(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
  connect(_showDB, SIGNAL(toggled(bool)), _showDBPrcnt, SLOT(setEnabled(bool)));
  connect(_showEnding, SIGNAL(toggled(bool)), _showEndingPrcnt, SLOT(setEnabled(bool)));
  connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
  connect(_showBeginning, SIGNAL(toggled(bool)), _showBeginningPrcnt, SLOT(setEnabled(bool)));

  _flheadid = -1;
  _flspecid = -1;
}

financialLayoutSpecial::~financialLayoutSpecial()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayoutSpecial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutSpecial::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
    sFillGroupList();
  }

  param = pParams.value("flgrp_id", &valid);
  if (valid)
    _flgrpid = param.toInt();

  param = pParams.value("flspec_id", &valid);
  if (valid)
  {
    _flspecid = param.toInt();
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
    }
  }

  return NoError;
}

void financialLayoutSpecial::sCheck()
{
}

void financialLayoutSpecial::sSave()
{
  XSqlQuery financialSave;
  int order = 1;
  if (_mode == cNew)
  {
    financialSave.prepare("SELECT COALESCE(MAX(ord),0) + 1 AS neworder"
              "  FROM (SELECT flgrp_order AS ord"
              "          FROM flgrp"
              "         WHERE ((flgrp_flgrp_id=:flgrp_id)"
              "           AND  (flgrp_flhead_id=:flhead_id))"
              "         UNION"
              "        SELECT flitem_order AS ord"
              "          FROM flitem"
              "         WHERE ((flitem_flgrp_id=:flgrp_id)"
              "           AND  (flitem_flhead_id=:flhead_id))"
              "         UNION"
              "        SELECT flspec_order AS ord"
              "          FROM flspec"
              "         WHERE ((flspec_flgrp_id=:flgrp_id)"
              "           AND  (flspec_flhead_id=:flhead_id)) ) AS data;" );
    financialSave.bindValue(":flgrp_id", _flgrpid);
    financialSave.bindValue(":flhead_id", _flheadid);
    financialSave.exec();

    if(financialSave.first())
      order = financialSave.value("neworder").toInt();
  }

  if (_mode == cNew)
  {
    financialSave.exec("SELECT NEXTVAL('flspec_flspec_id_seq') AS flspec_id;");
    if (financialSave.first())
      _flspecid = financialSave.value("flspec_id").toInt();
    
    financialSave.prepare( "INSERT INTO flspec "
               "( flspec_id, flspec_flhead_id, flspec_flgrp_id, flspec_order,"
               "  flspec_name, flspec_type, flspec_showstart, flspec_showend, flspec_showdelta,"
               "  flspec_showbudget, flspec_showdiff, flspec_showcustom,"
               "  flspec_subtract, flspec_showstartprcnt, flspec_showendprcnt, flspec_showdeltaprcnt,"
               "  flspec_showbudgetprcnt, flspec_showdiffprcnt, flspec_showcustomprcnt,"
               "  flspec_prcnt_flgrp_id, flspec_custom_source) "
               "VALUES "
               "( :flspec_id, :flspec_flhead_id, :flspec_flgrp_id, :flspec_order,"
               "  :flspec_name, :flspec_type, :flspec_showstart, :flspec_showend, :flspec_showdelta,"
               "  :flspec_showbudget, :flspec_showdiff, :flspec_showcustom,"
               "  :flspec_subtract, :flspec_showstartprcnt, :flspec_showendprcnt, :flspec_showdeltaprcnt,"
               "  :flspec_showbudgetprcnt, :flspec_showdiffprcnt, :flspec_showcustomprcnt,"
               "  :flspec_prcnt_flgrp_id, :flspec_custom_source);" );
  }
  else if (_mode == cEdit)
    financialSave.prepare( "UPDATE flspec "
               "SET flspec_name=:flspec_name, flspec_type=:flspec_type,"
               "    flspec_showstart=:flspec_showstart,"
               "    flspec_showend=:flspec_showend, flspec_showdelta=:flspec_showdelta,"
               "    flspec_showbudget=:flspec_showbudget, flspec_subtract=:flspec_subtract,"
               "    flspec_showdiff=:flspec_showdiff, flspec_showcustom=:flspec_showcustom,"
               "    flspec_showstartprcnt=:flspec_showstartprcnt, flspec_showendprcnt=:flspec_showendprcnt,"
               "    flspec_showdeltaprcnt=:flspec_showdeltaprcnt, flspec_showbudgetprcnt=:flspec_showbudgetprcnt,"
               "    flspec_showdiffprcnt=:flspec_showdiffprcnt, flspec_showcustomprcnt=:flspec_showcustomprcnt,"
               "    flspec_prcnt_flgrp_id=:flspec_prcnt_flgrp_id, flspec_custom_source=:flspec_custom_source "
               "WHERE (flspec_id=:flspec_id);" );
  
  financialSave.bindValue(":flspec_flhead_id", _flheadid);
  financialSave.bindValue(":flspec_flgrp_id", _flgrpid);
  financialSave.bindValue(":flspec_order", order);
  financialSave.bindValue(":flspec_name", _name->text());
  financialSave.bindValue(":flspec_showstart", QVariant(_showBeginning->isChecked()));
  financialSave.bindValue(":flspec_showend", QVariant(_showEnding->isChecked()));
  financialSave.bindValue(":flspec_showdelta", QVariant(_showDB->isChecked()));
  financialSave.bindValue(":flspec_showbudget", QVariant(_showBudget->isChecked()));
  financialSave.bindValue(":flspec_showdiff", QVariant(_showDiff->isChecked()));
  financialSave.bindValue(":flspec_showcustom", QVariant(_showCustom->isChecked()));
  financialSave.bindValue(":flspec_subtract", QVariant(_subtract->isChecked()));
  financialSave.bindValue(":flspec_showstartprcnt", QVariant(_showBeginning->isChecked() && _showBeginningPrcnt->isChecked()));
  financialSave.bindValue(":flspec_showendprcnt", QVariant(_showEnding->isChecked() && _showEndingPrcnt->isChecked()));
  financialSave.bindValue(":flspec_showdeltaprcnt", QVariant(_showDB->isChecked() && _showDBPrcnt->isChecked()));
  financialSave.bindValue(":flspec_showbudgetprcnt", QVariant(_showBudget->isChecked() && _showBudgetPrcnt->isChecked()));
  financialSave.bindValue(":flspec_showdiffprcnt", QVariant(_showDiff->isChecked() && _showDiffPrcnt->isChecked()));
  financialSave.bindValue(":flspec_showcustomprcnt", QVariant(_showCustom->isChecked() && _showCustomPrcnt->isChecked()));
  financialSave.bindValue(":flspec_prcnt_flgrp_id", _group->id());
  financialSave.bindValue(":flspec_id", _flspecid);

  if(_customUseBeginning->isChecked())
    financialSave.bindValue(":flspec_custom_source", "S");
  else if(_customUseEnding->isChecked())
    financialSave.bindValue(":flspec_custom_source", "E");
  else if(_customUseDebits->isChecked())
    financialSave.bindValue(":flspec_custom_source", "D");
  else if(_customUseCredits->isChecked())
    financialSave.bindValue(":flspec_custom_source", "C");
  else if(_customUseBudget->isChecked())
    financialSave.bindValue(":flspec_custom_source", "B");
  else if(_customUseDiff->isChecked())
    financialSave.bindValue(":flspec_custom_source", "F");

  switch(_type->currentIndex())
  {
    case 1:
      financialSave.bindValue(":flspec_type", "OpenAP");
      break;
    case 0:
    default:
      financialSave.bindValue(":flspec_type", "OpenAR");
  }

  financialSave.exec();
  
  done(_flspecid);
}

void financialLayoutSpecial::populate()
{
  XSqlQuery financialpopulate;
  financialpopulate.prepare( "SELECT * "
             "FROM flspec "
             "WHERE (flspec_id=:flspec_id);" );
  financialpopulate.bindValue(":flspec_id", _flspecid);
  financialpopulate.exec();
  if (financialpopulate.first())
  {
    _name->setText(financialpopulate.value("flspec_name").toString());
    _showBeginning->setChecked(financialpopulate.value("flspec_showstart").toBool());
    _showEnding->setChecked(financialpopulate.value("flspec_showend").toBool());
    _showDB->setChecked(financialpopulate.value("flspec_showdelta").toBool());
    _showBudget->setChecked(financialpopulate.value("flspec_showbudget").toBool());
    _showDiff->setChecked(financialpopulate.value("flspec_showdiff").toBool());
    _showCustom->setChecked(financialpopulate.value("flspec_showcustom").toBool());
    _showBeginningPrcnt->setChecked(financialpopulate.value("flspec_showstartprcnt").toBool());
    _showEndingPrcnt->setChecked(financialpopulate.value("flspec_showendprcnt").toBool());
    _showDBPrcnt->setChecked(financialpopulate.value("flspec_showdeltaprcnt").toBool());
    _showBudgetPrcnt->setChecked(financialpopulate.value("flspec_showbudgetprcnt").toBool());
    _showDiffPrcnt->setChecked(financialpopulate.value("flspec_showdiffprcnt").toBool());
    _showCustomPrcnt->setChecked(financialpopulate.value("flspec_showcustomprcnt").toBool());

    QString src = financialpopulate.value("flspec_custom_source").toString();
    if("S" == src)
      _customUseBeginning->setChecked(true);
    else if("E" == src)
      _customUseEnding->setChecked(true);
    else if("D" == src)
      _customUseDebits->setChecked(true);
    else if("C" == src)
      _customUseCredits->setChecked(true);
    else if("B" == src)
      _customUseBudget->setChecked(true);
    else if("F" == src)
      _customUseDiff->setChecked(true);

    if(financialpopulate.value("flspec_subtract").toBool())
      _subtract->setChecked(true);
    else
      _add->setChecked(true);

    if(financialpopulate.value("flspec_type").toString() == "OpenAP")
      _type->setCurrentIndex(1);
    else //if(financialpopulate.value("flspec_type").toString() == "OpenAR")
      _type->setCurrentIndex(0);

    _flheadid = financialpopulate.value("flspec_flhead_id").toInt();

    int grpid = financialpopulate.value("flspec_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
  }
}

void financialLayoutSpecial::sFillGroupList()
{
  XSqlQuery financialFillGroupList;
  _group->clear();
  financialFillGroupList.prepare("SELECT flgrp_id, flgrp_name"
            "  FROM flgrp"
            " WHERE (flgrp_flhead_id=:flhead_id)"
            " ORDER BY flgrp_name;");
  financialFillGroupList.bindValue(":flhead_id", _flheadid);
  financialFillGroupList.exec();
  _group->append(-1, tr("Parent"));
  while(financialFillGroupList.next())
    _group->append(financialFillGroupList.value("flgrp_id").toInt(), financialFillGroupList.value("flgrp_name").toString());
}

