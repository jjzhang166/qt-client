/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

// TODO: create parent class that's inherited by both voucher and miscvoucher

#include "voucher.h"

#include <QAction>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <metasql.h>
#include <mqlutil.h>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "purchaseOrderList.h"
#include "purchaseOrderItem.h"
#include "storedProcErrorLookup.h"
#include "taxBreakdown.h"
#include "voucherItem.h"
#include "voucherMiscDistrib.h"

voucher::voucher(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_save,                     SIGNAL(clicked()),                                 this,          SLOT(sSave()));
  connect(_distributions,            SIGNAL(clicked()),                                 this,          SLOT(sDistributions()));
  connect(_distributeline,           SIGNAL(clicked()),                                 this,          SLOT(sDistributeLine()));
  connect(_clear,                    SIGNAL(clicked()),                                 this,          SLOT(sClear()));
  connect(_distributeall,            SIGNAL(clicked()),                                 this,          SLOT(sDistributeAll()));
  connect(_voucherNumber,            SIGNAL(editingFinished()),                         this,          SLOT(sHandleVoucherNumber()));
  connect(_poNumber,                 SIGNAL(newId(int, const QString&)),                this,          SLOT(sFillList()));
  connect(_poNumber,                 SIGNAL(newId(int, const QString&)),                this,          SLOT(sPopulatePoInfo()));
  connect(_amountToDistribute,       SIGNAL(valueChanged()),                            this,          SLOT(sPopulateBalanceDue()));
  connect(_new,                      SIGNAL(clicked()),                                 this,          SLOT(sNewMiscDistribution()));
  connect(_edit,                     SIGNAL(clicked()),                                 this,          SLOT(sEditMiscDistribution()));
  connect(_delete,                   SIGNAL(clicked()),                                 this,          SLOT(sDeleteMiscDistribution()));
  connect(_invoiceDate,              SIGNAL(newDate(const QDate&)),                     this,          SLOT(sPopulateDistDate()));
  connect(_distributionDate,         SIGNAL(newDate(const QDate&)),                     this,          SLOT(sNewDistDate()));
  connect(_taxLit,                   SIGNAL(leftClickedURL(const QString&)),            this,          SLOT(sTaxDetail()));
  connect(_terms,                    SIGNAL(newID(int)),                                this,          SLOT(sPopulateDueDate()));
  connect(_poitem,                   SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this,          SLOT(sPopulateMenu(QMenu*)));
  connect(_amountToDistribute,       SIGNAL(idChanged(int)),                            this,          SLOT(sFillList()));
  connect(_amountDistributed,        SIGNAL(valueChanged()),                            this,          SLOT(sPopulateBalanceDue()));
  connect(_freight,                  SIGNAL(valueChanged()),                            this,          SLOT(sPopulateDistributed()));

  _terms->setType(XComboBox::APTerms);
  _poNumber->setAllowedStatuses(OrderLineEdit::Open);
  _poNumber->setAllowedTypes(OrderLineEdit::Purchase);
  _poNumber->setLabel("");
  _poNumber->findChild<QWidget*>("_name")->hide();
  _poNumber->findChild<QWidget*>("_description")->hide();
  _poNumber->findChild<QWidget*>("_fromLit")->hide();
  _poNumber->findChild<QWidget*>("_from")->hide();
  _poNumber->findChild<QWidget*>("_toLit")->hide();
  _poNumber->findChild<QWidget*>("_to")->hide();
  _poNumber->setMinimumHeight(32);

  _poitem->addColumn(tr("#"),               _whsColumn,   Qt::AlignCenter, true,  "poitem_linenumber" );
  _poitem->addColumn(tr("Status"),          _uomColumn,   Qt::AlignCenter, true,  "poitemstatus" );
  _poitem->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,   true,  "itemnumber"   );
  _poitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "uom" );
  _poitem->addColumn(tr("Vend. Item #"),    -1,           Qt::AlignLeft,   true,  "poitem_vend_item_number"   );
  _poitem->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter, true,  "poitem_vend_uom" );
  _poitem->addColumn(tr("Ordered"),         _qtyColumn,   Qt::AlignRight,  true,  "poitem_qty_ordered"  );
  _poitem->addColumn(tr("Invoiced"),        _qtyColumn,   Qt::AlignRight,  false, "qtyinvoiced" );
  _poitem->addColumn(tr("Uninvoiced"),      _qtyColumn,   Qt::AlignRight,  true,  "qtyreceived"  );
  _poitem->addColumn(tr("Rejected"),        _qtyColumn,   Qt::AlignRight,  true,  "qtyrejected"  );
  _poitem->addColumn(tr("Quantity"),        _qtyColumn,   Qt::AlignRight,  false, "invoiceqty" );
  _poitem->addColumn(tr("Amount"),          _moneyColumn, Qt::AlignRight,  true,  "invoiceamount"  );
  _poitem->addColumn(tr("PO Unit Price"),   _moneyColumn, Qt::AlignRight,  true,  "poitem_unitprice" );
  _poitem->addColumn(tr("PO Ext Price"),    _moneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _poitem->addColumn(tr("PO Line Freight"), _moneyColumn, Qt::AlignRight,  true,  "poitem_freight" );

  _miscDistrib->addColumn(tr("Account"),    -1,           Qt::AlignLeft,   true,  "account"  );
  _miscDistrib->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight,  true,  "vodist_amount" );
  _miscDistrib->addColumn(tr("Notes"),      -1,           Qt::AlignLeft,   true,  "vodist_notes" );

  _charass->setType("VCH");

  _vendid = -1;

  setWindowModified(false);
}

