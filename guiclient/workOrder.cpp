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

#include "workOrder.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <QValidator>
#include <QMessageBox>
#include "printWoTraveler.h"
#include "itemCharacteristicDelegate.h"

workOrder::workOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLeadTime(int)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemChar(int)));
  connect(_dueDate, SIGNAL(newDate(const QDate&)), this, SLOT(sUpdateStartDate()));
  connect(_leadTime, SIGNAL(valueChanged(int)), this, SLOT(sUpdateStartDate()));

  _bomRevision->setMode(RevisionLineEdit::Use);
  _bomRevision->setType("BOM");
  _booRevision->setMode(RevisionLineEdit::Use);
  _booRevision->setType("BOO");

  _captive = FALSE;
  _planordid = -1;
  _woid = -1;

  _lastWarehousid = _warehouse->id();
  _lastItemid = -1;
  _comments->setReadOnly(TRUE);
  _woNumber->setValidator(omfgThis->orderVal());
  _qty->setValidator(omfgThis->qtyVal());

  _printTraveler->setEnabled(_privileges->check("PrintWorkOrderPaperWork"));

  _project->setType(ProjectLineEdit::WorkOrder);
  if(!_metrics->boolean("UseProjects"))
    _project->hide();

  _itemchar = new QStandardItemModel(0, 2, this);
  _itemchar->setHeaderData( 0, Qt::Horizontal, tr("Name"), Qt::DisplayRole);
  _itemchar->setHeaderData( 1, Qt::Horizontal, tr("Value"), Qt::DisplayRole);

  _itemcharView->setModel(_itemchar);
  ItemCharacteristicDelegate * delegate = new ItemCharacteristicDelegate(this);
  _itemcharView->setItemDelegate(delegate);

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

  //If not Routing, hide controls
  if (!_metrics->boolean("Routings"))
   _booGroup->hide();

  //If not Revision Control, hide controls
  if (!_metrics->boolean("RevControl"))
   _tabs->removeTab(_tabs->indexOf(_revision));
   
  //If not cost privileges, hide tab
  if (!_privileges->boolean("ViewCosts"))
   _tabs->removeTab(_tabs->indexOf(_costing));

}

/*
 *  Destroys the object and frees any allocated resources
 */
workOrder::~workOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void workOrder::languageChange()
{
    retranslateUi(this);
}

