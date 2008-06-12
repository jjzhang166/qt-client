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

#include "implodeWo.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "inputManager.h"

/*
 *  Constructs a implodeWo as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
implodeWo::implodeWo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_wo, SIGNAL(valid(bool)), _implode, SLOT(setEnabled(bool)));
    connect(_implode, SIGNAL(clicked()), this, SLOT(sImplode()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
implodeWo::~implodeWo()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void implodeWo::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>

void implodeWo::init()
{
  _captive = TRUE;
  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded);
}

enum SetResponse implodeWo::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _implode->setFocus();
  }

  return NoError;
}

void implodeWo::sImplode()
{
  q.prepare( "SELECT wo_adhoc "
             "FROM wo "
             "WHERE ( (wo_adhoc)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    if ( QMessageBox::warning( this, tr("Adhoc Work Order"),
                               tr( "The Work Order you have selected to Implode is adhoc, meaning\n"
                                   "that its W/O Materials Requirements and/or W/O Operations lists\n"
                                   "have been manually modified.  If you Implode the selected Work Order\n"
                                   "then these modifications will be lost.\n"
                                   "Are you sure that you want to Implode the selected Work Order?"),
                               tr("&Yes"), tr("&No"), QString::null, 0, 1) == 1)
      return;
  }

  q.prepare("SELECT implodeWo(:wo_id, TRUE) AS result;");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first() && q.value("result").toInt() < 0)
  {
    QString msg;
    if (q.value("result").toInt() == -1)
      msg = tr("The Work Order could not be imploded because time clock "
	       "entries exist for it.");
    else
      msg = tr("The Work Order could not be imploded (reason %1).")
	    .arg(q.value("result").toString());
    QMessageBox::information(this, tr("Work Order Not Imploded"), msg);
    return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _close->setText("&Close");
    _wo->setFocus();
  }
}

