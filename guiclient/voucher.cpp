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

#include "voucher.h"

#include <QCloseEvent>
#include <Q3PopupMenu>
#include <QKeyEvent>
#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <QStatusBar>
#include <QSqlError>
#include "purchaseOrderList.h"
#include "purchaseOrderItem.h"
#include "voucherItem.h"
#include "voucherMiscDistrib.h"
/*
 *  Constructs a voucher as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
voucher::voucher(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_poList, SIGNAL(clicked()), this, SLOT(sPoList()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_distributions, SIGNAL(clicked()), this, SLOT(sDistributions()));
  connect(_distributeline, SIGNAL(clicked()), this, SLOT(sDistributeLine()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_distributeall, SIGNAL(clicked()), this, SLOT(sDistributeAll()));
  connect(_voucherNumber, SIGNAL(lostFocus()), this, SLOT(sHandleVoucherNumber()));
  connect(_poNumber, SIGNAL(vendAddress1Changed(const QString&)), _vendAddress1, SLOT(setText(const QString&)));
  connect(_poNumber, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(itemSelected(int)), _distributions, SLOT(animateClick()));
  connect(_poitem, SIGNAL(valid(bool)), _distributions, SLOT(setEnabled(bool)));
  connect(_amountToDistribute, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));
  connect(_miscDistrib, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNewMiscDistribution()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEditMiscDistribution()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDeleteMiscDistribution()));
  connect(_miscDistrib, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_miscDistrib, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_invoiceDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDistDate()));
  connect(_poNumber, SIGNAL(vendAddress2Changed(const QString&)), _vendAddress2, SLOT(setText(const QString&)));
  connect(_invoiceDate, SIGNAL(newDate(const QDate&)), this, SLOT(sPopulateDueDate()));
  connect(_poNumber, SIGNAL(newId(int)), this, SLOT(sPopulatePoInfo()));
  connect(_poNumber, SIGNAL(vendNameChanged(const QString&)), _vendName, SLOT(setText(const QString&)));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_amountToDistribute, SIGNAL(idChanged(int)), _amountDistributed, SLOT(setId(int)));
  connect(_amountToDistribute, SIGNAL(idChanged(int)), _balance, SLOT(setId(int)));
  connect(_amountToDistribute, SIGNAL(idChanged(int)), this, SLOT(sFillList()));
  connect(_distributionDate, SIGNAL(newDate(const QDate&)), _amountToDistribute, SLOT(setEffective(const QDate&)));
  connect(_amountToDistribute, SIGNAL(effectiveChanged(const QDate&)), _amountDistributed, SLOT(setEffective(const QDate&)));
  connect(_amountToDistribute, SIGNAL(effectiveChanged(const QDate&)), _balance, SLOT(setEffective(const QDate&)));
  connect(_amountDistributed, SIGNAL(valueChanged()), this, SLOT(sPopulateBalanceDue()));

  statusBar()->hide();

#ifndef Q_WS_MAC
  _poList->setMaximumWidth(25);
#endif

  _terms->setType(XComboBox::APTerms);
  _poNumber->setType(cPOOpen);

  _poitem->addColumn(tr("#"),            _whsColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Status"),       _uomColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft   );
  _poitem->addColumn(tr("UOM"),          _uomColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Vend. Item #"), -1,           Qt::AlignLeft   );
  _poitem->addColumn(tr("UOM"),          _uomColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Ordered"),      _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn(tr("Invoiced"),     _qtyColumn,   Qt::AlignRight, false );
  _poitem->addColumn(tr("Uninvoiced"),   _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn(tr("Rejected"),     _qtyColumn,   Qt::AlignRight  );
  _poitem->addColumn(tr("Amount"),       _moneyColumn, Qt::AlignRight  );
  _poitem->addColumn(tr("PO Unit Price"), _moneyColumn, Qt::AlignRight );
  _poitem->addColumn(tr("PO Ext Price"), _moneyColumn, Qt::AlignRight  );
  _poitem->addColumn(tr("PO Line Freight"), _moneyColumn, Qt::AlignRight );

  _miscDistrib->addColumn(tr("Account"), -1,           Qt::AlignLeft  );
  _miscDistrib->addColumn(tr("Amount"),  _moneyColumn, Qt::AlignRight ); 
}

/*
 *  Destroys the object and frees any allocated resources
 */