enum SetResponse workOrder::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QString  returnValue;
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);

    _qty->setFocus();
  }

  param = pParams.value("qty", &valid);
  if (valid)
    _qty->setText(formatQty(param.toDouble()));

  param = pParams.value("dueDate", &valid);
  if (valid)
  {
    _dueDate->setDate(param.toDate());
    sUpdateStartDate();
  }

  param = pParams.value("wo_id", &valid);
  if (valid)
    _woid = param.toInt();

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

	  _item->setQuery("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                      "       item_active, item_config, item_type, uom_name "
                      "FROM item JOIN uom ON (item_inv_uom_id=uom_id) "
                      "WHERE (item_type IN ('M', 'B') "
					  "AND (item_active)) ");
      _qtyReceivedLit->clear();
      _tabs->removePage(_tabs->page(3));

      populateWoNumber();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

     _item->setQuery("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                     "       item_active, item_config, item_type, uom_name "
                     "FROM item JOIN uom ON (item_inv_uom_id=uom_id) "
                     "WHERE (item_type IN ('M', 'B', 'J')) ");
      XSqlQuery wo;
      wo.prepare( "SELECT wo_itemsite_id, wo_priority, wo_status,"
                  "       formatWoNumber(wo_id) AS f_wonumber,"
                  "       formatQty(wo_qtyord) AS f_ordered,"
                  "       formatQty(wo_qtyrcv) AS f_received,"
                  "       wo_startdate, wo_duedate,"
                  "       formatMoney(wo_wipvalue) AS f_wipvalue,"
				  "       formatMoney(wo_postedvalue) AS f_postedvalue,"
				  "       formatMoney(wo_postedvalue-wo_wipvalue) AS f_rcvdvalue,"
                  "       wo_prodnotes, wo_prj_id, "
				  "       wo_bom_rev_id, wo_boo_rev_id, "
				  "       wo_cosmethod "
                  "FROM wo "
                  "WHERE (wo_id=:wo_id);" );
      wo.bindValue(":wo_id", _woid);
      wo.exec();
      if (wo.first())
      {
        _oldPriority = wo.value("wo_priority").toInt();
        _oldStartDate = wo.value("wo_startdate").toDate();
        _oldDueDate = wo.value("wo_duedate").toDate();
        _oldQty = wo.value("f_ordered").toString();

        _woNumber->setText(wo.value("f_wonumber").toString());
        _item->setItemsiteid(wo.value("wo_itemsite_id").toInt());
        _priority->setValue(_oldPriority);
		_postedValue->setText(wo.value("f_postedvalue").toString());
		_rcvdValue->setText(wo.value("f_rcvdvalue").toString());
        _wipValue->setText(wo.value("f_wipvalue").toString());
        _qty->setText(wo.value("f_ordered").toString());
        _qtyReceived->setText(wo.value("f_received").toString());
        _startDate->setDate(_oldStartDate);
        _dueDate->setDate(_oldDueDate);
        _productionNotes->setText(wo.value("wo_prodnotes").toString());
        _comments->setId(_woid);
        _project->setId(wo.value("wo_prj_id").toInt());
		_bomRevision->setId(wo.value("wo_bom_rev_id").toInt());
		_booRevision->setId(wo.value("wo_boo_rev_id").toInt());

		if (wo.value("wo_cosmethod").toString() == "D")
		  _todate->setChecked(TRUE);
		else if (wo.value("wo_cosmethod").toString() == "P")
		  _proportional->setChecked(TRUE);
		else
	      _jobCosGroup->hide();

        // If the W/O is closed or Released don't allow changing some items.
        if(wo.value("wo_status").toString() == "C" || wo.value("wo_status") == "R")
        {
          _priority->setEnabled(false);
          _qty->setEnabled(false);
          _dueDate->setEnabled(false);
          _startDate->setEnabled(false);
        }

        _startDate->setEnabled(true);
        _woNumber->setEnabled(false);
        _item->setReadOnly(true);
	    _bomRevision->setEnabled(wo.value("wo_status").toString() == "O");
        _booRevision->setEnabled(wo.value("wo_status").toString() == "O");
        _warehouse->setEnabled(false);
        _comments->setReadOnly(false);
        _leadTimeLit->hide();
        _leadTime->hide();
        _daysLit->hide();
        _printTraveler->hide();
        _bottomSpacer->hide();
        _create->setText(tr("&Save"));

        _close->setFocus();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2, W/O ID %3.")
                          .arg(__FILE__)
                          .arg(__LINE__)
                          .arg(_woid) );
        close();
      }
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setQuery("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                      "       item_active, item_config, item_type, uom_name "
                      "FROM item JOIN uom ON (item_inv_uom_id=uom_id) "
                      "WHERE (item_type IN ('M', 'B', 'J')) ");
      XSqlQuery wo;
      wo.prepare( "SELECT wo_itemsite_id, wo_priority,"
                  "       formatWoNumber(wo_id) AS f_wonumber,"
                  "       formatQty(wo_qtyord) AS f_ordered,"
                  "       formatQty(wo_qtyrcv) AS f_received,"
                  "       wo_startdate, wo_duedate,"
                  "       formatMoney(wo_wipvalue) AS f_wipvalue,"
				  "       formatMoney(wo_postedvalue) AS f_postedvalue,"
				  "       formatMoney(wo_postedvalue-wo_wipvalue) AS f_rcvdvalue,"
                  "       wo_prodnotes, wo_prj_id, wo_bom_rev_id, "
				  "       wo_boo_rev_id, wo_cosmethod "
                  "FROM wo "
                  "WHERE (wo_id=:wo_id);" );
      wo.bindValue(":wo_id", _woid);
      wo.exec();
      if (wo.first())
      {
        _mode = cView;

        _woNumber->setText(wo.value("f_wonumber").toString());
        _item->setItemsiteid(wo.value("wo_itemsite_id").toInt());
        _priority->setValue(wo.value("wo_priority").toInt());
		_postedValue->setText(wo.value("f_postedvalue").toString());
		_rcvdValue->setText(wo.value("f_rcvdvalue").toString());
        _wipValue->setText(wo.value("f_wipvalue").toString());
        _qty->setText(wo.value("f_ordered").toString());
        _qtyReceived->setText(wo.value("f_received").toString());
        _startDate->setDate(wo.value("wo_startdate").toDate());
        _dueDate->setDate(wo.value("wo_duedate").toDate());
        _productionNotes->setText(wo.value("wo_prodnotes").toString());
        _comments->setId(_woid);
        _project->setId(wo.value("wo_prj_id").toInt());
		_bomRevision->setId(wo.value("wo_bom_rev_id").toInt());
		_booRevision->setId(wo.value("wo_boo_rev_id").toInt());

		if (wo.value("wo_cosmethod").toString() == "D")
		  _todate->setChecked(TRUE);
		else if (wo.value("wo_cosmethod").toString() == "P")
		  _proportional->setChecked(TRUE);
		else
	      _jobCosGroup->hide();
 
        _woNumber->setEnabled(FALSE);
        _item->setReadOnly(TRUE);
        _bomRevision->setEnabled(FALSE);
        _booRevision->setEnabled(FALSE);
        _warehouse->setEnabled(FALSE);
        _priority->setEnabled(FALSE);
        _qty->setEnabled(FALSE);
        _startDate->setEnabled(FALSE);
        _dueDate->setEnabled(FALSE);
        _productionNotes->setReadOnly(TRUE);
        _create->hide();
        _leadTimeLit->hide();
        _leadTime->hide();
        _daysLit->hide();
        _printTraveler->hide();
        _bottomSpacer->hide();
        _close->setText(tr("&Close"));
        _project->setEnabled(FALSE);
        _itemcharView->setEnabled(false);
		_jobCosGroup->setEnabled(FALSE);
        
        _close->setFocus();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2, W/O ID %3.")
                          .arg(__FILE__)
                          .arg(__LINE__)
                          .arg(_woid) );
        close();
      }
    }
    else if (param.toString() == "release")
    {
      _mode = cRelease;
      _tabs->removePage(_tabs->page(3));

      q.prepare( "SELECT planord_itemsite_id, planord_duedate,"
                 "       CASE WHEN(planord_mps) THEN 'P'"
                 "            ELSE 'M'"
                 "       END AS sourcetype,"
                 "       CASE WHEN(planord_mps) THEN planord_pschitem_id"
                 "            ELSE planord_id"
                 "       END AS sourceid,"
                 "       formatQty(planord_qty) AS qty "
                 "FROM planord "
                 "WHERE (planord_id=:planord_id);" );
      q.bindValue(":planord_id", _planordid);
      q.exec();
      if (q.first())
      {
        _item->setReadOnly(TRUE);
        _warehouse->setEnabled(FALSE);

        _planordtype=q.value("sourcetype").toString();
        _sourceid=q.value("sourceid").toInt();
        _qty->setText(q.value("qty").toString());
        _dueDate->setDate(q.value("planord_duedate").toDate());
        _item->setItemsiteid(q.value("planord_itemsite_id").toInt());

        sUpdateStartDate();
        populateWoNumber();

        _qty->setEnabled(FALSE);
        _qtyReceivedLit->clear();
        _startDate->setEnabled(FALSE);
        _dueDate->setEnabled(FALSE);
        _wipValueLit->hide();
        _wipValue->hide();
        _leadTimeLit->hide();
        _leadTime->hide();
        _daysLit->hide();

        _create->setFocus();
      }
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2, Planned Order ID %3.")
                          .arg(__FILE__)
                          .arg(__LINE__)
                          .arg(_planordid) );
        close();
      }
    }
  }
  
  return NoError;
}

