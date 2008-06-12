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

#include "correctOperationsPosting.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "inputManager.h"
#include "distributeInventory.h"

correctOperationsPosting::correctOperationsPosting(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sHandleWoid(int)));
  connect(_wooper, SIGNAL(newID(int)), this, SLOT(sHandleWooperid(int)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleQty()));
  connect(_productionUOM, SIGNAL(toggled(bool)), this, SLOT(sHandleQty()));

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _wooper->setAllowNull(TRUE);
  _qty->setValidator(omfgThis->qtyVal());

  _specifiedSutime->setValidator(omfgThis->runTimeVal());
  _specifiedRntime->setValidator(omfgThis->runTimeVal());

  _womatl->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft  );
  _womatl->addColumn(tr("Description"), -1,          Qt::AlignLeft  );
  _womatl->addColumn(tr("Iss. UOM"),    _uomColumn,  Qt::AlignLeft  );
  _womatl->addColumn(tr("Qty. per"),    _qtyColumn,  Qt::AlignRight );
}

correctOperationsPosting::~correctOperationsPosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void correctOperationsPosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse correctOperationsPosting::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);

    _qty->setFocus();
  }

  return NoError;
}

void correctOperationsPosting::sHandleWoid(int pWoid)
{
  q.prepare( "SELECT wooper_id, (wooper_seqnumber || ' - ' || wooper_descrip1 || ' ' || wooper_descrip2) "
             "FROM wooper "
             "WHERE (wooper_wo_id=:wo_id) "
             "ORDER BY wooper_seqnumber;" );
  q.bindValue(":wo_id", pWoid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _wooper->populate(q);

  q.prepare( "SELECT uom_name "
             "FROM wo, itemsite, item, uom "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (item_inv_uom_id=uom_id)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", pWoid);
  q.exec();
  if (q.first())
    _inventoryUOM->setText( tr("Post in Inventory UOMs (%1)")
                            .arg(q.value("uom_name").toString()) );
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void correctOperationsPosting::sHandleWooperid(int)
{
  if (_wooper->id() != -1)
  {
    q.prepare( "SELECT formatQty(wo_qtyord) AS ordered,"
               "       formatQty(COALESCE(wooper_qtyrcv,0)) AS received,"
               "       formatQty(noNeg(wo_qtyord - COALESCE(wooper_qtyrcv,0))) AS balance,"
               "       wooper_issuecomp, wooper_rcvinv, wooper_produom,"
               "       wooper_sucomplete, wooper_rncomplete,"
               "       wooper_suconsumed, formatTime(wooper_suconsumed) AS suconsumed,"
               "       wooper_rnconsumed, formatTime(wooper_rnconsumed) AS rnconsumed,"
               "       wooper_rnqtyper, wooper_invproduomratio "
               "FROM wo, wooper "
               "WHERE ( (wooper_wo_id=wo_id)"
               " AND (wooper_id=:wooper_id) );" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.first())
    {
      _rnqtyper = q.value("wooper_rnqtyper").toDouble();
      _invProdUOMRatio = q.value("wooper_invproduomratio").toDouble();

      _qtyOrdered->setText(q.value("ordered").toString());
      _qtyReceived->setText(q.value("received").toString());
      _qtyBalance->setText(q.value("balance").toString());
      _productionUOM->setText( tr("Post in Production UOMs (%1)") 
                               .arg(q.value("wooper_produom").toString()) );

      _correctSutime->setEnabled(q.value("wooper_suconsumed").toDouble() > 0.0);
      _correctSutime->setChecked(q.value("wooper_suconsumed").toDouble() > 0.0);
      _correctRntime->setEnabled(q.value("wooper_rnconsumed").toDouble() > 0.0);
      _correctRntime->setChecked(q.value("wooper_rnconsumed").toDouble() > 0.0);

      _clearSuComplete->setEnabled(q.value("wooper_sucomplete").toBool());
      _clearSuComplete->setChecked(q.value("wooper_sucomplete").toBool());
      _clearRnComplete->setEnabled(q.value("wooper_rncomplete").toBool());
      _clearRnComplete->setChecked(q.value("wooper_rncomplete").toBool());

      _correctInventory->setEnabled(q.value("wooper_rcvinv").toBool());
      _correctInventory->setChecked(q.value("wooper_rcvinv").toBool());

      if (q.value("wooper_issuecomp").toBool())
      {
        q.prepare( "SELECT womatl_id, item_number, (item_descrip1 || ' ' || item_descrip2),"
                   "       uom_name, formatQtyPer(womatl_qtyper) "
                   "FROM womatl, itemsite, item, uom "
                   "WHERE ( (womatl_itemsite_id=itemsite_id)"
                   " AND (womatl_uom_id=uom_id)"
                   " AND (womatl_issuemethod IN ('L', 'M'))"
                   " AND (itemsite_item_id=item_id)"
                   " AND (womatl_wooper_id=:wooper_id) ) "
                   "ORDER BY item_number;" );
        q.bindValue(":wooper_id", _wooper->id());
        q.exec();
	if (q.lastError().type() != QSqlError::None)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
        _womatl->populate(q);

        if (q.first())
        {
          _returnComponents->setEnabled(TRUE);
          _returnComponents->setChecked(TRUE);
        }
        else
        {
          _returnComponents->setEnabled(FALSE);
          _returnComponents->setChecked(FALSE);
        }
      }
      else
      {
        _womatl->clear();
        _returnComponents->setEnabled(FALSE);
        _returnComponents->setChecked(FALSE);
      }

      sHandleQty();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();
    _productionUOM->setText(tr("Post in Production UOMs"));

    _qty->clear();

    _returnComponents->setChecked(FALSE);
    _correctInventory->setChecked(FALSE);
    _womatl->clear();

    _correctSutime->setEnabled(FALSE);
    _correctSutime->setChecked(FALSE);
    _clearSuComplete->setChecked(FALSE);
    _standardSutime->clear();
    _specifiedSutime->clear();
    
    _correctRntime->setEnabled(FALSE);
    _correctRntime->setChecked(FALSE);
    _clearRnComplete->setChecked(FALSE);
    _standardSutime->clear();
    _specifiedSutime->clear();
  }
}

void correctOperationsPosting::sHandleQty()
{
  if (_wooper->id() == -1)
  {
    _standardRntime->clear();
    _clearRnComplete->setChecked(FALSE);
    return;
  }
  else
  {
    double qty = _qty->toDouble();

    if (_productionUOM->isChecked())
      _standardRntime->setText(formatQty(_rnqtyper * qty));
    else
      _standardRntime->setText(formatQty(_rnqtyper / _invProdUOMRatio * qty));

    if (qty < _qtyOrdered->text().toDouble())
      _clearRnComplete->setChecked(TRUE);
    else
      _clearRnComplete->setChecked(FALSE);
  }
}

void correctOperationsPosting::sPost()
{
  if (_wooper->id() == -1)
  {
    QMessageBox::critical( this, tr("Select W/O Operation to Post"),
                           tr("Please select the W/O Operation you wish to post.") );

    _wooper->setFocus();
    return;
  }

  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Post"),
                           tr( "Please enter the Quantity of Production, in Inventory or Production UOM, that you wish to post." ) );
    _qty->setFocus();
    return;
  }

  double qty;

  if (_productionUOM->isChecked())
    qty = (_qty->toDouble() / _invProdUOMRatio);
  else
    qty = _qty->toDouble();

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT correctOperationsPosting(:wooper_id, :qty, :returnComponents, :correctInventory, :correctSuTime, :setupTime, :clearSetupComplete, :correctRnTime, :runTime, :clearRunComplete) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.bindValue(":qty", qty);
  q.bindValue(":returnComponents", QVariant(_returnComponents->isChecked(), 0));
  q.bindValue(":correctInventory", QVariant(_correctInventory->isChecked(), 0));
  q.bindValue(":correctSuTime", QVariant(_correctSutime->isChecked(), 0));
  if (_correctSutime->isChecked())
  {
    double sutime;

    if (_correctStandardSutime->isChecked())
      sutime = _standardSutime->text().toDouble();
    else
      sutime = _specifiedSutime->toDouble();

    q.bindValue(":setupTime", (sutime * -1));
    q.bindValue(":clearSetupComplete", QVariant(_clearSuComplete->isChecked(), 0));
  }
  q.bindValue(":correctRnTime", QVariant(_correctRntime->isChecked(), 0));
  if (_correctRntime->isChecked())
  {
    double rntime;

    if (_correctStandardRntime->isChecked())
      rntime = _standardRntime->text().toDouble();
    else
      rntime = _specifiedRntime->toDouble();

    q.bindValue(":runTime", (rntime * -1));
    q.bindValue(":clearRunComplete", QVariant(_clearRnComplete->isChecked(), 0));
  }

  q.exec();
  q.first();
  if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  int result = q.value("result").toInt();

  if (!_correctSutime->isChecked() && _clearSuComplete->isChecked())
  {
    q.prepare( "UPDATE wooper "
               "SET wooper_sucomplete=FALSE "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (!_correctRntime->isChecked() && _clearRnComplete->isChecked())
  {
    q.prepare( "UPDATE wooper "
               "SET wooper_rncomplete=FALSE "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    { 
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  
  if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
  {
    rollback.exec();
    QMessageBox::information( this, tr("Correct Operation Posting"), tr("Transaction Canceled") );
    return;
  }

  q.exec("COMMIT;");
  
  omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    _wooper->setNull();
    _wooper->setFocus();
  }
}

