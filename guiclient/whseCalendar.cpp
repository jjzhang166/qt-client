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

#include "whseCalendar.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a whseCalendar as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
whseCalendar::whseCalendar(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _description->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
whseCalendar::~whseCalendar()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void whseCalendar::languageChange()
{
    retranslateUi(this);
}

enum SetResponse whseCalendar::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("whsecal_id", &valid);
  if (valid)
  {
    _whsecalid = param.toInt();
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

      _warehouse->setEnabled(false);
      _description->setEnabled(false);
      _dates->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
      _activeGroup->setEnabled(false);

      _close->setFocus();
    }
  }

  return NoError;
}

void whseCalendar::sSave()
{
  if (!_dates->allValid())
  {
    QMessageBox::critical( this, tr("Enter Start/End Date"),
                           tr("You must enter a valid start/end date for this Warehouse Calendar before saving it.") );
    _dates->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('whsecal_whsecal_id_seq') AS _whsecal_id;");
    if (q.first())
      _whsecalid = q.value("_whsecal_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO whsecal "
               "( whsecal_id, whsecal_warehous_id, whsecal_descrip,"
               "  whsecal_effective, whsecal_expires, whsecal_active ) "
               "VALUES "
               "( :whsecal_id, :warehous_id, :whsecal_descrip,"
               "  :startDate, :endDate, :whsecal_active );" );
  }
  else
    q.prepare( "UPDATE whsecal "
               "SET whsecal_warehous_id=:warehous_id,"
               "    whsecal_descrip=:whsecal_descrip,"
               "    whsecal_effective=:startDate,"
               "    whsecal_expires=:endDate,"
               "    whsecal_active=:whsecal_active "
               "WHERE (whsecal_id=:whsecal_id);" );

  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":whsecal_id", _whsecalid);
  q.bindValue(":whsecal_descrip", _description->text());
  q.bindValue(":whsecal_active", QVariant(_active->isChecked(), 0));
  q.exec();

  done(_whsecalid);
}

void whseCalendar::populate()
{
  q.prepare( "SELECT whsecal_warehous_id, whsecal_descrip,"
             "       whsecal_effective, whsecal_expires,"
             "       whsecal_active "
             "FROM whsecal "
             "WHERE (whsecal_id=:whsecal_id);" );
  q.bindValue(":whsecal_id", _whsecalid);
  q.exec();
  if (q.first())
  {
    int warehousid = q.value("whsecal_warehous_id").toInt();
    if(warehousid < 1)
      _warehouse->setAll();
    else
      _warehouse->setId(warehousid);

    _description->setText(q.value("whsecal_descrip").toString());
    _dates->setStartDate(q.value("whsecal_effective").toDate());
    _dates->setEndDate(q.value("whsecal_expires").toDate());

    if(q.value("whsecal_active").toBool())
      _active->setChecked(true);
    else
      _inactive->setChecked(true);
  }
}

