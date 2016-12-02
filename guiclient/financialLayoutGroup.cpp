/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayoutGroup.h"

#include <QVariant>

#define cIncome   0
#define cBalance  1
#define cCash     2
#define cAdHoc    3

financialLayoutGroup::financialLayoutGroup(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_showSubtotal, SIGNAL(toggled(bool)), this, SLOT(sToggled()));
  connect(_summarize, SIGNAL(toggled(bool)), this, SLOT(sToggled()));
  connect(_summarize, SIGNAL(toggled(bool)), _showSubtotal, SLOT(setDisabled(bool)));
  connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
  connect(_showDelta, SIGNAL(toggled(bool)), _showDeltaPrcnt, SLOT(setEnabled(bool)));
  connect(_showEnd, SIGNAL(toggled(bool)), _showEndPrcnt, SLOT(setEnabled(bool)));
  connect(_showStart, SIGNAL(toggled(bool)), _showStartPrcnt, SLOT(setEnabled(bool)));
  connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
  connect(_showCustom, SIGNAL(toggled(bool)), _showCustomPrcnt, SLOT(setEnabled(bool)));
  connect(_showPrcnt, SIGNAL(toggled(bool)), this, SLOT(sToggleShowPrcnt()));
}

financialLayoutGroup::~financialLayoutGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void financialLayoutGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse financialLayoutGroup::set(const ParameterList &pParams)
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

  param = pParams.value("flgrp_flgrp_id", &valid);
  if (valid)
    _flgrp_flgrpid = param.toInt();

  param = pParams.value("flgrp_id", &valid);
  if (valid)
  {
    _flgrpid = param.toInt();
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
      
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _name->setEnabled(false);
      _description->setEnabled(false);
      _subSummGroup->setEnabled(false);
      _operationGroup->setEnabled(false);
    }
  }
  param = pParams.value("type", &valid);
    if (valid)
    {
        if (param.toString() == "adHoc")
        {
			_rpttype = cAdHoc;
			_showPrcnt->setHidden(true);
		}
        else
        {
        
          _showStart->setHidden(true);
		  _showEnd->setHidden(true);
          _showDelta->setHidden(true);
          _showBudget->setHidden(true);
          _showDiff->setHidden(true);
          _showCustom->setHidden(true);
          _showStartPrcnt->setHidden(true);
          _showEndPrcnt->setHidden(true);
          _showDeltaPrcnt->setHidden(true);
          _showBudgetPrcnt->setHidden(true);
          _showDiffPrcnt->setHidden(true);
          _showCustomPrcnt->setHidden(true);
    
        }
        if (param.toString() == "income")
        {
          _rpttype = cIncome;
          _showPrcnt->setHidden(false);
		  _showStart->setChecked(false);
          _showEnd->setChecked(false);
          _showDelta->setChecked(false);
          _showBudget->setChecked(true);
          _showDiff->setChecked(true);
      
        }
        else if (param.toString() == "balance")
        {
        _rpttype = cBalance;
      _showPrcnt->setHidden(false);
      _showStart->setChecked(false);
      _showEnd->setChecked(true);
      _showDelta->setChecked(false);
      _showBudget->setChecked(true);
      _showDiff->setChecked(false);
        }
        else if (param.toString() == "cash")
        {
         _rpttype = cCash;
      _showPrcnt->setHidden(false);
      _showStart->setChecked(false);
      _showEnd->setChecked(false);
      _showDelta->setChecked(true);
      _showBudget->setChecked(true);
      _showDiff->setChecked(true);
      }
    }

  return NoError;
}

void financialLayoutGroup::sCheck()
{
}

