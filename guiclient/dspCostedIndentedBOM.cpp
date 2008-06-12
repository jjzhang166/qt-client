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

#include "dspCostedIndentedBOM.h"

#include <QVariant>
#include <QWorkspace>
#include <QStatusBar>
#include <QMenu>
#include <QStack>
#include <QMessageBox>
#include <openreports.h>
#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"

/*
 *  Constructs a dspCostedIndentedBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCostedIndentedBOM::dspCostedIndentedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  _revision->setType("BOM");
  _revision->setMode(RevisionLineEdit::View);

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),         80,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),       _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Unit Cost"),     _costColumn,  Qt::AlignRight  );
  _bomitem->addColumn(tr("Ext'd Cost"),    _priceColumn, Qt::AlignRight  );
  _bomitem->setIndentation(10);

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCostedIndentedBOM::~dspCostedIndentedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCostedIndentedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCostedIndentedBOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspCostedIndentedBOM::sPrint()
{
  ParameterList params;

  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if(_useActualCosts->isChecked())
    params.append("useActualCosts");

  orReport report("CostedIndentedBOM", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCostedIndentedBOM::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("View Item Costing..."), this, SLOT(sViewItemCosting()), 0);
}

void dspCostedIndentedBOM::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg  = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedIndentedBOM::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedIndentedBOM::sFillList()
{
  double totalCosts = 0;

  _bomitem->clear();

  if (_item->isValid())
  {
      QString sql( "SELECT 2 AS seqord, "
		           "       bomdata_bomwork_id, bomdata_item_id, bomdata_bomwork_parent_id,"
                   "       bomdata_bomwork_seqnumber, bomdata_item_number, bomdata_uom_name,"
                   "       bomdata_itemdescription,"
                   "       bomdata_qtyper,"
                   "       bomdata_scrap,"
                   "       bomdata_effective,"
                   "       bomdata_expires," );

     if (_useStandardCosts->isChecked())
       sql += " formatCost(bomdata_stdunitcost) AS f_unitcost,"
              " formatCost(bomdata_stdextendedcost) AS f_extendedcost,"
              " bomdata_stdextendedcost AS extendedcost, ";
     else if (_useActualCosts->isChecked())
       sql += " formatCost(bomdata_actunitcost) AS f_unitcost,"
              " formatCost(bomdata_actextendedcost) AS f_extendedcost,"
              " bomdata_actextendedcost AS extendedcost, ";

	 sql += "bomdata_bomwork_level "
		    "FROM indentedbom(:item_id,:revision_id,0,0) ";
      q.prepare(sql);
      q.bindValue(":item_id", _item->id());
      q.bindValue(":revision_id", _revision->id());
      q.exec();

      QStack<XTreeWidgetItem*> parent;
      XTreeWidgetItem *last = 0;
      int level = 1;
      while(q.next())
      {
        // If the level this item is on is lower than the last level we just did then we need
        // to pop the stack a number of times till we are equal.
        while(q.value("bomdata_bomwork_level").toInt() < level)
        {
          level--;
          last = parent.pop();
        }

        // If the level this item is on is higher than the last level we need to push the last
        // item onto the stack a number of times till we are equal. (Should only ever be 1.)
        while(q.value("bomdata_bomwork_level").toInt() > level)
        {
          level++;
          parent.push(last);
          last = 0;
        }

        // If there is an item in the stack use that as the parameter to the new xlistviewitem
        // otherwise we'll just use the xlistview _layout
       if(!parent.isEmpty() && parent.top())
	   {
	     last = new XTreeWidgetItem(parent.top(), last, q.value("bomwork_id").toInt(),
						q.value("bomdata_item_id").toInt(),
						q.value("bomdata_bomwork_seqnumber"),
						q.value("bomdata_item_number"),
						q.value("bomdata_itemdescription"),
						q.value("bomdata_uom_name"),
						q.value("bomdata_qtyper"),
						q.value("bomdata_scrap"),
						q.value("bomdata_effective"),
						q.value("bomdata_expires"),
						q.value("f_unitcost"),
						q.value("f_extendedcost") );
	   }
       else
	   {
         if (q.value("bomdata_bomwork_parent_id").toInt() == -1)
         {
           if (q.value("bomdata_bomwork_id").toInt() == -1)
           {
             last = new XTreeWidgetItem( _bomitem,last, -1, -1,
                                      "", q.value("bomdata_item_number"),
                                      "", "", "", "", "" );
             last->setText(7, "");
             last->setText(8, q.value("f_unitcost").toString());
             last->setText(9, q.value("f_extendedcost").toString());
           }
           else
           {
             last = new XTreeWidgetItem( _bomitem, last, q.value("bomdata_bomwork_id").toInt(), q.value("bomdata_item_id").toInt(),
                                      q.value("bomdata_bomwork_seqnumber"), q.value("bomdata_item_number"),
                                      q.value("bomdata_itemdescription"), q.value("bomdata_uom_name"),
                                      q.value("bomdata_qtyper"), q.value("bomdata_scrap"),
                                      q.value("bomdata_effective"), q.value("bomdata_expires"),
                                      q.value("f_unitcost"), q.value("f_extendedcost") );
           }
		 }
		 else
	     {
            last = new XTreeWidgetItem(_bomitem, last, q.value("bomwork_id").toInt(),
						q.value("bomdata_item_id").toInt(),
						q.value("bomdata_bomwork_seqnumber"),
						q.value("bomdata_item_number"),
						q.value("bomdata_itemdescription"),
						q.value("bomdata_uom_name"),
						q.value("bomdata_qtyper"),
						q.value("bomdata_scrap"),
						q.value("bomdata_effective"),
						q.value("bomdata_expires"),
						q.value("f_unitcost"),
						q.value("f_extendedcost") );
         }
	     totalCosts += q.value("extendedcost").toDouble();
	  }
	}
    last = new XTreeWidgetItem(_bomitem, -1, -1);
    last->setText(1, tr("Total Cost"));
    last->setText(9, formatCost(totalCosts));

    q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
                 "       formatCost(stdcost(:item_id)) AS standard;" );
      q.bindValue(":item_id", _item->id());
      q.exec();
      if (q.first())
      {
        last = new XTreeWidgetItem(_bomitem, -1, -1);
        last->setText(1, tr("Actual Cost"));
        last->setText(9, q.value("actual").toString());

        last = new XTreeWidgetItem( _bomitem, -1, -1);
        last->setText(1, tr("Standard Cost"));
        last->setText(9, q.value("standard").toString());
      }
    _bomitem->expandAll();
  }
}
