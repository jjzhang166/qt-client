/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printPackingList.h"

#include <QMessageBox>
#include <QVariant>

#include <metasql.h>
#include <mqlutil.h>
#include <xsqlquery.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "inputManager.h"

/* printPackingList is a subclass of printSinglecopyDocument that
   overrides most of the behavior of its parent. The logic of this
   window is so complex that it's hard to know how to simplify it
   to better fit the printSinglecopyDocument model.
   TODO: figure out how to simplify it anyway. use orderhead table?
 */
class printPackingListPrivate
{
  public:
    printPackingListPrivate(::printPackingList *parent) :
      _shipformid(-1)
    {
      Q_UNUSED(parent);
    }

    int          _shipformid;
};

printPackingList::printPackingList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());
  _pldata = new printPackingListPrivate(this);

  setDoctype("P");

  // a simple singleton select allows using printSinglecopyDocument::sPrint()
  _docinfoQueryString = "SELECT <? value('docid') ?>     AS docid,"
                        "       <? value('docnumber') ?> AS docnumber,"
                        "       false                    AS printed,"
                        "       <? value('reportname' ?> AS reportname"
                        ";" ;

  connect(_order,     SIGNAL(valid(bool)), this, SLOT(sPopulate()));
  connect(_reprint, SIGNAL(toggled(bool)), this, SLOT(sHandleReprint()));
  connect(_shipment,   SIGNAL(newId(int)), this, SLOT(sHandleShipment()));

  _order->setAllowedStatuses(OrderLineEdit::Open);
  _order->setAllowedTypes(OrderLineEdit::Sales | OrderLineEdit::Transfer);
  _order->setFromSitePrivsEnforced(TRUE);
  _shipment->setStatus(ShipmentClusterLineEdit::Unshipped);

  _orderDate->setEnabled(false);

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _order, SLOT(setId(int)));
}

printPackingList::~printPackingList()
{
  if (_pldata)
  {
    delete _pldata;
    _pldata = 0;
  }
}

void printPackingList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printPackingList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
    _order->setId(param.toInt(), "SO");
  }

  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
    _order->setId(param.toInt(), "TO");
  }

  QString ordertype;
  param = pParams.value("head_type", &valid);
  if (valid)
    ordertype = param.toString();

  param = pParams.value("head_id", &valid);
  if (valid)
  {
    if (ordertype.isEmpty())
      return UndefinedError;
    else
	{
	  _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
      _order->setId(param.toInt(), ordertype);
	}
  }

  param = pParams.value("shiphead_id", &valid);
  if (valid)
  {
    _shipment->setId(param.toInt());
    XSqlQuery shipq;
    shipq.prepare("SELECT shiphead_order_type, shiphead_order_id, shiphead_shipform_id "
                  "FROM shiphead "
                  "WHERE shiphead_id=:shiphead_id;");
    shipq.bindValue(":shiphead_id", param);
    shipq.exec();
    if (shipq.first())
    {
      _pldata->_shipformid = shipq.value("shiphead_shipform_id").toInt();
      ordertype            = shipq.value("shiphead_order_type").toString();
      if (ordertype == "SO" || ordertype == "TO")
	  {
	    _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
        _order->setId(shipq.value("shiphead_order_id").toInt(), ordertype);
	  }
      else
        return UndefinedError;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Shipment"),
                                  shipq, __FILE__, __LINE__))
      return UndefinedError;
  }
  sHandleReprint();
  setId(_order->id());

  return printSinglecopyDocument::set(pParams);
}

