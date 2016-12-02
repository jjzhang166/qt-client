/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "purchaseRequest.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>
#include <QSqlError>

#include <QCloseEvent>

#include "errorReporter.h"
#include "guiErrorCheck.h"

purchaseRequest::purchaseRequest(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckWarehouse(int)));
  connect(_number, SIGNAL(textEdited(QString)), this, SLOT(sReleaseNumber()));

  _prid = -1;
  _planordid = -1;
  _NumberGen = -1;
  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);
  _lastWarehousid = _warehouse->id();

  _number->setValidator(omfgThis->orderVal());
  _qty->setValidator(omfgThis->qtyVal());
  
  _project->setType(ProjectLineEdit::PurchaseOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

purchaseRequest::~purchaseRequest()
{
    // no need to delete child widgets, Qt does it all for us
}

void purchaseRequest::languageChange()
{
    retranslateUi(this);
}

enum SetResponse purchaseRequest::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pr_id", &valid);
  if (valid)
    _prid = param.toInt();
  
  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _warehouse->setEnabled(false);
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setEnabled(false);
  }
  
  param = pParams.value("qty", &valid);
  if (valid)
    _qty->setDouble(param.toDouble());

  param = pParams.value("dueDate", &valid);
  if (valid)
    _dueDate->setDate(param.toDate());

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      populateNumber();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _captive = true;
      _create->setText(tr("Save"));
      
      populate();
      
      _number->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _captive = true;
      
      populate();
      
      _number->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _qty->setEnabled(false);
      _dueDate->setEnabled(false);
      _notes->setEnabled(false);
      _project->setEnabled(false);
      _create->hide();
    }
    else if (param.toString() == "release")
    {
      _mode = cRelease;
      _captive = true;

      _number->setEnabled(false);
      _item->setReadOnly(true);
      _warehouse->setEnabled(false);
      _qty->setEnabled(false);
      _dueDate->setEnabled(false);

      XSqlQuery purchaseet;
      purchaseet.prepare( "SELECT planord_itemsite_id, planord_duedate,"
                 "       planord_qty, planord_comments "
                 "FROM planord "
                 "WHERE (planord_id=:planord_id);" );
      purchaseet.bindValue(":planord_id", _planordid);
      purchaseet.exec();
      if (purchaseet.first())
      {
        _item->setItemsiteid(purchaseet.value("planord_itemsite_id").toInt());
        _qty->setDouble(purchaseet.value("planord_qty").toDouble());
        _dueDate->setDate(purchaseet.value("planord_duedate").toDate());
        _notes->setText(purchaseet.value("planord_comments").toString());

        populateNumber();
      }
    }
  }

  return NoError;
}

void purchaseRequest::sClose()
{
  closeEvent(NULL);
  reject();
}

void purchaseRequest::sSave()
{
  XSqlQuery purchaseCreate;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_number->text().length(), _number,
                          tr("<p>You must enter a valid Purchase Request number."))
  << GuiErrorCheck(!_item->isValid(), _item,
                   tr("<p>You must enter or select a valid Item number."))
  << GuiErrorCheck(_qty->toDouble() == 0.0, _qty,
                   tr("<p>You have entered an invalid Qty. Ordered."))
  << GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                   tr("<p>You have entered an invalid Due Date."))
  ;
  
  purchaseCreate.prepare("SELECT itemsite_id "
                         "FROM itemsite "
                         "WHERE ( (itemsite_item_id=:item_id)"
                         " AND (itemsite_warehous_id=:warehous_id) );" );
  purchaseCreate.bindValue(":item_id", _item->id());
  purchaseCreate.bindValue(":warehous_id", _warehouse->id());
  purchaseCreate.exec();
  if (!purchaseCreate.first())
    errors << GuiErrorCheck(true, _warehouse,
                            tr("The selected Site for this Purchase Request is not\n"
                               "a \"Supplied At\" Site. You must select a different\n"
                              "Site before creating the Purchase Request.") );

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Purchase Request"), errors))
    return;
  
  int itemsiteid = purchaseCreate.value("itemsite_id").toInt();

  if (_mode == cNew)
  {
    purchaseCreate.prepare( "SELECT createPr( :orderNumber, :itemsite_id, :qty,"
               "                 :dueDate, :notes ) AS prid;" );
    purchaseCreate.bindValue(":orderNumber", _number->text().toInt());
    purchaseCreate.bindValue(":itemsite_id", itemsiteid);
    purchaseCreate.bindValue(":qty", _qty->toDouble());
    purchaseCreate.bindValue(":dueDate", _dueDate->date());
    purchaseCreate.bindValue(":notes", _notes->toPlainText());
    purchaseCreate.exec();
    if (!purchaseCreate.first())
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Purchase Request"),
                           purchaseCreate, __FILE__, __LINE__);
      return;
    }
    else
    {
      _prid = purchaseCreate.value("prid").toInt();
      if (_prid != -1)
      {
        purchaseCreate.prepare("UPDATE pr SET pr_prj_id=:prj_id WHERE (pr_id=:pr_id);");
        purchaseCreate.bindValue(":pr_id",  _prid);
        purchaseCreate.bindValue(":prj_id", _project->id());
        purchaseCreate.exec();
      }
    }
  }
  else if (_mode == cEdit)
  {
    purchaseCreate.prepare("UPDATE pr SET pr_qtyreq=:qty,"
                           "              pr_duedate=:dueDate,"
                           "              pr_prj_id=:prj_id,"
                           "              pr_releasenote=:notes "
                           "WHERE (pr_id=:pr_id);");
    purchaseCreate.bindValue(":pr_id", _prid);
    purchaseCreate.bindValue(":qty", _qty->toDouble());
    purchaseCreate.bindValue(":dueDate", _dueDate->date());
    purchaseCreate.bindValue(":prj_id", _project->id());
    purchaseCreate.bindValue(":notes", _notes->toPlainText());
    purchaseCreate.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Purchase Request"),
                                  purchaseCreate, __FILE__, __LINE__))
    {
      return;
    }
  }
  else if (_mode == cRelease)
  {
    purchaseCreate.prepare("SELECT createPr(:orderNumber, 'F', :planord_id, :notes) AS prid;");
    purchaseCreate.bindValue(":orderNumber", _number->text().toInt());
    purchaseCreate.bindValue(":planord_id", _planordid);
    purchaseCreate.bindValue(":notes", _notes->toPlainText());
    purchaseCreate.exec();
    if (purchaseCreate.first())
    {
      _prid = purchaseCreate.value("prid").toInt();

      purchaseCreate.prepare("SELECT releasePlannedOrder(:planord_id, false) AS result;");
      purchaseCreate.bindValue(":planord_id", _planordid);
      purchaseCreate.exec();
    }
  }

  omfgThis->sPurchaseRequestsUpdated();

  if (_captive)
    done(_prid);
  else
  {
    populateNumber();
    _item->setId(-1);
    _qty->clear();
    _dueDate->setNull();
    _notes->clear();

    _item->setFocus();
  }
}

