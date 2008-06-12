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

#include "purchaseRequest.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

#include <QCloseEvent>

purchaseRequest::purchaseRequest(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sCheckWarehouse(int)));

  _planordid = -1;
  _item->setType(ItemLineEdit::cGeneralPurchased);
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

/*
 *  Destroys the object and frees any allocated resources
 */
purchaseRequest::~purchaseRequest()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void purchaseRequest::languageChange()
{
    retranslateUi(this);
}

enum SetResponse purchaseRequest::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  int      prid = -1;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qty->setText(formatQty(param.toDouble()));

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
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _project->setEnabled(FALSE);
      _create->hide();

      q.prepare( "SELECT pr_itemsite_id,"
                 "       pr_number,"
                 "       formatQty(wo_qtyreq) AS ordered,"
                 "       pr_duedate,"
                 "       pr_prj_id "
                 "FROM pr "
                 "WHERE (pr_id=:pr_id);" );
      q.bindValue(":pr_id", prid);
      if (q.first())
      {
        _number->setText(q.value("pr_number").toString());
        _item->setItemsiteid(q.value("pr_itemsite_id").toInt());
        _qty->setText(q.value("ordered").toString());
        _dueDate->setDate(q.value("pr_duedate").toDate());
        _project->setId(q.value("pr_prj_id").toInt());
      }
    }
    else if (param.toString() == "release")
    {
      _mode = cRelease;
      _captive = TRUE;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);

      q.prepare( "SELECT planord_itemsite_id, planord_duedate,"
                 "       formatQty(planord_qty) AS qty "
                 "FROM planord "
                 "WHERE (planord_id=:planord_id);" );
      q.bindValue(":planord_id", _planordid);
      q.exec();
      if (q.first())
      {
        _item->setItemsiteid(q.value("planord_itemsite_id").toInt());
        _qty->setText(q.value("qty").toString());
        _dueDate->setDate(q.value("planord_duedate").toDate());

        populateNumber();
      }

      _create->setFocus();
    }
  }

  return NoError;
}

void purchaseRequest::sClose()
{
  closeEvent(NULL);
  reject();
}

void purchaseRequest::sCreate()
{
  if (!_number->text().length())
  {
    QMessageBox::information( this, tr("Invalid Purchase Request Number"),
                              tr("You must enter a valid Purchase Request number before creating this Purchase Request")  );
    _number->setFocus();
    return;
  }

  if (!(_item->isValid()))
  {
    QMessageBox::information( this, tr("No Item Number Selected"),
                              tr("You must enter or select a valid Item number before creating this Purchase Request")  );
    return;
  }

  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::information( this, tr("Invalid Quantity Ordered"),
                              tr( "You have entered an invalid Qty. Ordered.\n"
                                  "Please correct before creating this Purchase Request."  ) );
    _qty->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::information( this, tr("Invalid Due Date Entered"),
                              tr( "You have entered an invalid Due Date.\n"
                                  "Please correct before creating this Purchase Request."  ) );
    _dueDate->setFocus();
    return;
  }

  q.prepare( "SELECT itemsite_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (!q.first())
  {
    QMessageBox::warning(this, tr("Invalid Warehouse"),
        tr("The selected Warehouse for this Purchase Request is not\n"
           "a \"Supplied At\" Warehouse. You must select a different\n"
           "Warehouse before creating the Purchase Request.") );
    return;
  }

  int prid = -1;
  int itemsiteid = q.value("itemsite_id").toInt();

  if (_mode == cNew)
  {
    q.prepare( "SELECT createPr( :orderNumber, :itemsite_id, :qty,"
               "                 :dueDate, :notes ) AS prid;" );
    q.bindValue(":orderNumber", _number->text().toInt());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":dueDate", _dueDate->date());
    q.bindValue(":notes", _notes->text());
    q.exec();
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
    else
      prid = q.value("prid").toInt();
  }
  else if (_mode == cRelease)
  {
    q.prepare("SELECT createPr(:orderNumber, 'F', :planord_id) AS prid;");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.bindValue(":planord_id", _planordid);
    q.exec();
    if (q.first())
    {
      prid = q.value("prid").toInt();

      q.prepare("SELECT releasePlannedOrder(:planord_id, FALSE) AS result;");
      q.bindValue(":planord_id", _planordid);
      q.exec();
    }
  }

  if(-1 != prid)
  {
    q.prepare("UPDATE pr SET pr_prj_id=:prj_id WHERE (pr_id=:pr_id);");
    q.bindValue(":pr_id", prid);
    q.bindValue(":prj_id", _project->id());
    q.exec();
  }

  if (_captive)
    done(prid);
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

void purchaseRequest::populateNumber()
{
  QString generationMethod = _metrics->value("PrNumberGeneration");

  if ((generationMethod == "A") || (generationMethod == "O"))
  {
    q.exec("SELECT fetchPrNumber() AS number;");
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );

      _number->setText("Error");
      return;
    }

    _number->setText(q.value("number").toString());
  }

  if (generationMethod == "M")
  {
    _number->setEnabled(TRUE);
    _number->setFocus();
  }
  else if (generationMethod == "O")
  {
    _number->setEnabled(TRUE);
    _item->setFocus();
  }
  else if (generationMethod == "A")
  {
    _number->setEnabled(FALSE);
    _item->setFocus();
  } 
}

void purchaseRequest::closeEvent(QCloseEvent *pEvent)
{
  if ( (_mode == cNew) &&
       ((_metrics->value("PrNumberGeneration") == "A") || (_metrics->value("PrNumberGeneration") == "O")) )
  {
    q.prepare("SELECT releasePrNumber(:orderNumber);");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.exec();
  }

  if (pEvent != NULL)
    XDialog::closeEvent(pEvent);
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
    QMessageBox::warning(this, tr("Invalid Warehouse"),
        tr("The selected Warehouse for this Purchase Request is not\n"
           "a \"Supplied At\" Warehouse. You must select a different\n"
           "Warehouse before creating the Purchase Request.") );
    _warehouse->setEnabled(TRUE);
  }
}