voucher::~voucher()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucher::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucher::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if (_metrics->value("VoucherNumberGeneration") == "A")
        populateNumber();

      XSqlQuery insq;
      insq.prepare("INSERT INTO vohead (vohead_number, vohead_posted)"
                   "            VALUES (:vohead_number, false)"
                   " RETURNING vohead_id;" );
      insq.bindValue(":vohead_number", _voucherNumber->text());
      insq.exec();
      if (insq.first())
      {
        _voheadid = insq.value("vohead_id").toInt();
        _documents->setId(_voheadid);
        _charass->setId(_voheadid);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Inserting Voucher"),
                                    insq, __FILE__, __LINE__))
        return UndefinedError;

      enableWindowModifiedSetting();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _voucherNumber->setEnabled(false);
      _poNumber->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _poNumber->setAllowedStatuses(OrderLineEdit::AnyStatus);

      _voucherNumber->setEnabled(false);
      _poNumber->setEnabled(false);
      _taxzone->setEnabled(false);
      _amountToDistribute->setEnabled(false);
      _distributionDate->setEnabled(false);
      _invoiceDate->setEnabled(false);
      _dueDate->setEnabled(false);
      _invoiceNum->setEnabled(false);
      _reference->setEnabled(false);
      _distributions->setEnabled(false);
      _miscDistrib->setEnabled(false);
      _new->setEnabled(false);
      _terms->setEnabled(false);
      _terms->setType(XComboBox::Terms);
      _flagFor1099->setEnabled(false);
      _distributeall->setEnabled(false);
      _notes->setEnabled(false);
//      _documents->setReadOnly(true);
      _charass->setReadOnly(true);
      _close->setText(tr("&Close"));
      _freight->setEnabled(false);
      _freightExpcat->setEnabled(false);
   
      _save->hide();

      disconnect(_poitem, SIGNAL(valid(bool)), _distributions, SLOT(setEnabled(bool)));
      disconnect(_poitem, SIGNAL(valid(bool)), _distributeline, SLOT(setEnabled(bool)));
      disconnect(_poitem, SIGNAL(valid(bool)), _clear, SLOT(setEnabled(bool)));
      disconnect(_poNumber, SIGNAL(valid(bool)), _distributeall, SLOT(setEnabled(bool)));
    }
  }

  param = pParams.value("pohead_id", &valid);
  if (valid)
    _poNumber->setId(param.toInt(), "PO");

  param = pParams.value("vohead_id", &valid);
  if (valid)
  {
    _voheadid = param.toInt();
    _documents->setId(_voheadid);
    _charass->setId(_voheadid);
    populate();
    enableWindowModifiedSetting();
  }

  return NoError;
}

int voucher::id() const
{
  return _voheadid;
}

int voucher::mode() const
{
  return _mode;
}