voucher::~voucher()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void voucher::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucher::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('vohead_vohead_id_seq') AS vohead_id");
      if (q.first())
        _voheadid = q.value("vohead_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      if (_metrics->value("VoucherNumberGeneration") == "A")
      {
        populateNumber();
        _poNumber->setFocus();
      }
      else
        _voucherNumber->setFocus();

      q.prepare("INSERT INTO vohead (vohead_id,   vohead_number, vohead_posted)"
		"            VALUES (:vohead_id, :vohead_number, false);" );
      q.bindValue(":vohead_id",     _voheadid);
      q.bindValue(":vohead_number", _voucherNumber->text());
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _poList->hide();

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _poNumber->setType(0); // allow any potype when viewing
 
      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _poList->hide();
      _amountToDistribute->setEnabled(FALSE);
      _distributionDate->setEnabled(FALSE);
      _invoiceDate->setEnabled(FALSE);
      _dueDate->setEnabled(FALSE);
      _invoiceNum->setEnabled(FALSE);
      _reference->setEnabled(FALSE);
      _poitem->setEnabled(FALSE);
      _distributions->setEnabled(FALSE);
      _miscDistrib->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _terms->setEnabled(FALSE);
      _flagFor1099->setEnabled(FALSE);
      _distributeall->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
      disconnect(_poNumber, SIGNAL(valid(bool)), _distributeall, SLOT(setEnabled(bool)));
    }
  }

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    populate();
  }

  return NoError;
}

void voucher::sSave()
{
  if (_poNumber->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("You must enter an PO Number before you may save this Voucher.") );
    _poNumber->setFocus();
    return;
  }

  if (!_invoiceDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("You must enter an Invoice Date before you may save this Voucher.") );
    _invoiceDate->setFocus();
    return;
  }

  if (!_dueDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("You must enter a Due Date before you may save this Voucher.") );
    _dueDate->setFocus();
    return;
  }

  if (!_distributionDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("You must enter a Distribution Date before you may save this Voucher.") );
    _distributionDate->setFocus();
    return;
  }

  if (_invoiceNum->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Voucher"),
                           tr("You must enter a Vendor Invoice Number before you may save this Voucher.") );
    _invoiceNum->setFocus();
    return;
  }
  else
  {
    q.prepare( "SELECT vohead_id "
               "FROM vohead, pohead "
               "WHERE ( (vohead_pohead_id=pohead_id)"
               " AND (vohead_invcnumber=:vohead_invcnumber)"
               " AND (pohead_vend_id=:vend_id)"
               " AND (vohead_id<>:vohead_id) );" );
    q.bindValue(":vohead_invcnumber", _invoiceNum->text().stripWhiteSpace());
    q.bindValue(":vend_id", _poNumber->vendId());
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      if (QMessageBox::question( this, windowTitle(),
                             tr( "A Voucher for this Vendor has already been entered with the same Vendor Invoice Number. "
                                 "Are you sure you want to use this number again?" ),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
      {
        _invoiceNum->setFocus();
        return;
      }
    }
  }

  q.prepare( "UPDATE vohead "
	     "SET vohead_pohead_id=:vohead_pohead_id,"
	     "    vohead_vend_id=:vohead_vend_id,"
	     "    vohead_terms_id=:vohead_terms_id,"
	     "    vohead_distdate=:vohead_distdate,"
	     "    vohead_docdate=:vohead_docdate,"
	     "    vohead_duedate=:vohead_duedate,"
	     "    vohead_invcnumber=:vohead_invcnumber,"
	     "    vohead_reference=:vohead_reference,"
	     "    vohead_amount=:vohead_amount,"
	     "    vohead_1099=:vohead_1099, "
	     "    vohead_curr_id=:vohead_curr_id "
	     "WHERE (vohead_id=:vohead_id);" );

  q.bindValue(":vohead_id", _voheadid);
  q.bindValue(":vohead_pohead_id", _poNumber->id());
  q.bindValue(":vohead_vend_id", _poNumber->vendId());
  q.bindValue(":vohead_terms_id", _terms->id());
  q.bindValue(":vohead_distdate", _distributionDate->date());
  q.bindValue(":vohead_docdate", _invoiceDate->date());
  q.bindValue(":vohead_duedate", _dueDate->date());
  q.bindValue(":vohead_invcnumber", _invoiceNum->text().stripWhiteSpace());
  q.bindValue(":vohead_reference", _reference->text().stripWhiteSpace());
  q.bindValue(":vohead_amount", _amountToDistribute->localValue());
  q.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked(), 0));
  q.bindValue(":vohead_curr_id", _amountToDistribute->id());
  q.exec();

  omfgThis->sVouchersUpdated();

  _voheadid = -1;

  if (cNew != _mode)
  {
    close();
    return;
  }

  _poNumber->setId(-1);
  _amountToDistribute->clear();
  _amountDistributed->clear();
  _balance->clear();
  _flagFor1099->setChecked(false);
  _invoiceDate->setNull();
  _distributionDate->setNull();
  _dueDate->setNull();
  _invoiceNum->clear();
  _reference->clear();
  _poitem->clear();
  _miscDistrib->clear();

  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void voucher::sHandleVoucherNumber()
{
  if (_voucherNumber->text().length() == 0)
  {
    if ((_metrics->value("VoucherNumberGeneration") == "A") || (_metrics->value("VoucherNumberGeneration") == "O"))
      populateNumber();
    else
    {
      QMessageBox::critical( this, tr("Enter Voucher Number"),
                             tr("You must enter a valid Voucher Number before continuing") );

      _voucherNumber->setFocus();
      return;
    }
  }
  else
  {
    q.prepare( "SELECT vohead_id "
               "FROM vohead "
               "WHERE (vohead_number=:vohead_number);" );
    q.bindValue(":vohead_number", _voucherNumber->text());
    q.exec();
    if (q.first())
    {
      _voheadid = q.value("vohead_id").toInt();

      _voucherNumber->setEnabled(FALSE);
      _poNumber->setEnabled(FALSE);
      _poList->hide();

      _mode = cEdit;
      populate();

      return;
    }
  }
}