void workOrder::sCreate()
{
  if(cRelease == _mode || cNew == _mode)
  {
    if (!_woNumber->text().length())
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr("You must enter a valid Work Order number before creating this Work Order")  );
      _woNumber->setFocus();
      return;
    }
  
    if (!(_item->isValid()))
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr("You must enter or select a valid Item number before creating this Work Order")  );
      return;
    }
  
    if ((!_qty->text().length()) || (_qty->toDouble() == 0.0))
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr( "You have entered an invalid Qty. Ordered.\n"
                                    "Please correct before creating this Work Order"  ) );
      _qty->setFocus();
      return;
    }
  
    if (!_dueDate->isValid())
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr( "You have entered an invalid Due Date.\n"
                                    "Please correct before creating this Work Order"  ) );
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
          tr("The Warehouse which you selected for this Item is not\n"
             "a \"Supplied At\" Warehouse. You must select a different\n"
             " Warehouse before creating this Work Order.") );
      return;
    }
  
    int itemsiteid = q.value("itemsite_id").toInt();
  
    q.prepare("SELECT validateOrderQty(:itemsite_id, :qty, TRUE) AS qty;");
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":qty", _qty->toDouble());
    q.exec();
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
  
    double orderQty = q.value("qty").toDouble();
    if (orderQty != _qty->toDouble())
    {
      if ( QMessageBox::warning( this, tr("Invalid Order Quantitiy"),
                                 tr( "Order Parameters for this Item do not allow a quantitiy of %1 to be created.\n"
                                     "You must create an order for at least %2 of this item.\n"
                                     "Do you want to update the order quantity and create the order?" )
                                 .arg(formatQty(_qty->toDouble()))
                                 .arg(formatQty(orderQty)),
                                 tr("&Yes"), tr("&No"), 0, 1 ) == 1)
        return; 
    }
  
    q.prepare( "SELECT createWo( :woNumber, :itemsite_id, :priority, :orderQty,"
               "                 :leadTime, :dueDate, :productionNotes, :ordtype, :ordid, :prj_id,"
			   "                 :bom_rev_id, :boo_rev_id) AS result;" );
    q.bindValue(":woNumber", _woNumber->text().toInt());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":priority", _priority->value());
    q.bindValue(":orderQty", orderQty);
    q.bindValue(":leadTime", _leadTime->value());
    q.bindValue(":dueDate", _dueDate->date());
    q.bindValue(":productionNotes", _productionNotes->text());
    q.bindValue(":prj_id", _project->id());
	q.bindValue(":bom_rev_id", _bomRevision->id());
	q.bindValue(":boo_rev_id", _booRevision->id());
    if(cRelease == _mode)
    {
      q.bindValue(":ordtype", _planordtype);
      q.bindValue(":ordid", _sourceid);
    }
    else
    {
      q.bindValue(":ordtype", QString(""));
      q.bindValue(":ordid", -1);
    }
    q.exec();
    if (!q.first())
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
  
    int _woid = q.value("result").toInt();
  
    q.prepare("SELECT updateCharAssignment('W', :target_id, :char_id, :char_value);");
  
    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_id", _woid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
    }
  
  
    if (_mode == cRelease)
    {
      q.prepare("SELECT releasePlannedOrder(:planord_id, FALSE) AS result;");
      q.bindValue(":planord_id", _planordid);
      q.exec();
    }
  
    if (_woid == -1)
      QMessageBox::critical( this, tr("Work Order not Exploded"),
                             tr( "The Work Order was created but not Exploded as there is no valid Bill of Materials for the selected Item.\n"
                                 "You must create a valid Bill of Materials before you may explode this Work Order." ));
    else if (_woid == -2)
      QMessageBox::critical( this, tr("Work Order not Exploded"),
                             tr( "The Work Order was created but not Exploded as Component Items defined in the Bill of Materials for\n"
                                 "the selected Work Order Item do not exist in the selected  Work Order Warehouse.\n"
                                 "You must create Item Sites for these Component Items before you may explode this Work Order." ));
    else
    {
      if (_printTraveler->isChecked())
      {
        ParameterList params;
        params.append("wo_id", _woid);
  
        printWoTraveler newdlg(this, "", TRUE);
        newdlg.set(params);
        newdlg.exec();
      }
    }
  }
  else if(cEdit == _mode)
  {
    if ((!_qty->text().length()) || (_qty->toDouble() == 0.0))
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr( "You have entered an invalid Qty. Ordered.\n"
                                    "Please correct before creating this Work Order"  ) );
      _qty->setFocus();
      return;
    }
  
    if (!_dueDate->isValid())
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr( "You have entered an invalid Due Date.\n"
                                    "Please correct before updating this Work Order"  ) );
      _dueDate->setFocus();
      return;
    }
  
    if (!_startDate->isValid())
    {
      QMessageBox::information( this, tr("Cannot Create Work Order"),
                                tr( "You have entered an invalid Start Date.\n"
                                    "Please correct before updating this Work Order"  ) );
      _dueDate->setFocus();
      return;
    }

    q.prepare("UPDATE wo"
              "   SET wo_prodnotes=:productionNotes,"
              "       wo_prj_id=:prj_id,"
			  "       wo_bom_rev_id=:bom_rev_id,"
			  "       wo_boo_rev_id=:boo_rev_id,"
			  "       wo_cosmethod=:wo_cosmethod"
              " WHERE (wo_id=:wo_id); ");
    q.bindValue(":wo_id", _woid);
    q.bindValue(":productionNotes", _productionNotes->text());
    q.bindValue(":prj_id", _project->id());
	q.bindValue(":bom_rev_id", _bomRevision->id());
	q.bindValue(":boo_rev_id", _booRevision->id());
	if (_todate->isChecked() && _jobCosGroup->isEnabled())
		q.bindValue(":wo_cosmethod",QString("D"));
	else if (_proportional->isChecked() && _jobCosGroup->isEnabled())
		q.bindValue(":wo_cosmethod",QString("P"));
    q.exec();

    q.prepare("SELECT updateCharAssignment('W', :target_id, :char_id, :char_value);");
    QModelIndex idx1, idx2;
    for(int i = 0; i < _itemchar->rowCount(); i++)
    {
      idx1 = _itemchar->index(i, 0);
      idx2 = _itemchar->index(i, 1);
      q.bindValue(":target_id", _woid);
      q.bindValue(":char_id", _itemchar->data(idx1, Qt::UserRole));
      q.bindValue(":char_value", _itemchar->data(idx2, Qt::DisplayRole));
      q.exec();
    }

    if(_startDate->date() != _oldStartDate || _dueDate->date() != _oldDueDate)
    {
      q.prepare("SELECT changeWoDates(:wo_id, :startDate, :dueDate, :rescheduleChildren);");
      q.bindValue(":wo_id", _woid);
      q.bindValue(":startDate", _startDate->date());
      q.bindValue(":dueDate", _dueDate->date());
      q.bindValue(":rescheduleChildren", QVariant(true, 0));
      q.exec();
    }

    if(_priority->value() != _oldPriority)
    {
      q.prepare("SELECT reprioritizeWo(:wo_id, :newPriority, :reprioritizeChildren);");
      q.bindValue(":wo_id", _woid);
      q.bindValue(":newPriority", _priority->value());
      q.bindValue(":reprioritizeChildren", QVariant(true, 0));
      q.exec();
    }

    if(_qty->text() != _oldQty)
    {
      double newQty = _qty->toDouble();
      q.prepare( "SELECT validateOrderQty(wo_itemsite_id, :qty, TRUE) AS qty "
                 "FROM wo "
                 "WHERE (wo_id=:wo_id);" );
      q.bindValue(":wo_id", _woid);
      q.bindValue(":qty", newQty);
      q.exec();
      if (q.first())
      {
        if (q.value("qty").toDouble() != newQty)
        {
          if ( QMessageBox::warning( this, tr("Invalid Order Qty"),
                                     tr( "The new Order Quantity that you have entered does not meet the Order Parameters set\n"
                                         "for the parent Item Site for this Work Order.  In order to meet the Item Site Order\n"
                                         "Parameters the new Order Quantity must be increased to %1.\n"
                                         "Do you want to change the Order Quantity for this Work Order to %2?" )
                                     .arg(formatQty(q.value("qty").toDouble()))
                                     .arg(formatQty(q.value("qty").toDouble())),
                                     tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 1 )
            return;
          else
            newQty = q.value("qty").toDouble();
        }

        q.prepare("SELECT changeWoQty(:wo_id, :qty, TRUE);");
        q.bindValue(":wo_id", _woid);
        q.bindValue(":qty", newQty);
        q.exec();
      }
    }
  }
  
  omfgThis->sWorkOrdersUpdated(_woid, TRUE);
  
  if (_captive)
    close();
  else
  {
    populateWoNumber();
    _item->setId(-1);
    _qty->clear();
    _dueDate->setNull();
    _leadTime->setValue(0);
    _startDate->clear();
    _productionNotes->clear();
    _itemchar->removeRows(0, _itemchar->rowCount());
    _close->setText(tr("&Close"));

    _item->setFocus();
  }

}

