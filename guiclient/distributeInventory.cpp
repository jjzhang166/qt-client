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

#include "distributeInventory.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include "assignLotSerial.h"
#include "distributeToLocation.h"
#include "inputManager.h"

#define cIncludeLotSerial   0x01
#define cNoIncludeLotSerial 0x02

distributeInventory::distributeInventory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bcDistribute,    SIGNAL(clicked()), this, SLOT(sBcDistribute()));
  connect(_default,         SIGNAL(clicked()), this, SLOT(sDefault()));
  connect(_defaultAndPost,  SIGNAL(clicked()), this, SLOT(sDefaultAndPost()));
  connect(_distribute,      SIGNAL(clicked()), this, SLOT(sSelectLocation()));
  connect(_itemloc, SIGNAL(itemSelected(int)), this, SLOT(sSelectLocation()));
  connect(_post,            SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_taggedOnly,  SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bc,   SIGNAL(textChanged(QString)), this, SLOT(sBcChanged(QString)));
  connect(_qtyOnly,     SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  omfgThis->inputManager()->notify(cBCLotSerialNumber, this, this, SLOT(sCatchLotSerialNumber(QString)));

  _trapClose = TRUE;

  _item->setReadOnly(TRUE);
  _qtyToDistribute->setPrecision(omfgThis->qtyVal());
  _qtyTagged->setPrecision(omfgThis->qtyVal());
  _qtyRemaining->setPrecision(omfgThis->qtyVal());
  
  _itemloc->addColumn(tr("Location"),     _itemColumn, Qt::AlignLeft,  true, "locationname");
  _itemloc->addColumn(tr("Default"),      _ynColumn,   Qt::AlignLeft,  true, "defaultlocation");
  _itemloc->addColumn(tr("Netable"),      _ynColumn,   Qt::AlignCenter,true, "location_netable");
  _itemloc->addColumn(tr("Lot/Serial #"), -1,          Qt::AlignLeft,  true, "lotserial");
  _itemloc->addColumn(tr("Expiration"),   _dateColumn, Qt::AlignCenter,true, "f_expiration");
  _itemloc->addColumn(tr("Qty. Before"),  _qtyColumn,  Qt::AlignRight, true, "qty");
  _itemloc->addColumn(tr("Tagged Qty."),  _qtyColumn,  Qt::AlignRight, true, "qtytagged");
  _itemloc->addColumn(tr("Qty. After"),   _qtyColumn,  Qt::AlignRight, true, "balance");
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
  
  //If not lot serial control, hide info
  if (!_metrics->boolean("LotSerialControl"))
  {
    _lotSerial->hide();
    _lotSerialLit->hide();
    _bcLit->hide();
    _bc->hide();
    _bcQtyLit->hide();
    _bcQty->hide();
    _bcDistribute->hide();
  }

  _itemlocdistid = -1;
}

distributeInventory::~distributeInventory()
{
  // no need to delete child widgets, Qt does it all for us
}

