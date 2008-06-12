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

#include "bom.h"

#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QValidator>
#include <QVariant>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "bomItem.h"

BOM::BOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire()));

  _totalQtyPerCache = 0.0;
  
  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cPlanning | ItemLineEdit::cJob);
  _batchSize->setValidator(omfgThis->qtyVal());
  
  _bomitem->addColumn(tr("#"),            _seqColumn,   Qt::AlignCenter, true, "bomitem_seqnumber");
  _bomitem->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft,   true, "item_number");
  _bomitem->addColumn(tr("Description"),  -1,           Qt::AlignLeft,   true, "item_description");
  _bomitem->addColumn(tr("Issue UOM"),    _uomColumn,   Qt::AlignCenter, true, "issueuom");
  _bomitem->addColumn(tr("Issue Method"), _itemColumn,  Qt::AlignCenter, true, "issuemethod");
  _bomitem->addColumn(tr("Qty. Per"),     _qtyColumn,   Qt::AlignRight,  true, "bomitem_qtyper" );
  _bomitem->addColumn(tr("Scrap %"),      _prcntColumn, Qt::AlignRight,  true, "bomitem_scrap" );
  _bomitem->addColumn(tr("Effective"),    _dateColumn,  Qt::AlignCenter, true, "effective");
  _bomitem->addColumn(tr("Expires"),      _dateColumn,  Qt::AlignCenter, true, "expires");
  _bomitem->setDragString("bomid=");
  _bomitem->setAltDragString("itemid=");
  
  if (!_privileges->check("ViewCosts"))
  {
    _currentStdCostLit->hide();
    _currentActCostLit->hide();
    _maxCostLit->hide();
    _currentStdCost->hide();
    _currentActCost->hide();
    _maxCost->hide();
  }
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  _activate->hide();
  _revision->setMode(RevisionLineEdit::Maintain);
  _revision->setType("BOM");
}

