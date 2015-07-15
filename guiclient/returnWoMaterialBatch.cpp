/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnWoMaterialBatch.h"

#include <QVariant>
#include <QMessageBox>
#include "inputManager.h"
#include "distributeInventory.h"

returnWoMaterialBatch::returnWoMaterialBatch(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  XSqlQuery returnreturnWoMaterialBatch;
  setupUi(this);

  connect(_wo, SIGNAL(valid(bool)), _return, SLOT(setEnabled(bool)));
  connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = false;
  _transDate->setEnabled(_privileges->check("AlterTransactionDates"));
  _transDate->setDate(omfgThis->dbDate(), true);
  returnreturnWoMaterialBatch.bindValue(":date",  _transDate->date());

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  // Change to Issued only
  //_wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _wo->setType(cWoIssued);
  _wo->setFocus();
}

returnWoMaterialBatch::~returnWoMaterialBatch()
{
  // no need to delete child widgets, Qt does it all for us
}

void returnWoMaterialBatch::languageChange()
{
  retranslateUi(this);
}

enum SetResponse returnWoMaterialBatch::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
  }

  return NoError;
}

void returnWoMaterialBatch::sReturn()
{
  XSqlQuery returnReturn;
  if (!_transDate->isValid())
  {
    QMessageBox::critical(this, tr("Invalid date"),
                          tr("You must enter a valid transaction date.") );
    _transDate->setFocus();
    return;
  }
  
  returnReturn.prepare( "SELECT wo_qtyrcv, wo_status "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  returnReturn.bindValue(":wo_id", _wo->id());
  returnReturn.exec();
  if (returnReturn.first())
  {
    if (_wo->method() == "A" && returnReturn.value("wo_qtyrcv").toDouble() != 0)
    {
      QMessageBox::warning( this, tr("Cannot return Work Order Material"),
                            tr( "This Work Order has had material received against it\n"
                                "and thus the material issued against it cannot be returned.\n"
                                "You must instead return each Work Order Material item individually.\n" ) );
      _wo->setId(-1);
      _wo->setFocus();
    }
    else
    {
      if(returnReturn.value("wo_status").toString() == "E" || 
         returnReturn.value("wo_status").toString() == "R" ||
         returnReturn.value("wo_status").toString() == "I")
      {
        XSqlQuery rollback;
        rollback.prepare("ROLLBACK;");
  
        XSqlQuery items;
        items.prepare("SELECT womatl_id,"
                      "       CASE WHEN wo_qtyord >= 0 THEN"
                      "         womatl_qtyiss"
                      "       ELSE"
                      "         ((womatl_qtyreq - womatl_qtyiss) * -1)"
                      "       END AS qty"
                      "  FROM wo, womatl, itemsite"
                      " WHERE((wo_id=womatl_wo_id)"
                      "   AND (womatl_itemsite_id=itemsite_id)"
                      "   AND ( (wo_qtyord < 0) OR (womatl_issuemethod IN ('S','M')) )"
                      "   AND (womatl_wo_id=:wo_id))");
        items.bindValue(":wo_id", _wo->id());
        items.exec();
        while(items.next())
        {
          if(items.value("qty").toDouble() == 0.0)
            continue;

          returnReturn.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
          returnReturn.prepare("SELECT returnWoMaterial(:womatl_id, :qty, 0, :date) AS result;");
          returnReturn.bindValue(":womatl_id", items.value("womatl_id").toInt());
          returnReturn.bindValue(":qty", items.value("qty").toDouble());
          returnReturn.bindValue(":date",  _transDate->date());
          returnReturn.exec();
          if (returnReturn.first())
          {
            if (returnReturn.value("result").toInt() < 0)
            {
              rollback.exec();
              systemError( this, tr("A System Error occurred at returnWoMaterialBatch::%1, W/O ID #%2, Error #%3.")
                                 .arg(__LINE__)
                                 .arg(_wo->id())
                                 .arg(returnReturn.value("result").toInt()) );
              return;
            }
            if (distributeInventory::SeriesAdjust(returnReturn.value("result").toInt(), this) == XDialog::Rejected)
            {
              rollback.exec();
              QMessageBox::information( this, tr("Material Return"), tr("Transaction Canceled") );
              return;
            }
          }
          else
          {
            rollback.exec();
            systemError( this, tr("A System Error occurred at returnWoMaterialBatch::%1, W/O ID #%2.")
                               .arg(__LINE__)
                               .arg(_wo->id()) );
            return;
          }
          returnReturn.exec("COMMIT;");
        }
      }
    }
  }

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _wo->setFocus();
  }
}
