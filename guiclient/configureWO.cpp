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

#include "configureWO.h"

#include <QVariant>
#include <QValidator>
#include "guiclient.h"

/*
 *  Constructs a configureWO as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureWO::configureWO(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _nextWoNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT orderseq_number "
          "FROM orderseq "
          "WHERE (orderseq_name='WoNumber')" );
  if (q.first())
    _nextWoNumber->setText(q.value("orderseq_number").toString());

  _autoExplode->setChecked(_metrics->boolean("AutoExplodeWO"));
  _workOrderChangeLog->setChecked(_metrics->boolean("WorkOrderChangeLog"));
  _postopFillQty->setChecked(_metrics->boolean("AutoFillPostOperationQty"));

  _woNumGeneration->insertItem(tr("Automatic"));
  _woNumGeneration->insertItem(tr("Manual"));
  _woNumGeneration->insertItem(tr("Automatic, Allow Override"));

  if (_metrics->value("WONumberGeneration") == "A")
    _woNumGeneration->setCurrentItem(0);
  else if (_metrics->value("WONumberGeneration") == "M")
    _woNumGeneration->setCurrentItem(1);
  else if (_metrics->value("WONumberGeneration") == "O")
    _woNumGeneration->setCurrentItem(2);

  if (_metrics->value("ExplodeWOEffective") == "E")
    _explodeDateEffective->setChecked(TRUE);
  else
    _startDateEffective->setChecked(TRUE);

  if (_metrics->value("WOExplosionLevel") == "S")
    _singleLevel->setChecked(TRUE);
  else if (_metrics->value("WOExplosionLevel") == "M")
    _multiLevel->setChecked(TRUE);

  _materialVariances->setChecked(_metrics->boolean("PostMaterialVariances"));
  
  if (_metrics->boolean("Routings"))
    _laborVariances->setChecked(_metrics->boolean("PostLaborVariances"));
  else
  {
    _laborVariances->setChecked(FALSE);
    _laborVariances->hide();
    _wotcPostStyle->hide();
    _postopFillQty->hide();
  }

  if (_metrics->value("WOTCPostStyle") == "Production")
    _production->setChecked(TRUE);
  else
    _operations->setChecked(TRUE);

  if (_metrics->value("JobItemCosDefault") == "P")
    _proportional->setChecked(TRUE);
  else
    _todate->setChecked(TRUE);
    
  this->setCaption("Manufacture Configuration");
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureWO::~configureWO()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureWO::languageChange()
{
  retranslateUi(this);
}

void configureWO::sSave()
{
  q.prepare("SELECT setNextWoNumber(:woNumber) AS result;");
  q.bindValue(":woNumber", _nextWoNumber->text().toInt());
  q.exec();

  _metrics->set("AutoExplodeWO", _autoExplode->isChecked());
  _metrics->set("WorkOrderChangeLog", _workOrderChangeLog->isChecked());
  _metrics->set("AutoFillPostOperationQty", _postopFillQty->isChecked());

  if (_woNumGeneration->currentItem() == 0)
    _metrics->set("WONumberGeneration", QString("A"));
  else if (_woNumGeneration->currentItem() == 1)
    _metrics->set("WONumberGeneration", QString("M"));
  else if (_woNumGeneration->currentItem() == 2)
    _metrics->set("WONumberGeneration", QString("O"));

  _metrics->set("ExplodeWOEffective", ((_explodeDateEffective->isChecked()) ? QString("E") : QString("S")));
  _metrics->set("WOExplosionLevel", ((_singleLevel->isChecked()) ? QString("S") : QString("M")));
  _metrics->set("PostMaterialVariances", _materialVariances->isChecked());
  _metrics->set("PostLaborVariances", _laborVariances->isChecked());

  if (_production->isChecked())
    _metrics->set("WOTCPostStyle", QString("Production"));
  else if (_operations->isChecked())
    _metrics->set("WOTCPostStyle", QString("Operations"));

  if (_todate->isChecked())
    _metrics->set("JobItemCosDefault", QString("D"));
  else 
    _metrics->set("JobItemCosDefault", QString("P"));

  _metrics->load();

  accept();
}

