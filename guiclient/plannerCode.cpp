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

#include "plannerCode.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a plannerCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
plannerCode::plannerCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    QButtonGroup* _explosionGroupInt = new QButtonGroup(this);
    _explosionGroupInt->addButton(_singleLevel);
    _explosionGroupInt->addButton(_multipleLevel);

    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_autoExplode, SIGNAL(toggled(bool)), _explosionGroup, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
plannerCode::~plannerCode()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void plannerCode::languageChange()
{
    retranslateUi(this);
}


void plannerCode::init()
{
}

enum SetResponse plannerCode::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _plancodeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();

      connect(_autoExplode, SIGNAL(toggled(bool)), _explosionGroup, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _code->setFocus();

      connect(_autoExplode, SIGNAL(toggled(bool)), _explosionGroup, SLOT(setEnabled(bool)));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _autoExplode->setEnabled(FALSE);
      _explosionGroup->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  return NoError;
}

void plannerCode::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ((_mode == cNew) && (_code->text().length() != 0))
  {
    q.prepare( "SELECT plancode_id "
               "FROM plancode "
               "WHERE (UPPER(plancode_code)=UPPER(:plancode_code));" );
    q.bindValue(":plancode_code", _code->text());
    q.exec();
    if (q.first())
    {
      _plancodeid = q.value("plancode_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void plannerCode::sSave()
{
  _code->setText(_code->text().stripWhiteSpace().upper());
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Planner Code"),
                              tr("You must enter a valid Code for this Planner.") );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('plancode_plancode_id_seq') AS plancode_id");
    if (q.first())
      _plancodeid = q.value("plancode_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO plancode "
               "( plancode_id, plancode_code, plancode_name,"
               "  plancode_mpsexplosion, plancode_consumefcst ) "
               "VALUES "
               "( :plancode_id, :plancode_code, :plancode_name,"
               "  :plancode_mpsexplosion, :plancode_consumefcst );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE plancode "
               "SET plancode_code=:plancode_code, plancode_name=:plancode_name,"
               "    plancode_mpsexplosion=:plancode_mpsexplosion,"
               "    plancode_consumefcst=:plancode_consumefcst "
               "WHERE (plancode_id=:plancode_id);" );

  q.bindValue(":plancode_id", _plancodeid);
  q.bindValue(":plancode_code", _code->text());
  q.bindValue(":plancode_name", _description->text().stripWhiteSpace());
  q.bindValue(":plancode_consumefcst", QVariant(FALSE, 0));

  if (_autoExplode->isChecked())
  {
    if (_singleLevel->isChecked())
      q.bindValue(":plancode_mpsexplosion", "S");
    else
      q.bindValue(":plancode_mpsexplosion", "M");
  }
  else
    q.bindValue(":plancode_mpsexplosion", "N");

  q.exec();

  done(_plancodeid);
}

void plannerCode::populate()
{
  q.prepare( "SELECT plancode_code, plancode_name, plancode_mpsexplosion, plancode_consumefcst "
             "FROM plancode "
             "WHERE (plancode_id=:plancode_id);" );
  q.bindValue(":plancode_id", _plancodeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("plancode_code"));
    _description->setText(q.value("plancode_name"));

    if (q.value("plancode_mpsexplosion").toString() == "N")
      _autoExplode->setChecked(FALSE);
    else
    {
      _autoExplode->setChecked(TRUE);

      if (q.value("plancode_mpsexplosion").toString() == "S")
        _singleLevel->setChecked(TRUE);
      else if (q.value("plancode_mpsexplosion").toString() == "M")
        _multipleLevel->setChecked(TRUE);
    }
  }
} 

