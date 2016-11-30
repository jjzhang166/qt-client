/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "closeWo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "returnWoMaterialItem.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

closeWo::closeWo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_closeWo, SIGNAL(clicked()), this, SLOT(sCloseWo()));

  _captive = false;
  _transDate->setDate(omfgThis->dbDate(), true);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoOpen | cWoExploded | cWoReleased | cWoIssued);
  
  _postMaterialVariance->setChecked(_metrics->boolean("PostMaterialVariances"));
}

closeWo::~closeWo()
{
    // no need to delete child widgets, Qt does it all for us
}

void closeWo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse closeWo::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("transDate", &valid);
  if (valid)
    _transDate->setDate(param.toDate());

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }

  return NoError;
}

bool closeWo::okToSave()
{
  XSqlQuery closeokToSave;

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(!_transDate->isValid(), _transDate,
                        tr("You must enter a valid transaction date."));

  if(GuiErrorCheck::reportErrors(this,tr("Invalid Date"),errors))
      return false;

  // Return any tools that have been issued
  XSqlQuery tool;
  tool.prepare( "SELECT womatl_id"
                "  FROM womatl "
                "  JOIN itemsite ON (womatl_itemsite_id = itemsite_id) "
                "  JOIN item ON ((itemsite_item_id = item_id) AND (item_type = 'T')) "
                " WHERE ((womatl_wo_id=:wo_id)"
                "   AND  (womatl_qtyiss > 0 ));");
  tool.bindValue(":wo_id", _wo->id());
  tool.exec();
  if (tool.first())
  {
    do
    {
      returnWoMaterialItem newdlg(this);
      ParameterList params;
      params.append("womatl_id", tool.value("womatl_id").toInt());
      if (newdlg.set(params) == NoError)
        newdlg.exec();
    }
    while (tool.next());
  }

  XSqlQuery type;
  type.prepare( "SELECT itemsite_costmethod "
                "FROM itemsite,wo "
                "WHERE ((wo_id=:wo_id) "
                "AND (wo_itemsite_id=itemsite_id)); ");
  type.bindValue(":wo_id", _wo->id());
  type.exec();
  if (type.first())
  {
    if (type.value("itemsite_costmethod").toString() == "J")
    {
      if (QMessageBox::critical(this, tr("Job Costed Item"),
                                tr("<p>Work Orders for item sites with the Job cost "
                                   "method are posted when shipping the Sales Order "
                                   "they are associated with."
                                   "<p>Are you sure you want to close this "
                                   "Work Order?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
      {
        _wo->setFocus();
        return false;
      }
    }

    closeokToSave.prepare("SELECT wo_qtyrcv, womatl_issuemethod, womatl_qtyiss "
                          "  FROM wo "
                          "  JOIN womatl ON (womatl_wo_id = wo_id) "
                          "  JOIN itemsite ON (womatl_itemsite_id = itemsite_id) "
                          "  JOIN item ON ((itemsite_item_id = item_id) AND (NOT item_type = 'T')) "
                          " WHERE (wo_id=:wo_id);" );
    closeokToSave.bindValue(":wo_id", _wo->id());
    closeokToSave.exec();
    if (closeokToSave.first())
    {
      if (closeokToSave.value("wo_qtyrcv").toDouble() == 0.0)
        QMessageBox::warning(this, tr("No Production Posted"),
                             tr("<p>There has not been any Production "
                                "received from this Work Order. This "
                                "probably means Production Postings for "
                                "this Work Order have been overlooked." ));
      
      bool unissuedMaterial = false;
      bool unpushedMaterial = false;
      do
      {
        if (! unissuedMaterial &&
            (closeokToSave.value("womatl_issuemethod") == "S") &&
            (closeokToSave.value("womatl_qtyiss").toDouble() == 0.0) )
        {
          QMessageBox::warning(this, tr("Unissued Push Items"),
                               tr("<p>The selected Work Order has Material "
                                  "Requirements that are Push-Issued but "
                                  "have not had any material issued to them. "
                                  "This probably means that required manual "
                                  "material issues have been overlooked."));
          unissuedMaterial = true;
        }
        else if (! unpushedMaterial &&
                 ( (closeokToSave.value("womatl_issuemethod") == "L") ||
                  (closeokToSave.value("womatl_issuemethod") == "M") ) &&
                 (closeokToSave.value("womatl_qtyiss").toDouble() == 0.0) )
        {
          QMessageBox::warning(this, tr("Unissued Pull Items"),
                               tr("<p>The selected Work Order has Material "
                                  "Requirements that are Pull-Issued but "
                                  "have not had any material issued to them. "
                                  "This probably means that Production was "
                                  "posted for this Work Order through "
                                  "posting Operations. The BOM for this Item "
                                  "should be modified to list Used At "
                                  "selections for each BOM Item." ) );
          unpushedMaterial = true;
        }
      }
      while (closeokToSave.next());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                                  closeokToSave, __FILE__, __LINE__))
    {
      return false;
    }
    
    return true;      // this is the only successful case
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                                type, __FILE__, __LINE__))
  {
    return false;
  }
  else // type not found
  {
    ErrorReporter::error(QtCriticalMsg, this,tr("Error Closing Work Order"),
                         tr("Cannot close Work Order %1 because the item does not "
                         "appear to have an Item Type!  Please check the Item "
                         "definition.  You may need to reset the Type and save "
                         "the Item record.")
                         .arg(_wo->woNumber()),
                          __FILE__, __LINE__);
    return false;
  }

  return false;
}

void closeWo::sCloseWo()
{
  XSqlQuery closeCloseWo;
  if (okToSave() &&
      QMessageBox::question(this, tr("Close Work Order"),
                            tr("<p>Are you sure you want to close this "
                               "Work Order?"),
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
  {
    closeCloseWo.prepare("SELECT closeWo(:wo_id, :postMatVar, :date);");
    closeCloseWo.bindValue(":wo_id", _wo->id());
    closeCloseWo.bindValue(":postMatVar",   QVariant(_postMaterialVariance->isChecked()));
    closeCloseWo.bindValue(":date",  _transDate->date());
    closeCloseWo.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Closing Work Order"),
                                  closeCloseWo, __FILE__, __LINE__))
    {
      return;
    }

    omfgThis->sWorkOrdersUpdated(_wo->id(), true);

    if (_captive)
      close();
    else
    {
      clear();
    }
  }
}

void closeWo::clear()
{
  _wo->setId(-1);
  _close->setText(tr("&Close"));
  _wo->setFocus();
}