void distributeInventory::languageChange()
{
  retranslateUi(this);
}
int distributeInventory::SeriesAdjust(int pItemlocSeries, QWidget *pParent, const QString & pPresetLotnum, const QDate & pPresetLotexp, const QDate & pPresetLotwarr)
{
  if (pItemlocSeries != 0)
  {
    XSqlQuery itemloc;
    itemloc.prepare( "SELECT itemlocdist_id, itemlocdist_reqlotserial," 
                     "       itemlocdist_distlotserial, itemlocdist_qty,"
                     "       itemsite_loccntrl, itemsite_controlmethod,"
                     "       itemsite_perishable, itemsite_warrpurc "
                     "FROM itemlocdist, itemsite "
                     "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
                     " AND (itemlocdist_series=:itemlocdist_series) ) "
                     "ORDER BY itemlocdist_id;" );
    itemloc.bindValue(":itemlocdist_series", pItemlocSeries);
    itemloc.exec();
    while (itemloc.next())
    {
      if (itemloc.value("itemlocdist_reqlotserial").toBool())
      {
        int itemlocSeries = -1;
        XSqlQuery query;
        // Check to see if this is a lot controlled item and if we have
        // a predefined lot#/expdate to use. If so assign that information
        // with itemlocdist_qty and move on. otherwise do the normal dialog
        // to ask the user for that information.
        if(itemloc.value("itemsite_controlmethod").toString() == "L" && !pPresetLotnum.isEmpty())
        {
          query.exec("SELECT nextval('itemloc_series_seq') AS _itemloc_series;");
          if(query.first())
          {
            itemlocSeries = query.value("_itemloc_series").toInt();
            query.prepare( "SELECT createlotserial(itemlocdist_itemsite_id,:lotserial,:itemlocdist_series,'I',itemlocdist_id,:qty,:expiration,:warranty)"
                           "FROM itemlocdist "
                           "WHERE (itemlocdist_id=:itemlocdist_id);"
                           
                           "UPDATE itemlocdist "
                           "SET itemlocdist_source_type='O' "
                           "WHERE (itemlocdist_series=:itemlocdist_series);"
              
                           "DELETE FROM itemlocdist "
                           "WHERE (itemlocdist_id=:itemlocdist_id);" );
            query.bindValue(":lotserial", pPresetLotnum);
            query.bindValue(":qty", itemloc.value("itemlocdist_qty"));
            query.bindValue(":itemlocdist_series", itemlocSeries);
            query.bindValue(":itemlocdist_id", itemloc.value("itemlocdist_id"));
            if(itemloc.value("itemsite_perishable").toBool())
              query.bindValue(":expiration", pPresetLotexp);
            else
              query.bindValue(":expiration", omfgThis->startOfTime());
            if(itemloc.value("itemsite_warrpurc").toBool())
              query.bindValue(":warranty", pPresetLotwarr);
            query.exec();
          }
        }

        if(itemlocSeries == -1)
        {
          ParameterList params;
          params.append("itemlocdist_id", itemloc.value("itemlocdist_id").toInt());

          assignLotSerial newdlg(pParent, "", TRUE);
          newdlg.set(params);
          itemlocSeries = newdlg.exec();
          if (itemlocSeries == -1)
            return XDialog::Rejected;
        }
        
        if (itemloc.value("itemsite_loccntrl").toBool())
        {
          query.prepare( "SELECT itemlocdist_id " 
                         "FROM itemlocdist "
                         "WHERE (itemlocdist_series=:itemlocdist_series) "
                         "ORDER BY itemlocdist_id;" );
          query.bindValue(":itemlocdist_series", itemlocSeries);
          query.exec();
          while (query.next())
          {
            ParameterList params;
            params.append("itemlocdist_id", query.value("itemlocdist_id").toInt());
            distributeInventory newdlg(pParent, "", TRUE);
            newdlg.set(params);
            if (newdlg.exec() == XDialog::Rejected)
              return XDialog::Rejected;
          }
        }
        else
        {
          query.prepare( "UPDATE itemlocdist "
                         "SET itemlocdist_source_type='L', itemlocdist_source_id=-1 "
                         "WHERE (itemlocdist_series=:itemlocdist_series); ");
          query.bindValue(":itemlocdist_series", itemlocSeries);
          query.exec();

          query.prepare( "SELECT distributeItemlocSeries(:itemlocdist_series) AS result;");
          query.bindValue(":itemlocdist_series", itemlocSeries);
          query.exec();
        }
      }
      else
      {
        ParameterList params;
        params.append("itemlocdist_id", itemloc.value("itemlocdist_id").toInt());

        if (itemloc.value("itemlocdist_distlotserial").toBool())
          params.append("includeLotSerialDetail");

        distributeInventory newdlg(pParent, "", TRUE);
        newdlg.set(params);
        if (newdlg.exec() == XDialog::Rejected)
          return XDialog::Rejected;
      }
    }
    
    XSqlQuery post;
    post.prepare("SELECT postItemlocseries(:itemlocseries) AS result;");
    post.bindValue(":itemlocseries",  pItemlocSeries);
    post.exec();
    if (post.first())
    {
      if (!post.value("result").toBool())
            QMessageBox::warning( 0, tr("Inventory Distribution"), 
        tr("There was an error posting the transaction.  Contact your administrator") );
    }
    else if (post.lastError().type() != QSqlError::None)
    {
      systemError(0, post.lastError().databaseText(), __FILE__, __LINE__);
      return XDialog::Rejected;
    }
  }
  
  return XDialog::Accepted;
}

enum SetResponse distributeInventory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("includeLotSerialDetail", &valid);
  if (valid && _metrics->boolean("LotSerialControl"))
  {
    _mode = cIncludeLotSerial;
  }
  else
    _mode = cNoIncludeLotSerial;

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();
    populate();
  }

  return NoError;
}

