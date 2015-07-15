/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "firmPlannedOrder.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>

firmPlannedOrder::firmPlannedOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_firm, SIGNAL(clicked()), this, SLOT(sFirm()));
  _quantity->setValidator(omfgThis->qtyVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

firmPlannedOrder::~firmPlannedOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

void firmPlannedOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse firmPlannedOrder::set(const ParameterList &pParams)
{
  XSqlQuery firmet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _planordid = param.toInt();
    _item->setReadOnly(true);

    firmet.prepare( "SELECT planord.*, (planord_duedate - planord_startdate) AS leadtime "
               "FROM planord "
               "WHERE (planord_id=:planord_id);" );
    firmet.bindValue(":planord_id", _planordid);
    firmet.exec();
    if (firmet.first())
    {
      _item->setItemsiteid(firmet.value("planord_itemsite_id").toInt());
      _quantity->setDouble(firmet.value("planord_qty").toDouble());
      _dueDate->setDate(firmet.value("planord_duedate").toDate());
      _comments->setText(firmet.value("planord_comments").toString());
      _number = firmet.value("planord_number").toInt();
      _itemsiteid = firmet.value("planord_itemsite_id").toInt();
      _leadTime = firmet.value("leadtime").toInt();

      _type = firmet.value("planord_type").toString();  
      if (firmet.value("planord_type").toString() == "P")
        _orderType->setText(tr("Purchase Order"));
      else if (firmet.value("planord_type").toString() == "W")
        _orderType->setText(tr("Work Order"));
    }
    else if (firmet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, firmet.lastError().databaseText(), __FILE__, __LINE__);
      reject();
    }
  }

  return NoError;
}

void firmPlannedOrder::sFirm()
{
  XSqlQuery firmFirm;
  firmFirm.prepare( "UPDATE planord "
             "SET planord_firm=true, "
             "    planord_comments=:planord_comments, "
             "    planord_qty=:planord_qty, "
             "    planord_duedate=:planord_dueDate, "
             "    planord_startdate=(DATE(:planord_dueDate) - :planord_leadTime) "
             "WHERE (planord_id=:planord_id);" );
  firmFirm.bindValue(":planord_qty", _quantity->toDouble());
  firmFirm.bindValue(":planord_dueDate", _dueDate->date());
  firmFirm.bindValue(":planord_leadTime", _leadTime);
  firmFirm.bindValue(":planord_comments", _comments->toPlainText());
  firmFirm.bindValue(":planord_id", _planordid);
  firmFirm.exec();
  if (firmFirm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, firmFirm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  firmFirm.prepare( "SELECT explodePlannedOrder( :planord_id, true) AS result;" );
  firmFirm.bindValue(":planord_id", _planordid);
  firmFirm.exec();
  if (firmFirm.first())
  {
    double result = firmFirm.value("result").toDouble();
    if (result < 0.0)
    {
      systemError(this, tr("ExplodePlannedOrder returned %, indicating an "
                           "error occurred.").arg(result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (firmFirm.lastError().type() != QSqlError::NoError)
  {
    systemError(this, firmFirm.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_planordid);
}
