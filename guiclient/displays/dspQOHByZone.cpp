/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQOHByZone.h"

#include <QAction>
#include <QMenu>
#include <QVariant>
#include <QMessageBox>

#include "adjustmentTrans.h"
#include "enterMiscCount.h"
#include "transferTrans.h"
#include "createCountTagsByItem.h"
#include "dspInventoryLocator.h"
#include "parameterwidget.h"

dspQOHByZone::dspQOHByZone(QWidget* parent, const char*, Qt::WindowFlags fl)
    : display(parent, "dspQOHByZone", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Quantities on Hand By Zone"));
  setReportName("QOHByZone");
  setMetaSQLOptions("qoh", "detailByZone");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  QString _zoneSQL ("SELECT whsezone_id, warehous_code||' :- '||whsezone_name||' - '||whsezone_descrip "
	            "FROM whsezone "
                    "JOIN whsinfo ON (warehous_id=whsezone_warehous_id) "
                    " ORDER BY warehous_code, whsezone_name;");

  //if (_metrics->boolean("MultiWhs"))
  //  parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site, "", true);
  // TODO filter Zone based on Site selection
  parameterWidget()->appendComboBox(tr("Zone"), "whsezone_id", _zoneSQL, "", true);

  if (_metrics->boolean("MultiWhs"))
    list()->addColumn(tr("Site"),               _whsColumn,  Qt::AlignCenter, true,  "warehous_code" );
  list()->addColumn(tr("Zone"),               _whsColumn,  Qt::AlignCenter, true,  "whsezone_name" );
  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "item_descrip1"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Location"),           _itemColumn, Qt::AlignLeft,   false, "location_descrip" );
  list()->addColumn(tr("QOH"),                _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  list()->addColumn(tr("Aisle"),              -1,          Qt::AlignLeft,   false, "location_aisle"  );
  list()->addColumn(tr("Rack"),               -1,          Qt::AlignLeft,   false, "location_rack"  );
  list()->addColumn(tr("Bin"),                -1,          Qt::AlignLeft,   false, "location_bin"  );

}

void dspQOHByZone::languageChange()
{
  display::languageChange();
  retranslateUi(this);
}

SetResponse dspQOHByZone::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  parameterWidget()->setSavedFilters();

  param = pParams.value("whsezone_id", &valid);
  if (valid)
    parameterWidget()->setDefault(tr("Zone"), param);

  parameterWidget()->applyDefaultFilterSet();

  return NoError;
}

void dspQOHByZone::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
  {
    QAction *menuItem;
  
    if (((XTreeWidgetItem *)pSelected)->altId())
    {
      pMenu->addAction(tr("View Location/Lot/Serial # Detail..."), this, SLOT(sViewDetail()));
      pMenu->addSeparator();
    }

    if (_metrics->boolean("MultiWhs"))
    {
      menuItem = pMenu->addAction(tr("Transfer to another Site..."), this, SLOT(sTransfer()));;
      if (!_privileges->check("CreateInterWarehouseTrans"))
        menuItem->setEnabled(false);
    }

    menuItem = pMenu->addAction(tr("Adjust this Quantity..."), this, SLOT(sAdjust()));;
    if (!_privileges->check("CreateAdjustmentTrans"))
      menuItem->setEnabled(false);

    menuItem = pMenu->addAction(tr("Reset this Quantity to 0..."), this, SLOT(sReset()));;
    if (!_privileges->check("CreateAdjustmentTrans"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Enter Misc. Count..."), this, SLOT(sMiscCount()));;
    if (!_privileges->check("EnterMiscCounts"))
      menuItem->setEnabled(false);

    pMenu->addSeparator();

    menuItem = pMenu->addAction(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()));;
    if (!_privileges->check("IssueCountTags"))
      menuItem->setEnabled(false);
  } 
}

bool dspQOHByZone::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  if (!params.inList("whsezone_id"))
  {
    QMessageBox::warning( this, tr("Missing Selection"),
                           tr("<p>Please select a Warehouse Zone") );
    return false;
  }
  return true;
}

void dspQOHByZone::sViewDetail()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  params.append("run");

  dspInventoryLocator *newdlg = new dspInventoryLocator();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByZone::sTransfer()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  transferTrans *newdlg = new transferTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByZone::sAdjust()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByZone::sReset()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("itemsite_id", list()->id());
  params.append("qty", 0.0);

  adjustmentTrans *newdlg = new adjustmentTrans();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQOHByZone::sMiscCount()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());

  enterMiscCount newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec())
    sFillList();
}

void dspQOHByZone::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", list()->id());
  
  createCountTagsByItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