void distributeInventory::closeEvent(QCloseEvent *pEvent)
{
  if (_trapClose)
  {
    QMessageBox::critical( this, tr("Cannot Cancel Distribution"),
                           tr( "<p>You must select locations to distribute to/from "
			       "and select the 'Post' button. You may not cancel "
			       "this action." ) );
    pEvent->ignore();
  }
  else
    pEvent->accept();
}

void distributeInventory::populate()
{
  q.prepare("SELECT itemsite_controlmethod "
	    "FROM itemsite, itemlocdist "
	    "WHERE ((itemlocdist_itemsite_id=itemsite_id)"
	    "  AND  (itemlocdist_id=:itemlocdist_id));");

  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.first())
  {
    _controlMethod = q.value("itemsite_controlmethod").toString();
    _bc->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    _bcQty->setEnabled(_controlMethod == "L");
    _bcDistribute->setEnabled(_controlMethod == "L" || _controlMethod == "S");
    if (_controlMethod == "S")
      _bcQty->setText("1");
    else
      _bcQty->clear();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

//  Auto distribute location reservations
  if ( (_metrics->boolean("EnableSOReservationsByLocation")) && (_mode == cIncludeLotSerial) )
  {
    q.prepare("SELECT distributeToReservedItemLoc(:itemlocdist_id) AS result;");
    q.bindValue(":itemlocdist_id", _itemlocdistid);
    q.exec();
  }

  sFillList();
}

void distributeInventory::sSelectLocation()
{
  ParameterList params;
  params.append("source_itemlocdist_id", _itemlocdistid);

  if (_itemloc->altId() == cLocation)
    params.append("location_id", _itemloc->id());
  else if (_itemloc->altId() == cItemloc)
    params.append("itemlocdist_id", _itemloc->id());

  distributeToLocation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    sFillList();
}


void distributeInventory::sPost()
{
  if (_qtyRemaining->toDouble() != 0.0)
  {
    QMessageBox::critical( this, tr("Cannot Perform Partial Distribution"),
                           tr( "<p>You must completely distribute the quantity "
			       "before selecting the 'Post' button." ) );
    return;
  }

  q.prepare("SELECT distributeToLocations(:itemlocdist_id) AS result;");
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(0, q.lastError().databaseText(), __FILE__, __LINE__);
    reject();
  }
    
  _trapClose = FALSE;
  accept();
}

void distributeInventory::sDefault()
{
  if(_mode == cIncludeLotSerial)
    q.prepare("SELECT distributeToDefaultItemLoc(:itemlocdist_id) AS result;");
  else
    q.prepare("SELECT distributeToDefault(:itemlocdist_id) AS result;");
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  sFillList();
}

void distributeInventory::sDefaultAndPost()
{
  sDefault();
  sPost();
}

