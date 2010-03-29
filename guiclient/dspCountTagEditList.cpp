/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountTagEditList.h"

#include <math.h>

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "countSlip.h"
#include "countTag.h"
#include "dspInventoryHistoryByItem.h"
#include "dspCountSlipEditList.h"
#include "storedProcErrorLookup.h"

dspCountTagEditList::dspCountTagEditList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _highlightGroupInt = new QButtonGroup(this);
  _highlightGroupInt->addButton(_noHighlight);
  _highlightGroupInt->addButton(_highlightValue);
  _highlightGroupInt->addButton(_highlightPercent);

  _codeGroup = new QButtonGroup(this);
  _codeGroup->addButton(_plancode);
  _codeGroup->addButton(_classcode);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_showSlips, SIGNAL(clicked()), this, SLOT(sToggleList()));
  connect(_enterSlip, SIGNAL(clicked()), this, SLOT(sEnterCountSlip()));
  connect(_cnttag, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_codeGroup, SIGNAL(buttonClicked(int)), this, SLOT(sParameterTypeChanged()));

  _parameter->setType(ParameterGroup::ClassCode);
  _variancePercent->setValidator(omfgThis->percentVal());

  _cnttag->setRootIsDecorated(TRUE);
  _cnttag->addColumn(tr("Pri."), (_whsColumn + 10), Qt::AlignCenter,true, "invcnt_priority");
  _cnttag->addColumn(tr("Tag/Slip #"),_orderColumn, Qt::AlignRight, true, "tagnumber");
  _cnttag->addColumn(tr("Tag Date"),   _dateColumn, Qt::AlignCenter,true, "tagdate");
  _cnttag->addColumn(tr("Item Number"),         -1, Qt::AlignLeft,  true, "item_number");
  _cnttag->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _cnttag->addColumn(tr("Location"),     _ynColumn, Qt::AlignCenter,true, "loc_specific");
  _cnttag->addColumn(tr("QOH"),         _qtyColumn, Qt::AlignRight, true, "qoh");
  _cnttag->addColumn(tr("Count Qty."),  _qtyColumn, Qt::AlignRight, true, "qohafter");
  _cnttag->addColumn(tr("Variance"),    _qtyColumn, Qt::AlignRight, true, "variance");
  _cnttag->addColumn(tr("%"),         _prcntColumn, Qt::AlignRight, true, "varianceprcnt");
  _cnttag->addColumn(tr("Amount"),     _costColumn, Qt::AlignRight, true, "variancecost");
  _cnttag->setIndentation(10);
  
  if (_privileges->check("EnterCountTags"))
  {
    connect(_cnttag, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_cnttag, SIGNAL(valid(bool)), this, SLOT(sHandleButtons(bool)));
  }

  if (_privileges->check("DeleteCountTags"))
    connect(_cnttag, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  if (_privileges->check("PostCountTags"))
    connect(_cnttag, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  if (_privileges->check("EnterCountSlips"))
    connect(_cnttag, SIGNAL(valid(bool)), _enterSlip, SLOT(setEnabled(bool)));

  _searchFor->setFocus();
}

dspCountTagEditList::~dspCountTagEditList()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCountTagEditList::languageChange()
{
  retranslateUi(this);
}

void dspCountTagEditList::sToggleList()
{
  if (_showSlips->isChecked())
    _cnttag->setSelectionMode(QAbstractItemView::SingleSelection);
  else
    _cnttag->setSelectionMode(QAbstractItemView::ExtendedSelection);
  sFillList();
}

void dspCountTagEditList::setParams(ParameterList &params)
{
  if (_parameter->type() == ParameterGroup::ClassCode)
    params.append("ParameterType", "ClassCode");
  else
    params.append("ParameterType", "PlannerCode");
  _warehouse->appendValue(params);
  _parameter->appendValue(params);
}

void dspCountTagEditList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  params.append("maxTags", 10000);

  orReport report("CountTagEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountTagEditList::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->insertItem("Enter Count Slip...", this, SLOT(sEnterCountSlip()), 0);
    if (!_privileges->check("EnterCountSlips"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem("Count Slip Edit List...", this, SLOT(sCountSlipEditList()), 0);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem("View Pending Inventory History...", this, SLOT(sViewInventoryHistory()), 0);
    if (!_privileges->check("ViewInventoryHistory"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem("Edit Count Tag...", this, SLOT(sEdit()), 0);
    if (!_privileges->check("EnterCountTags"))
      pMenu->setItemEnabled(menuItem, FALSE);

    if (pSelected->text(5) != "")
    {
      menuItem = pMenu->insertItem("Post Count Tag...", this, SLOT(sPost()), 0);
      if (!_privileges->check("PostCountTags"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }

    menuItem = pMenu->insertItem("Delete Count Tag", this, SLOT(sDelete()), 0);
    if (!_privileges->check("DeleteCountTags"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    if (pSelected->text(3) == tr("Unposted"))
    {
      menuItem = pMenu->insertItem("Edit Count Slip...", this, SLOT(sEdit()), 0);
      if (!_privileges->check("EnterCountSlips"))
        pMenu->setItemEnabled(menuItem, FALSE);
    }
  }
}

void dspCountTagEditList::sEnterCountSlip()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("cnttag_id", _cnttag->id());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();

    _searchFor->setFocus();
    _searchFor->setSelection(0, _searchFor->text().length());
  }
}

void dspCountTagEditList::sCountSlipEditList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttag->id());
  params.append("run");

  dspCountSlipEditList *newdlg = new dspCountSlipEditList();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCountTagEditList::sViewInventoryHistory()
{
  q.prepare( "SELECT invcnt_itemsite_id, invcnt_tagdate "
                 "FROM invcnt "
                 "WHERE (invcnt_id=:invcnt_id);" );
  q.bindValue(":invcnt_id", _cnttag->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("itemsite_id", q.value("invcnt_itemsite_id").toInt());
    params.append("startDate", q.value("invcnt_tagdate").toDate());
    params.append("endDate", omfgThis->dbDate());
    params.append("run");

    dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCountTagEditList::sEdit()
{
  bool update  = FALSE;
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sEditTag();
  else if (_showSlips->isChecked())
    sEditSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());

      countTag newdlg(this, "", TRUE);
      newdlg.set(params);

      if (newdlg.exec() != XDialog::Rejected)
	update = TRUE;
    }
    if (update)
      sFillList();
  }
}

void dspCountTagEditList::sEditTag()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sEditSlip()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cntslip_id", _cnttag->altId());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sDelete()
{
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sDeleteTag();
  else if (_showSlips->isChecked())
    sDeleteSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      int itemsiteid = -1;

      q.prepare( "SELECT cntslip_id FROM cntslip "
		 "WHERE (cntslip_cnttag_id=:cnttag_id);" );
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.first())
      {
	QMessageBox::critical( this, tr("Cannot Delete Count tag"),
			      tr("<p>There are Count Slips entered for this "
				 "Count Tag. You must delete Count Slips for "
				 "the Count Tag before you may delete this Tag.") );
	continue;
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
      q.prepare("SELECT itemsite_id"
	        "  FROM invcnt JOIN itemsite ON (invcnt_itemsite_id=itemsite_id AND itemsite_freeze) "
		" WHERE (invcnt_id=:cnttag_id); ");
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      if(q.exec() && q.first())
	itemsiteid = q.value("itemsite_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      q.prepare( "DELETE FROM cntslip "
	     "WHERE (cntslip_cnttag_id=:cnttag_id); "
	     "DELETE FROM invcnt "
	     "WHERE (invcnt_id=:cnttag_id); " );
      q.bindValue(":cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());
      q.exec();
      if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }

      if (itemsiteid != -1)
      {
	if (QMessageBox::question(this, tr("Unfreeze Itemsite?"),
				  tr("<p>The Itemsite for the Count Tag you "
				     "deleted is frozen. Would you like to "
				     "unfreeze the Itemsite at this time?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No | QMessageBox::Escape ) == QMessageBox::Yes )
	{
	  q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
	  q.bindValue(":itemsite_id", itemsiteid);
	  q.exec();
	  if (q.first())
	  {
	    int result = q.value("result").toInt();
	    if (result < 0)
	    {
	      systemError(this, storedProcErrorLookup("thawItemsite", result),
			  __FILE__, __LINE__);
	      continue;
	    }
	    else if (q.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	      return;
	    }
	  }
	}
      }
    }
    sFillList();
  }
}

void dspCountTagEditList::sDeleteTag()
{
  int itemsiteid = -1;
  q.prepare( "SELECT cntslip_id FROM cntslip "
                 "WHERE (cntslip_cnttag_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttag->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Count tag"),
                           tr("<p>There are Count Slips entered for this count "
			      "tag. You must delete Count Slips for the Count "
			      "Tag before you may delete this Tag.") );
    return;
  }
  q.prepare( "SELECT itemsite_id"
             "  FROM invcnt JOIN itemsite ON (invcnt_itemsite_id=itemsite_id AND itemsite_freeze) "
             " WHERE (invcnt_id=:cnttag_id); ");
  q.bindValue(":cnttag_id", _cnttag->id());
  if (q.exec() && q.first())
    itemsiteid = q.value("itemsite_id").toInt();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  q.prepare( "DELETE FROM invcnt "
	     "WHERE (invcnt_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttag->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(itemsiteid != -1)
  {
    if (QMessageBox::question( this, tr("Unfreeze Itemsite?"),
			      tr("<p>The Itemsite for the Count Tag you deleted"
				 " is frozen. Would you like to unfreeze the "
				 "Itemsite at this time?"),
			      QMessageBox::Yes | QMessageBox::Default,
			      QMessageBox::No | QMessageBox::Escape ) == QMessageBox::Yes )
    {
      q.prepare("SELECT thawItemsite(:itemsite_id) AS result;");
      q.bindValue(":itemsite_id", itemsiteid);
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("thawItemsite", result),
		      __FILE__, __LINE__);
	  return;
	}
	else if (q.lastError().type() != QSqlError::NoError)
	{
	  systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
  }

  sFillList();
}

void dspCountTagEditList::sDeleteSlip()
{
  q.prepare( "DELETE FROM cntslip "
                 "WHERE (cntslip_id=:cntslip_id);" );
  q.bindValue(":cntslip_id", _cnttag->altId());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void dspCountTagEditList::sPost()
{
  bool update  = FALSE;
  if ( (_showSlips->isChecked()) &&
      (((XTreeWidgetItem *)_cnttag->currentItem())->altId() == -1) )
    sPostTag();
  else if  (_showSlips->isChecked())
    sPostSlip();
  else
  {
    QList<XTreeWidgetItem*> selected = _cnttag->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (((XTreeWidgetItem*)(selected[i]))->altId() == -1)
      {
	ParameterList params;
	params.append("mode", "post");
	params.append("cnttag_id", ((XTreeWidgetItem*)(selected[i]))->id());

	countTag newdlg(this, "", TRUE);
	newdlg.set(params);

	if (newdlg.exec() != XDialog::Rejected)
	  update = TRUE;
      }
    }
    if (update)
      sFillList();
  }
}

void dspCountTagEditList::sPostTag()
{
  ParameterList params;
  params.append("mode", "post");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sPostSlip()
{
  ParameterList params;
  params.append("mode", "post");
  params.append("cntslip_id", _cnttag->altId());
  
  countSlip newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspCountTagEditList::sSearch(const QString &pTarget)
{
  QList<XTreeWidgetItem*> search = _cnttag->findItems(pTarget,
						      Qt::MatchStartsWith, 3);

  if (search.size() > 0)
  {
    _cnttag->setCurrentItem(search[0], TRUE);
    _cnttag->scrollToItem(search[0]);
  }
}

void dspCountTagEditList::sFillList()
{
  QString sql( "SELECT *, "
               "       CASE WHEN (xtindentrole = 1) THEN NULL "
               "<? if exists(\"varianceValue\") ?>"
               "            WHEN (ABS(variancecost) >  <? value(\"varianceValue\") ?>) THEN 'error'"
               "<? elseif exists(\"variancePercent\") ?>"
               "            WHEN (ABS(varianceprcnt) >  <? value(\"variancePercent\") ?>) THEN 'error'"
               "<? else ?>"
               "            ELSE NULL"
               "<? endif ?> END AS qtforegroundrole,"
               "       CASE WHEN (xtindentrole = 1) THEN NULL "
               "            WHEN (qohafter IS NOT NULL) THEN 'emphasis'"
               "       END AS qohafter_qtforegroundrole,"
               "       CASE WHEN (xtindentrole = 0) THEN NULL ELSE '' END AS invcnt_priority_qtdisplayrole,"
               "       'qty' AS qoh_xtnumericrole,"
               "       'qty' AS qohafter_xtnumericrole,"
               "       'qty' AS variance_xtnumericrole,"
               "       'percent' AS varianceprcnt_xtnumericrole,"
               "       'curr' AS variancecost_xtnumericrole "
               " FROM ("
               "SELECT invcnt_id, -1 AS cntslip_id, invcnt_priority,"
               "       COALESCE(invcnt_tagnumber, 'Misc.') AS tagnumber,"
               "       invcnt_tagdate AS tagdate,"
               "       item_number, warehous_code,"
	       "       CASE WHEN (location_id IS NOT NULL) THEN"
	       "                 location_name"
	       "             ELSE <? value(\"all\") ?>  END AS loc_specific, "
               "       CASE WHEN (invcnt_location_id IS NOT NULL)"
               "                 THEN (SELECT SUM(itemloc_qty)"
               "                         FROM itemloc"
               "                        WHERE ((itemloc_itemsite_id=itemsite_id)"
               "                          AND  (itemloc_location_id=invcnt_location_id)) )"
               "            ELSE itemsite_qtyonhand"
               "       END AS qoh,"
               "       COALESCE(invcnt_qoh_after, (SELECT SUM(cntslip_qty)"
               "                                   FROM cntslip"
               "                                   WHERE (cntslip_cnttag_id=invcnt_id) )"
               "               ) AS qohafter,"
               "       (invcnt_qoh_after - itemsite_qtyonhand) AS variance,"
               "       CASE WHEN (invcnt_qoh_after IS NULL) THEN NULL"
               "            WHEN ((itemsite_qtyonhand = 0) AND (invcnt_qoh_after > 0)) THEN 1"
               "            WHEN ((itemsite_qtyonhand = 0) AND (invcnt_qoh_after < 0)) THEN -1"
               "            WHEN ((itemsite_qtyonhand = 0) AND (invcnt_qoh_after = 0)) THEN 0"
               "            ELSE ((1 - (invcnt_qoh_after / itemsite_qtyonhand)) * -1)"
               "       END  * 100 AS varianceprcnt,"
               "       (stdcost(item_id) * (invcnt_qoh_after - itemsite_qtyonhand)) AS variancecost,"
               "       item_number AS orderby,"
               "       0 AS xtindentrole "
               "FROM invcnt LEFT OUTER JOIN location ON (invcnt_location_id=location_id),"
	       "     item, warehous, itemsite "
               "WHERE ( (invcnt_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (NOT invcnt_posted)"
	       " <? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       " <? endif ?>"
	       " <? if exists(\"classcode_id\") ?>"
	       " AND (item_classcode_id=<? value(\"classcode_id\") ?>)"
	       " <? elseif exists(\"classcode_pattern\") ?>"
	       " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ <? value(\"classcode_pattern\") ?>)))"
	       " <? elseif exists(\"plancode_id\") ?>"
	       " AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	       " <? elseif exists(\"plancode_pattern\") ?>"
	       " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	       " <? endif ?>"
	       " <? if exists(\"showSlips\") ?>"
	       " ) "
	       "UNION "
	       "SELECT invcnt_id, cntslip_id, invcnt_priority,"
               "       cntslip_number AS tagnumber,"
	       "       cntslip_entered AS tagdate,"
	       "       CASE WHEN (cntslip_posted) THEN <? value(\"posted\") ?>"
	       "            ELSE <? value(\"unposted\") ?>"
	       "       END AS item_number,"
	       "       '' AS warehous_code, "
	       "       '' AS loc_specific, "
	       "       NULL AS qoh,"
	       "       cntslip_qty AS qohafter,"
	       "       NULL AS variance, NULL AS varianceprcnt, 0 AS variancecost,"
	       "       item_number AS orderby,"
               "       1 AS xtindentrole "
	       "FROM cntslip, invcnt, itemsite, item "
	       "WHERE ( (cntslip_cnttag_id=invcnt_id)"
	       " AND (invcnt_itemsite_id=itemsite_id)"
	       " AND (itemsite_item_id=item_id)"
	       " AND (NOT invcnt_posted)"
	       " <? if exists(\"warehous_id\") ?>"
	       " AND (itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
	       " <? endif ?>"
	       " <? if exists(\"classcode_id\") ?>"
	       " AND (item_classcode_id=<? value(\"classcode_id\") ?>)"
	       " <? elseif exists(\"classcode_pattern\") ?>"
	       " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ <? value(\"classcode_pattern\") ?>)))"
	       " <? elseif exists(\"plancode_id\") ?>"
	       " AND (itemsite_plancode_id=<? value(\"plancode_id\") ?>)"
	       " <? elseif exists(\"plancode_pattern\") ?>"
	       " AND (itemsite_plancode_id IN (SELECT plancode_id FROM plancode WHERE (plancode_code ~ <? value(\"plancode_pattern\") ?>)))"
	       " <? endif ?>"
	       " <? endif ?>"
	       " ) "
               " ) AS dummy "
	       "ORDER BY invcnt_priority DESC, orderby, invcnt_id, cntslip_id;" );

  ParameterList params;
  setParams(params);
  params.append("all",	    tr("All"));
  params.append("posted",   tr("Posted"));
  params.append("unposted", tr("Unposted"));
  if (_showSlips->isChecked())
    params.append("showSlips");
  if (_highlightValue->isChecked())
    params.append("varianceValue",   _varianceValue->localValue());
  else if (_highlightPercent->isChecked())
    params.append("variancePercent", _variancePercent->toDouble());

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _cnttag->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _cnttag->expandAll();

  if (_searchFor->text().trimmed().length())
    sSearch(_searchFor->text());
}

void dspCountTagEditList::sHandleAutoUpdate(bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

void dspCountTagEditList::sHandleButtons(bool valid)
{
  if (valid)
  {
    // Handle Edit Button
    if (_cnttag->currentItem()->altId() == -1) // count tag
      _edit->setEnabled(true);
    else if (_cnttag->currentItem()->rawValue("item_number") == tr("Unposted")) // unposted count slip
      _edit->setEnabled(true);
    else
      _edit->setEnabled(false);
  }
  else
    _edit->setEnabled(false);
}

void dspCountTagEditList::sParameterTypeChanged()
{
  if(_plancode->isChecked())
    _parameter->setType(ParameterGroup::PlannerCode);
  else
    _parameter->setType(ParameterGroup::ClassCode);

}
