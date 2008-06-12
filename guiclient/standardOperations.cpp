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

#include "standardOperations.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <openreports.h>
#include "standardOperation.h"

/*
 *  Constructs a standardOperations as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
standardOperations::standardOperations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_stdopn, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardOperations::~standardOperations()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardOperations::languageChange()
{
    retranslateUi(this);
}


void standardOperations::init()
{
  statusBar()->hide();
  
  if (_privileges->check("MaintainStandardOperations"))
  {
    connect(_stdopn, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_stdopn, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_stdopn, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_stdopn, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _stdopn->addColumn(tr("Number"),      _itemColumn, Qt::AlignLeft );
  _stdopn->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  sFillList();

}

void standardOperations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardOperations::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  int stdopnid = newdlg.exec();

  if (stdopnid != XDialog::Rejected)
    sFillList(stdopnid);
}

void standardOperations::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardOperations::sDelete()
{
  XSqlQuery booitemid;
  booitemid.prepare( "SELECT booitem_id "
                   "FROM booitem "
                   "WHERE (booitem_stdopn_id=:stdopn_id) "
                   "LIMIT 1;" );
  booitemid.bindValue(":stdopn_id", _stdopn->id());
  booitemid.exec();
  if (booitemid.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Standard Operation"),
                           tr( "The selected Standard Operation cannot be deleted as it is currently in use in one or more\n"
                               "Bill of Operations items.  You must remove the selected Standard Operation's use from all \n"
                               "Bills of Operations before you may delete it." ) );
    return;
  }

  XSqlQuery wooperid;
  wooperid.prepare( "SELECT wooper_id "
                    "FROM wooper "
                    "WHERE (wooper_stdopn_id=:stdopn_id) "
                    "LIMIT 1;" );
  wooperid.bindValue(":stdopn_id", _stdopn->id());
  wooperid.exec();
  if (wooperid.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Standard Operation"),
                           tr( "The selected Standard Operation cannot be deleted as it is currently in use in one or more\n"
                               "Work Order Operations.  You must remove close all Work Orders whose Operations use the selected\n"
                               "Standard Operation before you may delete it." ) );
    return;
  }

  XSqlQuery deleteOperation;
  deleteOperation.prepare( "DELETE FROM stdopn "
                           "WHERE (stdopn_id=:stdopn_id);" );
  deleteOperation.bindValue(":stdopn_id", _stdopn->id());
  deleteOperation.exec();
  sFillList();
}

void standardOperations::sFillList()
{
  sFillList(-1);
}

void standardOperations::sFillList(int pStdopnid)
{
  _stdopn->populate( "SELECT stdopn_id, stdopn_number, (stdopn_descrip1 || ' ' || stdopn_descrip2) "
                     "FROM stdopn "
                     "ORDER BY stdopn_number;", pStdopnid );
}

void standardOperations::sPrint()
{
  orReport report("StdOperationsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