bool voucher::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(! _poNumber->isValid(), _poNumber,
                           tr("<p>You must enter an PO Number before you may "
                              "save this Voucher."))
         << GuiErrorCheck(!_invoiceDate->isValid(), _invoiceDate,
                           tr("<p>You must enter an Invoice Date before you "
                              "may save this Voucher."))
         << GuiErrorCheck(!_dueDate->isValid(), _dueDate,
                           tr("<p>You must enter a Due Date before you may "
                              "save this Voucher."))
         << GuiErrorCheck(!_distributionDate->isValid(), _distributionDate,
                           tr("<p>You must enter a Distribution Date before "
                              "you may save this Voucher."))
         << GuiErrorCheck(_metrics->boolean("ReqInvRegVoucher") &&
                          _invoiceNum->text().trimmed().isEmpty(),
                          _invoiceNum,
                          tr("<p>You must enter a Vendor Invoice Number "
                             "before you may save this Voucher."))
         << GuiErrorCheck(_freight->localValue() > 0 && !_freightExpcat->isValid(), _freightExpcat,
                           tr("<p>You must select an Expense Category to post the  "
                              "freight value to."))
  ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Voucher"), errors))
    return false;

  if (_invoiceNum->text().trimmed().length() > 0)
  {
    XSqlQuery dupq;
    dupq.prepare( "SELECT vohead_id "
               "FROM vohead, pohead "
               "WHERE ( (vohead_pohead_id=pohead_id)"
               " AND (vohead_invcnumber=:vohead_invcnumber)"
               " AND (pohead_vend_id=:vend_id)"
               " AND (vohead_id<>:vohead_id) );" );
    dupq.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
    dupq.bindValue(":vend_id",   _vendid);
    dupq.bindValue(":vohead_id", _voheadid);
    dupq.exec();
    if (dupq.first())
    {
      if (QMessageBox::question( this, windowTitle(),
                             tr("<p>A Voucher for this Vendor has already been "
                                "entered with the same Vendor Invoice Number. "
                                "Are you sure you want to use this number again?" ),
                              QMessageBox::No | QMessageBox::Yes,
                              QMessageBox::No) == QMessageBox::No)
        return false;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Reused Invoice Check"),
                                  dupq, __FILE__, __LINE__))
      return false;
  }

  XSqlQuery updq;
  updq.prepare( "UPDATE vohead "
             "SET vohead_pohead_id=:vohead_pohead_id,"
             "    vohead_taxzone_id=:vohead_taxzone_id,  "
             "    vohead_vend_id=:vohead_vend_id,"
             "    vohead_terms_id=:vohead_terms_id,"
             "    vohead_distdate=:vohead_distdate,"
             "    vohead_docdate=:vohead_docdate,"
             "    vohead_duedate=:vohead_duedate,"
             "    vohead_invcnumber=:vohead_invcnumber,"
             "    vohead_reference=:vohead_reference,"
             "    vohead_amount=:vohead_amount,"
             "    vohead_freight=:vohead_freight,"
             "    vohead_freight_expcat_id=:vohead_freight_expcat,"
             "    vohead_1099=:vohead_1099, "
             "    vohead_curr_id=:vohead_curr_id, "
             "    vohead_notes=:vohead_notes "
             "WHERE (vohead_id=:vohead_id);" );

  updq.bindValue(":vohead_id", _voheadid);
  updq.bindValue(":vohead_pohead_id", _poNumber->id());
  if (_taxzone->isValid())
    updq.bindValue(":vohead_taxzone_id", _taxzone->id());
  updq.bindValue(":vohead_vend_id",  _vendid);
  updq.bindValue(":vohead_terms_id", _terms->id());
  updq.bindValue(":vohead_distdate", _distributionDate->date());
  updq.bindValue(":vohead_docdate", _invoiceDate->date());
  updq.bindValue(":vohead_duedate", _dueDate->date());
  updq.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
  updq.bindValue(":vohead_reference", _reference->text().trimmed());
  updq.bindValue(":vohead_amount", _amountToDistribute->localValue());
  updq.bindValue(":vohead_freight", _freight->localValue());
  if (_freightExpcat->isValid())
    updq.bindValue(":vohead_freight_expcat", _freightExpcat->id());
  updq.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked()));
  updq.bindValue(":vohead_curr_id", _amountToDistribute->id());
  updq.bindValue(":vohead_notes", _notes->toPlainText());
  updq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Saving Voucher"),
                           updq, __FILE__, __LINE__))
    return false;

  omfgThis->sVouchersUpdated();

  _voheadid = -1;

  if (cNew != _mode)
  {
    setWindowModified(false);
    close();
    return true;
  }

  _poNumber->setEnabled(true);
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
  _notes->setText("");
  _charass->setId(-1);
  _freight->clear();
  _freightExpcat->setId(-1);

  setWindowModified(false);

  ParameterList params;
  params.append("mode", "new");
  set(params);
  _poNumber->setFocus();
  return true;
}

