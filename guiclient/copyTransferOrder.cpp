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

#include "copyTransferOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "storedProcErrorLookup.h"
#include "transferOrder.h"

copyTransferOrder::copyTransferOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_copy,	SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_to,	       SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = FALSE;

  _item->addColumn(tr("#"),           _seqColumn,     Qt::AlignCenter );
  _item->addColumn(tr("Item"),        _itemColumn,    Qt::AlignLeft   );
  _item->addColumn(tr("Description"), -1,             Qt::AlignLeft   );
  _item->addColumn(tr("Ordered"),     _qtyColumn,     Qt::AlignRight  );
}

copyTransferOrder::~copyTransferOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyTransferOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyTransferOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _to->setId(param.toInt());
    _to->setEnabled(FALSE);

    _copy->setFocus();
  }

  return NoError;
}

void copyTransferOrder::populate()
{
  _item->clear();
  if (_to->id() == -1)
    _orderDate->clear();
  else
  {
    q.prepare( "SELECT tohead_orderdate "
              "FROM tohead "
              "WHERE (tohead_id=:tohead_id);" );
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setDate(q.value("tohead_orderdate").toDate());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("SELECT toitem_id,"
	      "       toitem_linenumber, item_number,"
	      "       (item_descrip1 || ' ' || item_descrip2) AS description,"
	      "       formatQty(toitem_qty_ordered) AS f_ordered "
	      "FROM item, toitem "
	      "WHERE ((toitem_item_id=item_id)"
	      "  AND  (toitem_tohead_id=:tohead_id)) "
	      "ORDER BY toitem_linenumber;" );
    q.bindValue(":tohead_id", _to->id());
    q.exec();
    _item->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void copyTransferOrder::sCopy()
{
  q.prepare("SELECT copyTransferOrder(:tohead_id, :scheddate) AS result;");
  q.bindValue(":tohead_id",	_to->id());

  if (_reschedule->isChecked())
    q.bindValue(":scheddate", _scheduleDate->date());
  else
    q.bindValue(":scheddate", QDate::currentDate());

  int toheadid = 0;

  q.exec();
  if (q.first())
  {
    toheadid = q.value("result").toInt();
    if (toheadid < 0)
    {
      QMessageBox::critical(this, tr("Could Not Copy Transfer Order"),
			  storedProcErrorLookup("copyTransferOrder", toheadid));
      return;
    }
    transferOrder::editTransferOrder(toheadid, true);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sTransferOrdersUpdated(toheadid);
  if (_captive)
    done(toheadid);
  else
  {
    _to->setId(-1);
    _to->setFocus();
  }
}
