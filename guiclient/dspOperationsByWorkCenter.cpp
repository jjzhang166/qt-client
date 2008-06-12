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

#include "dspOperationsByWorkCenter.h"

#include <QVariant>
#include <QWorkspace>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "boo.h"
#include "booItem.h"

/*
 *  Constructs a dspOperationsByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspOperationsByWorkCenter::dspOperationsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_booitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );
	
  _booitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _booitem->addColumn(tr("Seq #"),       _seqColumn,  Qt::AlignCenter );
  _booitem->addColumn(tr("Std. Oper."),  _itemColumn, Qt::AlignLeft   );
  _booitem->addColumn(tr("Description"), -1,          Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList()));
	
  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspOperationsByWorkCenter::~dspOperationsByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspOperationsByWorkCenter::languageChange()
{
    retranslateUi(this);
}

void dspOperationsByWorkCenter::sPrint()
{
  ParameterList params;
  params.append("wrkcnt_id", _wrkcnt->id());

  orReport report("OperationsByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspOperationsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if (!_privileges->check("ViewBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Bill of Operations..."), this, SLOT(sViewBOO()), 0);
  if (!_privileges->check("ViewBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Bill of Operations..."), this, SLOT(sEditBOO()), 0);
  if (!_privileges->check("MaintainBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspOperationsByWorkCenter::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOperationsByWorkCenter::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOperationsByWorkCenter::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _booitem->altId());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspOperationsByWorkCenter::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _booitem->altId());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspOperationsByWorkCenter::sFillList()
{
  q.prepare( "SELECT wrkcnt_descrip, warehous_code "
             "FROM wrkcnt, warehous "
             "WHERE ( (wrkcnt_warehous_id=warehous_id)"
             " AND (wrkcnt_id=:wrkcnt_id) );" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  if (q.first())
  {
    _description->setText(q.value("wrkcnt_descrip").toString());
    _warehouse->setText(q.value("warehous_code").toString());
  }

  q.prepare( "SELECT booitem_id, item_id,"
             "       item_number, booitem_seqnumber,"
             "       stdopn_number, (booitem_descrip1 || ' ' || booitem_descrip2) "
             "FROM booitem JOIN item ON (booitem_item_id=item_id) "
             " LEFT OUTER JOIN stdopn ON (booitem_stdopn_id=stdopn_id) "
             "WHERE (booitem_wrkcnt_id=:wrkcnt_id) "
             "ORDER BY item_number, booitem_seqnumber;" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  _booitem->populate(q, TRUE );
}
