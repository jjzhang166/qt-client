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

#include "customCommandArgument.h"

#include <qvariant.h>
#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a customCommandArgument as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
customCommandArgument::customCommandArgument(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
customCommandArgument::~customCommandArgument()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void customCommandArgument::languageChange()
{
    retranslateUi(this);
}


void customCommandArgument::init()
{
  _mode = cNew;
  _cmdargid = -1;
  _cmdid = -1;
}

enum SetResponse customCommandArgument::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("cmd_id", &valid);
  if(valid)
    _cmdid = param.toInt();
  
  param = pParams.value("cmdarg_id", &valid);
  if(valid)
  {
    _cmdargid = param.toInt();
    populate();
  }
  
  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      _mode = cNew;
    else if("edit" == mode)
      _mode = cEdit;
  }
  
  return NoError;
}

void customCommandArgument::sSave()
{
  if(_argument->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::warning(this, tr("No Argument Specified"),
                      tr("You must specify an argument in order to save.") );
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO cmdarg"
              "      (cmdarg_cmd_id, cmdarg_order, cmdarg_arg) "
              "VALUES(:cmd_id, :order, :argument);");
  else if(cEdit == _mode)
    q.prepare("UPDATE cmdarg"
              "   SET cmdarg_order=:order,"
              "       cmdarg_arg=:argument"
              " WHERE (cmdarg_id=:cmdarg_id); ");

  q.bindValue(":cmd_id", _cmdid);
  q.bindValue(":cmdarg_id", _cmdargid);
  q.bindValue(":order", _order->value());
  q.bindValue(":argument", _argument->text());

  if(q.exec())
    accept();
  else
    systemError( this, tr("A System Error occurred at customCommandArgument::%1")
                             .arg(__LINE__) );
}

void customCommandArgument::populate()
{
  q.prepare("SELECT cmdarg_cmd_id, cmdarg_order, cmdarg_arg"
            "  FROM cmdarg"
            " WHERE (cmdarg_id=:cmdarg_id);");
  q.bindValue(":cmdarg_id", _cmdargid);
  q.exec();
  if(q.first())
  {
    _cmdid = q.value("cmdarg_cmd_id").toInt();
    _order->setValue(q.value("cmdarg_order").toInt());
    _argument->setText(q.value("cmdarg_arg").toString());
  }
}



