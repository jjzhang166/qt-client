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

#include "copyPurchaseOrder.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "purchaseOrderList.h"
#include "storedProcErrorLookup.h"

copyPurchaseOrder::copyPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_close,	SIGNAL(clicked()),	this,	SLOT(reject()));
    connect(_copy,	SIGNAL(clicked()),	this,	SLOT(sCopy()));
    connect(_po,	SIGNAL(newId(int)),	this,	SLOT(sPopulatePoInfo(int)));
    connect(_po,	SIGNAL(requestList()),	this,	SLOT(sPoList()));
    connect(_poList,	SIGNAL(clicked()),	this,	SLOT(sPoList()));
    connect(_reschedule,SIGNAL(toggled(bool)),	_scheduleDate,	SLOT(setEnabled(bool)));

    _captive = FALSE;

#ifdef Q_WS_MAC
    _poList->setMaximumWidth(50);
#else
    _poList->setMaximumWidth(25);
#endif

    //omfgThis->inputManager()->notify(cBCPurchaseOrder, this, _po, SLOT(setId(int)));

    _poitem->addColumn(tr("#"),           _seqColumn,     Qt::AlignCenter );
    _poitem->addColumn(tr("Item"),        _itemColumn,    Qt::AlignLeft   );
    _poitem->addColumn(tr("Description"), -1,             Qt::AlignLeft   );
    _poitem->addColumn(tr("Whs."),        _whsColumn,     Qt::AlignCenter );
    _poitem->addColumn(tr("Ordered"),     _qtyColumn,     Qt::AlignRight  );
    _poitem->addColumn(tr("Price"),       _qtyColumn,     Qt::AlignRight  );
    _poitem->addColumn(tr("Extended"),    _qtyColumn,     Qt::AlignRight  );

    _currency->setType(XComboBox::Currencies);
    _currency->setLabel(_currencyLit);
}

copyPurchaseOrder::~copyPurchaseOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

void copyPurchaseOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse copyPurchaseOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _po->setId(param.toInt());
    _po->setEnabled(FALSE);
    _poList->setEnabled(FALSE);

    _copy->setFocus();
  }

  return NoError;
}

void copyPurchaseOrder::sPoList()
{
  ParameterList params;
  params.append("pohead_id", _po->id());
  
  purchaseOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _po->setId(newdlg.exec());
}

void copyPurchaseOrder::sPopulatePoInfo(int)
{
  if (_po->id() != -1)
  {
    q.prepare( "SELECT formatDate(pohead_orderdate) AS orderdate,"
              "        vend_id, vend_phone1, pohead_curr_id "
              "FROM pohead, vend "
              "WHERE ( (pohead_vend_id=vend_id)"
              " AND (pohead_id=:pohead_id) );" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    if (q.first())
    {
      _orderDate->setText(q.value("orderdate").toString());
      _vend->setId(q.value("vend_id").toInt());
      _vendPhone->setText(q.value("vend_phone1").toString());
      _currency->setId(q.value("pohead_curr_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT poitem_id, poitem_linenumber,"
               "       COALESCE(item_number,poitem_vend_item_number),"
               "       COALESCE((item_descrip1 || ' ' || item_descrip2),poitem_vend_item_descrip),"
               "       warehous_code,"
               "       formatQty(poitem_qty_ordered),"
               "       formatSalesPrice(poitem_unitprice),"
               "       formatMoney(poitem_qty_ordered * poitem_unitprice / poitem_invvenduomratio) "
               "FROM poitem LEFT OUTER JOIN "
               "      (itemsite JOIN item ON (itemsite_item_id=item_id) "
               "                JOIN warehous ON (itemsite_warehous_id=warehous_id) ) "
               "      ON (poitem_itemsite_id=itemsite_id) "
               "WHERE (poitem_pohead_id=:pohead_id) "
               "ORDER BY poitem_linenumber;" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
    _poitem->populate(q);
  }
  else
  {
    _orderDate->clear();
    _vend->setId(-1);
    _vendPhone->clear();
    _poitem->clear();
  }
}

void copyPurchaseOrder::sCopy()
{
  q.prepare("SELECT copyPo(:pohead_id, :vend_id, :scheddate, :recheck) AS pohead_id;");
  q.bindValue(":pohead_id",	_po->id());
  q.bindValue(":vend_id",	_vend->id());

  if (_reschedule->isChecked())
    q.bindValue(":scheddate", _scheduleDate->date());
  else
    q.bindValue(":scheddate", QDate::currentDate());

  q.bindValue(":recheck", QVariant(_recheck->isChecked(), 0));

  int poheadid = 0;

  q.exec();
  if (q.first())
  {
    poheadid = q.value("pohead_id").toInt();
    if (poheadid < 0)
    {
      QMessageBox::critical(this, tr("Could Not Copy Purchase Order"),
			    storedProcErrorLookup("copyPo", poheadid));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_captive)
  {
    omfgThis->sPurchaseOrdersUpdated(poheadid, true);
    done(poheadid);
  }
  else
  {
    _po->setId(-1);
    _po->setFocus();
  }
}
