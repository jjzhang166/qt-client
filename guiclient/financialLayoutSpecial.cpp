/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "financialLayoutSpecial.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a financialLayoutSpecial as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
financialLayoutSpecial::financialLayoutSpecial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_showBudget, SIGNAL(toggled(bool)), _showBudgetPrcnt, SLOT(setEnabled(bool)));
    connect(_showDB, SIGNAL(toggled(bool)), _showDBPrcnt, SLOT(setEnabled(bool)));
    connect(_showEnding, SIGNAL(toggled(bool)), _showEndingPrcnt, SLOT(setEnabled(bool)));
    connect(_showDiff, SIGNAL(toggled(bool)), _showDiffPrcnt, SLOT(setEnabled(bool)));
    connect(_showBeginning, SIGNAL(toggled(bool)), _showBeginningPrcnt, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
financialLayoutSpecial::~financialLayoutSpecial()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void financialLayoutSpecial::languageChange()
{
    retranslateUi(this);
}


void financialLayoutSpecial::init()
{
  _flheadid = -1;
  _flspecid = -1;
}

enum SetResponse financialLayoutSpecial::set(ParameterList &pParams)
{
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
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _save->setFocus();
    }
  }

  return NoError;
}

void financialLayoutSpecial::sCheck()
{
}

void financialLayoutSpecial::sSave()
{
  int order = 1;
  if (_mode == cNew)
  {
    q.prepare("SELECT COALESCE(MAX(ord),0) + 1 AS neworder"
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
    q.bindValue(":flgrp_id", _flgrpid);
    q.bindValue(":flhead_id", _flheadid);
    q.exec();

    if(q.first())
      order = q.value("neworder").toInt();
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('flspec_flspec_id_seq') AS flspec_id;");
    if (q.first())
      _flspecid = q.value("flspec_id").toInt();
    
    q.prepare( "INSERT INTO flspec "
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
    q.prepare( "UPDATE flspec "
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
  
  q.bindValue(":flspec_flhead_id", _flheadid);
  q.bindValue(":flspec_flgrp_id", _flgrpid);
  q.bindValue(":flspec_order", order);
  q.bindValue(":flspec_name", _name->text());
  q.bindValue(":flspec_showstart", QVariant(_showBeginning->isChecked(), 0));
  q.bindValue(":flspec_showend", QVariant(_showEnding->isChecked(), 0));
  q.bindValue(":flspec_showdelta", QVariant(_showDB->isChecked(), 0));
  q.bindValue(":flspec_showbudget", QVariant(_showBudget->isChecked(), 0));
  q.bindValue(":flspec_showdiff", QVariant(_showDiff->isChecked(), 0));
  q.bindValue(":flspec_showcustom", QVariant(_showCustom->isChecked(), 0));
  q.bindValue(":flspec_subtract", QVariant(_subtract->isChecked(), 0));
  q.bindValue(":flspec_showstartprcnt", QVariant(_showBeginning->isChecked() && _showBeginningPrcnt->isChecked(), 0));
  q.bindValue(":flspec_showendprcnt", QVariant(_showEnding->isChecked() && _showEndingPrcnt->isChecked(), 0));
  q.bindValue(":flspec_showdeltaprcnt", QVariant(_showDB->isChecked() && _showDBPrcnt->isChecked(), 0));
  q.bindValue(":flspec_showbudgetprcnt", QVariant(_showBudget->isChecked() && _showBudgetPrcnt->isChecked(), 0));
  q.bindValue(":flspec_showdiffprcnt", QVariant(_showDiff->isChecked() && _showDiffPrcnt->isChecked(), 0));
  q.bindValue(":flspec_showcustomprcnt", QVariant(_showCustom->isChecked() && _showCustomPrcnt->isChecked(), 0));
  q.bindValue(":flspec_prcnt_flgrp_id", _group->id());
  q.bindValue(":flspec_id", _flspecid);

  if(_customUseBeginning->isChecked())
    q.bindValue(":flspec_custom_source", "S");
  else if(_customUseEnding->isChecked())
    q.bindValue(":flspec_custom_source", "E");
  else if(_customUseDebits->isChecked())
    q.bindValue(":flspec_custom_source", "D");
  else if(_customUseCredits->isChecked())
    q.bindValue(":flspec_custom_source", "C");
  else if(_customUseBudget->isChecked())
    q.bindValue(":flspec_custom_source", "B");
  else if(_customUseDiff->isChecked())
    q.bindValue(":flspec_custom_source", "F");

  switch(_type->currentItem())
  {
    case 1:
      q.bindValue(":flspec_type", "OpenAP");
      break;
    case 0:
    default:
      q.bindValue(":flspec_type", "OpenAR");
  }

  q.exec();
  
  done(_flspecid);
}

void financialLayoutSpecial::populate()
{
  q.prepare( "SELECT * "
             "FROM flspec "
             "WHERE (flspec_id=:flspec_id);" );
  q.bindValue(":flspec_id", _flspecid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("flspec_name").toString());
    _showBeginning->setChecked(q.value("flspec_showstart").toBool());
    _showEnding->setChecked(q.value("flspec_showend").toBool());
    _showDB->setChecked(q.value("flspec_showdelta").toBool());
    _showBudget->setChecked(q.value("flspec_showbudget").toBool());
    _showDiff->setChecked(q.value("flspec_showdiff").toBool());
    _showCustom->setChecked(q.value("flspec_showcustom").toBool());
    _showBeginningPrcnt->setChecked(q.value("flspec_showstartprcnt").toBool());
    _showEndingPrcnt->setChecked(q.value("flspec_showendprcnt").toBool());
    _showDBPrcnt->setChecked(q.value("flspec_showdeltaprcnt").toBool());
    _showBudgetPrcnt->setChecked(q.value("flspec_showbudgetprcnt").toBool());
    _showDiffPrcnt->setChecked(q.value("flspec_showdiffprcnt").toBool());
    _showCustomPrcnt->setChecked(q.value("flspec_showcustomprcnt").toBool());

    QString src = q.value("flspec_custom_source").toString();
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

    if(q.value("flspec_subtract").toBool())
      _subtract->setChecked(true);
    else
      _add->setChecked(true);

    if(q.value("flspec_type").toString() == "OpenAP")
      _type->setCurrentItem(1);
    else //if(q.value("flspec_type").toString() == "OpenAR")
      _type->setCurrentItem(0);

    _flheadid = q.value("flspec_flhead_id").toInt();

    int grpid = q.value("flspec_prcnt_flgrp_id").toInt();
    sFillGroupList();
    _group->setId(grpid);
  }
}

void financialLayoutSpecial::sFillGroupList()
{
  _group->clear();
  q.prepare("SELECT flgrp_id, flgrp_name"
            "  FROM flgrp"
            " WHERE (flgrp_flhead_id=:flhead_id)"
            " ORDER BY flgrp_name;");
  q.bindValue(":flhead_id", _flheadid);
  q.exec();
  _group->append(-1, tr("Parent"));
  while(q.next())
    _group->append(q.value("flgrp_id").toInt(), q.value("flgrp_name").toString());
}