void voucher::sPoList()
{
  ParameterList params;
  params.append("pohead_id", _poNumber->id());
  params.append("poType", cPOOpen);
  
  purchaseOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _poNumber->setId(newdlg.exec());
}

void voucher::sPopulate()
{
  setCaption(tr("Voucher for P/O #") + _poNumber->text());
}

void voucher::sDistributions()
{
  QList<QTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
      ParameterList params;
      params.append("mode", "new");
      params.append("vohead_id", _voheadid);
      params.append("poitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
      params.append("curr_id", _amountToDistribute->id());
      params.append("effective", _amountToDistribute->effective());

      voucherItem newdlg(this, "", TRUE);
      newdlg.set(params);
      if (newdlg.exec() == XDialog::Rejected)
      {
        // nothing to do:
        // voucherItem wraps itself in a transaction and rolls back on reject
    }
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sDistributeLine()
{
  QList<QTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
        QString sql ( "SELECT distributevoucherline(:vohead_id,:poitem_id,:curr_id) AS result" );
        q.prepare(sql);
        q.bindValue(":vohead_id", _voheadid);
        q.bindValue(":poitem_id",  ((XTreeWidgetItem*)(selected[i]))->id());
        q.bindValue(":curr_id", _amountToDistribute->id());
        q.exec();


        if (q.first())
        {
                if (q.value("result").toInt() == -2)
                QMessageBox::warning( this, tr("Zero Amount"),
                              tr( "<p>Distribution would result in zero quantity and amount.\n"
                                  "Please distribute manually." ) );
                else if (q.value("result").toInt() == -3)
                QMessageBox::warning( this, tr("Different Currencies"),
                              tr( "<p>The purchase order and voucher have different currencies.\n"
                                  "Please distribute manually." ) );
                else if (q.value("result").toInt() == -4)
                QMessageBox::warning( this, tr("Negative Amount"),
                              tr( "<p>Distribution would result in a negative amount.\n"
                                  "Please distribute manually." ) );
                else if (q.value("result").toInt() == -5)
                QMessageBox::warning( this, tr("Multiple Cost Elements"),
                              tr( "<p>Item has multiple cost elements.\n"
                                  "Please distribute manually." ) );
        }
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sClear()
{
    QList<QTreeWidgetItem*> selected = _poitem->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
        QString sql( "DELETE FROM vodist "
             "  WHERE ((vodist_poitem_id=:poitem_id) "
             "  AND (vodist_vohead_id=:vohead_id)); "

             "  UPDATE recv SET recv_vohead_id=NULL, recv_voitem_id=NULL "
             "  WHERE ((recv_vohead_id=:vohead_id) "
             "  AND (recv_order_type='PO') "
             "  AND (recv_orderitem_id=:poitem_id)); "

             "  DELETE FROM voitem "
             "  WHERE ((voitem_poitem_id=:poitem_id) "
             "  AND (voitem_vohead_id=:vohead_id)); "

             "  UPDATE poreject SET poreject_vohead_id=NULL, poreject_voitem_id=NULL "
             "  WHERE ((poreject_vohead_id=:vohead_id) "
             "  AND (poreject_poitem_id=:poitem_id)); " );
        q.prepare(sql);
        q.bindValue(":vohead_id", _voheadid);
        q.bindValue(":poitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();

        if (q.lastError().type() != QSqlError::NoError)
        {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
        }
     }
  sFillList();
  sPopulateDistributed();
}

void voucher::sDistributeAll()
{
  _poitem->selectAll();
  QList<QTreeWidgetItem*> selected = _poitem->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    QString sql ( "SELECT distributevoucherline(:vohead_id,:poitem_id,:curr_id) AS result" );
    q.prepare(sql);
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":poitem_id", ((XTreeWidgetItem*)(selected[i]))->id());
    q.bindValue(":curr_id", _amountToDistribute->id());
    q.exec();


    if (q.first())
    {
          if (q.value("result").toInt() == -2)
                QMessageBox::warning( this, tr("Zero Amount"),
                              tr( "<p>Distribution would result in zero quantity and amount.\n"
                                  "Please distribute manually." ) );
          else if (q.value("result").toInt() == -3)
                QMessageBox::warning( this, tr("Different Currencies"),
                              tr( "<p>The purchase order and voucher have different currencies.\n"
                                  "Please distribute manually." ) );
          else if (q.value("result").toInt() == -4)
          QMessageBox::warning( this, tr("Negative Amount"),
                              tr( "<p>Distribution would result in a negative amount.\n"
                                  "Please distribute manually." ) );
          else if (q.value("result").toInt() == -5)
          QMessageBox::warning( this, tr("Multiple Cost Elements"),
                              tr( "<p>Item has multiple cost elements.\n"
                                  "Please distribute manually." ) );
    }
  }
  sFillList();
  sPopulateDistributed();

}

void voucher::sNewMiscDistribution()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("vohead_id", _voheadid);
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  params.append("amount", _balance->localValue());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sEditMiscDistribution()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vodist_id", _miscDistrib->id());
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());

  voucherMiscDistrib newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sDeleteMiscDistribution()
{
  q.prepare( "DELETE FROM vodist "
             "WHERE (vodist_id=:vodist_id);" );
  q.bindValue(":vodist_id", _miscDistrib->id());
  q.exec();
  sFillMiscList();
  sPopulateDistributed();
}