void voucher::sHandleVoucherNumber()
{
  if (_voucherNumber->text().length() == 0)
  {
    if ((_metrics->value("VoucherNumberGeneration") == "A") ||
        (_metrics->value("VoucherNumberGeneration") == "O"))
      populateNumber();
    else
    {
      QMessageBox::critical( this, tr("Enter Voucher Number"),
                             tr("<p>You must enter a valid Voucher Number "
                                "before continuing") );

      _voucherNumber->setFocus();
      return;
    }
  }
  else
  {
    XSqlQuery editq;
    editq.prepare( "SELECT vohead_id "
               "FROM vohead "
               "WHERE (vohead_number=:vohead_number);" );
    editq.bindValue(":vohead_number", _voucherNumber->text());
    editq.exec();
    if (editq.first())
    {
      _voheadid = editq.value("vohead_id").toInt();

      _voucherNumber->setEnabled(false);
      _poNumber->setEnabled(false);

      _mode = cEdit;
      populate();

      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Looking for Existing Voucher"),
                                  editq, __FILE__, __LINE__))
      return;
  }
}

void voucher::sPopulate()
{
  setWindowTitle(tr("Voucher for P/O # %1").arg(_poNumber->number()));
}

void voucher::sDistributions()
{
  saveDetail();
  foreach (XTreeWidgetItem *item, _poitem->selectedItems())
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("vohead_id", _voheadid);
    params.append("poitem_id",  item->id());
    params.append("curr_id", _amountToDistribute->id());
    params.append("effective", _amountToDistribute->effective());

    voucherItem newdlg(this, "", true);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Rejected)
    {
      // nothing to do:
      // voucherItem wraps itself in a transaction and rolls back on reject
    }
    else
      _poNumber->setEnabled(false);
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sDistributeLine()
{
  saveDetail();
  foreach (XTreeWidgetItem *item, _poitem->selectedItems())
  {
    XSqlQuery distq;
    distq.prepare("SELECT distributeVoucherLine(:vohead_id,:poitem_id,:curr_id) "
              "AS result;");
    distq.bindValue(":vohead_id", _voheadid);
    distq.bindValue(":poitem_id",  item->id());
    distq.bindValue(":curr_id", _amountToDistribute->id());
    distq.exec();
    if (distq.first())
    {
      int result = distq.value("result").toInt();
      if (result < 0)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Distributing"),
                    storedProcErrorLookup("distributeVoucherLine", result),
                    __FILE__, __LINE__);
      else
      {
        _poNumber->setEnabled(false);
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Distributing Line"),
                                  distq, __FILE__, __LINE__))
      return;
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sClear()
{
  XSqlQuery clearq;
  // TODO: split these or write a function
  clearq.prepare("DELETE FROM vodist "
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
                 "  AND (poreject_poitem_id=:poitem_id)); ");

  foreach (XTreeWidgetItem *item, _poitem->selectedItems())
  {
    clearq.bindValue(":vohead_id", _voheadid);
    clearq.bindValue(":poitem_id", item->id());
    clearq.exec();
    // no return - do what we can
    ErrorReporter::error(QtCriticalMsg, this, tr("Clearing Distributions"),
                         clearq, __FILE__, __LINE__);
  }

  _poNumber->setEnabled(true);
  sFillList();
  sPopulateDistributed();
}

void voucher::sDistributeAll()
{
  saveDetail();
  _poitem->selectAll();

  XSqlQuery distq;
  distq.prepare("SELECT distributeVoucherLine(:vohead_id,:poitem_id,:curr_id) "
                "AS result;");

  foreach (XTreeWidgetItem *item, _poitem->selectedItems())
  {
    distq.bindValue(":vohead_id", _voheadid);
    distq.bindValue(":poitem_id", item->id());
    distq.bindValue(":curr_id", _amountToDistribute->id());
    distq.exec();
    if (distq.first())
    {
      int result = distq.value("result").toInt();
      if (result < 0)
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Distributing"),
                             storedProcErrorLookup("distributeVoucherLine", result),
                             __FILE__, __LINE__);
      else
        _poNumber->setEnabled(false);
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Distributing"),
                                  distq, __FILE__, __LINE__))
      return;
  }
  sFillList();
  sPopulateDistributed();
}

