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

#include "copyPlannedSchedule.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a copyPlannedSchedule as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyPlannedSchedule::copyPlannedSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_newDate, SIGNAL(newDate(const QDate&)), this, SLOT(sHandleDates()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _pschheadid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyPlannedSchedule::~copyPlannedSchedule()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyPlannedSchedule::languageChange()
{
    retranslateUi(this);
}

enum SetResponse copyPlannedSchedule::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if (valid)
  {
    _pschheadid = param.toInt();

    q.prepare("SELECT pschhead_number, pschhead_start_date"
              "  FROM pschhead"
              " WHERE (pschhead_id=:pschhead_id);");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
    if(q.first())
    {
      _scheduleName->setText(q.value("pschhead_number").toString());
      _oldDate->setDate(q.value("pschhead_start_date").toDate());
    }
  }

  return NoError;
}

void copyPlannedSchedule::sHandleDates()
{
  int offset = _oldDate->date().daysTo(_newDate->date());
  _offset->setText(tr("%1 day(s)").arg(offset));
}

void copyPlannedSchedule::sCopy()
{
  QString number = _number->text().trimmed().toUpper();

  if(number.isEmpty())
  {
    QMessageBox::information( this, tr("Incomplete Data"),
      tr("You have not specified a Number for the new schedule.") );
    return;
  }

  q.prepare( "SELECT pschhead_id "
             "  FROM pschhead "
             " WHERE (pschhead_number=:pschhead_number) "
             "LIMIT 1;" );
  q.bindValue(":pschhead_number", number);
  q.exec();
  if (q.first())
  {
    _number->setText("");
    _number->setFocus();
    QMessageBox::warning( this, tr("Invalid Number"),
      tr("The Number you specified for this Planned Schedule already exists.") );
    return;
  }

  if(!_newDate->isValid())
  {
    QMessageBox::warning( this, tr("Incomplete data"),
      tr("You must specify a new Start Date for the copied schedule.") );
    return;
  }

  q.prepare("SELECT copyPlannedSchedule(:pschhead_id, :number, :newdate) AS result;");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":number", number);
  q.bindValue(":newdate", _newDate->date());
  q.exec();

  accept();
}

