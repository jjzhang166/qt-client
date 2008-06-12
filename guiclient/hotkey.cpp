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

#include "hotkey.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a hotkey as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
hotkey::hotkey(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _hotkey->insertItem("F1");
  _hotkey->insertItem("F2");
  _hotkey->insertItem("F3");
  _hotkey->insertItem("F4");
  _hotkey->insertItem("F5");
  _hotkey->insertItem("F6");
  _hotkey->insertItem("F7");
  _hotkey->insertItem("F8");
  _hotkey->insertItem("F9");
  _hotkey->insertItem("Ctrl+B");
  _hotkey->insertItem("Ctrl+C");
  _hotkey->insertItem("Ctrl+D");
  _hotkey->insertItem("Ctrl+E");
  _hotkey->insertItem("Ctrl+F");
  _hotkey->insertItem("Ctrl+G");
  _hotkey->insertItem("Ctrl+H");
  _hotkey->insertItem("Ctrl+I");
  _hotkey->insertItem("Ctrl+J");
  _hotkey->insertItem("Ctrl+K");
  _hotkey->insertItem("Ctrl+M");
  _hotkey->insertItem("Ctrl+N");
  _hotkey->insertItem("Ctrl+O");
  _hotkey->insertItem("Ctrl+P");
  _hotkey->insertItem("Ctrl+Q");
  _hotkey->insertItem("Ctrl+R");
  _hotkey->insertItem("Ctrl+T");
  _hotkey->insertItem("Ctrl+U");
  _hotkey->insertItem("Ctrl+V");
  _hotkey->insertItem("Ctrl+W");
  _hotkey->insertItem("Ctrl+X");
  _hotkey->insertItem("Ctrl+Y");
  _hotkey->insertItem("Ctrl+Z");
  _hotkey->insertItem("Ctrl+0");
  _hotkey->insertItem("Ctrl+1");
  _hotkey->insertItem("Ctrl+2");
  _hotkey->insertItem("Ctrl+3");
  _hotkey->insertItem("Ctrl+4");
  _hotkey->insertItem("Ctrl+5");
  _hotkey->insertItem("Ctrl+6");
  _hotkey->insertItem("Ctrl+7");
  _hotkey->insertItem("Ctrl+8");
  _hotkey->insertItem("Ctrl+9");

  _action->addColumn( tr("Action Name"),  200, Qt::AlignLeft );
  _action->addColumn( tr("Display Name"), -1,  Qt::AlignLeft );

  QStringList addedactions;
  XTreeWidgetItem *last = 0;
  for (ActionSet::iterator action = omfgThis->actions.begin(); action != omfgThis->actions.end(); action++)
  {
    if(!addedactions.contains((*action)->name()))
    {
      addedactions.append((*action)->name());
      last = new XTreeWidgetItem(_action, last, -1,
				 QVariant((*action)->name()),
				 (*action)->displayName()); 
    }
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
hotkey::~hotkey()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void hotkey::languageChange()
{
  retranslateUi(this);
}

enum SetResponse hotkey::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _hotkey->setEnabled(FALSE);
    }
  }

  param = pParams.value("username", &valid);
  if (valid)
  {
    _username = param.toString();
    _currentUser = FALSE;
  }

  param = pParams.value("currentUser", &valid);
  if (valid)
    _currentUser = TRUE;

  param = pParams.value("hotkey", &valid);
  if (valid)
  {
    QString value;

    if (_currentUser)
      value = _preferences->value(param.toString());
    else
    {
      q.prepare( "SELECT usrpref_value "
                 "FROM usrpref "
                 "WHERE ( (usrpref_username=:username)"
                 " AND (usrpref_name=:name) );" );
      q.bindValue(":username", _username);
      q.bindValue(":name", param.toString());
      q.exec();
      if (q.first())
        value = q.value("usrpref_value").toString();
//  ToDo
    }

    if (!value.isNull())
    {
      for (int i = 0; i < _action->topLevelItemCount(); i++)
      {
	XTreeWidgetItem *cursor = _action->topLevelItem(i);
        if (param.toString().left(1) == "F")
          _hotkey->setText(QString("F%1").arg(param.toString().right(1)));
        else if (param.toString().left(1) == "C")
          _hotkey->setText(QString("Ctrl+%1").arg(param.toString().right(1)));

        if (cursor->text(0) == value)
        {
          _action->setCurrentItem(cursor);
          _action->scrollToItem(cursor);
          break;
        }
      }
    }
  }

  return NoError;
}

void hotkey::sSave()
{
  QString keyValue;

  if(_action->currentItem() == 0)
  { 
    QMessageBox::information( this, tr("No Action Selected"),
      tr("You must select an Action before saving this Hotkey.") );
    return;
  }

  if (_hotkey->currentText().left(1) == tr("F"))
    keyValue = QString("F%1").arg(_hotkey->currentText().right(1));
  else if (_hotkey->currentText().left(5) == "Ctrl+")
    keyValue = QString("C%1").arg(_hotkey->currentText().right(1));

  if (_currentUser)
  {
    _preferences->set(keyValue, _action->currentItem()->text(0));
    _preferences->load();
  }
  else
  {
    q.prepare("SELECT setUserPreference(:username, :name, :value);");
    q.bindValue(":username", _username);
    q.bindValue(":name", keyValue);
    q.bindValue(":value", _action->currentItem()->text(0));
    q.exec();
  }

  accept();
}