void voucher::sNewMiscDistribution()
{
  saveDetail();
  ParameterList params;
  params.append("mode", "new");
  params.append("vohead_id", _voheadid);
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  params.append("amount", _balance->localValue());
  if (_taxzone->isValid())
    params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sUpdateVoucherTax();
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sEditMiscDistribution()
{
  saveDetail();
  ParameterList params;
  params.append("mode", "edit");
  params.append("vohead_id", _voheadid);
  params.append("vodist_id", _miscDistrib->id());
  params.append("curr_id", _amountToDistribute->id());
  params.append("curr_effective", _amountToDistribute->effective());
  if (_taxzone->isValid())
    params.append("taxzone_id", _taxzone->id());

  voucherMiscDistrib newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
  {
    sUpdateVoucherTax();
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sDeleteMiscDistribution()
{
  saveDetail();
  XSqlQuery delq;
  // TODO: split or write a function
  delq.prepare("DELETE FROM voheadtax"
               " WHERE (taxhist_parent_id=:vohead_id)"
               "   AND (taxhist_taxtype_id=getadjustmenttaxtypeid())"
               "   AND (taxhist_tax_id = (SELECT vodist_tax_id"
               "                            FROM vodist"
               "                           WHERE (vodist_id=:vodist_id)));"
               "DELETE FROM vodist"
               " WHERE (vodist_id=:vodist_id);");

  delq.bindValue(":vohead_id", _voheadid);
  delq.bindValue(":vodist_id", _miscDistrib->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Distributions"),
                           delq, __FILE__, __LINE__))
    return;
  else
  {
    sUpdateVoucherTax();
    sFillMiscList();
    sPopulateDistributed();
  }
}

void voucher::sFillList()
{
  if (_poNumber->isValid())
  {
    MetaSQLQuery mql = mqlLoad("voucher", "poitems");
    ParameterList params;

    params.append("vohead_id", _voheadid);
    params.append("pohead_id", _poNumber->id());
    params.append("closed",    tr("Closed"));
    params.append("unposted",  tr("Unposted"));
    params.append("partial",   tr("Partial"));
    params.append("received",  tr("Received"));
    params.append("open",      tr("Open"));

    XSqlQuery poitemFillList = mql.toQuery(params);
    _poitem->populate(poitemFillList, true);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving PO Items"),
                                poitemFillList, __FILE__, __LINE__))
        return;

    _freight->setId(_amountToDistribute->id());
  }
  else
    _poitem->clear();
}

void voucher::sFillMiscList()
{
  if (_poNumber->isValid())
  {
    MetaSQLQuery mql = mqlLoad("voucher", "miscDistr");
    ParameterList params;

    params.append("vohead_id", _voheadid);
    XSqlQuery miscFillList = mql.toQuery(params);
    _miscDistrib->populate(miscFillList, true);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Misc Distributions"),
                                miscFillList, __FILE__, __LINE__))
        return;
  }
}

void voucher::sPopulatePoInfo()
{
  XSqlQuery po;
  po.prepare("SELECT pohead_terms_id, pohead_taxzone_id, vend_1099, "
             "       pohead_curr_id, vend_id, vend_number, vend_name,"
             "       addr_line1, addr_line2"
             "  FROM pohead"
             "  JOIN vendinfo ON (pohead_vend_id=vend_id)"
             "  LEFT OUTER JOIN addr ON (vend_addr_id=addr_id)"
             " WHERE (pohead_id=:pohead_id);" );
  po.bindValue(":pohead_id", _poNumber->id());
  po.exec();
  if (po.first())
  {
    bool gets1099 = po.value("vend_1099").toBool();

    _flagFor1099->setChecked(gets1099);
    _terms->setId(po.value("pohead_terms_id").toInt());
    _taxzone->setId(po.value("pohead_taxzone_id").toInt());
    _amountToDistribute->setId(po.value("pohead_curr_id").toInt());
    _freight->setId(po.value("pohead_curr_id").toInt());
    _vendid = po.value("vend_id").toInt();
    _vendor->setText(po.value("vend_number").toString());
    _vendName->setText(po.value("vend_name").toString());
    // TODO: replace with a compact AddressCluster when such exists
    _vendAddress1->setText(po.value("addr_line1").toString());
    _vendAddress2->setText(po.value("addr_line2").toString());
    if (_mode != cView)
      _new->setEnabled(true);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting P/O Information"),
                                po, __FILE__, __LINE__))
    return;
}

