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

#include "workOrderMaterials.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "dspInventoryAvailabilityByItem.h"
#include "dspSubstituteAvailabilityByItem.h"
#include "inputManager.h"
#include "returnWoMaterialItem.h"
#include "storedProcErrorLookup.h"
#include "substituteList.h"
#include "woMaterialItem.h"

workOrderMaterials::workOrderMaterials(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_womatl, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Component Item"), _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _womatl->addColumn(tr("Description"),    -1,           Qt::AlignLeft,  true, "description");
  _womatl->addColumn(tr("Iss. Meth."),     _orderColumn, Qt::AlignCenter, true, "issuemethod");
  _womatl->addColumn(tr("Iss. UOM"),       _uomColumn,   Qt::AlignLeft,  true, "uom_name");
  _womatl->addColumn(tr("Qty. Per"),       _qtyColumn,   Qt::AlignRight, true, "womatl_qtyper");
  _womatl->addColumn(tr("Scrap %"),        _prcntColumn, Qt::AlignRight, true, "womatl_scrap");
  _womatl->addColumn(tr("Required"),       _qtyColumn,   Qt::AlignRight, true, "womatl_qtyreq");
  _womatl->addColumn(tr("Issued"),         _qtyColumn,   Qt::AlignRight, true, "womatl_qtyiss");
  _womatl->addColumn(tr("Scrapped"),       _qtyColumn,   Qt::AlignRight, true, "womatl_qtywipscrap");
  _womatl->addColumn(tr("Balance"),        _qtyColumn,   Qt::AlignRight, true, "balance");
  _womatl->addColumn(tr("Due Date"),       _dateColumn,  Qt::AlignCenter,true, "womatl_duedate");
  
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

  _wo->setFocus();
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
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _womatl->setFocus();
  }

  return NoError;
}

void workOrderMaterials::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  
  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  
  pMenu->insertSeparator();
  
  menuItem = pMenu->insertItem(tr("View Availability..."), this, SLOT(sViewAvailability()), 0);
  
  pMenu->insertSeparator();
  
  menuItem = pMenu->insertItem(tr("View Item-Defined Subsitute Availability..."), this, SLOT(sViewSubstituteAvailability()), 0);
  menuItem = pMenu->insertItem(tr("Substitute..."), this, SLOT(sSubstitute()), 0);
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
  int womatlid = _womatl->id();
  if (_womatl->currentItem()->data(7, Qt::UserRole).toMap().value("raw").toDouble() > 0)
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

        q.prepare("SELECT womatl_qtyiss AS qtyissued "
                  "FROM womatl "
                  "WHERE (womatl_id=:womatl_id) ");
        q.bindValue(":womatl_id", womatlid);
        q.exec();
        if (!q.first() || q.value("qtyissued").toInt() != 0)
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

  q.prepare("SELECT deleteWoMaterial(:womatl_id);");
  q.bindValue(":womatl_id", womatlid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteWoMaterial", result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatl->id(), TRUE);
}

