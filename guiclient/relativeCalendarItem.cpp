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

#include "relativeCalendarItem.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a relativeCalendarItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
relativeCalendarItem::relativeCalendarItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_offsetType, SIGNAL(activated(int)), this, SLOT(sHandleNewOffsetType(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
relativeCalendarItem::~relativeCalendarItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void relativeCalendarItem::languageChange()
{
    retranslateUi(this);
}


static const char *offsetTypes[] = { "D", "W", "M", "Y" };

void relativeCalendarItem::init()
{
}

enum SetResponse relativeCalendarItem::set(ParameterList &pParams)
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
      _name->setFocus();
    }
  }

  return NoError;
}

void relativeCalendarItem::populate()
{
  q.prepare( "SELECT calhead_id, calhead_name "
             "FROM calhead, rcalitem "
             "WHERE ( (rcalitem_calhead_id=calhead_id)"
             " AND (rcalitem_id=:rcalitem_id) );" );
  q.bindValue(":rcalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    _calheadid = q.value("calhead_id").toInt();
    _calendarName->setText(q.value("calhead_name").toString());
  }

  q.prepare( "SELECT rcalitem_name,"
             "       rcalitem_offsettype, rcalitem_offsetcount,"
             "       rcalitem_periodtype, rcalitem_periodcount "
             "FROM rcalitem "
             "WHERE (rcalitem_id=:rcalitem_id);" );
  q.bindValue(":rcalitem_id", _calitemid);
  q.exec();
  if (q.first())
  {
    int counter;

    _name->setText(q.value("rcalitem_name").toString());

    _offsetCount->setValue(q.value("rcalitem_offsetcount").toInt());

    for (counter = 0; counter < _offsetType->count(); counter++)
    {
      if (q.value("rcalitem_offsettype").toString() == offsetTypes[counter])
      {
        _offsetType->setCurrentItem(counter);
        break;
      }
    }

    _periodCount->setValue(q.value("rcalitem_periodcount").toInt());

    for (counter = 0; counter < _offsetType->count(); counter++)
    {
      if (q.value("rcalitem_periodtype").toString() == offsetTypes[counter])
      {
        _periodType->setCurrentItem(counter);
        break;
      }
    }

  }
  else
    reject();
}

void relativeCalendarItem::sHandleNewOffsetType(int pOffsetType)
{
  _periodType->setCurrentItem(pOffsetType);
}

void relativeCalendarItem::sSave()
{
  if (_mode == cNew)
    q.prepare( "SELECT rcalitem_id "
               "FROM rcalitem "
               "WHERE ( (rcalitem_calhead_id=:calhead_id)"
               " AND (rcalitem_offsettype=:offsetType)"
               " AND (rcalitem_offsetcount=:offsetCount)"
               " AND (rcalitem_periodtype=:periodType)"
               " AND (rcalitem_periodcount=:periodCount) );" );
  else if (_mode == cEdit)
    q.prepare( "SELECT rcalitem_id "
               "FROM rcalitem "
               "WHERE ( (rcalitem_calhead_id=:calhead_id)"
               " AND (rcalitem_offsettype=:offsetType)"
               " AND (rcalitem_offsetcount=:offsetCount)"
               " AND (rcalitem_periodtype=:periodType)"
               " AND (rcalitem_periodcount=:periodCount)"
               " AND (rcalitem_id<>:rcalitem_id) );" );

  q.bindValue(":rcalitem_id", _calitemid);
  q.bindValue(":calhead_id", _calheadid);
  q.bindValue(":offsetType", offsetTypes[_offsetType->currentItem()]);
  q.bindValue(":offsetCount", _offsetCount->value());
  q.bindValue(":periodType", offsetTypes[_periodType->currentItem()]);
  q.bindValue(":periodCount", _periodCount->value());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannon Create Duplicate Calendar Item"),
                           tr( "A Relative Calendar Item for the selected Calendar exists that has the save Offset and Period as this Calendar Item.\n"
                               "You may not create duplicate Calendar Items." ) );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('xcalitem_xcalitem_id_seq') AS _calitem_id;");
    if (q.first())
      _calitemid = q.value("_calitem_id").toInt();

    q.prepare( "INSERT INTO rcalitem "
               "( rcalitem_id, rcalitem_calhead_id, rcalitem_name,"
               "  rcalitem_offsettype, rcalitem_offsetcount,"
               "  rcalitem_periodtype, rcalitem_periodcount ) "
               "VALUES "
               "( :rcalitem_id, :rcalitem_calhead_id, :rcalitem_name,"
               "  :rcalitem_offsettype, :rcalitem_offsetcount,"
               "  :rcalitem_periodtype, :rcalitem_periodcount );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE rcalitem "
               "SET rcalitem_name=:rcalitem_name,"
               "    rcalitem_offsettype=:rcalitem_offsettype, rcalitem_offsetcount=:rcalitem_offsetcount,"
               "    rcalitem_periodtype=:rcalitem_periodtype, rcalitem_periodcount=:rcalitem_periodcount "
               "WHERE (rcalitem_id=:rcalitem_id);" );

  q.bindValue(":rcalitem_id", _calitemid);
  q.bindValue(":rcalitem_calhead_id", _calheadid);
  q.bindValue(":rcalitem_name", _name->text());
  q.bindValue(":rcalitem_offsettype", offsetTypes[_offsetType->currentItem()]);
  q.bindValue(":rcalitem_offsetcount", _offsetCount->value());
  q.bindValue(":rcalitem_periodtype", offsetTypes[_periodType->currentItem()]);
  q.bindValue(":rcalitem_periodcount", _periodCount->value());
  q.exec();

  done(_calitemid);
}


