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

#include "dspItemCostDetail.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include "bomItem.h"

/*
 *  Constructs a dspItemCostDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostDetail::dspItemCostDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _costTypeGroupInt = new QButtonGroup(this);
    _costTypeGroupInt->addButton(_standardCosts);
    _costTypeGroupInt->addButton(_actualCosts);

    // signals and slots connections
    connect(_costTypeGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_costType, SIGNAL(newID(int)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostDetail::~dspItemCostDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostDetail::languageChange()
{
    retranslateUi(this);
}

void dspItemCostDetail::init()
{
  statusBar()->hide();

  _bom->addColumn(tr("#"),               _seqColumn,   Qt::AlignCenter );
  _bom->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft   );
  _bom->addColumn(tr("Description"),     -1,           Qt::AlignLeft   );
  _bom->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter );
  _bom->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight  );
  _bom->addColumn(tr("Scrap/Absorb. %"), _itemColumn,  Qt::AlignRight  );
  _bom->addColumn(tr("Unit Cost"),       _costColumn,  Qt::AlignRight  );
  _bom->addColumn(tr("Ext'd Cost"),      _moneyColumn, Qt::AlignRight  );

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

enum SetResponse dspItemCostDetail::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("costtype", &valid);
  if (valid)
  {
    for (int cursor = 0; cursor < _costType->count(); cursor++)
    {
      if (_costType->text(cursor) == param.toString())
      {
        _costType->setCurrentItem(cursor);
        break;
      }
    }
  }

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemcost_item_id, itemcost_costelem_id "
               "FROM itemcost "
               "WHERE (itemcost_id=:itemcost_id)" );
    q.bindValue(":itemcost_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("itemcost_item_id").toInt());
      _item->setReadOnly(TRUE);
      _costType->setId(q.value("itemcost_costelem_id").toInt());
      _costType->setEnabled(FALSE);
    }
//  ToDo
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostDetail::sPopulate()
{
  q.prepare( "SELECT costelem_id, costelem_type "
             "FROM costelem, itemcost "
             "WHERE ( (itemcost_costelem_id=costelem_id)"
             " AND (itemcost_lowlevel)"
             " AND (itemcost_item_id=:item_id) );" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _costType->populate(q);
}

void dspItemCostDetail::sPrint()
{
}

void dspItemCostDetail::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View BOM Item..."),                  this, SLOT(sViewBomitem()),        0 );
  menuThis->insertItem(tr("View Material Costing Detail..."),   this, SLOT(sViewMaterialCosting()), 0 );
}

void dspItemCostDetail::sViewBomitem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bom->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemCostDetail::sViewMaterialCosting()
{
  ParameterList params;
  params.append("item_id", _bom->altId());
  params.append("costtype", "Material");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostDetail::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemCostDetail::sFillList(int pItemid, bool pLocale)
{
  if ( (pLocale) || (pItemid == _item->id()) )
  {
    _bom->clear();

    if (_item->isValid())
    {
      QString sql;

      if ( (_item->itemType() == "M") ||
           (_item->itemType() == "F") ||
           (_item->itemType() == "B") ||
           (_item->itemType() == "T") ||
           (_item->itemType() == "Y") ||
           (_item->itemType() == "R") ||
           (_item->itemType() == "O") ||
           (_item->itemType() == "P") )
      {
        sql = "SELECT bomitem_id, item_id, seqnumber, item_number,"
              "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
              "       formatQtyper(qtyper) AS f_qtyper,"
              "       formatScrap(bomitem_scrap) AS f_scrap,"
              "       formatCost(cost) AS f_cost,"
              "       formatCost(extendedcost) AS f_extendedcost,"
              "       extendedcost "
              "FROM ( SELECT bomitem_id, bomitem_seqnumber AS seqnumber, itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper, bomitem_scrap,"
              "              item_id, item_number, item_descrip1, item_descrip2, uom_name, ";

        if (_standardCosts->isChecked())
          sql += " itemcost_stdcost AS cost,"
                 " (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * itemcost_stdcost) AS extendedcost ";
        else
          sql += " itemcost_actcost AS cost,"
                 " (itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * itemcost_actcost) AS extendedcost ";

  
        sql += "FROM bomitem, item, itemcost, costelem, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (CURRENT_DATE BETWEEN bomitem_effective AND (bomitem_expires-1))"
               " AND (itemcost_item_id=item_id)"
               " AND (itemcost_costelem_id=costelem_id)"
               " AND (bomitem_parent_item_id=:item_id)"
               " AND (costelem_id=:costelem_id) ) ) AS data "
               "ORDER BY seqnumber;";
      }
      else if (_item->itemType() == "C")
      {
        sql = "SELECT bbomitem_id, item_id, seqnumber, item_number,"
              "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
              "       formatQtyPer(bbomitem_qtyper) AS f_qtyper,"
              "       formatScrap(bbomitem_costabsorb) AS f_scrap,"
              "       formatCost(cost) AS f_cost,"
              "       formatCost(extendedcost) AS f_extendedcost,"
              "       extendedcost "
              "FROM ( SELECT bbomitem_id, bbomitem_seqnumber AS seqnumber, bbomitem_qtyper, bbomitem_costabsorb,"
              "              item_id, item_number, item_descrip1, item_descrip2, uom_name, ";

        if (_standardCosts->isChecked())
          sql += " itemcost_stdcost AS cost,"
                 " (itemcost_stdcost / bbomitem_qtyper * bbomitem_costabsorb) AS extendedcost ";
        else
          sql += " itemcost_actcost AS cost,"
                 " (itemcost_actcost / bbomitem_qtyper * bbomitem_costabsorb) AS extendedcost ";


        sql += "FROM bbomitem, item, itemcost, uom "
               "WHERE ( (bbomitem_parent_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (CURRENT_DATE BETWEEN bbomitem_effective AND (bbomitem_expires-1))"
               " AND (itemcost_item_id=bbomitem_parent_item_id)"
               " AND (itemcost_costelem_id=:costelem_id)"
               " AND (bbomitem_item_id=:item_id) )"

               "UNION SELECT source.bbomitem_id, source.bbomitem_seqnumber AS seqnumber,"
               "             source.bbomitem_qtyper, target.bbomitem_costabsorb,"
               "             item_id, item_number, item_descrip1, item_descrip2, uom_name,";


        if (_standardCosts->isChecked())
          sql += " itemcost_stdcost AS cost,"
                 " (itemcost_stdcost * source.bbomitem_qtyper / target.bbomitem_qtyper * target.bbomitem_costabsorb) AS extendedcost ";
        else
          sql += " itemcost_actcost AS cost,"
                 " (itemcost_actcost * source.bbomitem_qtyper / target.bbomitem_qtyper * target.bbomitem_costabsorb) AS extendedcost ";

        sql += "FROM item, itemcost, bbomitem AS target, bbomitem AS source, uom "
               "WHERE ( (source.bbomitem_parent_item_id=target.bbomitem_parent_item_id)"
               " AND (CURRENT_DATE BETWEEN source.bbomitem_effective AND (source.bbomitem_expires-1))"
               " AND (CURRENT_DATE BETWEEN target.bbomitem_effective AND (target.bbomitem_expires-1))"
               " AND (source.bbomitem_item_id=itemcost_item_id)"
               " AND (source.bbomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (item_type='Y')"
               " AND (target.bbomitem_item_id=:item_id)"
               " AND (itemcost_costelem_id=:costelem_id) ) ) AS data "
               "ORDER BY seqnumber;";
      }

      q.prepare(sql);
      q.bindValue(":costelem_id", _costType->id());
      q.bindValue(":item_id", _item->id());
      q.exec();
      double extendedCost = 0.0;

      XTreeWidgetItem *last = 0;
      while (q.next())
      {
	last = new XTreeWidgetItem(_bom, last, q.value("bomitem_id").toInt(),
				   q.value("item_id").toInt(),
				   q.value("seqnumber"),
				   q.value("item_number"),
				   q.value("itemdescrip"),
				   q.value("uom_name"),
				   q.value("f_qtyper"),
				   q.value("f_scrap"),
				   q.value("f_cost"),
				   q.value("f_extendedcost") );

        extendedCost += q.value("extendedcost").toDouble();
      }

      last = new XTreeWidgetItem(_bom, last, -1, -1,
				 "", tr("Totals:"),
				 "", "", "", "", "", formatCost(extendedCost) );
    }
  }
}