void voucher::sFillList()
{  
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT poitem_id, poitem_linenumber,"
               "       CASE WHEN(poitem_status='C') THEN :closed"
               "            WHEN(poitem_status='U') THEN :unposted"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered>(poitem_qty_received-poitem_qty_returned))) THEN :partial"
               "            WHEN(poitem_status='O' AND ((poitem_qty_received-poitem_qty_returned) > 0) AND (poitem_qty_ordered=(poitem_qty_received-poitem_qty_returned))) THEN :received"
               "            WHEN(poitem_status='O') THEN :open"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       COALESCE(item_number, poitem_vend_item_number),"
               "       COALESCE(uom_name, poitem_vend_uom),"
               "       poitem_vend_item_number, poitem_vend_uom,"
               "       formatQty(poitem_qty_ordered) AS qtyordered,"
               "       formatQty(( SELECT COALESCE(SUM(porecv_qty), 0)"
               "                   FROM porecv"
               "                   WHERE ( (porecv_posted)"
               "                    AND (porecv_invoiced)"
               "                    AND (porecv_poitem_id=poitem_id) ) )) AS f_invoiced,"
               "       formatQty(( SELECT COALESCE(SUM(porecv_qty), 0)"
               "                   FROM porecv"
               "                   WHERE ( (porecv_posted)"
               "                    AND (NOT porecv_invoiced)"
               "                    AND (porecv_vohead_id IS NULL)"
               "                    AND (porecv_poitem_id=poitem_id) ) )) AS f_qtyreceived,"
               "       formatQty(( SELECT COALESCE(SUM(poreject_qty), 0)"
               "                   FROM poreject"
               "                   WHERE ( (poreject_posted)"
               "                    AND (NOT poreject_invoiced)"
               "                    AND (poreject_vohead_id IS NULL)"
               "                    AND (poreject_poitem_id=poitem_id) ) )) AS f_qtyrejected,"
               "       formatMoney( (SELECT COALESCE(SUM(vodist_amount), 0)"
               "                     FROM vodist"
               "                     WHERE vodist_poitem_id=poitem_id"
               "                      AND vodist_vohead_id=:vohead_id) "
               "                  + (SELECT COALESCE(SUM(COALESCE(voitem_freight,0)), 0)"
               "                       FROM voitem"
               "                      WHERE voitem_poitem_id=poitem_id"
               "                        AND   voitem_vohead_id=:vohead_id) "
	       "                  ) AS f_invoiceamount,"
               "       formatMoney(poitem_unitprice) AS f_unitprice,"
               "       formatMoney(poitem_unitprice * poitem_qty_ordered) AS f_extprice,"
               "       formatMoney(poitem_freight) AS f_itemfreight "
               "FROM poitem LEFT OUTER JOIN"
               "     ( itemsite JOIN item"
               "       ON (itemsite_item_id=item_id) JOIN uom ON (item_inv_uom_id=uom_id) )"
               "     ON (poitem_itemsite_id=itemsite_id), pohead "
               "WHERE (poitem_pohead_id=:pohead_id) "
	       "  AND pohead_id = poitem_pohead_id "
               "GROUP BY poitem_id, poitem_linenumber, poitem_status,"
               "         item_number, uom_name,"
               "         poitem_vend_item_number, poitem_vend_uom,"
               "         poitem_unitprice, poitem_freight,"
               "         poitem_qty_ordered, poitem_qty_received, poitem_qty_returned,"
               "         itemsite_id, pohead_curr_id;" );
    q.bindValue(":vohead_id", _voheadid);
    q.bindValue(":pohead_id", _poNumber->id());
    q.bindValue(":closed", tr("Closed"));
    q.bindValue(":unposted", tr("Unposted"));
    q.bindValue(":partial", tr("Partial"));
    q.bindValue(":received", tr("Received"));
    q.bindValue(":open", tr("Open"));
    q.exec();
    _poitem->populate(q);
  }
  else
    _poitem->clear();
}