void financialLayoutGroup::sSave()
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
    financialSave.bindValue(":flgrp_id", _flgrp_flgrpid);
    financialSave.bindValue(":flhead_id", _flheadid);
    financialSave.exec();

    if(financialSave.first())
      order = financialSave.value("neworder").toInt();
  }

  if (_mode == cNew)
  {
    financialSave.exec("SELECT NEXTVAL('flgrp_flgrp_id_seq') AS flgrp_id;");
    if (financialSave.first())
      _flgrpid = financialSave.value("flgrp_id").toInt();
    
    financialSave.prepare( "INSERT INTO flgrp "
               "( flgrp_id, flgrp_flhead_id, flgrp_flgrp_id, flgrp_name, flgrp_descrip, flgrp_order, flgrp_subtotal,"
               "  flgrp_summarize, flgrp_subtract, flgrp_showstart, flgrp_showend, flgrp_showdelta, flgrp_showbudget, flgrp_showdiff, flgrp_showcustom,"
               "  flgrp_showstartprcnt, flgrp_showendprcnt, flgrp_showdeltaprcnt, flgrp_showbudgetprcnt, flgrp_showdiffprcnt, flgrp_showcustomprcnt,"
               "  flgrp_prcnt_flgrp_id, flgrp_usealtsubtotal, flgrp_altsubtotal ) "
               "VALUES "
               "( :flgrp_id, :flgrp_flhead_id, :flgrp_flgrp_id, :flgrp_name, :flgrp_descrip, :flgrp_order, :flgrp_subtotal,"
               "  :flgrp_summarize, :flgrp_subtract, :flgrp_showstart, :flgrp_showend, :flgrp_showdelta, :flgrp_showbudget, :flgrp_showdiff, :flgrp_showcustom,"
               "  :flgrp_showstartprcnt, :flgrp_showendprcnt, :flgrp_showdeltaprcnt, :flgrp_showbudgetprcnt, :flgrp_showdiffprcnt, :flgrp_showcustomprcnt,"
               "  :flgrp_prcnt_flgrp_id, :flgrp_usealtsubtotal, :flgrp_altsubtotal );" );
  }
  else if (_mode == cEdit)
    financialSave.prepare( "UPDATE flgrp "
               "SET flgrp_name=:flgrp_name, flgrp_descrip=:flgrp_descrip,"
               "    flgrp_subtotal=:flgrp_subtotal,"
               "    flgrp_summarize=:flgrp_summarize,"
               "    flgrp_subtract=:flgrp_subtract,"
               "    flgrp_showstart=:flgrp_showstart,"
               "    flgrp_showend=:flgrp_showend,"
               "    flgrp_showdelta=:flgrp_showdelta,"
               "    flgrp_showbudget=:flgrp_showbudget,"
               "    flgrp_showdiff=:flgrp_showdiff,"
               "    flgrp_showcustom=:flgrp_showcustom,"
               "    flgrp_showstartprcnt=:flgrp_showstartprcnt,"
               "    flgrp_showendprcnt=:flgrp_showendprcnt,"
               "    flgrp_showdeltaprcnt=:flgrp_showdeltaprcnt,"
               "    flgrp_showbudgetprcnt=:flgrp_showbudgetprcnt,"
               "    flgrp_showdiffprcnt=:flgrp_showdiffprcnt,"
               "    flgrp_showcustomprcnt=:flgrp_showcustomprcnt,"
               "    flgrp_prcnt_flgrp_id=:flgrp_prcnt_flgrp_id,"
               "    flgrp_usealtsubtotal=:flgrp_usealtsubtotal,"
               "    flgrp_altsubtotal=:flgrp_altsubtotal "
               "WHERE (flgrp_id=:flgrp_id);" );
    
  financialSave.bindValue(":flgrp_id", _flgrpid);
  financialSave.bindValue(":flgrp_flgrp_id", _flgrp_flgrpid);
  financialSave.bindValue(":flgrp_flhead_id", _flheadid);
  financialSave.bindValue(":flgrp_name", _name->text());
  financialSave.bindValue(":flgrp_descrip", _description->text());
  financialSave.bindValue(":flgrp_order", order);
  financialSave.bindValue(":flgrp_subtotal",  QVariant(_showSubtotal->isChecked()));
  financialSave.bindValue(":flgrp_summarize", QVariant(_summarize->isChecked()));
  financialSave.bindValue(":flgrp_subtract",  QVariant(_subtract->isChecked()));
  financialSave.bindValue(":flgrp_showstart", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showStart->isChecked())));
  financialSave.bindValue(":flgrp_showend",   QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showEnd->isChecked())));
  financialSave.bindValue(":flgrp_showdelta", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDelta->isChecked())));
  financialSave.bindValue(":flgrp_showbudget", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showBudget->isChecked())));
  financialSave.bindValue(":flgrp_showdiff",  QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDiff->isChecked())));
  financialSave.bindValue(":flgrp_showcustom", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showCustom->isChecked())));
  financialSave.bindValue(":flgrp_showstartprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showStart->isChecked() && _showStartPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_showendprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showEnd->isChecked() && _showEndPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_showdeltaprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDelta->isChecked() && _showDeltaPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_showbudgetprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showBudget->isChecked() && _showBudgetPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_showdiffprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showDiff->isChecked() && _showDiffPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_showcustomprcnt", QVariant(((_showSubtotal->isChecked() || _summarize->isChecked()) && _showCustom->isChecked() && _showCustomPrcnt->isChecked())));
  financialSave.bindValue(":flgrp_prcnt_flgrp_id", _group->id());
  financialSave.bindValue(":flgrp_usealtsubtotal", QVariant(_altSubtotal->isChecked()));
  financialSave.bindValue(":flgrp_altsubtotal", _altSubtotalLabel->text());

  financialSave.exec();
  
  done(_flgrpid);
}