void purchaseRequest::populate()
{
  XSqlQuery purchaseet;
  purchaseet.prepare( "SELECT pr.* "
                     "FROM pr "
                     "WHERE (pr_id=:pr_id);" );
  purchaseet.bindValue(":pr_id", _prid);
  purchaseet.exec();
  if (purchaseet.first())
  {
    _number->setText(purchaseet.value("pr_number").toString());
    _item->setItemsiteid(purchaseet.value("pr_itemsite_id").toInt());
    _qty->setDouble(purchaseet.value("pr_qtyreq").toDouble());
    _dueDate->setDate(purchaseet.value("pr_duedate").toDate());
    _project->setId(purchaseet.value("pr_prj_id").toInt());
    _notes->setText(purchaseet.value("pr_releasenote").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Request Information"),
                                purchaseet, __FILE__, __LINE__))
  {
    return;
  }
}

void purchaseRequest::populateNumber()
{
  XSqlQuery purchasepopulateNumber;
  QString generationMethod = _metrics->value("PrNumberGeneration");

  if ((generationMethod == "A") || (generationMethod == "O"))
  {
    purchasepopulateNumber.exec("SELECT fetchPrNumber() AS number;");
    if (!purchasepopulateNumber.first())
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Request Information"),
                           purchasepopulateNumber, __FILE__, __LINE__);

      _number->setText("Error");
      return;
    }

    _number->setText(purchasepopulateNumber.value("number").toString());
    _NumberGen = purchasepopulateNumber.value("number").toInt();
  }

  if (generationMethod == "M")
  {
    _number->setEnabled(true);
    _number->setFocus();
  }
  else if (generationMethod == "O")
  {
    _number->setEnabled(true);
    _item->setFocus();
  }
  else if (generationMethod == "A")
  {
    _number->setEnabled(false);
    _item->setFocus();
  } 
}

void purchaseRequest::closeEvent(QCloseEvent *pEvent)
{
  if ( ((_mode == cNew) || (_mode == cRelease)) &&
       ((_metrics->value("PrNumberGeneration") == "A") || (_metrics->value("PrNumberGeneration") == "O")) )
    sReleaseNumber();

  if (pEvent != NULL)
    XDialog::closeEvent(pEvent);
}

void purchaseRequest::sNumberChanged()
{
  if(-1 != _NumberGen && _number->text().toInt() != _NumberGen)
    sReleaseNumber();
}

void purchaseRequest::sReleaseNumber()
{
  XSqlQuery purchaseReleaseNumber;
  if(-1 != _NumberGen)
  {
    purchaseReleaseNumber.prepare("SELECT releasePrNumber(:number);" );
    purchaseReleaseNumber.bindValue(":number", _NumberGen);
    purchaseReleaseNumber.exec();
    if (purchaseReleaseNumber.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Purchase Request Information"),
                         purchaseReleaseNumber, __FILE__, __LINE__);
    _NumberGen = -1;
  }
}

void purchaseRequest::sCheckWarehouse( int pWarehousid )
{
  if(_mode != cRelease)
    return;

  if(_lastWarehousid == pWarehousid)
    return;

  _lastWarehousid = pWarehousid;

  if(pWarehousid == -1)
  {
    QMessageBox::warning(this, tr("Invalid Site"),
        tr("The selected Site for this Purchase Request is not\n"
           "a \"Supplied At\" Site. You must select a different\n"
           "Site before creating the Purchase Request.") );
    _warehouse->setEnabled(true);
  }
}
