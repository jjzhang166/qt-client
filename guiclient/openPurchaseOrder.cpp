/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openPurchaseOrder.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QPushButton>

#include <parameter.h>
#include <errorReporter.h>

//#include "purchaseOrder.h"
openPurchaseOrder::openPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _select->setEnabled(false);
  connect(_po,     SIGNAL(valid(bool)),       _select, SLOT(setEnabled(bool)));
  connect(_po,     SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_select, SIGNAL(clicked()),         this,    SLOT(sSelect()));
  connect(_close,  SIGNAL(clicked()),         this,    SLOT(reject()));
  _po->addColumn(tr("PO #"),         -1,  Qt::AlignLeft,   true,  "pohead_number");
  _po->addColumn(tr("PO Date"),      -1,  Qt::AlignLeft,   true,  "pohead_orderdate");
  _po->addColumn(tr("Created By"),   -1,  Qt::AlignLeft,   true,  "pohead_agent_username");
  _po->addColumn(tr("Drop Ship"),    -1,  Qt::AlignLeft,   true,  "pohead_dropship");
  
  vendor_id = -1;
  sohead_id = -1;
  dropship = false;
}

openPurchaseOrder::~openPurchaseOrder()
{
  // no need to delete child widgets, Qt does it all for us

}

void openPurchaseOrder::languageChange()
{
 // retranslateUi(this);
}

void openPurchaseOrder::sSelect()
{
  done(_po->id());
}

enum SetResponse openPurchaseOrder::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;
  param = pParams.value("vend_name", &valid);
  if (valid)
    _vend_namelit->setText(param.toString());
  
  param = pParams.value("vend_id", &valid);
  if (valid)
    vendor_id = param.toInt();

  param = pParams.value("sohead_id", &valid);
  if (valid)
    sohead_id = param.toInt();
  
  param = pParams.value("drop_ship", &valid);
  if (valid)
    dropship = param.toBool();
  
  sFillList();

  return NoError;
}

void openPurchaseOrder::sFillList()
{
  XSqlQuery openFillList;
  
  openFillList.prepare( "SELECT * "
                        "FROM pohead "
                        "WHERE (pohead_vend_id = :vend_id)"
                        "  AND (pohead_status = 'U')"
                        "  AND (pohead_dropship = :dropship) "
                        "  AND (NOT pohead_dropship OR (pohead_cohead_id = :cohead_id)) "
                        "ORDER BY pohead_id DESC;" );
  openFillList.bindValue(":vend_id", vendor_id);
  openFillList.bindValue(":cohead_id", sohead_id);
  openFillList.bindValue(":dropship", dropship);
  openFillList.exec();
  _po->populate(openFillList,true);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Order Information"),
                                openFillList, __FILE__, __LINE__))
  {
    return;
  }
}
