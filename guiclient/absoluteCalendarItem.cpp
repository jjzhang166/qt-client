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

#include "absoluteCalendarItem.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a absoluteCalendarItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
absoluteCalendarItem::absoluteCalendarItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
absoluteCalendarItem::~absoluteCalendarItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void absoluteCalendarItem::languageChange()
{
    retranslateUi(this);
}


void absoluteCalendarItem::init()
{
}

enum SetResponse absoluteCalendarItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("calhead_id", &valid);
  if (valid)
    _calheadid = param.toInt();
  else
    _calheadid = -1;

  param = pParams.value("calendarName", &valid);
  if (valid)
    _calendarName->setText(param.toString());
  else if (_calheadid != -1)
  {
    q.prepare( "SELECT calhead_name "
               "FROM calhead "
               "WHERE (calhead_id=:calhead_id);" );
    q.bindValue(":calhead_id", _calheadid);
    q.exec();
    if (q.first())
      _calendarName->setText(q.value("calhead_name").toString());
  }

  param = pParams.value("calitem_id", &valid);
  if (valid)
  {
    _calitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
  }

  return NoError;
}

void absoluteCalendarItem::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('xcalitem_xcalitem_id_seq') AS _calitem_id");
    if (q.first())
      _calitemid = q.value("_calitem_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO acalitem "
               "( acalitem_id, acalitem_calhead_id, acalitem_name,"
               "  acalitem_periodstart, acalitem_periodlength )"
               "VALUES "
               "( :acalitem_id, :acalitem_calhead_id, :acalitem_name,"
               "  :acalitem_periodstart, :acalitem_periodlength )" );
  }
  else
    q.prepare( "UPDATE acalitem "
               "SET acalitem_name=:acalitem_name,"
               "    acalitem_periodstart=:acalitem_periodstart, acalitem_periodlength=:acalitem_periodlength "
               "WHERE (acalitem_id=:acalitem_id);" );

  q.bindValue(":acalitem_id", _calitemid);
  q.bindValue(":acalitem_calhead_id", _calheadid);
  q.bindValue(":acalitem_name", _name->text());
  q.bindValue(":acalitem_periodstart", _startDate->date());
  q.bindValue(":acalitem_periodlength", _periodLength->value());
  q.exec();

  done (_calitemid);
}

void absoluteCalendarItem::populate()
{
  q.prepare( "SELECT calhead_id, calhead_name "
             "FROM calhead, acalitem "
             "WHERE ( (acalitem_calhead_id=calhead_id)"
             " AND (acalitem_id=:acalitem_id));" );
  q.bindValue(":acalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    _calheadid = q.value("calhead_id").toInt();
    _calendarName->setText(q.value("calhead_name").toString());
  }

  q.prepare( "SELECT acalitem_name,"
             "       acalitem_periodstart, acalitem_periodlength "
             "FROM acalitem "
             "WHERE (acalitem_id=:acalitem_id);" );
  q.bindValue(":acalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("acalitem_name").toString());
    _startDate->setDate(q.value("acalitem_periodstart").toDate());
    _periodLength->setValue(q.value("acalitem_periodlength").toInt());
  }
//  ToDo
}
