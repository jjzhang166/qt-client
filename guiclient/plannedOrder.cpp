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

#include "plannedOrder.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

plannedOrder::plannedOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLeadTime(int)));
  connect(_dueDate, SIGNAL(newDate(const QDate &)), this, SLOT(sUpdateStartDate()));
  connect(_leadTime, SIGNAL(valueChanged(int)), this, SLOT(sUpdateStartDate()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _qty->setValidator(omfgThis->qtyVal());

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
plannedOrder::~plannedOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void plannedOrder::languageChange()
{
    retranslateUi(this);
}


enum SetResponse plannedOrder::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _item->setReadOnly(TRUE);
    _warehouse->setEnabled(FALSE);

    _qty->setFocus();
  }
  else
  {
    _captive = FALSE;

    _item->setFocus();
  }

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      populateFoNumber();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _item->setReadOnly(TRUE);
      _warehouse->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _leadTimeLit->hide();
      _leadTime->hide();
      _daysLit->hide();

      q.prepare( "SELECT fo_itemsite_id,"
                 "       formatFoNumber(fo_id) AS fonumber,"
                 "       formatQty(fo_qtyord) AS ordered,"
                 "       fo_startdate, fo_duedate "
                 "FROM fo "
                 "WHERE (fo_id=:fo_id);" );
      q.bindValue(":fo_id", _planordid);
      q.exec();
      if (q.first())
      {
        _number->setText(q.value("fonumber").toString());
        _qty->setText(q.value("ordered").toString());
        _startDate->setDate(q.value("fo_startdate").toDate());
        _dueDate->setDate(q.value("fo_duedate").toDate());
        _item->setItemsiteid(q.value("fo_itemsite_id").toInt());
      }
    }
  }

  return NoError;
}

void plannedOrder::sClose()
{
  if (_mode == cNew)
  {
    q.prepare("SELECT releasePlanNumber(:orderNumber);");
    q.bindValue(":orderNumber", _number->text().toInt());
    q.exec();
  }

  reject();
}

void plannedOrder::sCreate()
{
  if (!(_item->isValid()))
  {
    QMessageBox::information( this, tr("No Item Number Selected"),
                              tr("You must enter or select a valid Item number before creating this Planned Order")  );
    return;
  }

  if (!_qty->text().length())
  {
    QMessageBox::information( this, tr("Invalid Quantity Ordered"),
                              tr( "You have entered an invalid Qty. Ordered.\n"
                                  "Please correct before creating this Planned Order"  ) );
    _qty->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::information( this, tr("Invalid Due Date Entered"),
                              tr( "You have entered an invalid Due Date.\n"
                                  "Please correct before creating this Planned Order"  ) );
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
    QMessageBox::information( this, tr("Invalid Item Site"),
                              tr("The Item and Warehouse entered is and invalid Item Site combination.")  );
    return;
  }

  int itemsiteid = q.value("itemsite_id").toInt();

  q.prepare( "SELECT createPlannedOrder( :orderNumber, :itemsite_id, :qty, "
             "                           (DATE(:dueDate) - :leadTime), :dueDate) AS result;" );
  q.bindValue(":orderNumber", _number->text().toInt());
  q.bindValue(":itemsite_id", itemsiteid);
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":dueDate", _dueDate->date());
  q.bindValue(":leadTime", _leadTime->value());
  q.exec();
  if (!q.first())
  {
    systemError( this, tr("A System Error occurred at %1::%2.")
                       .arg(__FILE__)
                       .arg(__LINE__) );
    return;
  }

  int foid = XDialog::Rejected;
  switch (q.value("result").toInt())
  {
    case -1:
      QMessageBox::critical( this, tr("Planned Order not Exploded"),
                             tr( "The Planned Order was created but not Exploded as there is no valid Bill of Materials for the selected Item.\n"
                                 "You must create a valid Bill of Materials before you may explode this Planned Order." ));
      break;
  
    case -2:
      QMessageBox::critical( this, tr("Planned Order not Exploded"),
                             tr( "The Planned Order was created but not Exploded as Component Items defined in the Bill of Materials\n"
                                 "for the selected Planned Order Item do not exist in the selected Planned Order Warehouse.\n"
                                 "You must create Item Sites for these Component Items before you may explode this Planned Order." ));
      break;

    default:
      foid = q.value("result").toInt();
      break;
  }

  if (_captive)
    done(foid);
  else
  {
    populateFoNumber();
    _item->setId(-1);
    _qty->clear();
    _dueDate->setNull();
    _leadTime->setValue(0);
    _startDate->setNull();
    _close->setText(tr("&Close"));

    _item->setFocus();
  }
}

void plannedOrder::sUpdateStartDate()
{
  if(_dueDate->isValid())
    _startDate->setDate(_dueDate->date().addDays(_leadTime->value() * -1));
}

void plannedOrder::sPopulateLeadTime(int pWarehousid)
{
  q.prepare( "SELECT itemsite_leadtime "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  if (!q.first())
// ToDo - should issue a system error message here
    return;

  _leadTime->setValue(q.value("itemsite_leadtime").toInt());
}

void plannedOrder::populateFoNumber()
{
  q.exec("SELECT fetchPlanNumber() AS foNumber;");
  if (q.first())
    _number->setText(q.value("foNumber").toString());
  else
  {
//  ToDo - should issue a system error message here
    _number->setText("Error");
    return;
  }
}

