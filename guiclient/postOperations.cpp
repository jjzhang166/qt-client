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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
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
 * Powered by PostBooks, an open source solution from xTuple
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

#include "postOperations.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <math.h>
#include "inputManager.h"
#include "closeWo.h"
#include "distributeInventory.h"
#include "distributeBreederProduction.h"
#include "relocateInventory.h"
#include "scrapWoMaterialFromWIP.h"

postOperations::postOperations(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_scrap, SIGNAL(clicked()), this, SLOT(sScrap()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sHandleWoid(int)));
  connect(_wooper, SIGNAL(newID(int)), this, SLOT(sHandleWooperid(int)));
  connect(_postSutime, SIGNAL(toggled(bool)), this, SLOT(sHandlePostSetupTime(bool)));
  connect(_postRntime, SIGNAL(toggled(bool)), this, SLOT(sHandlePostRunTime(bool)));
  connect(_postSpecifiedRntime, SIGNAL(toggled(bool)), this, SLOT(sHandlePostRunTime(bool)));
  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleQty()));
  connect(_productionUOM, SIGNAL(toggled(bool)), this, SLOT(sHandleQty()));
  connect(_specifiedSutime, SIGNAL(textChanged(const QString&)), this, SLOT(sSetupChanged()));

  _captive = FALSE;
  _wrkcntid = -1;
  _wotcTime = 0.0;
  _usingWotc = false;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));
  omfgThis->inputManager()->notify(cBCWorkOrderOperation, this, this, SLOT(sCatchWooperid(int)));

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _wooper->setAllowNull(TRUE);

  _receiveInventory->setEnabled(_privleges->check("ChangeReceiveInventory"));
  _postStandardSutime->setEnabled(_privleges->check("OverrideWOTCTime"));
  _postStandardRntime->setEnabled(_privleges->check("OverrideWOTCTime"));
  _specifiedRntime->setEnabled(_privleges->check("OverrideWOTCTime"));

  _qty->setValidator(omfgThis->qtyVal());

  _specifiedSutime->setValidator(omfgThis->runTimeVal());
  _specifiedRntime->setValidator(omfgThis->runTimeVal());

  _womatl->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft  );
  _womatl->addColumn(tr("Description"), -1,          Qt::AlignLeft  );
  _womatl->addColumn(tr("Iss. UOM"),    _uomColumn,  Qt::AlignLeft  );
  _womatl->addColumn(tr("Qty. per"),    _qtyColumn,  Qt::AlignRight );
}

postOperations::~postOperations()
{
  // no need to delete child widgets, Qt does it all for us
}