void voucher::sFillMiscList()
{
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT vodist_id, (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               " formatMoney(vodist_amount) "
               "FROM vodist, accnt "
               "WHERE ( (vodist_poitem_id=-1)"
               " AND (vodist_accnt_id=accnt_id)"
               " AND (vodist_vohead_id=:vohead_id) ) "
               "UNION ALL "
               "SELECT vodist_id, (expcat_code || ' - ' || expcat_descrip) AS account,"
	       " formatMoney(vodist_amount) "
               "  FROM vodist, expcat "
               " WHERE ( (vodist_poitem_id=-1)"
               "   AND   (vodist_expcat_id=expcat_id)"
               "   AND   (vodist_vohead_id=:vohead_id) ) "
               "ORDER BY account;" );
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    _miscDistrib->populate(q);
  }
}

void voucher::sPopulatePoInfo()
{
  q.prepare( "SELECT pohead_terms_id, vend_1099, pohead_curr_id "
             "FROM pohead, vend "
             "WHERE ( (pohead_vend_id=vend_id)"
             " AND (pohead_id=:pohead_id) );" );
  q.bindValue(":pohead_id", _poNumber->id());
  q.exec();
  if (q.first())
  {
    bool gets1099 = q.value("vend_1099").toBool();

    _flagFor1099->setChecked(gets1099);
    _terms->setId(q.value("pohead_terms_id").toInt());
    _amountToDistribute->setId(q.value("pohead_curr_id").toInt());
  }
}