void voucher::sPopulateDistributed()
{
  if (_poNumber->isValid())
  {
    sCalculateTax();
    XSqlQuery getq;
    getq.prepare( "SELECT (COALESCE(dist,0) + COALESCE(freight,0) + COALESCE(tax,0)) AS distrib"
               "  FROM (SELECT SUM(COALESCE(voitem_freight,0)) AS freight"
               "          FROM voitem"
               "         WHERE (voitem_vohead_id=:vohead_id)) AS data1, "
               "       (SELECT SUM(COALESCE(vodist_amount, 0)) AS dist"
               "          FROM vodist"
               "         WHERE ( (vodist_vohead_id=:vohead_id)"
               "           AND   (vodist_tax_id=-1) )) AS data2, "
               "       (SELECT SUM(tax * -1.0) AS tax "
               "          FROM ("
               "            SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
               "              FROM tax "
               "              JOIN calculateTaxDetailSummary('VO', :vohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
               "             GROUP BY tax_id) AS taxdata) AS data3;" );
    getq.bindValue(":vohead_id", _voheadid);
    getq.exec();
    if (getq.first())
    {
      _amountDistributed->setLocalValue(getq.value("distrib").toDouble() + _freight->localValue());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Getting Distributions"),
                                  getq, __FILE__, __LINE__))
      return;
  }
}

void voucher::sPopulateBalanceDue()
{
  _balance->setLocalValue(_amountToDistribute->localValue() - 
                          _amountDistributed->localValue());

  if (_balance->isZero())
    _balance->setPaletteForegroundColor(QColor("black"));
  else
    _balance->setPaletteForegroundColor(namedColor("error"));

  if (_freight->localValue() <= 0)
    _freightExpcat->setId(-1);

  _freightExpcat->setEnabled(_freight->localValue() > 0 && _mode != cView);
 
}

void voucher::populateNumber()
{
  XSqlQuery getq;
  getq.exec("SELECT fetchVoNumber() AS vouchernumber;");
  if (getq.first())
  {
    _voucherNumber->setText(getq.value("vouchernumber").toString());
    _voucherNumber->setEnabled(false);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting ID"),
                                getq, __FILE__, __LINE__))
    return;
}

void voucher::populate()
{
  XSqlQuery vohead;
  vohead.prepare( "SELECT vohead_number, vohead_pohead_id, vohead_taxzone_id, vohead_terms_id,"
                  "       vohead_distdate, vohead_docdate, vohead_duedate,"
                  "       vohead_invcnumber, vohead_reference, vohead_freight, vohead_freight_expcat_id,"
                  "       vohead_1099, vohead_amount, vohead_curr_id, vohead_notes "
                  "FROM vohead "
                  "WHERE (vohead_id=:vohead_id);" );
  vohead.bindValue(":vohead_id", _voheadid);
  vohead.exec();
  if (vohead.first())
  {
    _voucherNumber->setText(vohead.value("vohead_number").toString());
    _poNumber->setId(vohead.value("vohead_pohead_id").toInt(), "PO");
    _taxzone->setId(vohead.value("vohead_taxzone_id").toInt());
    _terms->setId(vohead.value("vohead_terms_id").toInt());
    _amountToDistribute->set(vohead.value("vohead_amount").toDouble(),
                             vohead.value("vohead_curr_id").toInt(),
                             vohead.value("vohead_docdate").toDate(), false);
    _freight->set(vohead.value("vohead_freight").toDouble(),
                             vohead.value("vohead_curr_id").toInt(),
                             vohead.value("vohead_docdate").toDate(), false);
    _freightExpcat->setId(vohead.value("vohead_freight_expcat_id").toInt());

    _distributionDate->setDate(vohead.value("vohead_distdate").toDate(), true);
    _invoiceDate->setDate(vohead.value("vohead_docdate").toDate());
    _dueDate->setDate(vohead.value("vohead_duedate").toDate());
    _invoiceNum->setText(vohead.value("vohead_invcnumber").toString());
    _reference->setText(vohead.value("vohead_reference").toString());
    _flagFor1099->setChecked(vohead.value("vohead_1099").toBool());
    _notes->setText(vohead.value("vohead_notes").toString());

    sFillList();
    sFillMiscList();
    sPopulateDistributed();
    _charass->setId(_voheadid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Voucher"),
                                vohead, __FILE__, __LINE__))
    return;
}