void workOrder::sUpdateStartDate()
{
  XSqlQuery startDate;
  startDate.prepare("SELECT (DATE(:dueDate) - :leadTime) AS startdate;");
  startDate.bindValue(":dueDate", _dueDate->date());
  startDate.bindValue(":leadTime", _leadTime->value());
  startDate.exec();
  if (startDate.first())
    _startDate->setDate(startDate.value("startdate").toDate());
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void workOrder::sPopulateItemChar( int pItemid )
{
  _itemchar->removeRows(0, _itemchar->rowCount());
  if (pItemid != -1)
  {
    q.prepare( "SELECT DISTINCT char_id, char_name,"
               "       COALESCE(b.charass_value, (SELECT c.charass_value FROM charass c WHERE ((c.charass_target_type='I') AND (c.charass_target_id=:item_id) AND (c.charass_default) AND (c.charass_char_id=char_id)) LIMIT 1)) AS charass_value"
               "  FROM charass a, char "
               "    LEFT OUTER JOIN charass b"
               "      ON (b.charass_target_type='W'"
               "      AND b.charass_target_id=:wo_id"
               "      AND b.charass_char_id=char_id) "
               " WHERE ( (a.charass_char_id=char_id)"
               "   AND   (a.charass_target_type='I')"
               "   AND   (a.charass_target_id=:item_id) ) "
               " ORDER BY char_name;" );
    q.bindValue(":item_id", pItemid);
    q.bindValue(":wo_id", _woid);
    q.exec();
    int row = 0;
    QModelIndex idx;
    while(q.next())
    {
      _itemchar->insertRow(_itemchar->rowCount());
      idx = _itemchar->index(row, 0);
      _itemchar->setData(idx, q.value("char_name"), Qt::DisplayRole);
      _itemchar->setData(idx, q.value("char_id"), Qt::UserRole);
      idx = _itemchar->index(row, 1);
      _itemchar->setData(idx, q.value("charass_value"), Qt::DisplayRole);
      _itemchar->setData(idx, pItemid, Qt::UserRole);
      row++;
    }
  }
}

void workOrder::sPopulateLeadTime(int pWarehousid)
{
  if(_lastWarehousid==pWarehousid && _lastItemid==_item->id())
    return;

  _lastItemid = _item->id();
  _lastWarehousid = pWarehousid;

  q.prepare( "SELECT itemsite_leadtime "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();

  if (q.first())
    _leadTime->setValue(q.value("itemsite_leadtime").toInt());
  else
  {
    QMessageBox::warning(this, tr("Invalid Warehouse"),
        tr("The selected Warehouse for this Work Order is not\n"
           "a \"Supplied At\" Warehouse. You must select a different\n"
           "Warehouse before creating the Work Order.") );
    _warehouse->setEnabled(TRUE);
  }
}

void workOrder::populateWoNumber()
{
  QString generationMethod = _metrics->value("WONumberGeneration");

  if ((generationMethod == "A") || (generationMethod == "O"))
  {
    q.exec("SELECT fetchWoNumber() AS woNumber;");
    if (q.first())
      _woNumber->setText(q.value("woNumber").toString());
    else
    {
      _woNumber->setText("Error");

      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );

      return;
    }
  }

  if (generationMethod == "M")
  {
    _woNumber->setEnabled(TRUE);
    _woNumber->setFocus();
  }
  else if (generationMethod == "O")
  {
    _woNumber->setEnabled(TRUE);
    _item->setFocus();
  }
  else if (generationMethod == "A")
  {
    _woNumber->setEnabled(FALSE);
    _item->setFocus();
  } 
}

void workOrder::sClose()
{
  if ( ( (_mode == cNew) || (_mode == cRelease)) &&
       ((_metrics->value("WONumberGeneration") == "A") || (_metrics->value("WONumberGeneration") == "O")) )
  {
    q.prepare("SELECT releaseWoNumber(:woNumber);");
    q.bindValue(":woNumber", _woNumber->text().toInt());
    q.exec();
  }

  close();
}