BOM::~BOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void BOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse BOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());
   {
     param = pParams.value("revision_id", &valid);
     if (valid)
       _revision->setId(param.toInt());
   }
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_bomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_bomitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
      connect(_bomitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
      connect(_bomitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
      connect(_bomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_bomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    
    if (param.toString() == "new")
    {
      _mode = cNew;
      _item->setFocus();
	  _revision->setId(-1);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _item->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _revision->setEnabled(FALSE);
      _revisionDate->setEnabled(FALSE);
      _batchSize->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _expire->setEnabled(FALSE);
      _moveUp->setEnabled(FALSE);
      _moveDown->setEnabled(FALSE);
      _doRequireQtyPer->setEnabled(FALSE);
      _requiredQtyPer->setEnabled(FALSE);
      _save->setEnabled(FALSE);
      
      connect(_bomitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
      
      _close->setFocus();
    }
  }
  
  return NoError;
}

void BOM::sSave()
{
  if(!sCheckRequiredQtyPer())
    return;
  
  q.prepare( "SELECT bomhead_id "
             "FROM bomhead "
             "WHERE ((bomhead_item_id=:item_id) "
			 "AND (bomhead_rev_id=:bomhead_rev_id));" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":bomhead_rev_id", _revision->id());
  q.exec();
  if (q.first())
  {   
    q.prepare( "UPDATE bomhead "
               "SET bomhead_docnum=:bomhead_docnum,"
               "    bomhead_revision=:bomhead_revision, bomhead_revisiondate=:bomhead_revisiondate,"
               "    bomhead_batchsize=:bomhead_batchsize,"
               "    bomhead_requiredqtyper=:bomhead_requiredqtyper "
               "WHERE ((bomhead_item_id=:bomhead_item_id) "
			   "AND (bomhead_rev_id=:bomhead_rev_id));" );
    q.bindValue(":bomhead_item_id", _item->id());
    q.bindValue(":bomhead_rev_id", _revision->id());
  }
  else
  {
    q.prepare( "INSERT INTO bomhead "
               "( bomhead_item_id, bomhead_docnum,"
               "  bomhead_revision, bomhead_revisiondate,"
               "  bomhead_batchsize, bomhead_requiredqtyper ) "
               "VALUES "
               "( :bomhead_item_id, :bomhead_docnum,"
               "  :bomhead_revision, :bomhead_revisiondate, "
               "  :bomhead_batchsize, :bomhead_requiredqtyper ) " );
    q.bindValue(":bomhead_item_id", _item->id());
  }
  
  q.bindValue(":bomhead_docnum", _documentNum->text());
  q.bindValue(":bomhead_revision", _revision->number());
  q.bindValue(":bomhead_revisiondate", _revisionDate->date());
  q.bindValue(":bomhead_batchsize", _batchSize->toDouble());
  if(_doRequireQtyPer->isChecked())
    q.bindValue(":bomhead_requiredqtyper", _requiredQtyPer->text().toDouble());
  q.exec();
  
  close();
}

bool BOM::setParams(ParameterList &pParams)
{
  pParams.append("item_id",     _item->id());
  pParams.append("revision_id", _revision->id());
  pParams.append("push",        tr("Push"));
  pParams.append("pull",        tr("Pull"));
  pParams.append("mixed",       tr("Mixed"));
  pParams.append("error",       tr("Error"));
  pParams.append("always",      tr("Always"));
  pParams.append("never",       tr("Never"));

  if (_showExpired->isChecked())
  {
    pParams.append("showExpired");
    pParams.append("expiredDays", 999);
  }
  
  if (_showFuture->isChecked())
  {
    pParams.append("showFuture");
    pParams.append("futureDays", 999);
  }
  
  return true;
}

void BOM::sPrint()
{
  ParameterList params;
  setParams(params);
  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void BOM::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("View"), this, SLOT(sView()), 0);
  
  if ((_mode == cNew) || (_mode == cEdit))
  {
    menuThis->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    menuThis->insertItem(tr("Expire"), this, SLOT(sExpire()), 0);
    menuThis->insertItem(tr("Replace"), this, SLOT(sReplace()), 0);
    
    menuThis->insertSeparator();
    
    menuThis->insertItem(tr("Move Up"),   this, SLOT(sMoveUp()), 0);
    menuThis->insertItem(tr("Move Down"), this, SLOT(sMoveDown()), 0);
  }
}

void BOM::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  
  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", _bomitem->id());
  
  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bomitem->id());
  
  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sExpire()
{
  q.prepare( "UPDATE bomitem "
             "SET bomitem_expires=CURRENT_DATE "
             "WHERE (bomitem_id=:bomitem_id);" );
  q.bindValue(":bomitem_id", _bomitem->id());
  q.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), TRUE);
}

void BOM::sReplace()
{
  ParameterList params;
  params.append("mode", "replace");
  params.append("bomitem_id", _bomitem->id());
  params.append("revision_id", _revision->id());
  
  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sMoveUp()
{
  q.prepare("SELECT moveBomitemUp(:bomitem_id) AS result;");
  q.bindValue(":bomitem_id", _bomitem->id());
  q.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), TRUE);
}

void BOM::sMoveDown()
{
  q.prepare("SELECT moveBomitemDown(:bomitem_id) AS result;");
  q.bindValue(":bomitem_id", _bomitem->id());
  q.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), TRUE);
}

void BOM::sFillList()
{
  sFillList(_item->id(), TRUE);
}