void voucher::clear()
{
}

void voucher::closeEvent(QCloseEvent *pEvent)
{
  if (isWindowModified())
  {
    QMessageBox::StandardButtons buttons = omfgThis->shuttingDown() ?
                      QMessageBox::Yes | QMessageBox::No :
                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel;

    switch (QMessageBox::question(this, tr("Save Voucher?"),
                                  tr("Do you want to save this voucher?"),
                                  buttons, QMessageBox::Yes))
    {
      case QMessageBox::Yes:
        if (! sSave() && ! omfgThis->shuttingDown())
          pEvent->ignore();
        else if (_mode == cNew)
          break; // so we can clean up
        return;
      case QMessageBox::Cancel:
        pEvent->ignore();
        return;
      default:
        break;
    }
  }

  if (_mode == cNew)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM vohead WHERE (vohead_id=:vohead_id);"
                 "SELECT releaseVoNumber(:number);");
    delq.bindValue(":vohead_id", _voheadid);
    delq.bindValue(":number", _voucherNumber->text().toInt());
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Deleting Placeholder"),
                         delq, __FILE__, __LINE__);
  }

  pEvent->accept();
}

void voucher::sPopulateDistDate()
{
  if ( (_invoiceDate->isValid()) && (!_distributionDate->isValid()) )
  {
    _distributionDate->setDate(_invoiceDate->date(), true);
  }
  sPopulateDueDate();
}

void voucher::sPopulateDueDate()
{
  if ( (_invoiceDate->isValid()) && (!_dueDate->isValid()) )
  {
    XSqlQuery dateq;
    dateq.prepare("SELECT determineDueDate(:terms_id, :invoiceDate) AS duedate;");
    dateq.bindValue(":terms_id", _terms->id());
    dateq.bindValue(":invoiceDate", _invoiceDate->date());
    dateq.exec();
    if (dateq.first())
      _dueDate->setDate(dateq.value("duedate").toDate());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Due Date"),
                                  dateq, __FILE__, __LINE__))
      return;
  }
}

void voucher::sPopulateMenu( QMenu * pMenu )
{
  pMenu->addAction(tr("View P/O Item..."), this, SLOT(sView()));
}

void voucher::sView()
{
  if(_poitem->id() == -1)
    return;

  ParameterList params;
  params.append("poitem_id", _poitem->id());
  params.append("mode", "view");

  purchaseOrderItem newdlg(this, "", true);
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

bool voucher::saveDetail()
{
  if (_mode != cView)
  {
    XSqlQuery updq;
    updq.prepare( "UPDATE vohead "
               "SET vohead_pohead_id=:vohead_pohead_id,"
               "    vohead_taxzone_id=:vohead_taxzone_id,  "
               "    vohead_vend_id=:vohead_vend_id,"
               "    vohead_terms_id=:vohead_terms_id,"
               "    vohead_distdate=COALESCE(:vohead_distdate, CURRENT_DATE),"
               "    vohead_docdate=COALESCE(:vohead_docdate, CURRENT_DATE),"
               "    vohead_duedate=COALESCE(:vohead_duedate, CURRENT_DATE),"
               "    vohead_invcnumber=:vohead_invcnumber,"
               "    vohead_reference=:vohead_reference,"
               "    vohead_amount=:vohead_amount,"
               "    vohead_freight=:vohead_freight,"
               "    vohead_freight_expcat_id=:vohead_freightexpcat,"
               "    vohead_1099=:vohead_1099, "
               "    vohead_curr_id=:vohead_curr_id, "
               "    vohead_notes=:vohead_notes "
               "WHERE (vohead_id=:vohead_id);" );

    updq.bindValue(":vohead_id", _voheadid);
    updq.bindValue(":vohead_pohead_id", _poNumber->id());
    if (_taxzone->isValid())
      updq.bindValue(":vohead_taxzone_id", _taxzone->id());
    updq.bindValue(":vohead_vend_id",  _vendid);
    updq.bindValue(":vohead_terms_id", _terms->id());
    updq.bindValue(":vohead_distdate", _distributionDate->date());
    updq.bindValue(":vohead_docdate", _invoiceDate->date());
    updq.bindValue(":vohead_duedate", _dueDate->date());
    updq.bindValue(":vohead_invcnumber", _invoiceNum->text().trimmed());
    updq.bindValue(":vohead_reference", _reference->text().trimmed());
    updq.bindValue(":vohead_amount", _amountToDistribute->localValue());
    updq.bindValue(":vohead_freight", _freight->localValue());
    if (_freightExpcat->isValid())
      updq.bindValue(":vohead_freightexpcat", _freightExpcat->id());
    updq.bindValue(":vohead_1099", QVariant(_flagFor1099->isChecked()));
    updq.bindValue(":vohead_curr_id", _amountToDistribute->id());
    updq.bindValue(":vohead_notes", _notes->toPlainText());
    updq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Updating Voucher"),
                             updq, __FILE__, __LINE__))
      return false;
  }
  return true;
}