void voucher::sPopulateDistributed()
{
  if (_poNumber->isValid())
  {
    q.prepare( "SELECT (COALESCE(dist,0) + COALESCE(freight,0)) AS distrib"
               "  FROM (SELECT SUM(COALESCE(voitem_freight,0)) AS freight"
               "          FROM voitem"
               "         WHERE (voitem_vohead_id=:vohead_id)) AS data1, "
               "       (SELECT SUM(COALESCE(vodist_amount, 0)) AS dist"
               "          FROM vodist"
               "         WHERE (vodist_vohead_id=:vohead_id)) AS data2;" );
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      _amountDistributed->setLocalValue(q.value("distrib").toDouble());
      sPopulateBalanceDue();
    }
  }
}

void voucher::sPopulateBalanceDue()
{
  _balance->setLocalValue(_amountToDistribute->localValue() - _amountDistributed->localValue());
  if (_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("black"));
  else
    _balance->setPaletteForegroundColor(QColor("red"));
}

void voucher::populateNumber()
{
  q.exec("SELECT fetchVoNumber() AS vouchernumber;");
  if (q.first())
  {
    _voucherNumber->setText(q.value("vouchernumber").toString());
    _voucherNumber->setEnabled(FALSE);
  }
}
 
void voucher::populate()
{
  XSqlQuery vohead;
  vohead.prepare( "SELECT vohead_number, vohead_pohead_id, vohead_terms_id,"
                  "       vohead_distdate, vohead_docdate, vohead_duedate,"
                  "       vohead_invcnumber, vohead_reference,"
                  "       vohead_1099, vohead_amount, vohead_curr_id "
                  "FROM vohead "
                  "WHERE (vohead_id=:vohead_id);" );
  vohead.bindValue(":vohead_id", _voheadid);
  vohead.exec();
  if (vohead.first())
  {
    _voucherNumber->setText(vohead.value("vohead_number").toString());
    _poNumber->setId(vohead.value("vohead_pohead_id").toInt());
    _terms->setId(vohead.value("vohead_terms_id").toInt());
    _amountToDistribute->set(vohead.value("vohead_amount").toDouble(),
			     vohead.value("vohead_curr_id").toInt(),
			     vohead.value("vohead_docdate").toDate(), false);

    _distributionDate->setDate(vohead.value("vohead_distdate").toDate(), true);
    _invoiceDate->setDate(vohead.value("vohead_docdate").toDate());
    _dueDate->setDate(vohead.value("vohead_duedate").toDate());
    _invoiceNum->setText(vohead.value("vohead_invcnumber").toString());
    _reference->setText(vohead.value("vohead_reference").toString());
    _flagFor1099->setChecked(vohead.value("vohead_1099").toBool());

    sFillList();
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::clear()
{
}

void voucher::closeEvent(QCloseEvent *pEvent)
{
  if (_mode == cNew)
  {
    q.prepare("SELECT deleteVoucher(:vohead_id) AS result;");
    q.bindValue(":vohead_id", _voheadid);
    q.exec();
    if (q.first())
    {
      // TODO: change deleteVoucher to return INT instead of current BOOLEAN
      if (! q.value("result").toBool())
	systemError(this, tr("Error deleting temporary Voucher."),
		    __FILE__, __LINE__);
    }
    else if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    q.prepare("SELECT releaseVoNumber(:voucherNumber);" );
    q.bindValue(":voucherNumber", _voucherNumber->text());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  pEvent->accept();
}

void voucher::sPopulateDistDate()
{
  if ( (_invoiceDate->isValid()) && (!_distributionDate->isValid()) )
  {
    _distributionDate->setDate(_invoiceDate->date(), true);
    sPopulateDueDate();
  }
}

void voucher::sPopulateDueDate()
{
  if ( (_invoiceDate->isValid()) && (!_dueDate->isValid()) )
  {
    q.prepare("SELECT determineDueDate(:terms_id, :invoiceDate) AS duedate;");
    q.bindValue(":terms_id", _terms->id());
    q.bindValue(":invoiceDate", _invoiceDate->date());
    q.exec();
    if (q.first())
      _dueDate->setDate(q.value("duedate").toDate());
  }
}
 

void voucher::sPopulateMenu( QMenu * pMenu )
{
  pMenu->insertItem(tr("View P/O Item..."), this, SLOT(sView()), 0);
}

void voucher::sView()
{
  if(_poitem->id() == -1)
    return;

  ParameterList params;
  params.append("poitem_id", _poitem->id());
  params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void voucher::keyPressEvent( QKeyEvent * e )
{
  if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
  {
    _save->animateClick();
    e->accept();
  }
  else
    e->ignore();
}

