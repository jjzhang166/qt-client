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

#include "costCategory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

costCategory::costCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));

  _transformClearingLit->setVisible(_metrics->boolean("Transforms")); 
  _transformClearing->setVisible(_metrics->boolean("Transforms"));
  _laborAndOverheadClearingLit->setVisible(_metrics->boolean("Routings")); 
  _laborAndOverhead->setVisible(_metrics->boolean("Routings"));
  _toLiabilityClearingLit->setVisible(_metrics->boolean("MultiWhs"));
  _toLiabilityClearing->setVisible(_metrics->boolean("MultiWhs"));

}

costCategory::~costCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void costCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse costCategory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("costcat_id", &valid);
  if (valid)
  {
    _costcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _description->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _asset->setReadOnly(TRUE);
      _wip->setReadOnly(TRUE);
      _inventoryCost->setReadOnly(TRUE);
      _adjustment->setReadOnly(TRUE);
      _invScrap->setReadOnly(TRUE);
      _mfgScrap->setReadOnly(TRUE);
      _transformClearing->setReadOnly(TRUE);
      _purchasePrice->setReadOnly(TRUE);
      _laborAndOverhead->setReadOnly(TRUE);
      _liability->setReadOnly(TRUE);
      _freight->setReadOnly(TRUE);
      _shippingAsset->setReadOnly(TRUE);
      _toLiabilityClearing->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void costCategory::sCheck()
{
  if ((_mode == cNew) && (_category->text().length() != 0))
  {
    q.prepare( "SELECT costcat_id "
               "FROM costcat "
               "WHERE (UPPER(costcat_code)=UPPER(:costcat_code));" );
    q.bindValue(":costcat_code", _category->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      _costcatid = q.value("costcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void costCategory::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _category->text().stripWhiteSpace().length() == 0,
      tr("<p>You must enter a name for this Cost Category before saving it."),
      _category
    },
    { _asset->id() < 0,
      tr("<p>You must select an Inventory Asset Account before saving."),
      _asset
    },
    { _wip->id() < 0,
      tr("<p>You must select a WIP Asset Account before saving."),
      _wip
    },
    { _inventoryCost->id() < 0,
      tr("<p>You must select an Inventory Cost Variance Account before saving."),
      _inventoryCost
    },
    { _metrics->boolean("MultiWhs") && _transformClearing->id() < 0,
      tr("<p>You must select a Transform Clearing Account before saving."),
      _transformClearing
    },
    { _purchasePrice->id() < 0,
      tr("<p>You must select a Purchase Price Variance Account before saving."),
      _purchasePrice
    },
    { _adjustment->id() < 0,
      tr("<p>You must select an Inventory Adjustment Account before saving."),
      _adjustment
    },
    { _invScrap->id() < 0,
      tr("<p>You must select an Inventory Scrap Account before saving."),
      _invScrap
    },
    { _mfgScrap->id() < 0,
      tr("<p>You must select a Manufacturing Scrap Account before saving."),
      _mfgScrap
    },
    { (_metrics->boolean("Routings") && _laborAndOverhead->id() < 0),
      tr("<p>You must select a Labor and Overhead Costs Account before saving."),
      _laborAndOverhead
    },
    { _liability->id() < 0,
      tr("<p>You must select a P/O Liability Clearing Account before saving."),
      _liability
    },
    { _shippingAsset->id() < 0,
      tr("<p>You must select a Shipping Asset Account before saving."),
      _shippingAsset
    },
    { _freight->id() < 0,
      tr("<p>You must select a Line Item Freight Expense Account before saving."),
      _freight
    },
    { _metrics->boolean("MultiWhs") && _toLiabilityClearing->id() < 0,
      tr("<p>You must select a Transfer Order Liability Clearing Account before saving."),
      _toLiabilityClearing
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save CRM Account"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  QSqlQuery newCostCategory;

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    q.exec("SELECT NEXTVAL('costcat_costcat_id_seq') AS costcat_id");
    if (q.first())
      _costcatid = q.value("costcat_id").toInt();
    else
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO costcat"
               "( costcat_id, costcat_code, costcat_descrip,"
               "  costcat_asset_accnt_id, costcat_invcost_accnt_id,"
               "  costcat_liability_accnt_id, costcat_freight_accnt_id,"
               "  costcat_adjustment_accnt_id, costcat_scrap_accnt_id, costcat_mfgscrap_accnt_id,"
               "  costcat_transform_accnt_id, costcat_wip_accnt_id,"
               "  costcat_purchprice_accnt_id, costcat_laboroverhead_accnt_id,"
               "  costcat_shipasset_accnt_id, costcat_toliability_accnt_id ) "
               "VALUES "
               "( :costcat_id, :costcat_code, :costcat_descrip,"
               "  :costcat_asset_accnt_id, :costcat_invcost_accnt_id,"
               "  :costcat_liability_accnt_id, :costcat_freight_accnt_id,"
               "  :costcat_adjustment_accnt_id, :costcat_scrap_accnt_id, :costcat_mfgscrap_accnt_id,"
               "  :costcat_transform_accnt_id, :costcat_wip_accnt_id,"
               "  :costcat_purchprice_accnt_id, :costcat_laboroverhead_accnt_id,"
               "  :costcat_shipasset_accnt_id, :costcat_toliability_accnt_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE costcat "
               "SET costcat_code=:costcat_code, costcat_descrip=:costcat_descrip,"
               "    costcat_asset_accnt_id=:costcat_asset_accnt_id,"
               "    costcat_invcost_accnt_id=:costcat_invcost_accnt_id,"
               "    costcat_liability_accnt_id=:costcat_liability_accnt_id,"
               "    costcat_freight_accnt_id=:costcat_freight_accnt_id,"
               "    costcat_adjustment_accnt_id=:costcat_adjustment_accnt_id,"
               "    costcat_scrap_accnt_id=:costcat_scrap_accnt_id,"
               "    costcat_mfgscrap_accnt_id=:costcat_mfgscrap_accnt_id,"
               "    costcat_transform_accnt_id=:costcat_transform_accnt_id,"
               "    costcat_wip_accnt_id=:costcat_wip_accnt_id,"
               "    costcat_purchprice_accnt_id=:costcat_purchprice_accnt_id,"
               "    costcat_laboroverhead_accnt_id=:costcat_laboroverhead_accnt_id,"
               "    costcat_shipasset_accnt_id=:costcat_shipasset_accnt_id,"
               "    costcat_toliability_accnt_id=:costcat_toliability_accnt_id "
               "WHERE (costcat_id=:costcat_id);" );

  q.bindValue(":costcat_id", _costcatid);
  q.bindValue(":costcat_code", _category->text().stripWhiteSpace());
  q.bindValue(":costcat_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":costcat_asset_accnt_id", _asset->id());
  q.bindValue(":costcat_invcost_accnt_id", _inventoryCost->id());
  q.bindValue(":costcat_liability_accnt_id", _liability->id());
  q.bindValue(":costcat_freight_accnt_id", _freight->id());
  q.bindValue(":costcat_adjustment_accnt_id", _adjustment->id());
  q.bindValue(":costcat_scrap_accnt_id", _invScrap->id());
  q.bindValue(":costcat_mfgscrap_accnt_id", _mfgScrap->id());
  q.bindValue(":costcat_transform_accnt_id", _transformClearing->id());
  q.bindValue(":costcat_wip_accnt_id", _wip->id());
  q.bindValue(":costcat_purchprice_accnt_id", _purchasePrice->id());
  q.bindValue(":costcat_laboroverhead_accnt_id", _laborAndOverhead->id());
  q.bindValue(":costcat_shipasset_accnt_id", _shippingAsset->id());
  if (_toLiabilityClearing->isValid())
    q.bindValue(":costcat_toliability_accnt_id", _toLiabilityClearing->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_costcatid);
}

void costCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM costcat "
             "WHERE (costcat_id=:costcat_id);" );
  q.bindValue(":costcat_id", _costcatid);
  q.exec();
  if (q.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(q.value("costcat_code").toString());
      _description->setText(q.value("costcat_descrip").toString());
    }

    _asset->setId(q.value("costcat_asset_accnt_id").toInt());
    _inventoryCost->setId(q.value("costcat_invcost_accnt_id").toInt());
    _liability->setId(q.value("costcat_liability_accnt_id").toInt());
    _freight->setId(q.value("costcat_freight_accnt_id").toInt());
    _adjustment->setId(q.value("costcat_adjustment_accnt_id").toInt());
    _invScrap->setId(q.value("costcat_scrap_accnt_id").toInt());
    _mfgScrap->setId(q.value("costcat_mfgscrap_accnt_id").toInt());
    _wip->setId(q.value("costcat_wip_accnt_id").toInt());
    _transformClearing->setId(q.value("costcat_transform_accnt_id").toInt());
    _purchasePrice->setId(q.value("costcat_purchprice_accnt_id").toInt());
    _laborAndOverhead->setId(q.value("costcat_laboroverhead_accnt_id").toInt());
    _shippingAsset->setId(q.value("costcat_shipasset_accnt_id").toInt());
    _toLiabilityClearing->setId(q.value("costcat_toliability_accnt_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
