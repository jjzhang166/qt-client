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

#include "financialLayoutLabels.h"

#include <QVariant>

/*
 *  Constructs a financialLayoutLabels as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
financialLayoutLabels::financialLayoutLabels(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
financialLayoutLabels::~financialLayoutLabels()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void financialLayoutLabels::languageChange()
{
    retranslateUi(this);
}

enum SetResponse financialLayoutLabels::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
  {
    _flheadid = param.toInt();
    populate();
  }

  return NoError;
}

void financialLayoutLabels::sSave()
{
  q.prepare( "UPDATE flhead "
             "   SET flhead_custom_label=:flhead_custom_label,"
             "       flhead_usealtbegin=:flhead_usealtbegin,"
             "       flhead_altbegin=:flhead_altbegin,"
             "       flhead_usealtend=:flhead_usealtend,"
             "       flhead_altend=:flhead_altend,"
             "       flhead_usealtdebits=:flhead_usealtdebits,"
             "       flhead_altdebits=:flhead_altdebits,"
             "       flhead_usealtcredits=:flhead_usealtcredits,"
             "       flhead_altcredits=:flhead_altcredits,"
             "       flhead_usealtbudget=:flhead_usealtbudget,"
             "       flhead_altbudget=:flhead_altbudget,"
             "       flhead_usealtdiff=:flhead_usealtdiff,"
             "       flhead_altdiff=:flhead_altdiff,"
             "       flhead_usealttotal=:flhead_usealttotal,"
             "       flhead_alttotal=:flhead_alttotal"
             " WHERE (flhead_id=:flhead_id);" );
    
  q.bindValue(":flhead_id", _flheadid);
  q.bindValue(":flhead_custom_label", _customText->text());
  q.bindValue(":flhead_usealttotal", QVariant(_altTotal->isChecked(), 0));
  q.bindValue(":flhead_alttotal", _altTotalText->text());
  q.bindValue(":flhead_usealtbegin", QVariant(_altBegin->isChecked(), 0));
  q.bindValue(":flhead_altbegin", _altBeginText->text());
  q.bindValue(":flhead_usealtend", QVariant(_altEnd->isChecked(), 0));
  q.bindValue(":flhead_altend", _altEndText->text());
  q.bindValue(":flhead_usealtdebits", QVariant(_altDebits->isChecked(), 0));
  q.bindValue(":flhead_altdebits", _altDebitsText->text());
  q.bindValue(":flhead_usealtcredits", QVariant(_altCredits->isChecked(), 0));
  q.bindValue(":flhead_altcredits", _altCreditsText->text());
  q.bindValue(":flhead_usealtbudget", QVariant(_altBudget->isChecked(), 0));
  q.bindValue(":flhead_altbudget", _altBudgetText->text());
  q.bindValue(":flhead_usealtdiff", QVariant(_altDiff->isChecked(), 0));
  q.bindValue(":flhead_altdiff", _altDiffText->text());

  q.exec();
  
  done(_flheadid);
}

void financialLayoutLabels::populate()
{
  q.prepare( "SELECT flhead_custom_label,"
             "       flhead_usealtbegin, flhead_altbegin,"
             "       flhead_usealtend, flhead_altend,"
             "       flhead_usealtdebits, flhead_altdebits,"
             "       flhead_usealtcredits, flhead_altcredits,"
             "       flhead_usealtbudget, flhead_altbudget,"
             "       flhead_usealtdiff, flhead_altdiff,"
             "       flhead_usealttotal, flhead_alttotal "
             "  FROM flhead "
             " WHERE (flhead_id=:flhead_id);" );
  q.bindValue(":flhead_id", _flheadid);
  q.exec();
  if (q.first())
  {
    _customText->setText(q.value("flhead_custom_label").toString());
    _altTotal->setChecked(q.value("flhead_usealttotal").toBool());
    _altTotalText->setText(q.value("flhead_alttotal").toString());
    _altBegin->setChecked(q.value("flhead_usealtbegin").toBool());
    _altBeginText->setText(q.value("flhead_altbegin").toString());
    _altEnd->setChecked(q.value("flhead_usealtend").toBool());
    _altEndText->setText(q.value("flhead_altend").toString());
    _altDebits->setChecked(q.value("flhead_usealtdebits").toBool());
    _altDebitsText->setText(q.value("flhead_altdebits").toString());
    _altCredits->setChecked(q.value("flhead_usealtcredits").toBool());
    _altCreditsText->setText(q.value("flhead_altcredits").toString());
    _altBudget->setChecked(q.value("flhead_usealtbudget").toBool());
    _altBudgetText->setText(q.value("flhead_altbudget").toString());
    _altDiff->setChecked(q.value("flhead_usealtdiff").toBool());
    _altDiffText->setText(q.value("flhead_altdiff").toString());
  }
}


