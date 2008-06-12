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

/*
TODO:	make deleted line items more obvious
TODO:	return to the editor if validation fails

TODO:	refactor:
	move some of the logic from PoitemTableDelegate::setModelData()
		into poitemTableModel
	move PoitemTableDelegate into a separate file and split into
		PoitemDelegate and subclass it to PoitemTableDelegate
	rewrite purchaseOrderItem to use PoitemDelegate

*/

#include "poitemTableView.h"

#include <QFont>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QString>

#include "guiclient.h"
#include "datecluster.h"
#include "expensecluster.h"
#include "itemcluster.h"
#include "poitemTableModel.h"
#include "projectcluster.h"
#include "wcombobox.h"
#include "xtreewidget.h"	// for column widths

#define QE_NONINVENTORY

#define DEBUG   false

PoitemTableView::PoitemTableView(QWidget* parent) :
  QTableView(parent)
{
  PoitemTableDelegate *delegate = new PoitemTableDelegate(this);
  setItemDelegate(delegate);
  setShowGrid(false);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

void PoitemTableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
  if (DEBUG) qDebug("PoitemTableView::closeEditor(%p, %d)", editor, hint);

  QTableView::closeEditor(editor, hint);

  if (editor->objectName() == "poitem_duedate" &&
      hint == QAbstractItemDelegate::EditNextItem)
  {
    setCurrentIndex(model()->index(model()->rowCount() - 1, ITEM_NUMBER_COL));
  }

  if (DEBUG) qDebug("PoitemTableView::closeEditor() returning with index %d, %d",
                    currentIndex().column(), currentIndex().row());
}

void PoitemTableView::setModel(QAbstractItemModel* model)
{
  if (DEBUG) qDebug("PoitemTableView::setModel(%p)", model);
  QTableView::setModel(model);

  setColumnWidth(ITEM_NUMBER_COL,		_itemColumn);
  setColumnWidth(WAREHOUS_CODE_COL,		100); //_whsColumn too small
  setColumnWidth(POITEM_QTY_ORDERED_COL,	_qtyColumn);
  setColumnWidth(POITEM_UNITPRICE_COL,		_priceColumn);
  setColumnWidth(EXTPRICE_COL,			_moneyColumn);
  setColumnWidth(POITEM_FREIGHT_COL,		_priceColumn);
  setColumnWidth(POITEM_DUEDATE_COL,		_dateColumn);
#ifdef QE_NONINVENTORY
  setColumnWidth(EXPCAT_CODE_COL,		100);
#endif
  setColumnWidth(POITEM_VEND_ITEM_NUMBER_COL,	_itemColumn);

  QHeaderView *header = horizontalHeader();

  int dest = 0;
  header->moveSection(header->visualIndex(ITEM_NUMBER_COL),	 	dest++);
  header->moveSection(header->visualIndex(WAREHOUS_CODE_COL),		dest++);
  header->moveSection(header->visualIndex(POITEM_VEND_ITEM_NUMBER_COL),	dest++);
#ifdef QE_NONINVENTORY
  header->moveSection(header->visualIndex(EXPCAT_CODE_COL),		dest++);
#endif
  header->moveSection(header->visualIndex(POITEM_QTY_ORDERED_COL),	dest++);
  header->moveSection(header->visualIndex(POITEM_UNITPRICE_COL),	dest++);
  header->moveSection(header->visualIndex(EXTPRICE_COL),		dest++);
  header->moveSection(header->visualIndex(POITEM_FREIGHT_COL),		dest++);
  header->moveSection(header->visualIndex(POITEM_DUEDATE_COL),		dest++);

  // if we didn't explicitly place the logical section, hide it
  for (int i = dest; i < header->count(); i++)
    header->hideSection(header->logicalIndex(i));

  //header->setStretchLastSection(true);
  if (DEBUG) qDebug("PoitemTableView::setModel returning");
}

void PoitemTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous ) 
{
  if (DEBUG) qDebug("PoitemTableView::currentChanged(current %d,%d prev %d,%d)",
                    current.row(),  current.column(),
                    previous.row(), previous.column());
  if (current != QModelIndex() && current != previous)
  {
    if (DEBUG) qDebug("PoitemTableView::currentChanged setting current");
    setCurrentIndex(current);
    if (DEBUG) qDebug("PoitemTableView::currentChanged scrolling to current");
    scrollTo(current);
    if (DEBUG) qDebug("PoitemTableView::currentChanged editing current");
    edit(current);
  }
  if (DEBUG) qDebug("PoitemTableView::currentChanged returning");
}

///////////////////////////////////////////////////////////////////////////

PoitemTableDelegate::PoitemTableDelegate(QObject *parent)
  : QItemDelegate(parent)
{
}

QWidget *PoitemTableDelegate::createEditor(QWidget *parent,
					   const QStyleOptionViewItem &/*style*/,
					   const QModelIndex &index) const
{
  const QAbstractItemModel *model = index.model();
  QWidget *editor = 0;

  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = new ItemLineEdit(parent);
      item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cActive);
      if ((qobject_cast<const PoitemTableModel*>(model))->_vendrestrictpurch)
      {
	int vendid = (qobject_cast<const PoitemTableModel*>(model))->_vendid;
	// TODO: put queries in ItemLineEdit, trigger them with a setVendId()
        item->setQuery( QString("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
				"                uom_name, item_type, item_config "
				"FROM item, itemsite, itemsrc, uom  "
				"WHERE ( (itemsite_item_id=item_id)"
				" AND (itemsrc_item_id=item_id)"
                                " AND (item_inv_uom_id=uom_id)"
				" AND (itemsite_active)"
				" AND (item_active)"
				" AND (itemsrc_active)"
				" AND (itemsrc_vend_id=%1) ) "
				"ORDER BY item_number;" )
                         .arg(vendid) );
        item->setValidationQuery( QString("SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
					  "                uom_name, item_type, item_config "
					  "FROM item, itemsite, itemsrc, uom  "
					  "WHERE ( (itemsite_item_id=item_id)"
					  " AND (itemsrc_item_id=item_id)"
                                          " AND (item_inv_uom_id=uom_id)"
					  " AND (itemsite_active)"
					  " AND (item_active)"
					  " AND (itemsrc_active)"
					  " AND (itemsrc_vend_id=%1) "
					  " AND (itemsite_item_id=:item_id) ) "
					  "ORDER BY item_number;" )
				   .arg(vendid) );
      }
      else
        item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cActive);

      editor = item;
      break;
    }

    case WAREHOUS_CODE_COL:
    {
      int itemid = model->data(model->index(index.row(), ITEM_ID_COL)).toInt();
      if (itemid <= 0)	// probably non-inventory item so don't pick a whs
	break;

      WComboBox *whs = new WComboBox(parent);
      whs->setType(WComboBox::Supply);
      whs->findItemsites(itemid);
      editor = whs;
      break;
    }

    case POITEM_VEND_ITEM_NUMBER_COL:
    {
      editor = new XLineEdit(parent);
      editor->setObjectName("poitem_vend_item_number");
      break;
    }

    case POITEM_QTY_ORDERED_COL:
    {
      XLineEdit *qty = new XLineEdit(parent);
      qty->setValidator(omfgThis->qtyVal());
      editor = qty;
      break;
    }
    
    case POITEM_UNITPRICE_COL:
    case POITEM_FREIGHT_COL:
    {
      XLineEdit *price = new XLineEdit(parent);
      price->setValidator(omfgThis->priceVal());
      editor = price;
      break;
    }

    case POITEM_DUEDATE_COL:
    {
      XDateEdit *duedate = new XDateEdit(parent);
      duedate->setFocusPolicy(Qt::StrongFocus);
      editor = duedate;
      editor->setObjectName("poitem_duedate");
      break;
    }

#ifdef QE_NONINVENTORY
    case EXPCAT_CODE_COL:
    {
      ExpenseLineEdit *expcat = new ExpenseLineEdit(parent);
      editor = expcat;
      break;
    }
