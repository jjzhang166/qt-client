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

#include "correctProductionPosting.h"

#include <QVariant>
#include <QMessageBox>

#include "inputManager.h"
#include "distributeInventory.h"
#include "closeWo.h"

correctProductionPosting::correctProductionPosting(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_correct, SIGNAL(clicked()), this, SLOT(sCorrect()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = FALSE;
  _qtyReceivedCache = 0.0;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _qty->setValidator(omfgThis->qtyVal());

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _backFlush->setChecked(true);
    _backflushOperations->setChecked(true);
  }
  _nonPickItems->setEnabled(_backFlush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();

  if (!_metrics->boolean("Routings"))
  {
    _backflushOperations->setChecked(FALSE);
    _backflushOperations->hide();
  }
}

correctProductionPosting::~correctProductionPosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void correctProductionPosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse correctProductionPosting::set(const ParameterList &pParams)
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

void correctProductionPosting::sCorrect()
{
  if (_qty->toDouble() > _qtyReceivedCache)
  {
    QMessageBox::warning( this, tr("Cannot Post Correction"),
                          tr( "The Quantity to Correct value you entered is greater than the total Quantity Received from this W/O.\n"
                              "The Quantity to correct must be less than or equal to the Quantity Received." ) );
    _qty->setFocus();
    return;
  }

  q.prepare( "SELECT item_type "
             "FROM wo, itemsite, item "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    if (q.value("item_type").toString() == "B")
    {
      QMessageBox::critical( this, tr("Cannot Post Correction"),
                             tr( "You may not post a correction to a W/O that manufactures a Breeder Item.\n"
                                 "You must, instead, adjust the Breeder's, Co-Product's and By-Product's QOH." ) );
      return;
    }
    if (q.value("item_type").toString() == "J")
    {
      QMessageBox::critical( this, tr("Cannot Post Correction"),
                             tr( "You may not post a correction to a W/O for a Job Item.\n"
                                 "You must, instead, adjust shipped quantities." ) );
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT correctProduction(:wo_id, :qty, :backflushMaterials, :backflushOperations) AS result;");
  q.bindValue(":wo_id", _wo->id());
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":backflushMaterials", QVariant(_backFlush->isChecked(), 0));
  q.bindValue(":backflushOperations", QVariant(_backflushOperations->isChecked(), 0));
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result > 0)
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Correct Production Posting"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
      omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
    }
    else
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at correctProductionPosting::%1, Work Order ID #2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(result) );
      return;
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at correctProductionPosting::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _qty->clear();
    _close->setText(tr("&Close"));
    _wo->setFocus();
  }
}

void correctProductionPosting::populate()
{
  q.prepare("SELECT formatQtyPer(wo_qtyord) AS ordered,"
            "       formatQtyPer(wo_qtyrcv) AS received,"
            "       wo_qtyrcv,"
            "       formatQtyPer(noNeg(wo_qtyord - wo_qtyrcv)) AS balance"
            "  FROM wo"
            " WHERE (wo_id=:wo_id); ");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if(q.first())
  {
    _qtyOrdered->setText(q.value("ordered").toString());
    _qtyReceived->setText(q.value("received").toString());
    _qtyBalance->setText(q.value("balance").toString());

    _qtyReceivedCache = q.value("wo_qtyrcv").toDouble();
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();

    _qtyReceivedCache = 0.0;
  }
}
