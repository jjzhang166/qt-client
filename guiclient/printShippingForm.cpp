/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printShippingForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "mqlutil.h"

printShippingForm::printShippingForm(QWidget *parent, const char *name, Qt::WFlags fl)
    : printMulticopyDocument("ShippingFormCopies",     "ShippingFormWatermark",
                             "ShippingFormShowPrices", "",
                             parent, name, false, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("P");
  setReportKey("shiphead_id");
  _distributeInventory = false;

  _docinfoQueryString = 
             "SELECT shiphead_id               AS docid,"
             "       shiphead_number           AS docnumber,"
             "       (shiphead_sfstatus = 'P') AS printed,"
             "       NULL AS posted,"
             "       shipform_report_name      AS reportname,"
             "       shiphead_order_id,    shiphead_order_type,"
             "       shiphead_shipchrg_id,"
             "<? if exists('shipformid') ?>"
             "       <? value('shipformid') ?> AS shipform_id"
             "  FROM shiphead"
             "  JOIN shipform ON (shiphead_shipform_id=<? value('shipformid') ?>)"
             "<? else ?>"
             "       shipform_id"
             "  FROM shiphead"
             "  JOIN shipform ON (shiphead_shipform_id=shipform_id)"
             "<? endif ?>"
             " WHERE (shiphead_id=<? value('docid') ?>);" ;

  _markOnePrintedQry = "UPDATE shiphead"
                       "   SET shiphead_sfstatus='P'"
                       " WHERE (shiphead_id=<? value('docid') ?>);" ;

  // programatically hiding -- see issue # 5853. TODO: remove it altogether?
  _shipchrg->hide();
  _shipchrgLit->hide();

  connect(_shipment,                  SIGNAL(newId(int)), this, SLOT(sHandleShipment()));
  connect(_order, SIGNAL(numberChanged(QString,QString)), this, SLOT(sHandleOrder()));

  _order->setAllowedTypes(OrderLineEdit::Sales | OrderLineEdit::Transfer);
  _order->setLabel("");
  _shipment->setStatus(ShipmentClusterLineEdit::AllStatus);
  _shipment->setStrict(true);
}

printShippingForm::~printShippingForm()
{
  // no need to delete child widgets, Qt does it all for us
}

void printShippingForm::languageChange()
{
  retranslateUi(this);
}

void printShippingForm::clear()
{
  depopulate();

  _shipment->setId(-1);
  _order->setId(-1);
  _order->setEnabled(true);
  _order->setFocus();
}

ParameterList printShippingForm::getParamsOneCopy(const int row, XSqlQuery *qry)
{
  ParameterList params = printMulticopyDocument::getParamsOneCopy(row, qry);

  params.append("shipchrg_id", _shipchrg->id());

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  return params;
}

bool printShippingForm::isOkToPrint()
{
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(!_shipment->isValid(), _shipment,
                         tr("You must enter a Shipment Number."))
        << GuiErrorCheck(_shippingForm->id() == -1, _shippingForm,
                         tr("You must select a Shipping Form to print."))
    ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Print Shipping Form"),
                                  errors))
    return false;

  return true;
}

// TODO: is there a better way than overriding ::populate()?
void printShippingForm::populate()
{
  int orderid = -1;
  QString ordertype;

  _shipment->setId(id());
  XSqlQuery getq;
  getq.prepare("SELECT shiphead_order_id, shiphead_order_type,"
               "       shiphead_shipchrg_id, shiphead_shipform_id "
               "FROM shiphead "
               "WHERE (shiphead_id=:shiphead_id);" );
  getq.bindValue(":shiphead_id", _shipment->id());
  getq.exec();
  if (getq.first())
  {
    ordertype = getq.value("shiphead_order_type").toString();
    orderid = getq.value("shiphead_order_id").toInt();
    if (! getq.value("shiphead_shipform_id").isNull())
      _shippingForm->setId(getq.value("shiphead_shipform_id").toInt());
    _shipchrg->setId(getq.value("shiphead_shipchrg_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Shipment"),
                                getq, __FILE__, __LINE__))
    return;
  
  ParameterList headp;
  if (ordertype == "SO")
  {
    headp.append("sohead_id", orderid);
    _order->setId(orderid, "SO");
  }
  else if (ordertype == "TO")
  {
    headp.append("tohead_id", orderid);
    _order->setId(orderid,"TO");
  }

  QString heads = "<? if exists('sohead_id') ?>"
                "SELECT cohead_id AS order_id, cohead_shiptoname AS shipto,"
                "      cohead_shiptoaddress1 AS addr1,"
                "      cohead_shipform_id AS shipform_id "
                "FROM cohead "
                "WHERE (cohead_id=<? value('sohead_id') ?>);"
                "<? elseif exists('tohead_id') ?>"
                "SELECT tohead_id AS order_id, tohead_destname AS shipto,"
                "      tohead_destaddress1 AS addr1,"
                "      tohead_shipform_id AS shipform_id "
                "FROM tohead "
                "WHERE (tohead_id=<? value('tohead_id') ?>);"
                "<? endif ?>"
                ;
  MetaSQLQuery headm(heads);
  XSqlQuery headq = headm.toQuery(headp);
  if (headq.first())
  {
    _shipToName->setText(headq.value("shipto").toString());
    _shipToAddr1->setText(headq.value("addr1").toString());
    if (_shippingForm->id() <= 0)
      _shippingForm->setId(headq.value("shipform_id").toInt());

    _order->setEnabled(false);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Order"),
                                headq, __FILE__, __LINE__))
    return;
}

