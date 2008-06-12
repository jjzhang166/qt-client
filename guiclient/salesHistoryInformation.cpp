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

#include "salesHistoryInformation.h"

#include <qvariant.h>
#include <qvalidator.h>

/*
 *  Constructs a salesHistoryInformation as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
salesHistoryInformation::salesHistoryInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    init();

    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
salesHistoryInformation::~salesHistoryInformation()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void salesHistoryInformation::languageChange()
{
    retranslateUi(this);
}


void salesHistoryInformation::init()
{
  _orderNumber->setValidator(omfgThis->orderVal());
  _invoiceNumber->setValidator(omfgThis->orderVal());
  _shipped->setValidator(omfgThis->qtyVal());
  _unitPrice->setValidator(omfgThis->priceVal());
  _unitCost->setValidator(omfgThis->priceVal());
  _commission->setValidator(omfgThis->negMoneyVal());

  _salesrep->setType(XComboBox::SalesReps);
}

enum SetResponse salesHistoryInformation::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohist_id", &valid);
  if (valid)
  {
    _sohistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit")
    {
      _mode = cEdit;
      _orderNumber->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _orderNumber->setEnabled(FALSE);
      _invoiceNumber->setEnabled(FALSE);
      _orderDate->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _billtoName->setEnabled(FALSE);
      _billtoAddress1->setEnabled(FALSE);
      _billtoAddress2->setEnabled(FALSE);
      _billtoAddress3->setEnabled(FALSE);
      _billtoCity->setEnabled(FALSE);
      _billtoState->setEnabled(FALSE);
      _billtoZip->setEnabled(FALSE);
      _shiptoName->setEnabled(FALSE);
      _shiptoAddress1->setEnabled(FALSE);
      _shiptoAddress2->setEnabled(FALSE);
      _shiptoAddress3->setEnabled(FALSE);
      _shiptoCity->setEnabled(FALSE);
      _shiptoState->setEnabled(FALSE);
      _shiptoZip->setEnabled(FALSE);
      _shipped->setEnabled(FALSE);
      _unitPrice->setEnabled(FALSE);
      _unitCost->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _salesrep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _commissionPaid->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void salesHistoryInformation::sSave()
{
  q.prepare( "UPDATE cohist "
             "SET cohist_ordernumber=:cohist_ordernumber, cohist_invcnumber=:cohist_invcnumber,"
             "    cohist_orderdate=:cohist_orderdate, cohist_invcdate=:cohist_invcdate,"
             "    cohist_billtoname=:cohist_billtoname, cohist_billtoaddress1=:cohist_billtoaddress1,"
             "    cohist_billtoaddress2=cohist_billtoaddress2, cohist_billtoaddress3=:cohist_billtoaddress3,"
             "    cohist_billtocity=:cohist_billtocity, cohist_billtostate=:cohist_billtostate,"
             "    cohist_billtozip=:cohist_billtozip,"
             "    cohist_shiptoname=:cohist_shiptoname, cohist_shiptoaddress1=:cohist_shiptoaddress1,"
             "    cohist_shiptoaddress2=:cohist_shiptoaddress2, cohist_shiptoaddress3=:cohist_shiptoaddress3,"
             "    cohist_shiptocity=:cohist_shiptocity, cohist_shiptostate=:cohist_shiptostate,"
             "    cohist_shiptozip=:cohist_shiptozip,"
             "    cohist_itemsite_id=itemsite_id,"
             "    cohist_qtyshipped=:cohist_qtyshipped,"
             "    cohist_unitprice=:cohist_unitprice, cohist_unitcost=:cohist_unitcost,"
             "    cohist_salesrep_id=:cohist_salesrep_id, cohist_commission=:cohist_commission,"
             "    cohist_commissionpaid=:cohist_commissionpaid "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id)"
             " AND (cohist_id=:cohist_id) );" );

  q.bindValue(":cohist_id", _sohistid);
  q.bindValue(":cohist_ordernumber", _orderNumber->text().toInt());
  q.bindValue(":cohist_invcnumber", _invoiceNumber->text().toInt());
  q.bindValue(":cohist_orderdate", _orderDate->date());
  q.bindValue(":cohist_invcdate", _invoiceDate->date());
  q.bindValue(":cohist_billtoname", _billtoName->text().stripWhiteSpace());
  q.bindValue(":cohist_billtoaddress1", _billtoAddress1->text().stripWhiteSpace());
  q.bindValue(":cohist_billtoaddress2", _billtoAddress2->text().stripWhiteSpace());
  q.bindValue(":cohist_billtoaddress3", _billtoAddress3->text().stripWhiteSpace());
  q.bindValue(":cohist_billtocity", _billtoCity->text().stripWhiteSpace());
  q.bindValue(":cohist_billtostate", _billtoState->text().stripWhiteSpace());
  q.bindValue(":cohist_billtozip", _billtoZip->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptoname", _shiptoName->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptoaddress1", _shiptoAddress1->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptoaddress2", _shiptoAddress2->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptoaddress3", _shiptoAddress3->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptocity", _shiptoCity->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptostate", _shiptoState->text().stripWhiteSpace());
  q.bindValue(":cohist_shiptozip", _shiptoZip->text().stripWhiteSpace());
  q.bindValue(":cohist_qtyshipped", _shipped->toDouble());
  q.bindValue(":cohist_unitprice", _unitPrice->toDouble());
  q.bindValue(":cohist_unitcost", _unitCost->toDouble());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":cohist_salesrep_id", _salesrep->id());
  q.bindValue(":cohist_commission", _commission->toDouble());
  q.bindValue(":cohist_commissionpaid", QVariant(_commissionPaid->isChecked(), 0));
  q.exec();

  done(_sohistid);
}

void salesHistoryInformation::populate()
{
  q.prepare( "SELECT cohist_ordernumber, cohist_invcnumber,"
             "       cohist_orderdate, cohist_invcdate,"
             "       cohist_billtoname, cohist_billtoaddress1,"
             "       cohist_billtoaddress2, cohist_billtoaddress3,"
             "       cohist_billtocity, cohist_billtostate, cohist_billtozip,"
             "       cohist_shiptoname, cohist_shiptoaddress1,"
             "       cohist_shiptoaddress2, cohist_shiptoaddress3,"
             "       cohist_shiptocity, cohist_shiptostate, cohist_shiptozip,"
             "       cohist_itemsite_id, cohist_salesrep_id, cohist_commissionpaid,"
             "       cohist_qtyshipped, cohist_unitprice, cohist_unitcost,"
             "       formatMoney(cohist_qtyshipped * cohist_unitprice) AS extprice,"
             "       formatMoney(cohist_qtyshipped * cohist_unitcost) AS extcost,"
             "       formatMoney(cohist_commission) AS f_commission "
             "FROM cohist "
             "WHERE (cohist_id=:sohist_id);" );
  q.bindValue(":sohist_id", _sohistid);
  q.exec();
  if (q.first())
  {
    _orderNumber->setText(q.value("cohist_ordernumber"));
    _invoiceNumber->setText(q.value("cohist_invcnumber"));
    _orderDate->setDate(q.value("cohist_orderdate").toDate());
    _invoiceDate->setDate(q.value("cohist_invcdate").toDate());
    _billtoName->setText(q.value("cohist_billtoname"));
    _billtoAddress1->setText(q.value("cohist_billtoaddress1"));
    _billtoAddress2->setText(q.value("cohist_billtoaddress2"));
    _billtoAddress3->setText(q.value("cohist_billtoaddress3"));
    _billtoCity->setText(q.value("cohist_billtocity"));
    _billtoState->setText(q.value("cohist_billtostate"));
    _billtoZip->setText(q.value("cohist_billtozip"));
    _shiptoName->setText(q.value("cohist_shiptoname"));
    _shiptoAddress1->setText(q.value("cohist_shiptoaddress1"));
    _shiptoAddress2->setText(q.value("cohist_shiptoaddress2"));
    _shiptoAddress3->setText(q.value("cohist_shiptoaddress3"));
    _shiptoCity->setText(q.value("cohist_shiptocity"));
    _shiptoState->setText(q.value("cohist_shiptostate"));
    _shiptoZip->setText(q.value("cohist_shiptozip"));

    _item->setItemsiteid(q.value("cohist_itemsite_id").toInt());
    _shipped->setText(formatQty(q.value("cohist_qtyshipped").toDouble()));
    _unitPrice->setText(formatSalesPrice(q.value("cohist_unitprice").toDouble()));
    _unitCost->setText(formatSalesPrice(q.value("cohist_unitcost").toDouble()));
    _extendedPrice->setText(q.value("extprice").toString());
    _extendedCost->setText(q.value("extcost").toString());
    _salesrep->setId(q.value("cohist_salesrep_id").toInt());
    _commission->setText(q.value("f_commission"));
    _commissionPaid->setChecked(q.value("cohist_commissionpaid").toBool());
  }
}