void financialLayoutGroup::populate()
{
  XSqlQuery financialpopulate;
  financialpopulate.prepare( "SELECT flgrp_name, flgrp_descrip, flgrp_subtotal,"
             "       flgrp_summarize, flgrp_subtract,"
             "       flgrp_showstart, flgrp_showend,"
             "       flgrp_showdelta, flgrp_showbudget, flgrp_showdiff, flgrp_showcustom,"
             "       flgrp_showstartprcnt, flgrp_showendprcnt,"
             "       flgrp_showdeltaprcnt, flgrp_showbudgetprcnt, flgrp_showdiffprcnt, flgrp_showcustomprcnt,"
             "       flgrp_flhead_id, flgrp_prcnt_flgrp_id, flgrp_usealtsubtotal, flgrp_altsubtotal "
             "FROM flgrp "
             "WHERE (flgrp_id=:flgrp_id);" );
  financialpopulate.bindValue(":flgrp_id", _flgrpid);
  financialpopulate.exec();
  if (financialpopulate.first())
  {
    _flheadid = financialpopulate.value("flgrp_flhead_id").toInt();

    _name->setText(financialpopulate.value("flgrp_name").toString());
    _description->setText(financialpopulate.value("flgrp_descrip").toString());
    _showSubtotal->setChecked(financialpopulate.value("flgrp_subtotal").toBool());
    _summarize->setChecked(financialpopulate.value("flgrp_summarize").toBool());
    _altSubtotal->setChecked(financialpopulate.value("flgrp_usealtsubtotal").toBool());
    _altSubtotalLabel->setText(financialpopulate.value("flgrp_altsubtotal").toString());

    if(_summarize->isChecked() || _showSubtotal->isChecked())
    {
      _showStart->setChecked(financialpopulate.value("flgrp_showstart").toBool());
      _showEnd->setChecked(financialpopulate.value("flgrp_showend").toBool());
      _showDelta->setChecked(financialpopulate.value("flgrp_showdelta").toBool());
      _showBudget->setChecked(financialpopulate.value("flgrp_showbudget").toBool());
      _showDiff->setChecked(financialpopulate.value("flgrp_showdiff").toBool());
      _showCustom->setChecked(financialpopulate.value("flgrp_showcustom").toBool());
      _showStartPrcnt->setChecked(financialpopulate.value("flgrp_showstartprcnt").toBool());
      _showEndPrcnt->setChecked(financialpopulate.value("flgrp_showendprcnt").toBool());
      _showDeltaPrcnt->setChecked(financialpopulate.value("flgrp_showdeltaprcnt").toBool());
      _showBudgetPrcnt->setChecked(financialpopulate.value("flgrp_showbudgetprcnt").toBool());
      _showDiffPrcnt->setChecked(financialpopulate.value("flgrp_showdiffprcnt").toBool());
      _showCustomPrcnt->setChecked(financialpopulate.value("flgrp_showcustomprcnt").toBool());
    }

    if(financialpopulate.value("flgrp_subtract").toBool())
      _subtract->setChecked(true);
    else
      _add->setChecked(true);

    if ((_rpttype != cAdHoc) && ((_showEndPrcnt->isChecked()) || (_showDiffPrcnt->isChecked())))
      _showPrcnt->setChecked(true);

    int grpid = financialpopulate.value("flgrp_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
  }
}

void financialLayoutGroup::sToggled()
{
  bool on = (_showSubtotal->isChecked() || _summarize->isChecked());
  if(_summarize->isChecked())
  {
    _altSubtotal->setChecked(false);
    _altSubtotal->setEnabled(false);
  }
  else
    _altSubtotal->setEnabled(_showSubtotal->isEnabled());
  _showStart->setEnabled(on);
  _showEnd->setEnabled(on);
  _showDelta->setEnabled(on);
  _showBudget->setEnabled(on);
  _showDiff->setEnabled(on);
  _showCustom->setEnabled(on);
  _showStartPrcnt->setEnabled(on  && _showStart->isChecked());
  _showEndPrcnt->setEnabled(on    && _showEnd->isChecked());
  _showDeltaPrcnt->setEnabled(on  && _showDelta->isChecked());
  _showBudgetPrcnt->setEnabled(on && _showBudget->isChecked());
  _showDiffPrcnt->setEnabled(on   && _showDiff->isChecked());
  _showCustomPrcnt->setEnabled(on && _showCustom->isChecked());
}

void financialLayoutGroup::sToggleShowPrcnt()
{
  if (_rpttype == cIncome)
  {
    if (_showPrcnt->isChecked())
    {
      _showBudgetPrcnt->setChecked(true);
      _showDiffPrcnt->setChecked(true);
    }
    else
    {
      _showBudgetPrcnt->setChecked(false);
      _showDiffPrcnt->setChecked(false);
    }
  }
  else if (_rpttype == cBalance)
  {
    if (_showPrcnt->isChecked())
    {
      _showBudgetPrcnt->setChecked(true);
      _showEndPrcnt->setChecked(true);
    }
    else
    {
      _showBudgetPrcnt->setChecked(false);
      _showEndPrcnt->setChecked(false);
    }
  }
  else if (_rpttype == cCash)
  {
    if (_showPrcnt->isChecked())
    {
      _showDeltaPrcnt->setChecked(true);
      _showBudgetPrcnt->setChecked(true);
      _showDiffPrcnt->setChecked(true);
    }
    else
    {
      _showDeltaPrcnt->setChecked(false);
      _showBudgetPrcnt->setChecked(false);
      _showDiffPrcnt->setChecked(false);
    }
  }
}

void financialLayoutGroup::sFillGroupList()
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