void printShippingForm::sHandleShipment()
{
  if (_shipment->isValid())
  {
    setId(_shipment->id());

    bool         ok = false;
    QString      errmsg;
    MetaSQLQuery sfm = MQLUtil::mqlLoad("shippingForm", "shipment",
                                        errmsg, &ok);
    if (! ok)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Getting Shipping Form"),
                           errmsg, __FILE__, __LINE__);
      return;
    }

    ParameterList params;
    params.append("shiphead_id", _shipment->id());
    if (_metrics->boolean("MultiWhs"))
      params.append("MultiWhs");
    if (_order->isValid() && _order->isSO())
      params.append("sohead_id", _order->id());
    if (_order->isValid() && _order->isTO())
      params.append("tohead_id", _order->id());

    XSqlQuery sfq = sfm.toQuery(params);
    if (sfq.first())
    {
      int orderid = sfq.value("order_id").toInt();
      if ((sfq.value("shiphead_order_type").toString() == "SO") &&
          ((_order->id() != orderid) || (!_order->isSO())))
        _order->setId(orderid, "SO");
      else if ((sfq.value("shiphead_order_type").toString() == "TO") &&
               ((_order->id() != orderid) || (!_order->isTO())))
        _order->setId(orderid,"TO");

      _shipToName->setText(sfq.value("shipto").toString());
      _shipToAddr1->setText(sfq.value("addr1").toString());
      _shippingForm->setId(sfq.value("shipform_id").toInt());
      _shipchrg->setId(sfq.value("shiphead_shipchrg_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Getting Shipping Form"),
                                  sfq, __FILE__, __LINE__))
      return;
    else
    {
      QMessageBox::critical(this, tr("Could not find data"),
                            tr("<p>Could not find shipment on this order."));

      _shipment->setId(-1);
    }
  }
  else
  {
    depopulate();
  }
}

void printShippingForm::sHandleSo()
{
  _shipment->clear();
  _shipment->setType(ShipmentClusterLineEdit::SalesOrder);
  _shipment->limitToOrder(_order->id());

  QString sql("SELECT cohead_id AS order_id, cohead_shiptoname AS shipto, "
              "       cohead_shiptoaddress1 AS addr1, shiphead_order_type, "
              "       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
              "	COALESCE(shiphead_shipform_id, cohead_shipform_id) AS shipform_id "
              "FROM cohead, shiphead "
              "WHERE ((cohead_id=shiphead_order_id)"
              "  AND  (shiphead_order_type='SO')"
              "  AND  (cohead_id=<? value('sohead_id') ?> )"
              "<? if exists('shiphead_id') ?>"
              "  AND  (shiphead_id=<? value('shiphead_id') ?> )"
              "<? endif ?>"
              ") "
              "ORDER BY shiphead_shipped "
              "LIMIT 1;");

  ParameterList params;
  MetaSQLQuery mql(sql);
  params.append("sohead_id", _order->id());
  if (_shipment->isValid())
    params.append("shiphead_id", _shipment->id());
  XSqlQuery soq = mql.toQuery(params);

  if (soq.first())
  {
    if (_shipment->id() != soq.value("shiphead_id").toInt())
    {
      setId(soq.value("shiphead_id").toInt());
      _shipment->setId(soq.value("shiphead_id").toInt());
    }

    _shipToName->setText(soq.value("shipto").toString());
    _shipToAddr1->setText(soq.value("addr1").toString());
    _shippingForm->setId(soq.value("shipform_id").toInt());
    _shipchrg->setId(soq.value("shiphead_shipchrg_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Sales Order"),
                                soq, __FILE__, __LINE__))
    return;
  else
    depopulate();
}

void printShippingForm::sHandleOrder()
{
  if (_order->isSO())
    sHandleSo();
  else if (_order->isTO())
    sHandleTo();
  else
  {
    _shipment->removeOrderLimit();
    _shipment->clear();
    depopulate();
  }
}

void printShippingForm::sHandleTo()
{
  _shipment->clear();
  _shipment->setType(ShipmentClusterLineEdit::TransferOrder);
  _shipment->limitToOrder(_order->id());

  QString sql("SELECT tohead_id AS order_id, tohead_destname AS shipto, "
              "       tohead_destaddress1 AS addr1, shiphead_order_type, "
              "       shiphead_id, shiphead_shipchrg_id, shiphead_shipped, "
              "	COALESCE(shiphead_shipform_id, tohead_shipform_id) AS shipform_id "
              "FROM tohead, shiphead "
              "WHERE ((tohead_id=shiphead_order_id)"
              "  AND  (shiphead_order_type='TO')"
              "  AND  (tohead_id=<? value('tohead_id') ?> )"
              "<? if exists('shiphead_id') ?>"
              "  AND  (shiphead_id=<? value('shiphead_id') ?> )"
              "<? endif ?>"
              ") "
              "ORDER BY shiphead_shipped "
              "LIMIT 1;");

  ParameterList params;
  MetaSQLQuery mql(sql);
  params.append("tohead_id", _order->id());
  if (_shipment->isValid())
    params.append("shiphead_id", _shipment->id());
  XSqlQuery toq = mql.toQuery(params);

  if (toq.first())
  {
    if (_shipment->id() != toq.value("shiphead_id").toInt())
    {
      setId(toq.value("shiphead_id").toInt());
      _shipment->setId(toq.value("shiphead_id").toInt());
    }

    _shipToName->setText(toq.value("shipto").toString());
    _shipToAddr1->setText(toq.value("addr1").toString());
    _shippingForm->setId(toq.value("shipform_id").toInt());
    _shipchrg->setId(toq.value("shiphead_shipchrg_id").toInt());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Order"),
                                toq, __FILE__, __LINE__))
    return;
  else
    depopulate();
}

void printShippingForm::depopulate()
{
  //_shipment->clear();
  _shipToName->clear();
  _shipToAddr1->clear();
  _shippingForm->setId(-1);
  _shipchrg->setId(-1);
}
