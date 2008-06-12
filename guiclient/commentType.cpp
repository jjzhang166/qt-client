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

#include "commentType.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a commentType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
commentType::commentType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
commentType::~commentType()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void commentType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse commentType::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cmnttype_id", &valid);
  if (valid)
  {
    _cmnttypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void commentType::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Comment Type"),
                              tr("You must enter a valid Comment Type before saving this Item Type.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('cmnttype_cmnttype_id_seq') AS cmnttype_id");
    if (q.first())
      _cmnttypeid = q.value("cmnttype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO cmnttype "
               "( cmnttype_id, cmnttype_name, cmnttype_descrip ) "
               "VALUES "
               "( :cmnttype_id, :cmnttype_name, :cmnttype_descrip );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cmnttype "
               "SET cmnttype_name=:cmnttype_name, cmnttype_descrip=:cmnttype_descrip "
               "WHERE (cmnttype_id=:cmnttype_id);" );

  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.bindValue(":cmnttype_name", _name->text());
  q.bindValue(":cmnttype_descrip", _description->text());
  q.exec();

  done(_cmnttypeid);
}

void commentType::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT cmnttype_id "
               "FROM cmnttype "
               "WHERE (UPPER(cmnttype_name)=UPPER(:cmnttype_name));" );
    q.bindValue(":cmnttype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _cmnttypeid = q.value("cmnttype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void commentType::populate()
{
  q.prepare( "SELECT cmnttype_name, cmnttype_descrip "
             "FROM cmnttype "
             "WHERE (cmnttype_id=:cmnttype_id);" );
  q.bindValue(":cmnttype_id", _cmnttypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("cmnttype_name"));
    _description->setText(q.value("cmnttype_descrip"));
  }
}