void BOM::sFillList(int pItemid, bool)
{
  if (_item->isValid() && (pItemid == _item->id()))
  {
    q.prepare( "SELECT bomhead_id, bomhead_docnum, bomhead_rev_id,"
               "       bomhead_revision, bomhead_revisiondate,"
               "       formatQty(bomhead_batchsize) AS f_batchsize,"
               "       bomhead_requiredqtyper "
               "FROM bomhead "
               "WHERE ( (bomhead_item_id=:item_id) "
			   "AND (bomhead_rev_id=:revision_id) );" );
    q.bindValue(":item_id", _item->id());
	q.bindValue(":revision_id", _revision->id());
    q.exec();
    if (q.first())
    {
      _documentNum->setText(q.value("bomhead_docnum"));
	  _revision->setNumber(q.value("bomhead_revision").toString());
      _revisionDate->setDate(q.value("bomhead_revisiondate").toDate());
      _batchSize->setText(q.value("f_batchsize"));
      if(q.value("bomhead_requiredqtyper").toDouble()!=0)
      {
        _doRequireQtyPer->setChecked(true);
        _requiredQtyPer->setText(q.value("bomhead_requiredqtyper").toString());
      }
      if (_revision->description() == "Inactive")
	  {
		  _save->setEnabled(FALSE);
	      _new->setEnabled(FALSE);
		  _documentNum->setEnabled(FALSE);
		  _revisionDate->setEnabled(FALSE);
		  _batchSize->setEnabled(FALSE);
		  _bomitem->setEnabled(FALSE);
	  }

	  if ((_revision->description() == "Pending") || (_revision->description() == "Active"))
	  {
		  _save->setEnabled(TRUE);
	      _new->setEnabled(TRUE);
		  _documentNum->setEnabled(TRUE);
		  _revisionDate->setEnabled(TRUE);
		  _batchSize->setEnabled(TRUE);
		  _bomitem->setEnabled(TRUE);
	  }
    }
    else
    {
      _documentNum->clear();
      _revisionDate->clear();
      _batchSize->clear();
    }
    
    QString sql( "SELECT bomitem_id, item_id, *,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS item_description,"
                 "       uom_name AS issueuom,"
                 "       CASE WHEN (bomitem_issuemethod = 'S') THEN <? value(\"push\") ?>"
                 "            WHEN (bomitem_issuemethod = 'L') THEN <? value(\"pull\") ?>"
                 "            WHEN (bomitem_issuemethod = 'M') THEN <? value(\"mixed\") ?>"
                 "            ELSE <? value(\"error\") ?>"
                 "       END AS issuemethod,"
                 "       'qtyper' AS bomitem_qtyper_xtnumericrole,"
                 "       'percent' AS bomitem_scrap_xtnumericrole,"
                 "       CASE WHEN (bomitem_effective = startOfTime()) THEN NULL "
                 "            ELSE bomitem_effective END AS effective,"
                 "       CASE WHEN (bomitem_expires = endOfTime()) THEN NULL "
                 "            ELSE bomitem_expires END AS expires,"
                 "       <? value(\"always\") ?> AS effective_xtnullrole,"
                 "       <? value(\"never\") ?>  AS expires_xtnullrole,"
                 "       CASE WHEN (bomitem_configtype<>'N') THEN 'emphasis'"
                 "            WHEN (bomitem_expires < CURRENT_DATE) THEN 'expired'"
                 "            WHEN (bomitem_effective >= CURRENT_DATE) THEN 'future'"
                 "       END AS qtforegroundrole "
                 "FROM bomitem(<? value(\"item_id\") ?>,"
                 "             <? value(\"revision_id\") ?>), item, uom "
                 "WHERE ((bomitem_item_id=item_id)"
                 " AND (bomitem_uom_id=uom_id)"
                 "<? if not exists(\"showExpired\") ?>"
                 " AND (bomitem_expires > CURRENT_DATE)"
                 "<? endif ?>"
                 "<? if not exists(\"showFuture\") ?>"
                 " AND (bomitem_effective <= CURRENT_DATE)"
                 "<? endif ?>"
                 ") "
                 "ORDER BY bomitem_seqnumber, bomitem_effective;"
                 );
    ParameterList params;
    setParams(params);
    MetaSQLQuery mql(sql);
    q = mql.toQuery(params);
    
    _bomitem->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    sql = "SELECT item_picklist,"
          "       COUNT(*) AS total,"
          "       COALESCE(SUM(bomitem_qtyper * (1 + bomitem_scrap))) AS qtyper "
          "FROM bomitem(<? value(\"item_id\") ?>,"
          "             <? value(\"revision_id\") ?>), item "
          "WHERE ( (bomitem_item_id=item_id)"
          "<? if not exists(\"showExpired\") ?>"
          " AND (bomitem_expires > CURRENT_DATE)"
          "<? endif ?>"
          "<? if not exists(\"showFuture\") ?>"
          " AND (bomitem_effective <= CURRENT_DATE)"
          "<? endif ?>"
          " ) "
          "GROUP BY item_picklist;";
    MetaSQLQuery picklistmql(sql);
    q = picklistmql.toQuery(params);
    
    bool   foundPick    = FALSE;
    bool   foundNonPick = FALSE;
    int    totalNumber  = 0;
    double totalQtyPer  = 0.0;
    while (q.next())
    {
      totalNumber += q.value("total").toInt();
      totalQtyPer += q.value("qtyper").toDouble();
      
      if (q.value("item_picklist").toBool())
      {
        foundPick = TRUE;
        _pickNumber->setText(q.value("total").toString());
        _pickQtyPer->setText(formatQtyPer(q.value("qtyper").toDouble()));
      }
      else
      {
        foundNonPick = TRUE;
        _nonPickNumber->setText(q.value("total").toString());
        _nonPickQtyPer->setText(formatQtyPer(q.value("qtyper").toDouble()));
      }
    }
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    
    if (!foundPick)
    {
      _pickNumber->setText("0");
      _pickQtyPer->setText(formatQty(0.0));
    }
    
    if (!foundNonPick)
    {
      _nonPickNumber->setText("0");
      _nonPickQtyPer->setText(formatQtyPer(0.0));
    }
    
    _totalNumber->setText(QString("%1").arg(totalNumber));
    _totalQtyPer->setText(formatQtyPer(totalQtyPer));
    _totalQtyPerCache = totalQtyPer;
    
    if (_privileges->check("ViewCosts"))
    {
      sql = "SELECT formatCost(p.item_maxcost) AS f_maxcost,"
            "       formatCost(COALESCE(SUM(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * stdCost(c.item_id)))) AS f_stdcost,"
            "       formatCost(COALESCE(SUM(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * ROUND(actCost(c.item_id),4)))) AS f_actcost "
            "FROM bomitem(<? value(\"item_id\") ?>,"
            "             <? value(\"revision_id\") ?>), item AS c, item AS p "
            "WHERE ( (bomitem_item_id=c.item_id)"
            " AND (p.item_id=<? value(\"item_id\") ?>)"
            "<? if not exists(\"showExpired\") ?>"
            " AND (bomitem_expires > CURRENT_DATE)"
            "<? endif ?>"
            "<? if not exists(\"showFuture\") ?>"
            " AND (bomitem_effective <= CURRENT_DATE)"
            "<? endif ?>"
            " ) "
            "GROUP BY p.item_maxcost;";
      MetaSQLQuery costsmql(sql);
      q = costsmql.toQuery(params);
      if (q.first())
      {
        _currentStdCost->setText(q.value("f_stdcost").toString());
        _currentActCost->setText(q.value("f_actcost").toString());
        _maxCost->setText(q.value("f_maxcost").toString());
      }
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
  }
  else if (!_item->isValid())
  {
    _documentNum->clear();
    _revision->clear();
    _revisionDate->clear();
    _batchSize->clear();
    
    _bomitem->clear();
  }
}

void BOM::keyPressEvent( QKeyEvent * e )
{
#ifdef Q_WS_MAC
  if(e->key() == Qt::Key_S && e->state() == Qt::ControlModifier)
  {
    _save->animateClick();
    e->accept();
  }
  if(e->isAccepted())
    return;
#endif
  e->ignore();
}

void BOM::sClose()
{
  if(sCheckRequiredQtyPer())
    close();
}

bool BOM::sCheckRequiredQtyPer()
{
  if(cView == _mode || !_doRequireQtyPer->isChecked())
    return true;

  if(_requiredQtyPer->text().toDouble() != _totalQtyPerCache)
  {
    QMessageBox::warning( this, tr("Total Qty. Per Required"),
      tr("A required total Qty. Per was specified but not met.\n "
         "Please correct the problem before continuing.") );
    return false;
  }
  
  return true;
}
