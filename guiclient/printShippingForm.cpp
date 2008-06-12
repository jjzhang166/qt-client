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

#include "printShippingForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "editICMWatermark.h"
#include "mqlutil.h"
#include "salesOrderList.h"

printShippingForm::printShippingForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));
  connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));
  connect(_shipment,	SIGNAL(newId(int)),	this, SLOT(sHandleShipment()));
  connect(_so,	SIGNAL(newId(int)),	this, SLOT(sHandleSo()));
  connect(_to,	SIGNAL(newId(int)),	this, SLOT(sHandleTo()));

  _captive = FALSE;
  _so->setType(cSoAtShipping);

  _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );
  _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());

  if (_shipformNumOfCopies->value())
  {
    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
    {
    _shipformWatermarks->topLevelItem(counter)->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(counter)));
    _shipformWatermarks->topLevelItem(counter)->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(counter))) ? tr("Yes") : tr("No")));
    }
  }

  _to->setVisible(_metrics->boolean("MultiWhs"));
}

printShippingForm::~printShippingForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void printShippingForm::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printShippingForm::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cosmisc_id", &valid);	// deprecated
  if (valid)
  {
    _shipment->setId(param.toInt());
    q.prepare( "SELECT cohead_id, cohead_shiptoname, cohead_shiptoaddress1, cosmisc_shipchrg_id,"
               "       COALESCE(cosmisc_shipform_id, cohead_shipform_id) AS shipform_id "
               "FROM cosmisc, cohead "
               "WHERE ( (cosmisc_cohead_id=cohead_id)"
               " AND (cosmisc_id=:cosmisc_id) );" );
    q.bindValue(":cosmisc_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      _captive = TRUE;

      _so->setId(q.value("cohead_id").toInt());
      _so->setEnabled(FALSE);

      _shipToName->setText(q.value("cohead_shiptoname").toString());
      _shipToAddr1->setText(q.value("cohead_shiptoaddress1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("cosmisc_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _print->setFocus();
  }

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    int orderid = -1;
    QString ordertype;

    _shipment->setId(param.toInt());
    q.prepare( "SELECT shiphead_order_id, shiphead_order_type,"
	       "       shiphead_shipchrg_id, shiphead_shipform_id "
	       "FROM shiphead "
	       "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.first())
    {
      ordertype = q.value("shiphead_order_type").toString();
      orderid = q.value("shiphead_order_id").toInt();
      if (! q.value("shiphead_shipform_id").isNull())
	_shippingForm->setId(q.value("shiphead_shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }

    ParameterList headp;
    if (ordertype == "SO")
    {
      headp.append("sohead_id", orderid);
      _to->setId(-1);
      _so->setId(orderid);
    }
    else if (ordertype == "TO")
    {
      headp.append("tohead_id", orderid);
      _so->setId(-1);
      _to->setId(orderid);
    }

    QString heads = "<? if exists(\"sohead_id\") ?>"
		  "SELECT cohead_id AS order_id, cohead_shiptoname AS shipto,"
		  "      cohead_shiptoaddress1 AS addr1,"
		  "      cohead_shipform_id AS shipform_id "
		  "FROM cohead "
		  "WHERE (cohead_id=<? value(\"sohead_id\") ?>);"
		  "<? elseif exists(\"tohead_id\") ?>"
		  "SELECT tohead_id AS order_id, tohead_destname AS shipto,"
		  "      tohead_destaddress1 AS addr1,"
		  "      tohead_shipform_id AS shipform_id "
		  "FROM tohead "
		  "WHERE (tohead_id=<? value(\"tohead_id\") ?>);"
		  "<? endif ?>"
		  ;
    MetaSQLQuery headm(heads);
    XSqlQuery headq = headm.toQuery(headp);
    if (headq.first())
    {
      _captive = TRUE;

      _shipToName->setText(headq.value("shipto").toString());
      _shipToAddr1->setText(headq.value("addr1").toString());
      if (_shippingForm->id() <= 0)
	_shippingForm->setId(headq.value("shipform_id").toInt());

      _so->setEnabled(false);
      _to->setEnabled(false);
    }
    else if (headq.lastError().type() != QSqlError::None)
    {
      systemError(this, headq.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    _print->setFocus();
  }

  return NoError;
}

void printShippingForm::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    XTreeWidgetItem *last = static_cast<XTreeWidgetItem*>(_shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1));
    for (int counter = (_shipformWatermarks->topLevelItemCount()); counter <= pValue; counter++)
      last = new XTreeWidgetItem(_shipformWatermarks, last, counter, QVariant(tr("Copy #%1").arg(counter)), QVariant(""), QVariant(tr("Yes")));
  }
}

void printShippingForm::sEditShippingFormWatermark()
{
  QList<QTreeWidgetItem*>selected = _shipformWatermarks->selectedItems();
  for (int counter = 0; counter < selected.size(); counter++)
  {
    XTreeWidgetItem *cursor = static_cast<XTreeWidgetItem*>(selected[counter]);
    ParameterList params;
    params.append("watermark", cursor->text(1));
    params.append("showPrices", (cursor->text(2) == tr("Yes")));

    editICMWatermark newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Accepted)
    {
      cursor->setText(1, newdlg.watermark());
      cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
    }
  }
}

void printShippingForm::sPrint()
{
  _print->setFocus();

  if (!_shipment->isValid())
  {
    QMessageBox::warning(this, tr("Shipment Number Required"),
			       tr("<p>You must enter a Shipment Number.") );
    _shipment->setFocus();
    return;
  }

  if (_shippingForm->id() == -1)
  {
    QMessageBox::warning( this, tr("Cannot Print Shipping Form"),
                          tr("<p>You must select a Shipping Form to print.") );
    _shippingForm->setFocus();
    return;
  }

  q.prepare("SELECT report_name "
	    "FROM shipform, report "
	    "WHERE ((shipform_report_id=report_id)"
	    " AND (shipform_id=:shipform_id) );" );
  q.bindValue(":shipform_id", _shippingForm->id());
  q.exec();
  if (q.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool     setupPrinter = TRUE;
    bool userCanceled = false;

    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++ )
    {
      QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
      ParameterList params;
      params.append("cosmisc_id",  _shipment->id()); // for backwards compat
      params.append("shiphead_id", _shipment->id());
      params.append("watermark",   cursor->text(1));
      params.append("shipchrg_id", _shipchrg->id());

      if (_metrics->boolean("MultiWhs"))
	params.append("MultiWhs");

      if (cursor->text(2) == tr("Yes"))
        params.append("showcosts");

      orReport report(q.value("report_name").toString(), params);
      if (report.print(&printer, setupPrinter))
        setupPrinter = FALSE;
      else
      {
        report.reportError(this);
	orReport::endMultiPrint(&printer);
        return;
      }
    }
    orReport::endMultiPrint(&printer);

    q.prepare( "UPDATE shiphead "
               "SET shiphead_sfstatus='P' "
               "WHERE (shiphead_id=:shiphead_id);" );
    q.bindValue(":shiphead_id", _shipment->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (_captive)
      accept();
    else
    {
      _shipment->setId(-1);
      _so->setId(-1);
      _to->setId(-1);
      _so->setEnabled(true);
      _to->setEnabled(true);
      _so->setFocus();
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    systemError(this, tr("<p>The Shipping Head Record cannot be found for the "
			 "selected order."));
}

void printShippingForm::sHandleShipment()
{
  if (_shipment->isValid())
  {
    ParameterList params;
    MetaSQLQuery mql = mqlLoad(":/sr/forms/printShippingForm/HandleShipment.mql");
    params.append("shiphead_id", _shipment->id());
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");
    if (_so->isValid())
      params.append("sohead_id", _so->id());
    if (_to->isValid())
      params.append("tohead_id", _to->id());
    q = mql.toQuery(params);

    if (q.first())
    {
      int orderid = q.value("order_id").toInt();
      if (q.value("shiphead_order_type").toString() == "SO" && _so->id() != orderid)
      {
	_to->setId(-1);
	_so->setId(orderid);
      }
      else if (q.value("shiphead_order_type").toString() == "TO" && _to->id() != orderid)
      {
	_so->setId(-1);
	_to->setId(orderid);
      }

      _shipToName->setText(q.value("shipto").toString());
      _shipToAddr1->setText(q.value("addr1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_so->isValid())
    {
      _so->setId(-1);
      sHandleShipment();
    }
    else if (_to->isValid())
    {
      _to->setId(-1);
      sHandleShipment();
    }
    else
    {
      QMessageBox::critical(this, tr("Could not find data"),
			    tr("<p>Could not find a Sales Order or "
			       "Transfer Order for this Shipment."));

      depopulate();
    }
  }
  else
  {
    depopulate();
  }
}

void printShippingForm::sHandleSo()
{
  if (_so->isValid())
  {
    _to->setId(-1);
    QString sql("SELECT cohead_id AS order_id, cohead_shiptoname AS shipto, "
		"       cohead_shiptoaddress1 AS addr1, shiphead_order_type, "
		"       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
		"	COALESCE(shiphead_shipform_id, cohead_shipform_id) AS shipform_id "
		"FROM cohead, shiphead "
		"WHERE ((cohead_id=shiphead_order_id)"
		"  AND  (shiphead_order_type='SO')"
		"  AND  (cohead_id=<? value(\"sohead_id\") ?> )"
		"<? if exists(\"shiphead_id\") ?>"
		"  AND  (shiphead_id=<? value(\"shiphead_id\") ?> )"
		"<? endif ?>"
		") "
		"ORDER BY shiphead_shipped "
		"LIMIT 1;");

    ParameterList params;
    MetaSQLQuery mql(sql);
    params.append("sohead_id", _so->id());
    if (_shipment->isValid())
      params.append("shiphead_id", _shipment->id());
    q = mql.toQuery(params);

    if (q.first())
    {
      if (_shipment->id() != q.value("shiphead_id").toInt())
	_shipment->setId(q.value("shiphead_id").toInt());

      _shipToName->setText(q.value("shipto").toString());
      _shipToAddr1->setText(q.value("addr1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_shipment->isValid())
    {
      _shipment->setId(-1);
      sHandleSo();
    }
    else
    {
      QMessageBox::critical(this, tr("Could not find data"),
		      tr("<p>Could not find a Shipment for this Sales Order."));

      depopulate();
    }
  }
  else
  {
    depopulate();
  }
}

void printShippingForm::sHandleTo()
{
  if (_to->isValid())
  {
    _so->setId(-1);
    QString sql("SELECT tohead_id AS order_id, tohead_destname AS shipto, "
		"       tohead_destaddress1 AS addr1, shiphead_order_type, "
		"       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
		"	COALESCE(shiphead_shipform_id, tohead_shipform_id) AS shipform_id "
		"FROM tohead, shiphead "
		"WHERE ((tohead_id=shiphead_order_id)"
		"  AND  (shiphead_order_type='TO')"
		"  AND  (tohead_id=<? value(\"tohead_id\") ?> )"
		"<? if exists(\"shiphead_id\") ?>"
		"  AND  (shiphead_id=<? value(\"shiphead_id\") ?> )"
		"<? endif ?>"
		") "
		"ORDER BY shiphead_shipped "
		"LIMIT 1;");

    ParameterList params;
    MetaSQLQuery mql(sql);
    params.append("tohead_id", _to->id());
    if (_shipment->isValid())
      params.append("shiphead_id", _shipment->id());
    q = mql.toQuery(params);

    if (q.first())
    {
      if (_shipment->id() != q.value("shiphead_id").toInt())
	_shipment->setId(q.value("shiphead_id").toInt());

      _shipToName->setText(q.value("shipto").toString());
      _shipToAddr1->setText(q.value("addr1").toString());
      _shippingForm->setId(q.value("shipform_id").toInt());
      _shipchrg->setId(q.value("shiphead_shipchrg_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_shipment->isValid())
    {
      _shipment->setId(-1);
      sHandleTo();
    }
    else
    {
      QMessageBox::critical(this, tr("Could not find data"),
		  tr("<p>Could not find a Shipment for this Transfer Order."));

      depopulate();
    }
  }
  else
  {
    depopulate();
  }
}

void printShippingForm::depopulate()
{
  _shipment->removeOrderLimit();
  //_shipment->clear();
  _shipToName->clear();
  _shipToAddr1->clear();
  _shippingForm->setId(-1);
  _shipchrg->setId(-1);
}