void voucher::sCalculateTax()
{
  if (_vendid == -1)
    return;

  if (!saveDetail())
    return;

  XSqlQuery taxq;
  taxq.prepare( "SELECT ABS(SUM(tax)) AS tax "
                "FROM ("
                "SELECT ROUND(SUM(taxdetail_tax),2) AS tax "
                "FROM tax "
                " JOIN calculateTaxDetailSummary('VO', :vohead_id, 'T') ON (taxdetail_tax_id=tax_id)"
                "GROUP BY tax_id) AS data;" );
  taxq.bindValue(":vohead_id", _voheadid);
  taxq.exec();
  if (taxq.first())
    _tax->setLocalValue(taxq.value("tax").toDouble());
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Calculating Voucher Tax"),
                                    taxq, __FILE__, __LINE__))
        return;
}

void voucher::sTaxDetail()
{
  if (!saveDetail())
    return;

  ParameterList params;
  params.append("order_id", _voheadid);
  params.append("order_type", "VO");
  // mode => view since there are no fields to hold modified tax data
  if (_mode == cView)
    params.append("mode", "view");

  taxBreakdown newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void voucher::sUpdateVoucherTax()
{
  if (!_taxzone->isValid() || !_distributionDate->isValid())
    return;

  XSqlQuery updTax;
  updTax.prepare("SELECT updatemiscvouchertax(:voheadid,:taxzone,:distdate,:curr) as ret;");
  updTax.bindValue(":voheadid", _voheadid);
  updTax.bindValue(":taxzone",  _taxzone->id());
  updTax.bindValue(":distdate", _distributionDate->date());
  updTax.bindValue(":curr", _amountToDistribute->id());
  updTax.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Adding Tax to Voucher"),
                         updTax, __FILE__, __LINE__))
    return;
}

void voucher::sNewDistDate()
{
  if (_miscDistrib->topLevelItemCount())
  {
     sUpdateVoucherTax();
     sFillMiscList();
  }
}

void voucher::enableWindowModifiedSetting()
{
  connect(_amountToDistribute,     SIGNAL(valueChanged()), this, SLOT(sDataChanged()));
  connect(_freight,                SIGNAL(valueChanged()), this, SLOT(sDataChanged()));
  connect(_distributionDate,SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_dueDate,         SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_flagFor1099,             SIGNAL(toggled(bool)), this, SLOT(sDataChanged()));
  connect(_invoiceDate,     SIGNAL(newDate(const QDate&)), this, SLOT(sDataChanged()));
  connect(_invoiceNum,SIGNAL(textChanged(const QString&)), this, SLOT(sDataChanged()));
  connect(_notes,                   SIGNAL(textChanged()), this, SLOT(sDataChanged()));
  connect(_poNumber,                   SIGNAL(newId(int)), this, SLOT(sDataChanged()));
  connect(_reference, SIGNAL(textChanged(const QString&)), this, SLOT(sDataChanged()));
  connect(_taxzone,                    SIGNAL(newID(int)), this, SLOT(sDataChanged()));
  connect(_terms,                      SIGNAL(newID(int)), this, SLOT(sDataChanged()));
}

void voucher::sDataChanged()
{
  setWindowModified(true);
}

