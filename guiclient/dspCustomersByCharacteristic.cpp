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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "dspCustomersByCharacteristic.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "rptCustomersByCharacteristic.h"
#include "customer.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a dspCustomersByCharacteristic as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCustomersByCharacteristic::dspCustomersByCharacteristic(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_cust, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCustomersByCharacteristic::~dspCustomersByCharacteristic()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCustomersByCharacteristic::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspCustomersByCharacteristic::init()
{
  _char->populate( "SELECT char_id, char_name "
                   "FROM char "
                   "WHERE (char_customers) "
                   "ORDER BY char_name;" );

  _cust->addColumn(tr("Number"),         _itemColumn, Qt::AlignLeft   );
  _cust->addColumn(tr("Name"),           -1,          Qt::AlignLeft   );
  _cust->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignCenter );
  _cust->addColumn(tr("Value"),          _itemColumn, Qt::AlignLeft   );

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));
}

void dspCustomersByCharacteristic::setParams(ParameterList &params)
{
  params.append("char_id", _char->id());
  params.append("value", _value->text());

  if (_showInactive->isChecked())
    params.append("showInactive");
  if (_emptyValue->isChecked())
    params.append("emptyValue");
  if(_charIsSet->isChecked())
    params.append("hasCharacteristic");
}

void dspCustomersByCharacteristic::sPrint()
{
  ParameterList params;
  setParams(params);
  params.append("print");

  rptCustomersByCharacteristic newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspCustomersByCharacteristic::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainCustomerMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspCustomersByCharacteristic::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cust_id", _cust->id());

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCustomersByCharacteristic::sFillList()
{
  sFillList(-1, TRUE);
}

void dspCustomersByCharacteristic::sFillList(int pCustid, bool pLocal)
{
  QString sql = "SELECT cust_id, cust_number, cust_name, char_name, "
		"<? if exists(\"hasCharacteristic\") ?>"
		"       charass_value "
		"FROM cust, charass, char "
		"WHERE ( (charass_target_type='C')"
		" AND (charass_target_id=cust_id)"
		" AND (charass_char_id=char_id)"
		" AND (char_id=<? value(\"char_id\") ?>)"
		" <? if exists(\"emptyValue\") ?>"
		" AND (charass_value IS NULL OR LENGTH(TRIM(charass_value)) = 0)"
		" <? else ?>"
		" AND (charass_value ~* <? value(\"value\") ?>) "
		" <? endif ?>"
		") "
		"<? else ?>"	// if does not have characteristic
		"       '' AS charass_value "
		"FROM cust, char "
		"WHERE ((cust_id NOT IN (SELECT charass_target_id"
		"                        FROM charass"
		"                        WHERE ((charass_target_type='C')"
		"                          AND  (charass_char_id=char_id))"
		"       ))"
		" AND (char_id=<? value(\"char_id\") ?>)"
		") "
		"<? endif ?>"
		"<? if not exists(\"showInactive\") ?>"
		" AND (cust_active)"
		"<? endif ?>"
	        "ORDER BY cust_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);

  if ((pCustid != -1) && (pLocal))
    _cust->populate(q, pCustid);
  else
    _cust->populate(q);

  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _cust->setDragString("custid=");
}
