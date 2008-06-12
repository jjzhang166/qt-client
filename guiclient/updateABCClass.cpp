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

#include "updateABCClass.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "guiclient.h"

/*
 *  Constructs a updateABCClass as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateABCClass::updateABCClass(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateABCClass::~updateABCClass()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateABCClass::languageChange()
{
    retranslateUi(this);
}


void updateABCClass::init()
{
  _classCode->setType(ClassCode);
  _dates->setStartCaption(tr("Start Evaluation Date"));
  _dates->setEndCaption(tr("End Evaluation Date"));

}

void updateABCClass::sUpdate()
{
  if ( (_classCode->isPattern()) && (_classCode->pattern().length() == 0) )
  {
    QMessageBox::critical( this, tr("Enter Class Code Pattern"),
                           tr( "You must enter the Class Code pattern to be used when updating Item Site\n"
                               "Class Codes or select a Class Code from the Class Code list." ) );
    _classCode->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Evaluation Start Date"),
                           tr("You must enter the Start Date of the evaluation period." ) );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Evaluation End Date"),
                           tr("You must enter the End Date of the evaluation period." ) );
    _dates->setFocus();
    return;
  }

  if (_classACutoff->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Class A Cutoff %"),
                           tr("You must enter the cutoff point for Class A Items.") );
    _classACutoff->setFocus();
    return;
  }

  if (_classBCutoff->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Class B Cutoff %"),
                           tr("You must enter the cutoff point for Class B Items.") );
    _classACutoff->setFocus();
    return;
  }

  if (_warehouse->isAll())
  {
    if (_classCode->isPattern())
      q.prepare("SELECT updateABCClass(:classcode_pattern, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isAll())
      q.prepare("SELECT updateABCClass(-1, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
  }
  else if (_warehouse->isSelected())
  {
    if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_pattern, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isSelected())
      q.prepare("SELECT updateABCClass(:classcode_id, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
    else if (_classCode->isAll())
      q.prepare("SELECT updateABCClass(-1, :warehous_id, :aCutOff, :bCutOff, :startDate, :endDate) AS result;");
  }

  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":aCutOff", (_classACutoff->toDouble() / 100.0));
  q.bindValue(":bCutOff", (_classBCutoff->toDouble() / 100.0));
  q.exec();
  if (q.first())
    QMessageBox::information( this, tr("ABC Class Code Updated"),
                              tr("The ABC Class Code was updated for %1 Item Sites.")
                              .arg(q.value("result").toInt()) );
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );

  _close->setText(tr("&Close"));
}

