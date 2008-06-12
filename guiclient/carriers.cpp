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

#include "carriers.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include "carrier.h"

/*
 *  Constructs a carriers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
carriers::carriers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_carrier, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
carriers::~carriers()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void carriers::languageChange()
{
    retranslateUi(this);
}


void carriers::init()
{
  statusBar()->hide();
  
  _carrier->addColumn(tr("Name"), -1, Qt::AlignLeft);
  
  if (_privileges->check("MaintainDestinations"))
  {
    connect(_carrier, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_carrier, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_carrier, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_carrier, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void carriers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  carrier newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void carriers::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("carrier_id", _carrier->id());

  carrier newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void carriers::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("carrier_id", _carrier->id());

  carrier newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void carriers::sDelete()
{
  q.prepare( "SELECT rate_id "
             "FROM rate "
             "WHERE (rate_carrier_id=:carrier_id) "
             "LIMIT 1;" );
  q.bindValue(":carrier_id", _carrier->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Delete Carrier"),
                          tr( "The selected Carrier cannot be deleted as there are one or more Rates assigned to it.\n"
                              "You must delete these Rates before you may delete the selected Carrier." ) );
    return;
  }

  q.prepare( "DELETE FROM carrier "
             "WHERE (carrier_id=:carrier_id);" );
  q.bindValue(":carrier_id", _carrier->id());
  q.exec();
  sFillList();
}

void carriers::sFillList()
{
  _carrier->populate( "SELECT carrier_id, carrier_name "
                      "FROM carrier "
                      "ORDER BY carrier_name;" );
}
