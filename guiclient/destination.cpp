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

#include "destination.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a destination as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
destination::destination(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheckName()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
destination::~destination()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void destination::languageChange()
{
    retranslateUi(this);
}


void destination::init()
{
}

enum SetResponse destination::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("destination_id", &valid);
  if (valid)
  {
    _destinationid = param.toInt();
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
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _city->setEnabled(FALSE);
      _state->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void destination::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('destination_destination_id_seq') AS _destination_id");
    if (q.first())
      _destinationid = q.value("_destination_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO destination "
               "( destination_id, destination_name,"
               "  destination_city, destination_state, destination_comments ) "
               "VALUES "
               "( :destination_id, :destination_name,"
               "  :destination_city, :destination_state, :destination_comments );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE destination "
               "SET destination_name=:destination_name,"
               "    destination_city=:destination_city, destination_state=:destination_state,"
               "    destination_comments=:destination_comments "
               "WHERE (destination_id=:destination_id);" );

  q.bindValue(":destination_id", _destinationid);
  q.bindValue(":destination_name", _name->text().stripWhiteSpace());
  q.bindValue(":destination_city", _city->text().stripWhiteSpace());
  q.bindValue(":destination_state", _state->text().stripWhiteSpace());
  q.bindValue(":destination_comments", _notes->text().stripWhiteSpace());
  q.exec();

  done(_destinationid);
}

void destination::sCheckName()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) || (_name->text().length()) )
  {
    q.prepare( "SELECT destination_id "
               "FROM destination "
               "WHERE (UPPER(destination_name)=UPPER(:destination_name));" );
    q.bindValue(":destination_name", _name->text());
    q.exec();
    if (q.first())
    {
      _destinationid = q.value("destination_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void destination::populate()
{
  q.prepare( "SELECT destination_name, destination_city, destination_state, destination_comments "
             "FROM destination "
             "WHERE (destination_id=:destination_id);" );
  q.bindValue(":destination_id", _destinationid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("destination_name"));
    _city->setText(q.value("destination_city"));
    _state->setText(q.value("destination_state"));
    _notes->setText(q.value("destination_comments").toString());
  }
}

