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

#include "closePurchaseOrder.h"

#include <QVariant>

/*
 *  Constructs a closePurchaseOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
closePurchaseOrder::closePurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_closePo, SIGNAL(clicked()), this, SLOT(sClosePo()));

  _captive = FALSE;

  _po->addColumn(tr("Number"), _orderColumn, Qt::AlignRight  );
  _po->addColumn(tr("Vendor"), -1,           Qt::AlignLeft   );
  _po->addColumn(tr("Agent"),  _itemColumn,  Qt::AlignCenter );
  _po->addColumn(tr("Order Date"),  _dateColumn,    Qt::AlignLeft  );
  _po->addColumn(tr("First Item"),  _itemColumn,    Qt::AlignLeft  );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
closePurchaseOrder::~closePurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void closePurchaseOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse closePurchaseOrder::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _closePo->setFocus();
  }

  return NoError;
}

void closePurchaseOrder::sClosePo()
{
  q.prepare("SELECT closePo(:pohead_id) AS result;");

  QList<QTreeWidgetItem*>selected = _po->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":pohead_id", ((XTreeWidgetItem*)selected[i])->id());
    q.exec();
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));
    sFillList();
  }
}

void closePurchaseOrder::sFillList()
{
  QString sql( "SELECT pohead_id, pohead_number,"
               "       vend_name, pohead_agent_username,"
               "       pohead_orderdate,"
               "       item_number "
               "FROM vend, pohead LEFT OUTER JOIN"
               "     poitem ON (poitem_pohead_id=pohead_id AND poitem_linenumber=1)"
               "     LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
               "     LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
               "  AND   (pohead_status='O') "
               " ) "
               "ORDER BY pohead_number DESC;" );
  _po->populate(sql);
}

