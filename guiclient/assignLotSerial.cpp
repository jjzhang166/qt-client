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

#include "assignLotSerial.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "createLotSerial.h"

assignLotSerial::assignLotSerial(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(this, SIGNAL(rejected()), this, SLOT(sCancel()));

  _trapClose = TRUE;

  _item->setReadOnly(TRUE);

  _itemlocdist->addColumn( tr("Lot/Serial #"),     -1, Qt::AlignLeft,  true, "ls_number");
  _itemlocdist->addColumn( tr("Expires"), _dateColumn, Qt::AlignCenter,true, "itemlocdist_expiration");
  _itemlocdist->addColumn( tr("Warranty"),_dateColumn, Qt::AlignCenter,true, "itemlocdist_warranty");
  _itemlocdist->addColumn( tr("Qty."),     _qtyColumn, Qt::AlignRight, true, "itemlocdist_qty");

  _qtyToAssign->setPrecision(omfgThis->qtyVal());
  _qtyAssigned->setPrecision(omfgThis->qtyVal());
  _qtyBalance->setPrecision(omfgThis->qtyVal());
}

assignLotSerial::~assignLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void assignLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse assignLotSerial::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    q.exec("SELECT NEXTVAL('itemloc_series_seq') AS _itemloc_series;");
    if (q.first())
    {
      _itemlocdistid = param.toInt();
      _itemlocSeries = q.value("_itemloc_series").toInt();

      q.prepare( "SELECT itemlocdist_itemsite_id "
                 "FROM itemlocdist "
                 "WHERE (itemlocdist_id=:itemlocdist_id);" );
      q.bindValue(":itemlocdist_id", _itemlocdistid);
      q.exec();
      if (q.first())
        _item->setItemsiteid(q.value("itemlocdist_itemsite_id").toInt());
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      sFillList();
        sNew();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void assignLotSerial::closeEvent(QCloseEvent *pEvent)
{
  if (_trapClose)
  {
    QMessageBox::critical( this, tr("Cannot Cancel Distribution"),
                           tr("<p>You must indicate the Lot/Serial # to "
			      "assign and select the 'Assign' button. You may "
			      "not cancel this action." ) );
    pEvent->ignore();
  }
  else
    pEvent->accept();
}

void assignLotSerial::sNew()
{
  ParameterList params;
  params.append("itemloc_series", _itemlocSeries);
  params.append("itemlocdist_id", _itemlocdistid);
  params.append("qtyRemaining",    _qtyBalance->text());

  createLotSerial newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillList();
    if (_qtyBalance->toDouble() > 0)
      sNew();
  }
}

void assignLotSerial::sDelete()
{
  q.prepare( "DELETE FROM itemlocdist "
             "WHERE (itemlocdist_id=:itemlocdist_id);"

             "DELETE FROM lsdetail "
             "WHERE ( (lsdetail_source_type='I')"
             " AND (lsdetail_source_id=:itemlocdist_id) );" );
  q.bindValue(":itemlocdist_id", _itemlocdist->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void assignLotSerial::sClose()
{
  q.prepare( "DELETE FROM itemlocdist "
             "WHERE ( (itemlocdist_source_type='D')"
             " AND (itemlocdist_source_id=:source_id) );" );
  q.bindValue(":source_id", _itemlocdistid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  reject();
}

void assignLotSerial::sAssign()
{
  if (_qtyBalance->toDouble() != 0.0)
  {
    QMessageBox::warning( this, tr("Incomplete Assignment"),
                          tr( "<p>You must assign a Lot/Serial # to the "
			      "remaining Quantity before saving this "
			      "assignment." ) );
    _new->setFocus();
    return;
  }

  q.prepare( "UPDATE itemlocdist "
             "SET itemlocdist_source_type='O' "
             "WHERE (itemlocdist_series=:itemlocdist_series);"

             "DELETE FROM itemlocdist "
             "WHERE (itemlocdist_id=:itemlocdist_id);" );
  q.bindValue(":itemlocdist_series", _itemlocSeries);
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _trapClose = FALSE;
  done(_itemlocSeries);
}

void assignLotSerial::sFillList()
{
  double openQty = 0;

  q.prepare( "SELECT itemlocdist_qty AS qty "
             "FROM itemlocdist "
             "WHERE (itemlocdist_id=:itemlocdist_id);" );
  q.bindValue(":itemlocdist_id", _itemlocdistid);
  q.exec();
  if (q.first())
  {
    openQty = q.value("qty").toDouble();
    _qtyToAssign->setDouble(openQty);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT COALESCE(SUM(itemlocdist_qty), 0) AS totalqty "
             "FROM itemlocdist "
             "WHERE (itemlocdist_series=:itemlocdist_series);" );
  q.bindValue(":itemlocdist_series", _itemlocSeries);
  q.exec();
  if (q.first())
  {
    _qtyAssigned->setDouble(q.value("totalqty").toDouble());
    _qtyBalance->setDouble(openQty - q.value("totalqty").toDouble());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }


  q.prepare( "SELECT itemlocdist.*, ls_number,"
             "       CASE WHEN (NOT itemsite_perishable) THEN :na "
             "       END AS itemlocdist_expiration_qtdisplayrole, "
             "       CASE WHEN (NOT itemsite_warrpurc) THEN :na "
             "       END AS itemlocdist_warranty_qtdisplayrole, "
             "       'qty' AS itemlocdist_qty_xtnumericrole "
             "FROM itemlocdist "
             "LEFT OUTER JOIN itemsite ON (itemlocdist_itemsite_id = itemsite_id), "
             "ls "
             "WHERE (itemlocdist_series=:itemlocdist_series) "
             "AND (itemlocdist_ls_id=ls_id) "
             "ORDER BY ls_number;" );
  q.bindValue(":itemlocdist_series", _itemlocSeries);
  q.bindValue(":na", "N/A");
  q.exec();
  _itemlocdist->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void assignLotSerial::sCancel()
{
  done(-1);
}