void postOperations::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postOperations::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
    _wooper->setFocus();
  }

  param = pParams.value("wooper_id", &valid);
  if (valid)
  {
    q.prepare("SELECT wooper_wo_id"
              "  FROM wooper"
              " WHERE (wooper_id=:wooper_id);");
    q.bindValue(":wooper_id", param.toInt());
    q.exec();
    if(q.first())
    {
      _wo->setId(q.value("wooper_wo_id").toInt());
      _wo->setEnabled(false);
      _wooper->setId(param.toInt());
      _wooper->setEnabled(false);
      _qty->setFocus();
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  param = pParams.value("wotc_id", &valid);
  if (valid)
  {
    _wotc_id = param;
    q.prepare("SELECT wotc_wo_id, wotc_wooper_id,"
	      "      intervalToMinutes(wotcTime(wotc_id)) AS time"
	      "  FROM wotc"
	      " WHERE (wotc_id=:wotc_id);");
    q.bindValue(":wotc_id", _wotc_id);
    q.exec();
    if (q.first())
    {
      _usingWotc = true;
      _wo->setId(q.value("wotc_wo_id").toInt());
      _wo->setEnabled(false);
      _wooper->setId(q.value("wotc_wooper_id").toInt());
      _wooper->setEnabled(false);
      _wotcTime = q.value("time").toDouble();
      _specifiedSutime->clear();
      _specifiedRntime->setText(QString::number(_wotcTime));
      _qty->setFocus();
    }
    else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _setupUser->setId(param.toInt());
    _runUser->setId(param.toInt());
  }

  param = pParams.value("issueComponents", &valid);
  if (valid)
    _issueComponents->setChecked(param.toBool());

  param = pParams.value("fromWOTC", &valid);
  if (valid)
  {
    _scrap->setHidden(param.toBool());
    _setupUser->setEnabled(false);
    _runUser->setEnabled(false);
  }

  return NoError;
}

void postOperations::sHandleWoid(int pWoid)
{
  XSqlQuery w;
  w.prepare( "SELECT wooper_id, (wooper_seqnumber || ' - ' || wooper_descrip1 || ' ' || wooper_descrip2) "
             "FROM wooper "
             "WHERE (wooper_wo_id=:wo_id) "
             "ORDER BY wooper_seqnumber;" );
  w.bindValue(":wo_id", pWoid);
  w.exec();
  _wooper->populate(w);
  if (w.lastError().type() != QSqlError::None)
  {
    systemError(this, w.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  w.prepare( "SELECT uom_name "
             "FROM wo, itemsite, item, uom "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (item_inv_uom_id=uom_id)"
             " AND (wo_id=:wo_id) );" );
  w.bindValue(":wo_id", pWoid);
  w.exec();
  if (w.first())
    _inventoryUOM->setText( tr("Post in Inventory UOMs (%1)")
                            .arg(w.value("uom_name").toString()) );
  else if (w.lastError().type() != QSqlError::None)
  {
    systemError(this, w.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void postOperations::sHandleWooperid(int)
{
  if (_wooper->id() != -1)
  {
    double wooperSuConsumed = 0.0;

    XSqlQuery w;
    w.prepare( "SELECT formatQty(wo_qtyord) AS ordered,"
               "       formatQty(COALESCE(wooper_qtyrcv,0)) AS received,"
               "       formatQty(noNeg(wo_qtyord - COALESCE(wooper_qtyrcv,0))) AS f_balance,"
               "       noNeg(wo_qtyord - COALESCE(wooper_qtyrcv, 0)) AS balance,"
               "       wooper_issuecomp, wooper_rcvinv, wooper_produom,"
               "       wooper_sucomplete, wooper_rncomplete,"
	           "       wooper_sutime, wooper_suconsumed,"
               "       formatTime(noNeg(wooper_sutime - wooper_suconsumed)) AS suremaining,"
               "       wooper_rnqtyper, wooper_invproduomratio, wooper_wrkcnt_id,"
               "       (COALESCE(wooper_qtyrcv,0) = 0) AS noqty, "
			   "       item_type "
               "FROM wo, wooper, itemsite, item "
               "WHERE ( (wooper_wo_id=wo_id)"
               " AND (wooper_id=:wooper_id) "
			   " AND (wo_itemsite_id=itemsite_id) "
			   " AND (itemsite_item_id=item_id) );" );
    w.bindValue(":wooper_id", _wooper->id());
    w.exec();
    if (w.first())
    {
      _rnqtyper = w.value("wooper_rnqtyper").toDouble();
      _invProdUOMRatio = w.value("wooper_invproduomratio").toDouble();

      _qtyOrdered->setText(w.value("ordered").toString());
      _qtyReceived->setText(w.value("received").toString());
      _qtyBalance->setText(w.value("f_balance").toString());
      _balance = w.value("balance").toDouble();
      if(_metrics->boolean("AutoFillPostOperationQty"))
        _qty->setText(QString::number(_balance));
      else
        _qty->setFocus();
      _productionUOM->setText( tr("Post in Production UOMs (%1)")
                               .arg(w.value("wooper_produom").toString()) );

      _wrkcntid = w.value("wooper_wrkcnt_id").toInt();
      _wrkcnt->setId(_wrkcntid);
      _wrkcnt->setReadOnly(!w.value("noqty").toBool());

      wooperSuConsumed = w.value("wooper_suconsumed").toDouble();

      if (!w.value("wooper_sucomplete").toBool())
      {
        _postSutime->setEnabled(TRUE);
        _postSutime->setChecked(TRUE);
        _markSuComplete->setChecked(TRUE);
        _standardSutime->setText(w.value("suremaining").toString());
      }
      else
      {
        _postSutime->setEnabled(FALSE);
        _postSutime->setChecked(FALSE);
        _markSuComplete->setChecked(FALSE);
        _standardSutime->clear();
        _specifiedSutime->clear();
      }
  
      _postRntime->setEnabled(!w.value("wooper_rncomplete").toBool());
      _postRntime->setChecked(!w.value("wooper_rncomplete").toBool());

      _receiveInventory->setEnabled(w.value("wooper_rcvinv").toBool());
      _receiveInventory->setChecked(w.value("wooper_rcvinv").toBool());

      if (w.value("wooper_issuecomp").toBool())
      {
        w.prepare( "SELECT womatl_id, item_number, (item_descrip1 || ' ' || item_descrip2),"
                   "       uom_name, formatQtyPer(womatl_qtyper) "
                   "FROM womatl, itemsite, item, uom "
                   "WHERE ( (womatl_itemsite_id=itemsite_id)"
                   " AND (womatl_issuemethod IN ('L', 'M'))"
                   " AND (womatl_uom_id=uom_id)"
                   " AND (itemsite_item_id=item_id)"
                   " AND (womatl_wooper_id=:wooper_id) ) "
                   "ORDER BY item_number;" );
        w.bindValue(":wooper_id", _wooper->id());
	w.exec();
	_womatl->clear();
        _womatl->populate(w);
	if (w.lastError().type() != QSqlError::None)
	{
	  systemError(this, w.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}

        if (w.size() > 0)
        {
          _issueComponents->setEnabled(TRUE);
          _issueComponents->setChecked(TRUE);
        }
        else
        {
          _issueComponents->setEnabled(FALSE);
          _issueComponents->setChecked(FALSE);
        }
      }
      else
      {
        _womatl->clear();
        _issueComponents->setEnabled(FALSE);
        _issueComponents->setChecked(FALSE);
      }

	  _closeWO->setDisabled((w.value("item_type").toString() == "J" || !_privleges->check("CloseWorkOrders")));

      sHandleQty();
    }
    else if (w.lastError().type() != QSqlError::None)
    {
      systemError(this, w.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    w.prepare("SELECT intervalToMinutes(wooperTime(:wooper_id)) AS time;");
    w.bindValue(":wooper_id", _wooper->id());
    w.exec();
    if (w.first() && ! w.value("time").isNull())
    {
      _usingWotc = true;
      _wotcTime = w.value("time").toDouble();
      _specifiedSutime->setText(QString::number(wooperSuConsumed));
      _specifiedRntime->setText(QString::number(w.value("time").toDouble() - wooperSuConsumed));
      _postSpecifiedSutime->setChecked(TRUE);
      _postSpecifiedRntime->setChecked(TRUE);
    }
    else if (w.lastError().type() != QSqlError::NoError)
    {
      systemError(this, w.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
    {
      _usingWotc = false;
      _wotcTime = 0.0;
      _specifiedSutime->clear();
      _specifiedRntime->clear();
      _postStandardSutime->setChecked(TRUE);
      _postStandardRntime->setChecked(TRUE);
    }
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();
    _productionUOM->setText(tr("Post in Production UOMs"));

    _qty->clear();

    _wrkcntid = -1;
    _wrkcnt->setId(_wrkcntid);
    _wrkcnt->setReadOnly(true);

    _issueComponents->setChecked(FALSE);
    _receiveInventory->setChecked(FALSE);
    _womatl->clear();

    _postSutime->setEnabled(FALSE);
    _postSutime->setChecked(FALSE);
    _markSuComplete->setChecked(FALSE);
    _standardSutime->clear();
    _specifiedSutime->clear();
    
    _postRntime->setEnabled(FALSE);
    _postRntime->setChecked(FALSE);
    _markRnComplete->setChecked(FALSE);
    _standardSutime->clear();
    _specifiedSutime->clear();
  }
}

void postOperations::sHandleQty()
{
  double qty = _qty->toDouble();
  _markRnComplete->setChecked(qty >= _qtyBalance->text().toDouble());
  
  if (_wooper->id() == -1)
  {
    _standardRntime->clear();
    _markRnComplete->setChecked(FALSE);
    _closeWO->setChecked(FALSE);
    return;
  }
  else if (_closeWO->isEnabled())
  {
    if (_productionUOM->isChecked())
      _standardRntime->setText(formatQty(_rnqtyper * qty));
    else
      _standardRntime->setText(formatQty(_rnqtyper / _invProdUOMRatio * qty));

    _closeWO->setChecked(FALSE);

    if(qty >= _balance)
    {
      XSqlQuery w;
      w.prepare("SELECT boohead_closewo, wooper_wo_id"
                "  FROM wo, wooper, itemsite, boohead"
                " WHERE ((wo_id=wooper_wo_id)"
				"   AND (wo_itemsite_id=itemsite_id)"
				"   AND (itemsite_item_id=boohead_item_id)"
				"   AND (boohead_rev_id=wo_boo_rev_id)"
                "   AND (wooper_id=:wooper_id))"
                " LIMIT 1;");
      w.bindValue(":wooper_id", _wooper->id());
      w.exec();
      if(w.first() && w.value("boohead_closewo").toBool())
      {
        int woid = w.value("wooper_wo_id").toInt();
        w.prepare("SELECT wooper_id"
                  "  FROM wooper"
                  " WHERE((NOT wooper_rncomplete)"
                  "   AND (wooper_wo_id=:wo_id)"
                  "   AND (wooper_id != :wooper_id))"
                  " LIMIT 1;");
        w.bindValue(":wo_id", woid);
        w.bindValue(":wooper_id", _wooper->id());
        w.exec();
        if(!w.first())
          _closeWO->setChecked(TRUE);
      }
    }
  }
}

void postOperations::sScrap()
{
      ParameterList params;
      params.append("wo_id", _wo->id());
      params.append("allowTopLevel", QVariant(_receiveInventory->isChecked(), 0));

      scrapWoMaterialFromWIP newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
}

void postOperations::sPost()
{
  if (_wooper->id() == -1)
  {
    QMessageBox::critical( this, tr("Select W/O Operation to Post"),
                           tr("<p>Please select to W/O Operation to which you wish to post.") );

    _wooper->setFocus();
    return;
  }

  if (_qty->toDouble() == 0.0 &&
      QMessageBox::question(this, tr("Zero Quantity To Post"),
			      tr("<p>Are you sure that you want to post a "
			         "Quantity of Production = 0?"),
			      QMessageBox::Yes,
			      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _qty->setFocus();
    return;
  }

  if(_qty->toDouble() > _balance &&
    QMessageBox::warning(this, tr("Quantity To Post Greater than Balance"),
        tr("<p>The Quantity to post that you have specified is greater than the "
           "balance remaining for the Quantity to Receive. Are you sure you want to continue?"),
        QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _qty->setFocus();
    return;
  }

  double sutime = _specifiedSutime->text().toDouble();
  double rntime = _specifiedRntime->text().toDouble();
  if (_usingWotc && fabs(sutime + rntime - _wotcTime) >= 0.016 /* 1 sec */ &&
      _postSpecifiedSutime->isChecked() && _postSpecifiedRntime->isChecked())
  {
    /*
    qDebug(tr("setup: %1\trun: %2\ttotal: %3\ttimeclock: %4\tdiff: %5")
	     .arg(sutime) .arg(rntime) .arg(sutime+rntime) .arg(_wotcTime)
	     .arg(sutime + rntime - _wotcTime));
    */
    if (_privleges->check("OverrideWOTCTime"))
    {
      if (QMessageBox::question(this, tr("Work Times Mismatch"),
			    tr("<p>The specified setup and run times do not equal "
			       "the time recorded by users clocking in and out:"
			       "<br>%1 + %2 <> %3<p>Do you want to change the "
			       "setup and run times?")
			    .arg(_specifiedSutime->text())
			    .arg(_specifiedRntime->text())
			    .arg(_wotcTime),
			    QMessageBox::Yes | QMessageBox::Default,
			    QMessageBox::No) == QMessageBox::Yes)
      {
	_specifiedSutime->clear();
	_specifiedRntime->setText(QString::number(_wotcTime));
	_specifiedSutime->setFocus();
	return;
      }
    }
    else
    {
      QMessageBox::warning(this, tr("Work Times Mismatch"),
			    tr("<p>The specified setup and run times do not equal "
			       "the time recorded by users clocking in and out:"
			       "<br>%1 + %2 <> %3<p>Change the "
			       "setup or run time so they total %4.")
			    .arg(_specifiedSutime->text())
			    .arg(_specifiedRntime->text())
			    .arg(_wotcTime)
			    .arg(_wotcTime));
      _specifiedSutime->clear();
      _specifiedRntime->setText(QString::number(_wotcTime));
      _specifiedSutime->setFocus();
      return;
    }
  }

  int itemsiteid = -1;
  int thisLocid = -1;
  int nextLocid = -1;
  bool loccntrl = false;
  bool alreadyReceived = false;
  bool disallowBlankWIP = false;

  q.prepare("SELECT itemsite_id, COALESCE(itemsite_loccntrl, FALSE) AS loccntrl,"
            "       itemsite_disallowblankwip"
            "  FROM itemsite, wo"
            " WHERE ((wo_itemsite_id=itemsite_id)"
            "   AND  (wo_id=:wo_id));");
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if(q.first())
  {
    itemsiteid = q.value("itemsite_id").toInt();
    loccntrl = q.value("loccntrl").toBool();
    disallowBlankWIP = q.value("itemsite_disallowblankwip").toBool();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT wooper_id, wooper_seqnumber,"
            "       COALESCE(booitem_overlap, TRUE) AS overlap,"
            "       COALESCE( (SELECT p.wooper_rncomplete "
            "                    FROM wooper AS p "
            "                   WHERE ((p.wooper_wo_id=c.wooper_wo_id)"
            "                     AND  (p.wooper_seqnumber < c.wooper_seqnumber)) "
            "                   ORDER BY p.wooper_seqnumber DESC "
            "                   LIMIT 1), TRUE) AS prevcomplete,"
            "       wooper_wip_location_id "
            "  FROM wooper AS c LEFT OUTER JOIN booitem"
            "    ON (wooper_booitem_id=booitem_id) "
            " WHERE (wooper_id=:wooper_id);" );
  q.bindValue(":wooper_id", _wooper->id());
  q.exec();
  if(q.first())
  {
    if(!q.value("overlap").toBool() && !q.value("prevcomplete").toBool())
    {
      QMessageBox::critical( this, tr("Operation May Not Overlap"),
                             tr("<p>This Operation is not allowed to overlap "
				"with the preceding Operation. The preceding "
				"Operation must be completed before you may "
				"post Operations for this Operation.") );
      return;
    }

    thisLocid = q.value("wooper_wip_location_id").toInt();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_markRnComplete->isChecked())
  {
    q.prepare("SELECT count(*) AS countClockins "
	      "FROM wotc "
	      "WHERE ((wotc_timein IS NOT NULL)"
	      "  AND  (wotc_timeout IS NULL)"
	      "  AND  (wotc_wo_id=:wo_id)"
	      "  AND  (wotc_wooper_id=:wooper_id));");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":wooper_id", _wooper->id());
    if (q.exec() && q.first() && q.value("countClockins").toInt() > 0)
    {
      QMessageBox::critical(this, tr("Users Still Clocked In"),
			    tr("<p>This Operation still has %1 user(s) clocked "
			       "in. Have those users clock out before marking "
			       "this run as complete. For now, either click "
			       "Cancel, uncheck Mark Operation as Complete and "
			       "Post, or set the Quantity to 0 and Post.")
			    .arg(q.value("countClockins").toString()));
      return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  // If this is an MLC item we need to do some extra work to determine
  // some information for the WIP transfer functionality and if we should
  // even continue processing.
  if (_qty->toDouble() > 0.0 && loccntrl)
  {
    // first we need to determine what the next location_id is or the final location_id
    // if this is the last operation.
    q.prepare("SELECT b.wooper_wip_location_id AS location_id"
              "  FROM wooper AS a, wooper AS b"
              " WHERE ((a.wooper_id=:wooper_id)"
              "   AND  (b.wooper_wo_id=a.wooper_wo_id)"
              "   AND  (b.wooper_seqnumber > a.wooper_seqnumber))"
              " ORDER BY b.wooper_seqnumber"
              " LIMIT 1;");
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();
    if(q.first())
      nextLocid = q.value("location_id").toInt();
    else
    {
      // We didn't find a next operation so lets look at the boohead
      q.prepare("SELECT boohead_final_location_id AS location_id"
                "  FROM wo, wooper, itemsite, boohead"
				" WHERE ((boohead_rev_id=wo_boo_rev_id)"
				"   AND  (wo_itemsite_id=itemsite_id)"
				"   AND  (boohead_item_id=itemsite_item_id)"
				"   AND  (wo_id=wooper_wo_id)"
                "   AND  (wooper_id=:wooper_id))"
                " LIMIT 1;");
      q.bindValue(":wooper_id", _wooper->id());
      q.exec();
      if(q.first())
        nextLocid = q.value("location_id").toInt();
    }

    // Lets check the Location here and now. If the disallowBlankWIP option
    // is turned on we will have to stop and give an error message
    if(disallowBlankWIP && (nextLocid == -1))
    {
      QMessageBox::critical( this, tr("No WIP/Final Location"), 
        tr("No WIP/Final Location defined for next step. Please contact your System Administrator.") );
      return;
    }

    // If we are receiving inventory or if the next location is not
    // defined then there isn't much else to do here otherwise we
    // need to check if we have already received inventory and if
    // this location is not set then we need to determine the last
    // location that the inventory would have been placed into
    if((!_receiveInventory->isChecked()) && (nextLocid != -1))
    {
      // first lets determine if we have already received inventory.
      // if not the rest is kinda pointless.
      q.prepare("SELECT COALESCE(b.wooper_rcvinv, FALSE) AS result"
                "  FROM wooper AS a, wooper AS b"
                " WHERE ((a.wooper_id=:wooper_id)"
                "   AND  (b.wooper_wo_id=a.wooper_wo_id)"
                "   AND  (b.wooper_rcvinv=true)"
                "   AND  (b.wooper_seqnumber < a.wooper_seqnumber))"
                " ORDER BY b.wooper_seqnumber DESC"
                " LIMIT 1;");
      q.bindValue(":wooper_id", _wooper->id());
      q.exec();
      if(q.first())
        alreadyReceived = q.value("result").toBool();

      if(alreadyReceived)
      {
        if(thisLocid == -1)
        {
          // We need to find the last location that was used so we can
          // determine what location we need to move from.
          q.prepare("SELECT b.wooper_wip_location_id AS location_id"
                    "  FROM wooper AS a, wooper AS b"
                    " WHERE ((a.wooper_id=:wooper_id)"
                    "   AND  (b.wooper_wo_id=a.wooper_wo_id)"
                    "   AND  (b.wooper_wip_location_id <> -1)"
                    "   AND  (b.wooper_seqnumber < a.wooper_seqnumber))"
                    " ORDER BY b.wooper_seqnumber DESC"
                    " LIMIT 1;");
          q.bindValue(":wooper_id", _wooper->id());
          q.exec();
          if(q.first())
            thisLocid = q.value("location_id").toInt();
        }
      }
    }
  }

  double qty;
  double suTime;
  double rnTime;

  if (_productionUOM->isChecked())
    qty = (_qty->toDouble() / _invProdUOMRatio);
  else
    qty = _qty->toDouble();

  if (_postSutime->isChecked())
  {
    if (_postStandardSutime->isChecked())
      suTime = _standardSutime->text().toDouble();
    else
      suTime = _specifiedSutime->toDouble();
  }
  else
    suTime = 0;

  if (_postRntime->isChecked())
  {
    if (_postStandardRntime->isChecked())
      rnTime = _standardRntime->text().toDouble();
    else
      rnTime = _specifiedRntime->toDouble();
  }
  else
    rnTime = 0;

  if(_wrkcntid != _wrkcnt->id())
  {
    q.prepare("UPDATE wooper SET wooper_wrkcnt_id=:wrkcnt_id WHERE (wooper_id=:wooper_id);");
    q.bindValue(":wooper_id", _wooper->id());
    q.bindValue(":wrkcnt_id", _wrkcnt->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  // If the value alreadyReceived is set to true then we are going to popup the
  // relocate inventory screen first. This will allow a user to cancel the transaction.
  if(alreadyReceived)
  {
    ParameterList params;
    params.append("itemsite_id", itemsiteid);
    params.append("qty", qty);
    if(thisLocid != -1)
      params.append("source_location_id", thisLocid);
    if(nextLocid != -1)
      params.append("target_location_id", nextLocid);

    relocateInventory newdlg(this, "", true);
    newdlg.set(params);
    if(newdlg.exec() == QDialog::Rejected)
      return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare( "SELECT postOperation( :wooper_id, :qty, :issueComponents, :receiveInventory,"
             "                      :setupUser, :setupTime, :setupComplete, "
	     "                      :runUser, :runTime, :runComplete, :wotc_id ) AS result;");
  q.bindValue(":wooper_id", _wooper->id());
  q.bindValue(":qty", qty);
  q.bindValue(":issueComponents", QVariant(_issueComponents->isChecked(), 0));
  q.bindValue(":receiveInventory", QVariant(_receiveInventory->isChecked(), 0));
  q.bindValue(":setupComplete", QVariant(_markSuComplete->isChecked(), 0));
  q.bindValue(":runComplete", QVariant(_markRnComplete->isChecked(), 0));
  q.bindValue(":setupTime", suTime);
  q.bindValue(":runTime", rnTime);
  q.bindValue(":setupUser", _setupUser->username());
  q.bindValue(":runUser", _runUser->username());
  q.bindValue(":wotc_id", _wotc_id);
  q.exec();
  if (q.first())
  {
    int itemlocSeries = q.value("result").toInt();
    if (itemlocSeries < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at postOperations::%1, Work Order Operation ID #2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wooper->id())
                         .arg(itemlocSeries) );
      return;
    }
    else
    {
      if (qty > 0.0)
      {
        if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == QDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Post Operation"), tr("Transaction Canceled") );
          return;
        }

        q.exec("COMMIT;");
          
        omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

        // If this is a breeder item and we receive inventory at this operation
        // then distribute the production
        if (_receiveInventory->isChecked())
        {
          q.prepare( "SELECT item_type "
               "FROM wo, itemsite, item "
               "WHERE ( (wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (wo_id=:wo_id) );" );
          q.bindValue(":wo_id", _wo->id());
          q.exec();
          if (q.first() && q.value("item_type").toString() == "B")
          {
            ParameterList params;
            params.append("mode", "new");
            params.append("wo_id", _wo->id());

            distributeBreederProduction newdlg(this, "", TRUE);
            newdlg.set(params);
            newdlg.exec();
          }
          else if (q.lastError().type() != QSqlError::None)
          {
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
            return;
          }
        } // receiveInventory
      } // qty to receive
      else
        q.exec("COMMIT;");
        
      if (_closeWO->isChecked())
      {
        ParameterList params;
        params.append("wo_id", _wo->id());

        closeWo newdlg(this, "", TRUE);
        newdlg.set(params);
        newdlg.exec();
      }
    } // else postOperation succeeded
  } // postOperation query returned a row
  else
  {
    rollback.exec();
    systemError(this, tr("A System Error occurred posting Work Order Operation ID #%1.\n%2")
		  .arg(_wooper->id())
		  .arg(q.lastError().databaseText()),
		__FILE__, __LINE__);
    return;
  }

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

/* Old behavior -- clear operation but stay with this WO
    _wooper->setId(-1);
    _wooper->setFocus();
*/
    _wo->setId(-1);
    _wo->setFocus();
  }
}

void postOperations::sHandlePostRunTime(bool)
{
  _rntimeGroup->setEnabled(_postRntime->isChecked());
  _specifiedRntime->setEnabled(_postRntime->isChecked() && 
			       _postSpecifiedRntime->isChecked() &&
			       _privleges->check("OverrideWOTCTime"));
}


void postOperations::sHandlePostSetupTime(bool pPostTime)
{
  if (pPostTime)
  {
    _sutimeGroup->setEnabled(TRUE);
    _specifiedSutime->setEnabled(_postSpecifiedSutime->isChecked());
  }
  else
    _sutimeGroup->setEnabled(FALSE);
}

void postOperations::sSetupChanged()
{
  if (_wotcTime > 0)
  {
    if (! _privleges->check("OverrideWOTCTime") &&
        _specifiedSutime->text().toDouble() > _wotcTime)
      _specifiedSutime->setText(QString::number(_wotcTime));
    else
    {
      double runtimeDbl = _wotcTime - _specifiedSutime->text().toDouble();
      _specifiedRntime->setText(runtimeDbl > 0 ? QString::number(runtimeDbl) : "0");
    }
  }
}

void postOperations::sCatchWooperid(int pWooperid)
{
  XSqlQuery wo;
  wo.prepare("SELECT wooper_wo_id FROM wooper WHERE (wooper_id=:wooper_id);");
  wo.bindValue(":wooper_id", pWooperid);
  if(wo.exec() && wo.first())
  {
    _wo->setId(wo.value("wooper_wo_id").toInt());
    _wooper->setId(pWooperid);
    _qty->setFocus();
  }
}

