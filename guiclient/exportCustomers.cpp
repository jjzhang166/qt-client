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

#include "exportCustomers.h"

#include <QVariant>
#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "customer.h"

/*
 *  Constructs a exportCustomers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
exportCustomers::exportCustomers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_cust, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
exportCustomers::~exportCustomers()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void exportCustomers::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void exportCustomers::init()
{
  _cust->addColumn(tr("Number"),     _itemColumn, Qt::AlignLeft   );
  _cust->addColumn(tr("Name"),       200,         Qt::AlignLeft   );
  _cust->addColumn(tr("Address"),    -1,          Qt::AlignLeft   );
  _cust->addColumn(tr("Date Added"), _dateColumn, Qt::AlignCenter );

  connect(omfgThis, SIGNAL(customersUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  sFillList();
}

void exportCustomers::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Customer..."), this, SLOT(sView()), 0);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Mark as Exported..."), this, SLOT(sMarkAsExported()), 0);
}

void exportCustomers::sView()
{
  ParameterList params;
  params.append("cust_id", _cust->id());
  params.append("mode", "view");

  customer *newdlg = new customer();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void exportCustomers::sMarkAsExported()
{
  q.prepare( "UPDATE custinfo "
             "SET cust_exported=TRUE "
             "WHERE (cust_id=:cust_id);" );
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  sFillList();
}

void exportCustomers::sFillList()
{
  sFillList(-1, TRUE);
}

void exportCustomers::sFillList(int, bool)
{
  _cust->clear();

  q.prepare( "SELECT cust_id, cust_number, cust_name, cust_address1,"
             "       formatDate(cust_dateadded) AS f_dateadded,"
             "       COALESCE( ( SELECT aropen_id"
             "                   FROM aropen "
             "                   WHERE ( (NOT aropen_posted)"
             "                    AND (aropen_cust_id=cust_id) )"
             "                   LIMIT 1 ), 0) AS _aropenid "
             "FROM cust "
             "WHERE (NOT cust_exported) "
             "ORDER BY cust_number;" );
  q.exec();
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _cust, last, q.value("cust_id").toInt(),
			       q.value("cust_number"),
			       q.value("cust_name"),
			       q.value("cust_address1"),
			       q.value("f_dateadded") );

    if (q.value("_aropenid").toInt() != 0)
      last->setTextColor("red");
  }

  _cust->setDragString("custid=");
}
