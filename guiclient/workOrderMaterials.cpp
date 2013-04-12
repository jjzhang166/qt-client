/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "workOrderMaterials.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "dspInventoryAvailability.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "inputManager.h"
#include "returnWoMaterialItem.h"
#include "storedProcErrorLookup.h"
#include "substituteList.h"
#include "woMaterialItem.h"

workOrderMaterials::workOrderMaterials(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_womatl, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);
  _pickNumber->setPrecision(omfgThis->qtyVal());
  _nonPickNumber->setPrecision(omfgThis->qtyVal());
  _totalNumber->setPrecision(omfgThis->qtyVal());
  _pickQtyPer->setPrecision(omfgThis->qtyPerVal());
  _nonPickQtyPer->setPrecision(omfgThis->qtyPerVal());
  _totalQtyPer->setPrecision(omfgThis->qtyPerVal());
  _currentActCost->setPrecision(omfgThis->costVal());
  _currentStdCost->setPrecision(omfgThis->costVal());
  _maxCost->setPrecision(omfgThis->costVal());
  
  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Component Item"), _itemColumn,  Qt::AlignLeft,   true,  "item_number");
  _womatl->addColumn(tr("Description"),    -1,           Qt::AlignLeft,   true,  "description");
  _womatl->addColumn(tr("Iss. Meth."),     _orderColumn, Qt::AlignCenter, true,  "issuemethod");
  _womatl->addColumn(tr("Iss. UOM"),       _uomColumn,   Qt::AlignLeft,   true,  "uom_name");
  _womatl->addColumn(tr("Fxd. Qty."),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd");
  _womatl->addColumn(tr("Qty. Per"),       _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper");
  _womatl->addColumn(tr("Scrap %"),        _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap");
  _womatl->addColumn(tr("Required"),       _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq");
  _womatl->addColumn(tr("Issued"),         _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss");
  _womatl->addColumn(tr("Scrapped"),       _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap");
  _womatl->addColumn(tr("Balance"),        _qtyColumn,   Qt::AlignRight,  true,  "balance");
  _womatl->addColumn(tr("Due Date"),       _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate");
  _womatl->addColumn(tr("Notes"),          _itemColumn,  Qt::AlignLeft,   false, "womatl_notes");
  _womatl->addColumn(tr("Reference"),      _itemColumn,  Qt::AlignLeft,   false, "womatl_ref");
   
  if (_privileges->check("MaintainWoMaterials"))
  {
    connect(_womatl, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_womatl, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_womatl, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_womatl, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_womatl, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (!_privileges->check("ViewCosts"))
  {
    _currentStdCostLit->hide();
    _currentActCostLit->hide();
    _maxCostLit->hide();
    _currentStdCost->hide();
    _currentActCost->hide();
    _maxCost->hide();
  }

  connect(omfgThis, SIGNAL(workOrderMaterialsUpdated(int, int, bool)), this, SLOT(sCatchMaterialsUpdated(int, int, bool)));
}

workOrderMaterials::~workOrderMaterials()
{
    // no need to delete child widgets, Qt does it all for us
}

void workOrderMaterials::languageChange()
{
  retranslateUi(this);
}

enum SetResponse workOrderMaterials::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  return NoError;
}

void workOrderMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));
  
  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDelete()));
  
  pMenu->addSeparator();
  
  menuItem = pMenu->addAction(tr("View Availability..."), this, SLOT(sViewAvailability()));
  
  pMenu->addSeparator();
  
  menuItem = pMenu->addAction(tr("View Item-Defined Subsitute Availability..."), this, SLOT(sViewSubstituteAvailability()));
  menuItem = pMenu->addAction(tr("Substitute..."), this, SLOT(sSubstitute()));
}

void workOrderMaterials::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("wo_id", _wo->id());

  woMaterialItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderMaterials::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("womatl_id", _womatl->id());

  woMaterialItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderMaterials::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("womatl_id", _womatl->id());

  woMaterialItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void workOrderMaterials::sDelete()
{
  XSqlQuery workDelete;
  int womatlid = _womatl->id();
  if (_womatl->currentItem()->rawValue("womatl_qtyiss").toDouble() > 0)
  {
    if(_privileges->check("ReturnWoMaterials"))
    {
      if (QMessageBox::question(this, tr("W/O Material Requirement cannot be Deleted"),
				tr("<p>This W/O Material Requirement cannot "
				   "be deleted as it has has material issued "
				   "to it. You must return this material to "
				   "stock before you can delete this Material "
				   "Requirement. Would you like to return this "
				   "material to stock now?"  ),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
      {
        ParameterList params;
        params.append("womatl_id", womatlid);

        returnWoMaterialItem newdlg(omfgThis, "", TRUE);
        newdlg.set(params);

        newdlg.exec();
        sFillList();

        workDelete.prepare("SELECT womatl_qtyiss AS qtyissued "
                  "FROM womatl "
                  "WHERE (womatl_id=:womatl_id) ");
        workDelete.bindValue(":womatl_id", womatlid);
        workDelete.exec();
        if (!workDelete.first() || workDelete.value("qtyissued").toInt() != 0)
          return;
      }
      else
        return;
    }
    else
    {
      QMessageBox::critical( this, tr("W/O Material Requirement cannot be Deleted"),
                             tr("<p>This W/O Material Requirement cannot be "
				"deleted as it has material issued to it. "
                                "You must return this material to stock before "
				"you can delete this Material Requirement." ) );
      return;
    }
  }

  workDelete.prepare("SELECT deleteWoMaterial(:womatl_id);");
  workDelete.bindValue(":womatl_id", womatlid);
  workDelete.exec();
  if (workDelete.first())
  {
    int result = workDelete.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteWoMaterial", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (workDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, workDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatl->id(), TRUE);
}

void workOrderMaterials::sViewAvailability()
{
  XSqlQuery workViewAvailability;
  workViewAvailability.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  workViewAvailability.bindValue(":womatl_id", _womatl->id());
  workViewAvailability.exec();
  if (workViewAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", workViewAvailability.value("womatl_itemsite_id"));
    params.append("byDate", workViewAvailability.value("womatl_duedate"));
    params.append("run");

    dspInventoryAvailability *newdlg = new dspInventoryAvailability();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (workViewAvailability.lastError().type() != QSqlError::NoError)
  {
    systemError(this, workViewAvailability.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void workOrderMaterials::sViewSubstituteAvailability()
{
  XSqlQuery workViewSubstituteAvailability;
  workViewSubstituteAvailability.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  workViewSubstituteAvailability.bindValue(":womatl_id", _womatl->id());
  workViewSubstituteAvailability.exec();
  if (workViewSubstituteAvailability.first())
  {
    ParameterList params;
    params.append("itemsite_id", workViewSubstituteAvailability.value("womatl_itemsite_id"));
    params.append("byDate", workViewSubstituteAvailability.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (workViewSubstituteAvailability.lastError().type() != QSqlError::NoError)
  {
    systemError(this, workViewSubstituteAvailability.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void workOrderMaterials::sSubstitute()
{
  XSqlQuery workSubstitute;
  int womatlid = _womatl->id();

  XSqlQuery sub;
  sub.prepare( "SELECT womatl.*,"
               "       itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyper) AS qtyper,"
               "       itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyfxd) AS qtyfxd "
               "FROM womatl JOIN itemsite ON (womatl_itemsite_id=itemsite_id) "
               "WHERE (womatl_id=:womatl_id);" );
  sub.bindValue(":womatl_id", womatlid);
  sub.exec();
  if (sub.first())
  {
    ParameterList params;
    params.append("womatl_id", womatlid);
    params.append("byDate", sub.value("womatl_duedate"));
    params.append("run");

    substituteList substitute(this, "", TRUE);
    substitute.set(params);
    int result = substitute.exec();
    if (result != XDialog::Rejected)
    {
      ParameterList params;
      params.append("mode", "new");
      params.append("wo_id", sub.value("womatl_wo_id"));
      params.append("bomitem_id", sub.value("womatl_bomitem_id"));
      params.append("item_id", result);
      params.append("qtyFxd", (sub.value("qtyfxd").toDouble() * substitute._uomratio));
      params.append("qtyPer", (sub.value("qtyper").toDouble() * substitute._uomratio));
      params.append("scrap", (sub.value("womatl_scrap").toDouble() * 100.0));
      params.append("notes", sub.value("womatl_notes"));
      params.append("reference", sub.value("womatl_ref"));
      params.append("picklist", sub.value("womatl_picklist"));

      if (sub.value("womatl_issuemethod").toString() == "S")
        params.append("issueMethod", "push");
      else if (sub.value("womatl_issuemethod").toString() == "L")
        params.append("issueMethod", "pull");
      else if (sub.value("womatl_issuemethod").toString() == "M")
        params.append("issueMethod", "mixed");

      woMaterialItem newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() != XDialog::Rejected)
      {
        workSubstitute.prepare( "DELETE FROM womatl "
                   "WHERE (womatl_id=:womatl_id);" );
        workSubstitute.bindValue(":womatl_id", womatlid);
        workSubstitute.exec();

        omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatl->id(), TRUE);
      }
    }
  }
  else if (workSubstitute.lastError().type() != QSqlError::NoError)
  {
    systemError(this, workSubstitute.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void workOrderMaterials::sCatchMaterialsUpdated(int pWoid, int, bool)
{
  if (pWoid == _wo->id())
    sFillList();
}

void workOrderMaterials::sFillList()
{
  XSqlQuery workFillList;
  if (_wo->isValid())
  {
    workFillList.prepare( "SELECT womatl_id, *,"
               "       (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       CASE WHEN (womatl_issuemethod = 'S') THEN :push"
               "            WHEN (womatl_issuemethod = 'L') THEN :pull"
               "            WHEN (womatl_issuemethod = 'M') THEN :mixed"
               "            ELSE :error"
               "       END AS issuemethod,"
               "       'qty' AS womatl_qtyfxd_xtnumericrole,"
               "       'qtyper' AS womatl_qtyper_xtnumericrole,"
               "       'percent' AS womatl_scrap_xtnumericrole,"
               "       'qty' AS womatl_qtyreq_xtnumericrole,"
               "       'qty' AS womatl_qtyiss_xtnumericrole,"
               "       'qty' AS womatl_qtywipscrap_xtnumericrole,"
               "       noNeg(womatl_qtyreq - womatl_qtyiss) AS balance,"
               "       'qty' AS balance_xtnumericrole,"
               "       CASE WHEN (womatl_duedate <= CURRENT_DATE) THEN 'expired'"
               "       END AS womatl_duedate_qtforegroundrole "
               "FROM wo, womatl, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (wo_id=:wo_id) ) "
               "ORDER BY item_number;" );
    workFillList.bindValue(":wo_id", _wo->id());
    workFillList.bindValue(":push",  tr("Push"));
    workFillList.bindValue(":pull",  tr("Pull"));
    workFillList.bindValue(":mixed", tr("Mixed"));
    workFillList.bindValue(":error", tr("Error"));
    workFillList.exec();
    _womatl->populate(workFillList);
    if (workFillList.lastError().type() != QSqlError::NoError)
    {
      systemError(this, workFillList.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    workFillList.prepare( "SELECT item_picklist,"
               "       COUNT(*) AS total,"
               "       COALESCE(SUM(womatl_qtyper * (1 + womatl_scrap))) AS qtyper "
               "FROM womatl, itemsite, item "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_wo_id=:wo_id) ) "
               "GROUP BY item_picklist;" );
    workFillList.bindValue(":wo_id", _wo->id());
    workFillList.exec();
    bool   foundPick    = FALSE;
    bool   foundNonPick = FALSE;
    int    totalNumber  = 0;
    double totalQtyPer  = 0.0;
    while (workFillList.next())
    {
      totalNumber += workFillList.value("total").toInt();
      totalQtyPer += workFillList.value("qtyper").toDouble();

      if (workFillList.value("item_picklist").toBool())
      {
        foundPick = TRUE;
        _pickNumber->setText(workFillList.value("total").toDouble());
        _pickQtyPer->setText(formatQtyPer(workFillList.value("qtyper").toDouble()));
      }
      else
      {
        foundNonPick = TRUE;
        _nonPickNumber->setText(workFillList.value("total").toDouble());
        _nonPickQtyPer->setText(formatQtyPer(workFillList.value("qtyper").toDouble()));
      }
    }
    if (workFillList.lastError().type() != QSqlError::NoError)
    {
      systemError(this, workFillList.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (!foundPick)
    {
      _pickNumber->setText(QString("0").toDouble());
      _pickQtyPer->setText(formatQtyPer(0.0));
    }

    if (!foundNonPick)
    {
      _nonPickNumber->setText(QString("0").toDouble());
      _nonPickQtyPer->setText(formatQtyPer(0.0));
    }

    _totalNumber->setText(QString("%1").arg(totalNumber).toDouble());
    _totalQtyPer->setText(totalQtyPer);

    if (_privileges->check("ViewCosts"))
    {
      workFillList.prepare( "SELECT p.item_maxcost AS f_maxcost,"
                 "       COALESCE(SUM(itemuomtouom(ci.itemsite_item_id, womatl_uom_id, NULL, (womatl_qtyfxd + womatl_qtyper) * (1 + womatl_scrap)) * stdCost(c.item_id))) AS f_stdcost,"
                 "       COALESCE(SUM(itemuomtouom(ci.itemsite_item_id, womatl_uom_id, NULL, (womatl_qtyfxd + womatl_qtyper) * (1 + womatl_scrap)) * actCost(c.item_id))) AS f_actcost "
                 "FROM wo, womatl, itemsite AS ci, itemsite AS pi, item AS c, item AS p "
                 "WHERE ( (womatl_wo_id=wo_id)"
                 " AND (womatl_itemsite_id=ci.itemsite_id)"
                 " AND (ci.itemsite_item_id=c.item_id)"
                 " AND (wo_itemsite_id=pi.itemsite_id)"
                 " AND (pi.itemsite_item_id=p.item_id)"
                 " AND (wo_id=:wo_id) ) "
                 "GROUP BY p.item_maxcost;" );
      workFillList.bindValue(":wo_id", _wo->id());
      workFillList.exec();
      if (workFillList.first())
      {
        _currentStdCost->setText(workFillList.value("f_stdcost").toDouble());
        _currentActCost->setText(workFillList.value("f_actcost").toDouble());
        _maxCost->setText(workFillList.value("f_maxcost").toDouble());
      }
      else if (workFillList.lastError().type() != QSqlError::NoError)
      {
	systemError(this, workFillList.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }
  else
  {
    _pickNumber->clear();
    _pickQtyPer->clear();
    _nonPickNumber->clear();
    _nonPickQtyPer->clear();
    _totalNumber->clear();
    _totalQtyPer->clear();
  }
}