void workOrderMaterials::sViewAvailability()
{
  q.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("womatl_itemsite_id"));
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void workOrderMaterials::sViewSubstituteAvailability()
{
  q.prepare( "SELECT womatl_itemsite_id, womatl_duedate "
             "FROM womatl "
             "WHERE (womatl_id=:womatl_id);" );
  q.bindValue(":womatl_id", _womatl->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("womatl_itemsite_id"));
    params.append("byDate", q.value("womatl_duedate"));
    params.append("run");

    dspSubstituteAvailabilityByItem *newdlg = new dspSubstituteAvailabilityByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void workOrderMaterials::sSubstitute()
{
  int womatlid = _womatl->id();

  XSqlQuery sub;
  sub.prepare( "SELECT itemuomtouom(itemsite_item_id, womatl_uom_id, NULL, womatl_qtyper) AS qtyper, womatl_wo_id,"
               "       womatl_scrap, womatl_issuemethod,"
               "       womatl_duedate, womatl_bomitem_id "
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
      params.append("qtyPer", (sub.value("qtyper").toDouble() * substitute._uomratio));
      params.append("scrap", sub.value("womatl_scrap"));

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
        q.prepare( "DELETE FROM womatl "
                   "WHERE (womatl_id=:womatl_id);" );
        q.bindValue(":womatl_id", womatlid);
        q.exec();

        omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatl->id(), TRUE);
      }
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl_id, *,"
               "       (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       CASE WHEN (womatl_issuemethod = 'S') THEN :push"
               "            WHEN (womatl_issuemethod = 'L') THEN :pull"
               "            WHEN (womatl_issuemethod = 'M') THEN :mixed"
               "            ELSE :error"
               "       END AS issuemethod,"
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
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":push",  tr("Push"));
    q.bindValue(":pull",  tr("Pull"));
    q.bindValue(":mixed", tr("Mixed"));
    q.bindValue(":error", tr("Error"));
    q.exec();
    _womatl->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT item_picklist,"
               "       COUNT(*) AS total,"
               "       COALESCE(SUM(womatl_qtyper * (1 + womatl_scrap))) AS qtyper "
               "FROM womatl, itemsite, item "
               "WHERE ( (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_wo_id=:wo_id) ) "
               "GROUP BY item_picklist;" );
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    bool   foundPick    = FALSE;
    bool   foundNonPick = FALSE;
    int    totalNumber  = 0;
    double totalQtyPer  = 0.0;
    while (q.next())
    {
      totalNumber += q.value("total").toInt();
      totalQtyPer += q.value("qtyper").toDouble();

      if (q.value("item_picklist").toBool())
      {
        foundPick = TRUE;
        _pickNumber->setText(q.value("total").toString());
        _pickQtyPer->setText(formatQtyPer(q.value("qtyper").toDouble()));
      }
      else
      {
        foundNonPick = TRUE;
        _nonPickNumber->setText(q.value("total").toString());
        _nonPickQtyPer->setText(formatQtyPer(q.value("qtyper").toDouble()));
      }
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    if (!foundPick)
    {
      _pickNumber->setText("0");
      _pickQtyPer->setText(formatQtyPer(0.0));
    }

    if (!foundNonPick)
    {
      _nonPickNumber->setText("0");
      _nonPickQtyPer->setText(formatQtyPer(0.0));
    }

    _totalNumber->setText(QString("%1").arg(totalNumber));
    _totalQtyPer->setText(formatQtyPer(totalQtyPer));

    if (_privileges->check("ViewCosts"))
    {
      q.prepare( "SELECT formatCost(p.item_maxcost) AS f_maxcost,"
                 "       formatCost(COALESCE(SUM(itemuomtouom(ci.itemsite_item_id, womatl_uom_id, NULL, womatl_qtyper * (1 + womatl_scrap)) * stdCost(c.item_id)))) AS f_stdcost,"
                 "       formatCost(COALESCE(SUM(itemuomtouom(ci.itemsite_item_id, womatl_uom_id, NULL, womatl_qtyper * (1 + womatl_scrap)) * actCost(c.item_id)))) AS f_actcost "
                 "FROM wo, womatl, itemsite AS ci, itemsite AS pi, item AS c, item AS p "
                 "WHERE ( (womatl_wo_id=wo_id)"
                 " AND (womatl_itemsite_id=ci.itemsite_id)"
                 " AND (ci.itemsite_item_id=c.item_id)"
                 " AND (wo_itemsite_id=pi.itemsite_id)"
                 " AND (pi.itemsite_item_id=p.item_id)"
                 " AND (wo_id=:wo_id) ) "
                 "GROUP BY p.item_maxcost;" );
      q.bindValue(":wo_id", _wo->id());
      q.exec();
      if (q.first())
      {
        _currentStdCost->setText(q.value("f_stdcost").toString());
        _currentActCost->setText(q.value("f_actcost").toString());
        _maxCost->setText(q.value("f_maxcost").toString());
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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