#endif

    default:
    {
      editor = 0;
      break;
    }
  }

  if (editor)
  {
#ifdef Q_WS_MAC
    // compensate for many custom widgets making themselves smaller on OS X
    QFont f = editor->font();
    f.setPointSize(f.pointSize() + 2);
    editor->setFont(f);
#endif
    editor->installEventFilter(const_cast<PoitemTableDelegate*>(this));
    if (DEBUG)
      qDebug("createEditor: editor has focus policy %d", editor->focusPolicy());
  }
  return editor;
}

void PoitemTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  const QAbstractItemModel *model = index.model();
  XLineEdit *lineedit = 0;
  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = static_cast<ItemLineEdit*>(editor);
      if (model->data(model->index(index.row(),
				   POITEM_ITEMSITE_ID_COL)).toInt() > 0)
	item->setItemsiteid(model->data(model->index(index.row(),
					      POITEM_ITEMSITE_ID_COL)).toInt());
      break;
    }

    case WAREHOUS_CODE_COL:
    {
      WComboBox *whs = static_cast<WComboBox*>(editor);
      whs->setId(model->data(model->index(index.row(), WAREHOUS_ID_COL)).toInt());
      break;
    }

    case POITEM_VEND_ITEM_NUMBER_COL:
      lineedit = static_cast<XLineEdit*>(editor);
      lineedit->setText(model->data(index).toString());
      break;

    case POITEM_QTY_ORDERED_COL:
    case POITEM_UNITPRICE_COL:
    case POITEM_FREIGHT_COL:
      lineedit = static_cast<XLineEdit*>(editor);
      lineedit->setDouble(model->data(index).toDouble());
      break;

    case POITEM_DUEDATE_COL:
    {
      XDateEdit *duedate = static_cast<XDateEdit*>(editor);
      duedate->setDate(index.data().toDate());
      break;
    }

#ifdef QE_NONINVENTORY
    case EXPCAT_CODE_COL:
    {
      ExpenseLineEdit *exp = static_cast<ExpenseLineEdit*>(editor);
      exp->setId(model->data(model->index(index.row(), POITEM_EXPCAT_ID_COL)).toInt());
      break;
    }
#endif

    default:
      break;
  }
}

void PoitemTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *pModel, const QModelIndex &index) const
{
  if (DEBUG)
    qDebug("PoitemTableDelegate::setModelData() entered");
  bool hitError = false;
  QVariant oldval = pModel->data(index);
  PoitemTableModel *model = static_cast<PoitemTableModel*>(pModel);

  switch (index.column())
  {
    case ITEM_NUMBER_COL:
    {
      ItemLineEdit *item = static_cast<ItemLineEdit*>(editor);
      if (item->itemNumber() != oldval.toString())
      {
	if (item->itemNumber().isEmpty())
	{
	  model->setData(index, new QVariant());
	  model->setData(model->index(index.row(), ITEM_ID_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_ITEMSITE_ID_COL), new QVariant());
	  model->setData(model->index(index.row(), WAREHOUS_ID_COL), new QVariant());
	  model->setData(model->index(index.row(), WAREHOUS_CODE_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_ITEMSRC_ID_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_VEND_ITEM_NUMBER_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_VEND_ITEM_DESCRIP_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_VEND_UOM_COL), new QVariant());
	  model->setData(model->index(index.row(), ITEMSRC_MINORDQTY_COL), new QVariant());
	  model->setData(model->index(index.row(), ITEMSRC_MULTORDQTY_COL), new QVariant());
	  model->setData(model->index(index.row(), ITEMSRC_INVVENDORUOMRATIO_COL), new QVariant());
	  model->setData(model->index(index.row(), POITEM_INVVENDUOMRATIO_COL), new QVariant());
	  model->setData(model->index(index.row(), EARLIESTDATE_COL), new QVariant());
	}
	else
	{
	  model->setData(index, item->itemNumber());
	  model->setData(model->index(index.row(), ITEM_ID_COL), item->id());

	  if (item->isValid())
	  {
	    XSqlQuery itemq;
	    itemq.prepare("SELECT (warehous_id=:preferred) AS preferred, "
			  "       itemsite_id, warehous_id, warehous_code "
			  "FROM itemsite, whsinfo "
			  "WHERE ((itemsite_item_id=:item_id)"
			  "   AND (itemsite_warehous_id=warehous_id)) "
			  "ORDER BY preferred DESC, warehous_code "
			  "LIMIT 1;");
	    itemq.bindValue(":item_id", item->id());
	    itemq.bindValue(":preferred",
			   _x_preferences->value("PreferredWarehouse").toInt());
	    itemq.exec();
	    if (itemq.first())
	    {
	      model->setData(model->index(index.row(), POITEM_ITEMSITE_ID_COL), itemq.value("itemsite_id").toInt());
	      model->setData(model->index(index.row(), WAREHOUS_ID_COL), itemq.value("warehous_id").toInt());
	      model->setData(model->index(index.row(), WAREHOUS_CODE_COL), itemq.value("warehous_code").toString());
	    }
	    else if (itemq.lastError().type() != QSqlError::None)
	    {
	      systemError(0, itemq.lastError().databaseText(), __FILE__, __LINE__);
	      hitError = true;
	      break;
	    }
	    else
	    {
	      systemError(0, QString("Could not find Item Site for %1 (%2).")
			      .arg(item->itemNumber()).arg(item->id()));
	      hitError = true;
	      break;
	    }

	    if (_metrics->boolean("RequireStdCostForPOItem"))
	    {
	      XSqlQuery stdcostq;
	      stdcostq.prepare("SELECT stdCost(:item_id) AS result");
	      stdcostq.bindValue(":item_id", item->id());
	      stdcostq.exec();
	      if(stdcostq.first() && stdcostq.value("result").toDouble() == 0.0)
	      {
		QMessageBox::critical(0, tr("Selected Item Missing Cost"),
			tr("<p>The selected item has no Std. Costing information. "
			   "Please see your controller to correct this situation "
			   "before continuing."));
		model->setData(index, new QString());
		model->setData(model->index(index.row(), POITEM_ITEMSITE_ID_COL), new QVariant());
		model->setData(model->index(index.row(), WAREHOUS_ID_COL), new QVariant());
		model->setData(model->index(index.row(), WAREHOUS_CODE_COL), new QString());
		model->setData(index, new QString());
		hitError = true;
		break;
	      }
	      else if (stdcostq.lastError().type() != QSqlError::None)
	      {
		systemError(0, stdcostq.lastError().databaseText(), __FILE__, __LINE__);
		hitError = true;
		break;
	      }
	    }

	    XSqlQuery itemsrcq;
	    itemsrcq.prepare( "SELECT itemsrc_id, itemsrc_vend_item_number,"
		       "       itemsrc_vend_item_descrip, itemsrc_vend_uom,"
		       "       itemsrc_minordqty,"
		       "       itemsrc_multordqty,"
		       "       itemsrc_invvendoruomratio,"
		       "       (CURRENT_DATE + itemsrc_leadtime) AS earliestdate "
		       "FROM pohead, itemsrc "
		       "WHERE ( (itemsrc_vend_id=pohead_vend_id)"
		       " AND (itemsrc_item_id=:item_id)"
		       " AND (pohead_id=:pohead_id) );" );
	    itemsrcq.bindValue(":item_id", item->id());
	    itemsrcq.bindValue(":pohead_id", model->headId());
	    itemsrcq.exec();
	    if (itemsrcq.first())
	    {
	      model->setData(model->index(index.row(), POITEM_ITEMSRC_ID_COL), itemsrcq.value("itemsrc_id").toInt());
	      model->setData(model->index(index.row(), POITEM_VEND_ITEM_NUMBER_COL), itemsrcq.value("itemsrc_vend_item_number").toString());
	      model->setData(model->index(index.row(), POITEM_VEND_ITEM_DESCRIP_COL), itemsrcq.value("itemsrc_vend_item_descrip").toString());
	      model->setData(model->index(index.row(), POITEM_VEND_UOM_COL), itemsrcq.value("itemsrc_vend_uom").toString());
	      model->setData(model->index(index.row(), ITEMSRC_MINORDQTY_COL), itemsrcq.value("itemsrc_minordqty").toDouble());
	      model->setData(model->index(index.row(), ITEMSRC_MULTORDQTY_COL), itemsrcq.value("itemsrc_multordqty").toDouble());
	      model->setData(model->index(index.row(), ITEMSRC_INVVENDORUOMRATIO_COL), itemsrcq.value("itemsrc_invvendoruomratio").toDouble());
	      model->setData(model->index(index.row(), POITEM_INVVENDUOMRATIO_COL), itemsrcq.value("itemsrc_invvendoruomratio").toDouble());
	      model->setData(model->index(index.row(), EARLIESTDATE_COL), itemsrcq.value("earliestdate").toDate());

	      if (_metrics->boolean("UseEarliestAvailDateOnPOItem"))
		model->setData(model->index(index.row(), POITEM_DUEDATE_COL), itemsrcq.value("earliestdate").toDate());
	    }
	    else if (itemsrcq.lastError().type() != QSqlError::None)
	    {
	      systemError(0, itemsrcq.lastError().databaseText(), __FILE__, __LINE__);
	      hitError = true;
	      break;
	    }
	  }
	}
      }
      break;
    }

    case WAREHOUS_CODE_COL:
    {
      WComboBox *whs = static_cast<WComboBox*>(editor);
      if (whs->id() < 0)
      {
	model->setData(model->index(index.row(), WAREHOUS_ID_COL), QVariant());
	model->setData(index, QVariant());
      }
      else if (whs->id() != oldval.toInt())
      {
	model->setData(model->index(index.row(), WAREHOUS_ID_COL), whs->id());
	model->setData(index, whs->currentText());

	if (whs->isValid())
	{
	  XSqlQuery itemq;
	  itemq.prepare("SELECT itemsite_id "
			"FROM itemsite "
			"WHERE ((itemsite_item_id=:item_id)"
			"   AND (itemsite_warehous_id=:whs_id)) "
			"LIMIT 1;");
	  itemq.bindValue(":item_id", model->index(index.row(), ITEM_ID_COL).data());
	  itemq.bindValue(":whs_id",  whs->id());
	  itemq.exec();
	  if (itemq.first())
	  {
	    model->setData(model->index(index.row(), POITEM_ITEMSITE_ID_COL), itemq.value("itemsite_id").toInt());
	  }
	  else if (itemq.lastError().type() != QSqlError::None)
	  {
	    systemError(0, itemq.lastError().databaseText(), __FILE__, __LINE__);
	    hitError = true;
	    break;
	  }
	  else
	  {
	    systemError(0, QString("Could not find Item Site for %1 in %2.")
			    .arg(model->index(index.row(), ITEM_NUMBER_COL).data().toString())
			    .arg(whs->currentText()));
	    hitError = true;
	    break;
	  }
	}
      }
      break;
    }

    case POITEM_VEND_ITEM_NUMBER_COL:
    {
      XLineEdit *lineedit = static_cast<XLineEdit*>(editor);
      if (lineedit->text() != oldval.toString())
	model->setData(index, lineedit->text());
      break;
    }
    
    case POITEM_FREIGHT_COL:
    {
      XLineEdit *lineedit = static_cast<XLineEdit*>(editor);
      if (lineedit->toDouble() != oldval.toDouble())
	model->setData(index, lineedit->toDouble());
      break;
    }


    case POITEM_QTY_ORDERED_COL:
    {
      XLineEdit *lineedit = static_cast<XLineEdit*>(editor);
      if (lineedit->text().isEmpty())
      {
	model->setData(index, QVariant());
	model->setData(model->index(index.row(), EXTPRICE_COL), QVariant());
	break;
      }
      double qty = lineedit->toDouble();
      if (qty != oldval.toDouble())
      {
	model->setData(index, qty);
	if (model->data(model->index(index.row(), POITEM_QTY_ORDERED_COL)).toDouble() > 0 &&
	  model->data(model->index(index.row(), POITEM_ITEMSRC_ID_COL)).toInt() > 0)
	{
	  q.prepare( "SELECT ROUND(currToCurr(itemsrcp_curr_id, :curr_id, itemsrcp_price, :effective), :prec) "
		      "AS new_itemsrcp_price "
		     "FROM itemsrcp "
		     "WHERE ( (itemsrcp_itemsrc_id=:itemsrc_id)"
		     " AND (itemsrcp_qtybreak <= :qty) ) "
		     "ORDER BY itemsrcp_qtybreak DESC "
		     "LIMIT 1;" );
	  q.bindValue(":itemsrc_id", model->data(model->index(index.row(), POITEM_ITEMSRC_ID_COL)).toInt());
	  q.bindValue(":qty", model->data(model->index(index.row(), POITEM_QTY_ORDERED_COL)).toDouble());
	  q.bindValue(":curr_id", model->currId());
	  q.bindValue(":effective", model->transDate().toString());
	  q.bindValue(":prec", omfgThis->priceVal()->decimals());
	  q.exec();
	  if (q.first())
	  {
	    model->setData(model->index(index.row(), POITEM_UNITPRICE_COL), q.value("new_itemsrcp_price").toDouble());
	  }
	  else if (q.lastError().type() != QSqlError::None)
	  {
	    systemError(0, q.lastError().databaseText(), __FILE__, __LINE__);
	    hitError = true;
	    break;
	  }
	  else
	    model->setData(model->index(index.row(), POITEM_UNITPRICE_COL), 0);
	}

	double prc = model->data(model->index(index.row(), POITEM_UNITPRICE_COL)).toDouble();
	model->setData(model->index(index.row(), EXTPRICE_COL), (qty * prc));
      }
      break;
    }
    
    case POITEM_UNITPRICE_COL:
    {
      XLineEdit *lineedit = static_cast<XLineEdit*>(editor);
      if (lineedit->text().isEmpty())
      {
	model->setData(index, QVariant());
	model->setData(model->index(index.row(), EXTPRICE_COL), QVariant());
	break;
      }
      double prc = lineedit->text().toDouble();
      if (prc != oldval.toDouble())
      {
	model->setData(index, prc);
	double qty = model->data(model->index(index.row(),POITEM_QTY_ORDERED_COL)).toDouble();
	model->setData(model->index(index.row(), EXTPRICE_COL), (qty * prc));
      }
      break;
    }

    case POITEM_DUEDATE_COL:
    {
      XDateEdit *duedate = static_cast<XDateEdit*>(editor);
      duedate->parseDate();
      if (duedate->date() != oldval.toDate())
      {
        if (DEBUG)
          qDebug("PoitemTableDelegate::setModelData() setting duedate to %s with null %d, valid %d", 
                 qPrintable(duedate->date().toString()), duedate->isNull(),
                 duedate->isValid());
	model->setData(index, duedate->date());
      }

      break;
    }

#ifdef QE_NONINVENTORY
    case EXPCAT_CODE_COL:
    {
      ExpenseLineEdit *exp = static_cast<ExpenseLineEdit*>(editor);
      if (exp->id() != oldval.toInt())
      {
	model->setData(model->index(index.row(), POITEM_EXPCAT_ID_COL), exp->id());
	model->setData(index, exp->text());
      }
      break;
    }
#endif

    default:
      break;
  }

  QTableView *view = qobject_cast<QTableView*>(parent());
  if (view)
  {
    if (hitError)
      view->setCurrentIndex(index);
    else if (index.row() >= (model->rowCount() - 1))
    {
      QHeaderView* header = view->horizontalHeader();
      if (header->visualIndex(index.column()) >=
	  (header->count() - header->hiddenSectionCount() - 1))
      {
	model->insertRow(model->rowCount());
      }
    }
  }

  return;
}

void PoitemTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
  editor->setGeometry(option.rect);
}
