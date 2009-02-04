/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostHistory.h"

#include <QMessageBox>

#include <openreports.h>
#include <parameter.h>

dspItemCostHistory::dspItemCostHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _itemcost->addColumn(tr("Element"),              -1, Qt::AlignLeft,  true, "costelem_type");
  _itemcost->addColumn(tr("Lower"),       _costColumn, Qt::AlignCenter,true, "lowlevel");
  _itemcost->addColumn(tr("Type"),        _costColumn, Qt::AlignLeft,  true, "type");
  _itemcost->addColumn(tr("Time"),    _timeDateColumn, Qt::AlignCenter,true, "costhist_date");
  _itemcost->addColumn(tr("User"),         _qtyColumn, Qt::AlignCenter,true, "username");
  _itemcost->addColumn(tr("Old"),         _costColumn, Qt::AlignRight, true, "costhist_oldcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "oldcurr");
  _itemcost->addColumn(tr("New"),         _costColumn, Qt::AlignRight, true, "costhist_newcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "newcurr");

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn("oldcurr");
      _itemcost->hideColumn("newcurr");
  }
}

dspItemCostHistory::~dspItemCostHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostHistory::languageChange()
{
  retranslateUi(this);
}

void dspItemCostHistory::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostHistory::sFillList()
{
  _itemcost->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT costhist_id, costelem_type,"
              "        formatBoolYN(costhist_lowlevel) AS lowlevel,"
               "       CASE WHEN costhist_type='A' THEN :actual"
               "            WHEN costhist_type='S' THEN :standard"
               "            WHEN costhist_type='D' THEN :delete"
               "            WHEN costhist_type='N' THEN :new"
               "       END AS type,"
               "       costhist_date,"
               "       getUserName(costhist_user_id) AS username,"
               "       costhist_oldcost,"
               "       currConcat(costhist_oldcurr_id) AS oldcurr, "
	       "       costhist_newcost,"
               "       currConcat(costhist_newcurr_id) AS newcurr,"
               "       'cost' AS costhist_oldcost_xtnumericrole,"
               "       'cost' AS costhist_newcost_xtnumericrole "
               "FROM costhist, costelem "
               "WHERE ( (costhist_costelem_id=costelem_id)"
               " AND (costhist_item_id=:item_id) ) "
               "ORDER BY costhist_date, costelem_type;" );
    q.bindValue(":actual", tr("Actual"));
    q.bindValue(":standard", tr("Standard"));
    q.bindValue(":delete", tr("Delete"));
    q.bindValue(":new", tr("New"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    _itemcost->populate(q);
  }
}
