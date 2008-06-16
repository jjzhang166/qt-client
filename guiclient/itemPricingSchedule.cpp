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

#include "itemPricingSchedule.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "itemPricingScheduleItem.h"

/*
 *  Constructs a itemPricingSchedule as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemPricingSchedule::itemPricingSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_currency, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _ipsitem->addColumn(tr("Type"),            _ynColumn,    Qt::AlignLeft  );
  _ipsitem->addColumn(tr("Item/Prod. Cat."), _itemColumn,  Qt::AlignLeft  );
  _ipsitem->addColumn(tr("Description"),     -1,           Qt::AlignLeft  );
  _ipsitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter);
  _ipsitem->addColumn(tr("Qty. Break"),      _qtyColumn,   Qt::AlignRight );
  _ipsitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter);
  _ipsitem->addColumn(tr("Price/Discount"),  _priceColumn, Qt::AlignRight );

  _currency->setType(XComboBox::Currencies);
  _currency->setLabel(_currencyLit);
  _updated = QDate::currentDate();
  
  q.exec("BEGIN;");
  _rejectedMsg = tr("The application has encountered an error and must "
                    "stop editing this Pricing Schedule.\n%1");
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemPricingSchedule::~itemPricingSchedule()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemPricingSchedule::languageChange()
{
  retranslateUi(this);
}

enum SetResponse itemPricingSchedule::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _currency->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("ipshead_id", &valid);
  if (valid)
  {
    _ipsheadid = param.toInt();
    populate();
  }

  if ( (_mode == cNew) || (_mode == cEdit) || (_mode == cCopy) )
  {
    connect(_ipsitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ipsitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }

  if ( (_mode == cNew) )
  {
    q.exec("SELECT NEXTVAL('ipshead_ipshead_id_seq') AS ipshead_id;");
    if (q.first())
      _ipsheadid = q.value("ipshead_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
        return UndefinedError;
    }
  }

  return NoError;
}

void itemPricingSchedule::sSave(bool p)
{
  if (_name->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Name"),
                           tr("You must enter a Name for this Pricing Schedule.") );
    _name->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Effective Date"),
                           tr("You must enter an effective date for this Pricing Schedule.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("You must enter an expiration date for this Pricing Schedule.") );
    _dates->setFocus();
    return;
  }

  if (_mode == cNew) 
    q.prepare( "INSERT INTO ipshead "
               "( ipshead_id, ipshead_name, ipshead_descrip,"
               "  ipshead_effective, ipshead_expires, "
	       "  ipshead_curr_id, ipshead_updated ) "
               "VALUES "
               "( :ipshead_id, :ipshead_name, :ipshead_descrip,"
               "  :ipshead_effective, :ipshead_expires, "
	       "  :ipshead_curr_id, CURRENT_DATE );" );
  else if ( (_mode == cEdit) || (_mode == cCopy) )
    q.prepare( "UPDATE ipshead "
               "SET ipshead_name=:ipshead_name, ipshead_descrip=:ipshead_descrip,"
               "    ipshead_effective=:ipshead_effective, ipshead_expires=:ipshead_expires, "
	       "    ipshead_curr_id=:ipshead_curr_id, "
	       "    ipshead_updated=CURRENT_DATE "
               "WHERE (ipshead_id=:ipshead_id);" );

  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":ipshead_name", _name->text());
  q.bindValue(":ipshead_descrip", _descrip->text());
  q.bindValue(":ipshead_effective", _dates->startDate());
  q.bindValue(":ipshead_expires", _dates->endDate());
  q.bindValue(":ipshead_curr_id", _currency->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  _mode = cEdit;
  
  if (p)
  {
    q.exec("COMMIT;");
    done(_ipsheadid);
  }
}

void itemPricingSchedule::sSave()
{
  sSave(true) ;  
}

void itemPricingSchedule::sCheck()
{
}

void itemPricingSchedule::sNew()
{
  sSave(false);
  ParameterList params;
  params.append("mode", "new");
  params.append("ipshead_id", _ipsheadid);
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);

  itemPricingScheduleItem newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    if (result == -1)
      reject();
    else
      sFillList(result);
}

void itemPricingSchedule::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("curr_id", _currency->id());
  params.append("updated", _updated);

  if(_ipsitem->altId() == 1)
    params.append("ipsitem_id", _ipsitem->id());
  else if(_ipsitem->altId() == 2)
    params.append("ipsprodcat_id", _ipsitem->id());
  else
    return;
    // ToDo - tell the user why we're not showing the pricing sched?

  itemPricingScheduleItem newdlg(this, "", TRUE);
  newdlg.set(params);

  int result;
  if ((result = newdlg.exec()) != XDialog::Rejected)
    if (result == -1)
      reject();
    else
      sFillList(result);
}

void itemPricingSchedule::sDelete()
{
  if(_ipsitem->altId() == 1)
    q.prepare( "DELETE FROM ipsitem "
               "WHERE (ipsitem_id=:ipsitem_id);" );
  else if(_ipsitem->altId() == 2)
    q.prepare( "DELETE FROM ipsprodcat "
               "WHERE (ipsprodcat_id=:ipsitem_id);" );
  else
    return;
  q.bindValue(":ipsitem_id", _ipsitem->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  sFillList();
}

void itemPricingSchedule::sFillList()
{
  sFillList(-1);
}

void itemPricingSchedule::sFillList(int pIpsitemid)
{
  q.prepare( "SELECT ipsitem_id, 1 AS altid, :item, item_number AS number,"
             "       (item_descrip1 || ' ' || item_descrip2),"
             "       qty.uom_name, formatQty(ipsitem_qtybreak), price.uom_name, formatSalesPrice(ipsitem_price),"
             "       ipsitem_qtybreak AS qtybreak"
             "  FROM ipsitem, item, uom AS qty, uom AS price "
             " WHERE ( (ipsitem_item_id=item_id)"
             "   AND   (ipsitem_qty_uom_id=qty.uom_id)"
             "   AND   (ipsitem_price_uom_id=price.uom_id)"
             "   AND   (ipsitem_ipshead_id=:ipshead_id) )"
             " UNION "
             "SELECT ipsprodcat_id, 2 AS altid, :prodcat, prodcat_code AS number,"
             "       prodcat_descrip,"
             "       '', formatQty(ipsprodcat_qtybreak), '', formatPrcnt(ipsprodcat_discntprcnt)||'%',"
             "       ipsprodcat_qtybreak AS qtybreak"
             "  FROM ipsprodcat, prodcat"
             " WHERE ( (ipsprodcat_prodcat_id=prodcat_id)"
             "   AND   (ipsprodcat_ipshead_id=:ipshead_id) )"
             " ORDER BY altid, number, qtybreak;" );
  q.bindValue(":ipshead_id", _ipsheadid);
  q.bindValue(":item", tr("Item"));
  q.bindValue(":prodcat", tr("Prod. Cat."));
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
        reject();
  }

  if (pIpsitemid == -1)
    _ipsitem->populate(q, true);
  else
    _ipsitem->populate(q, pIpsitemid, true);

  _currency->setEnabled(_ipsitem->topLevelItemCount() <= 0);
}

void itemPricingSchedule::populate()
{
  XSqlQuery pop;
  pop.prepare( "SELECT ipshead_name, ipshead_descrip,"
             "       ipshead_effective, ipshead_expires, "
	     "       ipshead_curr_id, ipshead_updated "
             "FROM ipshead "
             "WHERE (ipshead_id=:ipshead_id);" );
  pop.bindValue(":ipshead_id", _ipsheadid);
  pop.exec();
  if (pop.first())
  {
    _name->setText(pop.value("ipshead_name").toString());
    _descrip->setText(pop.value("ipshead_descrip").toString());
    _dates->setStartDate(pop.value("ipshead_effective").toDate());
    _dates->setEndDate(pop.value("ipshead_expires").toDate());
    _currency->setId(pop.value("ipshead_curr_id").toInt());
    _currency->setEnabled(FALSE);
    QDate tmpDate = pop.value("ipshead_updated").toDate();
    if (tmpDate.isValid() && ! tmpDate.isNull())
	_updated = tmpDate;

    sFillList(-1);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
	systemError(this, _rejectedMsg.arg(q.lastError().databaseText()),
                  __FILE__, __LINE__);
		  reject();
  }
}

void itemPricingSchedule::reject()
{
  q.exec("ROLLBACK;");
  if(_mode == cCopy) 
  {
    q.prepare("DELETE FROM ipshead WHERE (ipshead_id=:ipshead_id);");
    q.bindValue(":ipshead_id", _ipsheadid);
    q.exec();
  }
  
  XDialog::reject();
}
