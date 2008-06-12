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

#include "dspJobCosting.h"

#include <QVariant>

#include <openreports.h>

dspJobCosting::dspJobCosting(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _wo->setQuery( "SELECT wo_id, formatWONumber(wo_id) AS wonumber,"
                  "       warehous_code, item_id, item_number, uom_name,"
                  "       item_descrip1, item_descrip2,"
                  "       wo_qtyord, wo_qtyrcv, wo_status,"
                  "       formatDate(wo_duedate) AS duedate,"
                  "       formatDate(wo_startdate) AS startdate,"
                  "       formatQtyPer(wo_qtyord) AS ordered,"
                  "       formatQtyPer(wo_qtyrcv) AS received, "
                  "       formatQtyPer(noNeg(wo_qtyord - wo_qtyrcv)) AS balance "
                  "FROM wo, itemsite, item, warehous, uom "
                  "WHERE ((wo_itemsite_id=itemsite_id)"
                  " AND (itemsite_item_id=item_id)"
                  " AND (item_inv_uom_id=uom_id)"
                  " AND (itemsite_warehous_id=warehous_id)"
                  " AND (item_type = 'J')) "
				  "ORDER BY formatWONumber(wo_id) DESC;");
  
  QString _codecol;
  if (!_metrics->boolean("Routings"))
  {
    _codecol = tr("Item Number");
	_showsu->hide();
	_showsu->setChecked(FALSE);
	_showrt->hide();
	_showrt->setChecked(FALSE);
	_showmatl->hide();
	_showmatl->setChecked(TRUE);
  }
  else
    _codecol = tr("Work Center/Item");

  _cost->addColumn(tr("Type"),        _itemColumn,   Qt::AlignLeft   );
  _cost->addColumn(_codecol,          _itemColumn,  Qt::AlignLeft   );
  _cost->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _cost->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight  );
  _cost->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter );
  _cost->addColumn(tr("Cost"),        _moneyColumn, Qt::AlignRight  );

  _wo->setFocus();

}

dspJobCosting::~dspJobCosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspJobCosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspJobCosting::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspJobCosting::sPrint()
{
  ParameterList params;
  params.append("wo_id", _wo->id());
  params.append("setup", tr("Setup"));
  params.append("runtime", tr("Run Time"));
  params.append("material", tr("Material"));
  params.append("timeuom", tr("Hours"));

  if (_showsu->isChecked())
    params.append("showsu");

  if (_showrt->isChecked())
    params.append("showrt");

  if (_showmatl->isChecked())
    params.append("showmatl");

  orReport report("JobCosting", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspJobCosting::sFillList()
{
  if (_wo->isValid())
    sFillList(-1, FALSE);
}

void dspJobCosting::sFillList(int, bool)
{
  _cost->clear();

  if (_wo->isValid() && 
	 (_showsu->isChecked() || 
	  _showrt->isChecked() || 
	  _showmatl->isChecked()))
  {
    QString sql("SELECT  * FROM (");
	
	if (_showsu->isChecked())
	{
	  sql += "SELECT wooper_id AS id,1 AS sort,:setup AS type, wrkcnt_code AS code, wooper_descrip1 AS descrip, "
		     "  formatqty(SUM(COALESCE(wooperpost_sutime,0))/60) AS f_qty,:uom AS uom, "
			 "  formatCost(SUM(COALESCE(wooperpost_sucost,0))) as f_cost, wooper_seqnumber, "
			 "  SUM(COALESCE(wooperpost_sucost,0)) AS cost "
		     "FROM wooper "
		     "  LEFT OUTER JOIN wooperpost ON (wooper_id=wooperpost_wooper_id), "
			 "  wrkcnt "
			 "WHERE ((wooper_wo_id=:wo_id) "
			 " AND (wooper_wrkcnt_id=wrkcnt_id)) "
			 "GROUP BY wooper_id, wrkcnt_code, wooper_descrip1, wooper_seqnumber ";
	}

    if (_showsu->isChecked() && _showrt->isChecked())
	  sql += "UNION ";

    if (_showrt->isChecked())
	{
	sql +=	"SELECT wooper_id AS id,2 AS sort,:runtime AS type, wrkcnt_code AS code, wooper_descrip1 AS descrip, "
		    "  formatqty(SUM(COALESCE(wooperpost_rntime,0))/60) AS f_qty,:uom AS uom, "
			"  formatCost(SUM(COALESCE(wooperpost_rncost,0))) AS f_cost, wooper_seqnumber, "
			"  SUM(COALESCE(wooperpost_rncost,0)) AS cost "
		    "FROM wooper "
		    "  LEFT OUTER JOIN wooperpost ON (wooper_id=wooperpost_wooper_id), "
			"  wrkcnt "
			"WHERE ((wooper_wo_id=:wo_id) "
			"  AND (wooper_wrkcnt_id=wrkcnt_id)) "
			"GROUP BY wooper_id, wrkcnt_code, wooper_descrip1, wooper_seqnumber ";
	}

    if ((_showsu->isChecked() || _showrt->isChecked()) && _showmatl->isChecked())
	  sql += "UNION ";

    if (_showmatl->isChecked())
	{
	  sql +=  "SELECT womatl_id AS id,3 AS sort,:material AS type, item_number AS code, item_descrip1 AS descrip, "
		      "  formatqty(SUM(COALESCE(invhist_invqty,0))) AS f_qty,uom_name AS uom, "
			  "  formatCost(SUM(COALESCE(invhist_invqty * invhist_unitcost,0))) AS f_cost, "
			  "  NULL as wooper_seqnumber, SUM(COALESCE(invhist_invqty * invhist_unitcost,0)) AS cost "
		      "FROM womatl "
		      "  LEFT OUTER JOIN womatlpost ON (womatl_id=womatlpost_womatl_id) "
			  "  LEFT OUTER JOIN invhist ON (womatlpost_invhist_id=invhist_id), "
			  "  itemsite, item, uom "
			  "WHERE ((womatl_wo_id=:wo_id) "
			  " AND (womatl_itemsite_id=itemsite_id) "
			  " AND (itemsite_item_id=item_id) "
			  " AND (item_inv_uom_id=uom_id)) "
			  "GROUP BY womatl_id, item_number, item_descrip1, uom_name ";
	}

	sql += ") as data ORDER BY ";
	
	if (_showsu->isChecked() || _showrt->isChecked())
	  sql += " wooper_seqnumber,";

	sql += "sort,code;";

    q.prepare(sql);
	q.bindValue(":wo_id", _wo->id());
	q.bindValue(":setup", tr("Setup"));
	q.bindValue(":runtime", tr("Run Time"));
	q.bindValue(":material", tr("Material"));
    q.bindValue(":uom", tr("Hours"));
    q.exec();
   // _cost->populate(q,TRUE);
	
	double totalCosts = 0;
    XTreeWidgetItem *last = 0;
    while (q.next())
    {
      last = new XTreeWidgetItem( _cost, last, q.value("id").toInt(),
				 q.value("type"),
				 q.value("code"),
				 q.value("descrip"),
				 q.value("f_qty"),
				 q.value("uom"),
				 q.value("f_cost"));

	  totalCosts = totalCosts + q.value("cost").toDouble();
    } 

    last = new XTreeWidgetItem(_cost, -1, -1);
    last->setText(0, tr("Total Cost"));
    last->setText(5, formatCost(totalCosts));
  }
}