ParameterList printPackingList::getParams(XSqlQuery *docq)
{
  Q_UNUSED(docq);
  // unlike other printSinglecopyDocuments, don't call parent::getParams
  ParameterList params;

  setReportKey(reportKey());
  params.append("head_id",    _order->id());
  if (_order->isSO())
    params.append("head_type",  "SO");
  else if (_order->isTO())
    params.append("head_type",  "TO");
  params.append(reportKey(),  _order->id());
  params.append("docid",      _order->id());
  params.append("form",       doctype());

  if (_pldata->_shipformid > 0 && ! _printPick->isChecked())
    params.append("shipformid", _pldata->_shipformid);

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (_metrics->boolean("EnableReturnAuth"))
    params.append("EnableReturnAuth");

  if (_shipment->id() > 0)
    params.append("shiphead_id", _shipment->id());

  if (_metrics->boolean("EnableSOReservations"))
    params.append("EnableSOReservations");

  if (_metrics->boolean("EnableSOReservationsByLocation"))
    params.append("EnableSOReservationsByLocation");

  QString msg;
  bool    valid = false;
  MetaSQLQuery formm = MQLUtil::mqlLoad("packingList", "getreport",
                                        msg, &valid);
  if (! valid)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Form"),
                         msg, __FILE__, __LINE__);
  XSqlQuery formq = formm.toQuery(params);
  if (formq.first())
    params.append("reportname", formq.value("reportname").toString());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Form"),
                                formq, __FILE__, __LINE__))
  {
    params.clear();
    return params;
  }

  return params;
}

ParameterList printPackingList::getParamsDocList()
{
  // unlike other printSinglecopyDocuments
  ParameterList params = getParams(0);
  return params;
}

bool printPackingList::isOkToPrint()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _order->isValid() && _order->type() == "SO",
                          _order,
                          tr("You must enter a Sales Order Number"))
         << GuiErrorCheck(! _order->isValid() && _order->type() == "TO",
                          _order,
                          tr("You must enter a Transfer Order Number"))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Print"), errors))
    return false;

  return true;
}

QString printPackingList::reportKey()
{
  if (_order->isSO())
    return "sohead_id";
  else if (_order->isTO())
    return "tohead_id";

  return "";
}

void printPackingList::sPopulate()
{
  if (_order->isSO() && _order->isValid())
  {
    _shipment->setType(ShipmentClusterLineEdit::SalesOrder);
    _shipment->limitToOrder(_order->id());
  }
  else if (_order->isTO() && _order->isValid())
  {
    _shipment->setType(ShipmentClusterLineEdit::TransferOrder);
    _shipment->limitToOrder(_order->id());
  }
  else
  {
    _shipment->setType(ShipmentClusterLineEdit::All);
    _shipment->removeOrderLimit();
  }

  setPrintEnabled(_order->isValid());

  if (! _order->type().isEmpty())
  {
    ParameterList destp;
    if (_order->isSO())
      destp.append("sohead_id", _order->id());
    else if (_order->isTO())
      destp.append("tohead_id", _order->id());

    QString dests = "<? if exists('sohead_id') ?>"
                    "SELECT cohead_number AS order_number,"
                    "       cohead_orderdate AS orderdate,"
                    "       cohead_custponumber AS alternate_number,"
                    "       cust_name AS name, cntct_phone AS phone "
                    "FROM cohead "
                    " JOIN custinfo ON (cohead_cust_id=cust_id)"
                    " LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
                    "WHERE ( (cohead_cust_id=cust_id)"
                    " AND (cohead_id=<? value('sohead_id') ?>) );"
                    "<? elseif exists('tohead_id') ?>"
                    "SELECT tohead_number AS order_number,"
                    "       tohead_orderdate AS orderdate,"
                    "       '' AS alternate_number,"
                    "       tohead_destname AS name, tohead_destphone AS phone "
                    "FROM tohead "
                    "WHERE (tohead_id=<? value('tohead_id') ?>);"
                    "<? endif ?>"
                    ;
    MetaSQLQuery destm(dests);
    XSqlQuery destq = destm.toQuery(destp);
    if (destq.first())
    {
      _orderDate->setDate(destq.value("orderdate").toDate());
      _poNumber->setText(destq.value("alternate_number").toString());
      _custName->setText(destq.value("name").toString());
      _custPhone->setText(destq.value("phone").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Recipient Information"),
                                  destq, __FILE__, __LINE__))
      return;

    ParameterList params;
    params.append("order_id", _order->id());
    params.append("ordertype", _order->type());
    if (_shipment->id() > 0)
      params.append("shiphead_id", _shipment->id());

    MetaSQLQuery mql("SELECT shiphead_id "
                     "FROM shiphead "
                     "WHERE ((shiphead_order_id=<? value('order_id') ?>) "
                     "  AND  (shiphead_order_type = <? value('ordertype') ?>)"
                     "<? if exists('shiphead_id') ?>"
                     "  AND (shiphead_id=<? value('shiphead_id') ?>)"
                     "<? else ?>"
                     "  AND  (NOT shiphead_shipped) "
                     "<? endif ?>"
                     ") "
                     "ORDER BY shiphead_number "
                     "LIMIT 1;");
    XSqlQuery shipq = mql.toQuery(params);
    if (shipq.first())
    {
      if (shipq.value("shiphead_id").toInt() != _shipment->id())
        _shipment->setId(shipq.value("shiphead_id").toInt());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Shipment"),
                                  shipq, __FILE__, __LINE__))
      return;
    else
    {
      _shipment->setId(-1);
    }
  }
  else
  {
    _shipment->removeOrderLimit();
    _shipment->clear();
    _orderDate->clear();
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
    _pldata->_shipformid = -1;
  }
}

