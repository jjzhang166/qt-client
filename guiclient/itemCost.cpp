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

#include "itemCost.h"

#include <qvariant.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include <currcluster.h>

/*
 *  Constructs a itemCost as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemCost::itemCost(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemCost::~itemCost()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemCost::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>
static bool _foundCostElems;

void itemCost::init()
{
  _captive = false;
  _itemcostid = -1;

  _postCost->setEnabled(_privileges->check("PostActualCosts"));
  _foundCostElems = true; // assume it's true until sPopulateCostelem
}

enum SetResponse itemCost::set(ParameterList &pParams)
{
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _lowerLevel->hide();
      _lowerLevelLit->hide();
      setCaption("Create Item Cost");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _costelem->setEnabled(false);
      setCaption("Enter Actual Cost");
    }
    else if (param.toString() == "view")
    {
      _actualCost->setEnabled(false);
      _costelem->setEnabled(false);
      setCaption("View Actual Cost");
    }
  }
      
  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
    _actualCost->setFocus();
  }
  else
    _item->setFocus();

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    _itemcostid = param.toInt();
    _item->setReadOnly(TRUE);

    q.prepare( "SELECT item_id, formatBoolYN(itemcost_lowlevel) AS lowlevel,"
               "       itemcost_actcost, itemcost_curr_id, itemcost_updated "
               "FROM item, itemcost "
               "WHERE ( (itemcost_item_id=item_id)"
               " AND    (itemcost_id=:itemcost_id) );" );
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("item_id").toInt());
      _lowerLevel->setText(q.value("lowlevel").toString());
      _actualCost->set(q.value("itemcost_actcost").toDouble(),
		       q.value("itemcost_curr_id").toInt(),
		       QDate::currentDate(),
		       false);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  sPopulateCostelem();

  return _foundCostElems ? NoError : UndefinedError;
}

void itemCost::sSave()
{
  if (_mode != cNew && _mode != cEdit)
    done(_itemcostid);

  if (_costelem->id() < 0)
  {
    QMessageBox::warning(this, tr("Costing Element Required"),
		 tr("You must select a Costing Element to save this Item Cost."));
    _costelem->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('itemcost_itemcost_id_seq') AS itemcost_id;");
    if (q.first())
    {
      _itemcostid = q.value("itemcost_id").toInt();

      q.prepare( "INSERT INTO itemcost "
		 "( itemcost_id, itemcost_item_id,"
		 "  itemcost_costelem_id, itemcost_lowlevel,"
		 "  itemcost_stdcost, itemcost_posted,"
		 "  itemcost_actcost, itemcost_updated, "
		 "  itemcost_curr_id ) "
		 "VALUES "
		 "( :itemcost_id, :itemcost_item_id,"
		 "  :itemcost_costelem_id, FALSE,"
		 "  0, startOfTime(),"
		 "  :itemcost_actcost, CURRENT_DATE, "
		 "  :itemcost_curr_id );" );
      q.bindValue(":itemcost_item_id", _item->id());
      q.bindValue(":itemcost_costelem_id", _costelem->id());
    }
  }
  else if (_mode == cEdit)
  {
      q.prepare( "UPDATE itemcost SET"
		 " itemcost_actcost=:itemcost_actcost,"
		 " itemcost_curr_id=:itemcost_curr_id "
		 "WHERE (itemcost_id=:itemcost_id);");
  }
  q.bindValue(":itemcost_id", _itemcostid);
  q.bindValue(":itemcost_actcost", _actualCost->localValue());
  q.bindValue(":itemcost_curr_id", _actualCost->id());

  if (q.exec() && _postCost->isChecked())
  {
    q.prepare("SELECT postCost(:itemcost_id) AS result;");
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
  }

  if (q.lastError().type() != QSqlError::NoError)	// if EITHER q.exec() failed
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_itemcostid);
}

void itemCost::sPopulateCostelem()
{
  if (_mode == cNew)
  {
    QString sql( "SELECT costelem_id, costelem_type "
		 "FROM costelem "
		 "WHERE ( ( (NOT costelem_sys) " );

    if ( ( (_item->itemType() == "M") ||
	 (_item->itemType() == "F") ||
	 (_item->itemType() == "B") ||
	 (_item->itemType() == "C") ||
	 (_item->itemType() == "T") ) && (_metrics->boolean("Routings") ) )
      sql += " OR (costelem_type IN ('Direct Labor', 'Overhead', 'Machine Overhead'))";
    else if ( (_item->itemType() == "O") ||
	      (_item->itemType() == "P") )
      sql += " OR (costelem_type IN ('Material'))";

    sql += " ) AND (costelem_id NOT IN ( SELECT itemcost_costelem_id"
	   "                           FROM itemcost"
	   "                           WHERE ( (NOT itemcost_lowlevel)"
	   "                            AND (itemcost_item_id=:item_id) ) ) ) ) "
	   "ORDER BY costelem_type;";

    q.prepare(sql);
    q.bindValue(":item_id", _item->id());
    q.exec();
    _costelem->populate(q);
    if (q.size() <= 0)
    {
      QMessageBox::warning(this, tr("No Costing Elements Remaining"),
		   tr("Item %1 already has all available costing elements "
		      "assigned.\nNo new Item Costs can be created for it until "
		      "more costing elements are defined.")
		      .arg(_item->itemNumber()));
      _foundCostElems = false;
    }
  }
  else // _mode == cEdit || cView)
  {
    QString sql( "SELECT costelem_id, costelem_type "
		 "FROM costelem, itemcost "
		 "WHERE ((costelem_id=itemcost_costelem_id)"
		 "  AND  (itemcost_id=:itemcost_id));");
    q.prepare(sql);
    q.bindValue(":itemcost_id", _itemcostid);
    q.exec();
    _costelem->populate(q);
  }
}