void distributeInventory::sFillList()
{
  q.prepare( "SELECT itemsite_id, "
             "       COALESCE(itemsite_location_id,-1) AS itemsite_location_id,"
             "       formatlotserialnumber(itemlocdist_ls_id) AS lotserial,"
             "       (itemlocdist_order_type || ' ' || formatSoItemNumber(itemlocdist_order_id)) AS order,"
             "       (itemsite_controlmethod IN ('L', 'S')) AS lscontrol,"
             "       parent.itemlocdist_qty AS qtytodistribute,"
             "       ( ( SELECT COALESCE(SUM(child.itemlocdist_qty), 0)"
             "             FROM itemlocdist AS child"
             "            WHERE (child.itemlocdist_itemlocdist_id=parent.itemlocdist_id) ) ) AS qtytagged,"
             "       (parent.itemlocdist_qty - ( SELECT COALESCE(SUM(child.itemlocdist_qty), 0)"
             "                                     FROM itemlocdist AS child"
             "                                    WHERE (child.itemlocdist_itemlocdist_id=parent.itemlocdist_id) ) ) AS qtybalance "
             "FROM itemsite, itemlocdist AS parent "
             "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
             " AND (itemlocdist_id=:itemlocdist_id) );" );
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.first())
  {
    _item->setItemsiteid(q.value("itemsite_id").toInt());
    _lotSerial->setText(q.value("lotserial").toString());
    _order->setText(q.value("order").toString());
    _qtyToDistribute->setDouble(q.value("qtytodistribute").toDouble());
    _qtyTagged->setDouble(q.value("qtytagged").toDouble());
    _qtyRemaining->setDouble(q.value("qtybalance").toDouble());

    if ( (q.value("itemsite_location_id").toInt() != -1) &&
         ( (_mode == cNoIncludeLotSerial) || ( (_mode == cIncludeLotSerial) && (!q.value("lscontrol").toBool()) ) ) )
    {
      _default->setEnabled(TRUE);
      _defaultAndPost->setEnabled(TRUE);
    }
    else
    {
      _default->setEnabled(FALSE);
      _defaultAndPost->setEnabled(FALSE);
    }
    
    if (q.value("qtytodistribute").toDouble() < 0)
      _qtyOnly->hide();

    QString sql( "SELECT id, type, locationname, defaultlocation,"
		 "       location_netable, lotserial, f_expiration, expired,"
                 "       qty, qtytagged, (qty + qtytagged) AS balance,"
                 "       'qty' AS qty_xtnumericrole,"
                 "       'qty' AS qtytagged_xtnumericrole,"
                 "       'qty' AS balance_xtnumericrole,"
                 "       CASE WHEN expired THEN 'error' END AS qtforegroundrole "
                 "FROM (" 
		 "<? if exists(\"cNoIncludeLotSerial\") ?>"
		 "SELECT location_id AS id, <? value(\"locationType\") ?> AS type,"
		 "       formatLocationName(location_id) AS locationname,"
		 "       (location_id=itemsite_location_id) AS defaultlocation,"
		 "       location_netable,"
		 "       TEXT('') AS lotserial,"
		 "       TEXT(<? value(\"na\") ?>) AS f_expiration, FALSE AS expired,"
		 "       qtyLocation(location_id, NULL, NULL, NULL, itemsite_id, itemlocdist_order_type, itemlocdist_order_id) AS qty,"
		 "       itemlocdistQty(location_id, itemlocdist_id) AS qtytagged "
		 "FROM itemlocdist, location, itemsite "
		 "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
		 " AND (itemsite_loccntrl)"
		 " AND (itemsite_warehous_id=location_warehous_id)"
		 " AND (validLocation(location_id, itemsite_id))"
		 " AND (itemlocdist_id=<? value(\"itemlocdist_id\") ?>) ) "
		 "<? elseif exists(\"cIncludeLotSerial\") ?>"
		 "SELECT itemloc_id AS id, <? value(\"itemlocType\") ?> AS type,"
		 "       COALESCE(formatLocationName(location_id),"
		 "                <? value(\"undefined\") ?>) AS locationname,"
		 "       (location_id IS NOT NULL"
		 "        AND location_id=itemsite_location_id) AS defaultlocation,"
		 "       COALESCE(location_netable, false) AS location_netable,"
		 "       ls_number AS lotserial,"
		 "       CASE WHEN (itemsite_perishable) THEN formatDate(itemloc_expiration)"
		 "            ELSE <? value(\"na\") ?>"
		 "       END AS f_expiration,"
		 "       CASE WHEN (itemsite_perishable) THEN (itemloc_expiration < CURRENT_DATE)"
		 "            ELSE FALSE" 
		 "       END AS expired,"
		 "       qtyLocation(itemloc_location_id, itemloc_ls_id, itemloc_expiration, itemloc_warrpurc, itemsite_id, itemlocdist_order_type, itemlocdist_order_id) AS qty,"
		 "       ( SELECT COALESCE(SUM(target.itemlocdist_qty), 0)"
		 "         FROM itemlocdist AS target"
		 "         WHERE ( (target.itemlocdist_source_type='I')"
		 "          AND (target.itemlocdist_source_id=itemloc_id)"
		 "          AND (target.itemlocdist_itemlocdist_id=source.itemlocdist_id)) ) AS qtytagged "
		 "FROM itemlocdist AS source, itemsite, itemloc "
                 "  LEFT OUTER JOIN location ON (itemloc_location_id=location_id) "
                 "  LEFT OUTER JOIN ls ON (itemloc_ls_id=ls_id) "
		 "WHERE ( (source.itemlocdist_itemsite_id=itemsite_id)"
		 " AND (itemloc_itemsite_id=itemsite_id)"
		 " AND (source.itemlocdist_id=<? value(\"itemlocdist_id\") ?>) ) "
		 " UNION "
		 "SELECT location_id AS id, <? value(\"locationType\") ?> AS type,"
		 "       formatLocationName(location_id) AS locationname,"
		 "       (location_id=itemsite_location_id) AS defaultlocation,"
		 "       location_netable,"
		 "       TEXT('') AS lotserial,"
		 "       TEXT(<? value(\"na\") ?>) AS f_expiration, FALSE AS expired,"
		 "       qtyLocation(location_id, NULL, NULL, NULL, itemsite_id, itemlocdist_order_type, itemlocdist_order_id) AS qty,"
		 "       itemlocdistQty(location_id, itemlocdist_id) AS qtytagged "
		 "FROM itemlocdist, location, itemsite "
		 "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
		 " AND (itemsite_loccntrl)"
		 " AND (itemsite_warehous_id=location_warehous_id)"
		 " AND (validLocation(location_id, itemsite_id))"
                 " AND (itemsite_id=<? value(\"itemsite_id\") ?> ) "
		 " AND (location_id NOT IN (SELECT DISTINCT itemloc_location_id FROM itemloc WHERE (itemloc_itemsite_id=itemsite_id)))"
		 " AND (itemlocdist_id=<? value(\"itemlocdist_id\") ?>) ) "
		 "<? endif ?>"
		 ") AS data "
                 "WHERE ((TRUE) "
		 "<? if exists(\"showOnlyTagged\") ?>"
		 "AND (qtytagged != 0) "
		 "<? endif ?>"
                 "<? if exists(\"showQtyOnly\") ?>"
                 "AND (qty > 0) "
                 "<? endif ?>"
		 ") ORDER BY locationname;");

    ParameterList params;

    if (_mode == cNoIncludeLotSerial)
      params.append("cNoIncludeLotSerial");
    else if (_mode == cIncludeLotSerial)
      params.append("cIncludeLotSerial");

    if (_taggedOnly->isChecked())
      params.append("showOnlyTagged");
      
    if ( (_qtyOnly->isChecked())  ||
        (q.value("qtytodistribute").toDouble() < 0) )
      params.append("showQtyOnly");

    params.append("locationType",   cLocation);
    params.append("itemlocType",    cItemloc);
    params.append("yes",            tr("Yes"));
    params.append("no",             tr("No"));
    params.append("na",             tr("N/A"));
    params.append("undefined",      tr("Undefined"));
    params.append("itemlocdist_id", _itemlocdistid);
    params.append("itemsite_id",    q.value("itemsite_id").toInt());

    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);

    _itemloc->populate(q, true);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void distributeInventory::sBcDistribute()
{
  if (_bc->text().isEmpty())
  {
    QMessageBox::warning(this, tr("No Bar Code scanned"),
			 tr("<p>Cannot search for Items by Bar Code without a "
			    "Bar Code."));
    _bc->setFocus();
    return;
  }

  q.prepare( "SELECT itemloc_id "
	     "FROM  itemlocdist, itemloc, itemsite, ls "
	     "WHERE ((itemlocdist_itemsite_id=itemloc_itemsite_id)"
	     "  AND  (itemloc_itemsite_id=itemsite_id)"
             "  AND  (itemsite_controlmethod IN ('L', 'S'))"
             "  AND  (itemsite_item_id=ls_item_id)"
             "  AND  (itemloc_ls_id=ls_id)"
             "  AND  (itemsite_warehous_id=:warehous_id)"
	     "  AND  (ls_number=:lotserial)"
	     "  AND  (itemlocdist_id=:itemlocdist_id));");

  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.bindValue(":lotserial",      _bc->text());
  q.bindValue(":warehous_id",    _warehouse->id());
  q.exec();

  if(!q.first())
  {
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    QMessageBox::warning(this, tr("No Match Found"),
			 tr("<p>No available lines match the specified Barcode.")); 
    _bc->clear();
    return;
  }

  ParameterList params;
  params.append("itemlocdist_id",        q.value("itemloc_id"));
  params.append("source_itemlocdist_id", _itemlocdistid);
  params.append("qty",                   _bcQty->text());
  params.append("distribute");

  distributeToLocation newdlg(this, "", TRUE);
  if (newdlg.set(params) != NoError)
    return;

  _bc->clear();
  if (_controlMethod == "S")
    _bcQty->setText("1");
  else
    _bcQty->clear();
  sFillList();
  _bc->setFocus();
}

void distributeInventory::sCatchLotSerialNumber(const QString plotserial)
{
  //qDebug("sCatchLotSerialNumber");
  _bc->setText(plotserial);
  if (_controlMethod == "S")
    _bcDistribute->setFocus();
  else
    _bcQty->setFocus();
}

void distributeInventory::sBcChanged(const QString p)
{
  _post->setDefault(p.isEmpty());
  _bcDistribute->setDefault(! p.isEmpty());
}