void printPackingList::clear()
{
  _order->setId(-1);
  _order->setFocus();
}

QString printPackingList::doctype()
{
  if (_printPack->isChecked())
    return "P";
  else if (_printPick->isChecked())
    return "L";
  else
    return (_shipment->id() > 0) ? "P" : "L";
}

void printPackingList::sHandleShipment()
{
  if (! _shipment->isValid())
    return;

  ParameterList params;
  params.append("shiphead_id", _shipment->id());
  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  QString msg;
  bool    valid = false;
  MetaSQLQuery mql = MQLUtil::mqlLoad("packingList", "shipment", msg, &valid);
  XSqlQuery plq = mql.toQuery(params);
  if (plq.first())
  {
    _pldata->_shipformid = plq.value("shiphead_shipform_id").toInt();
    QString ordertype = plq.value("shiphead_order_type").toString();
    int     orderid   = plq.value("shiphead_order_id").toInt();
    if (ordertype == "SO" && ! _order->isValid())
      _order->setId(orderid);
    else if (ordertype == "TO" && ! _order->isValid())
      _order->setId(orderid);
    else if (orderid != _order->id())
    {
      if (ordertype == "SO")
      {
        if (QMessageBox::question(this, tr("Shipment for different Order"),
                                        tr("<p>Shipment %1 is for Sales Order %2. "
                                           "Are you sure the Shipment Number is correct?")
                                       .arg(_shipment->number())
                                       .arg(plq.value("number").toString()),
                                  QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          _order->setId(plq.value("shiphead_order_id").toInt());
        else
        {
          _pldata->_shipformid = -1;
          _shipment->clear();
        }
      }
      else if (ordertype == "TO")
      {
        if (QMessageBox::question(this, tr("Shipment for different Order"),
                                        tr("<p>Shipment %1 is for Transfer Order %2. "
                                           "Are you sure the Shipment Number is correct?")
                                       .arg(_shipment->number())
                                       .arg(plq.value("number").toString()),
                                  QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
          _order->setId(plq.value("shiphead_order_id").toInt());
        else
        {
          _pldata->_shipformid = -1;
          _shipment->clear();
        }
      }
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Finding Packing List Data"),
                                plq, __FILE__, __LINE__))
    return;
}

void printPackingList::sHandleReprint()
{
  if (_reprint->isChecked())
  {
    _order->setAllowedStatuses(OrderLineEdit::AnyStatus);
    _shipment->setStatus(ShipmentClusterLineEdit::AllStatus);
  }
  else
  {
    _order->setAllowedStatuses(OrderLineEdit::Open);
    _shipment->setStatus(ShipmentClusterLineEdit::Unshipped);
  }
}
