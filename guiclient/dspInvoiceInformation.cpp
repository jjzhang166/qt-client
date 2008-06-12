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

#include "dspInvoiceInformation.h"

#include <QSqlError>
#include <QVariant>

#include "invoice.h"

#include <openreports.h>
#include <invoiceList.h>

dspInvoiceInformation::dspInvoiceInformation(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invoiceNumber, SIGNAL(newInvoiceNumber(QString)), this, SLOT(sParseInvoiceNumber()));
  connect(_invoiceList, SIGNAL(clicked()), this, SLOT(sInvoiceList()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sViewDetails()));

#ifndef Q_WS_MAC
  _invoiceList->setMaximumWidth(25);
#endif

  _cust->setReadOnly(TRUE);

  _arapply->addColumn( tr("Type"),        _dateColumn,  Qt::AlignCenter );
  _arapply->addColumn( tr("Doc./Ref. #"), -1,           Qt::AlignCenter );
  _arapply->addColumn( tr("Apply Date"),  _dateColumn,  Qt::AlignCenter );
  _arapply->addColumn( tr("Amount"),      _moneyColumn, Qt::AlignCenter );

  _invcheadid = -1;
}

dspInvoiceInformation::~dspInvoiceInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInvoiceInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInvoiceInformation::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("invoiceNumber", &valid);
  if (valid)
  {
    _invoiceNumber->setInvoiceNumber(param.toString());
    _invoiceNumber->setEnabled(FALSE);
    _invoiceList->hide();
  }

  return NoError;
}

void dspInvoiceInformation::sParseInvoiceNumber()
{
  q.prepare( "SELECT invchead_id, invchead_cust_id, invchead_ponumber,"
             "       invchead_shipdate, invchead_invcdate,"
             "       formatMoney((invchead_misc_amount + invchead_freight + invchead_tax + SUM(COALESCE(round((invcitem_billed * invcitem_qty_invuomratio) * (invcitem_price / COALESCE(invcitem_price_invuomratio, 1)),2),0)))) AS f_amount,"
             "       invchead_billto_name, invchead_billto_address1,"
             "       invchead_billto_address2, invchead_billto_address3,"
             "       invchead_billto_city, invchead_billto_state, invchead_billto_zipcode,"
             "       invchead_shipto_name, invchead_shipto_address1,"
             "       invchead_shipto_address2, invchead_shipto_address3,"
             "       invchead_shipto_city, invchead_shipto_state, invchead_shipto_zipcode,"
             "       invchead_notes "
             "FROM invchead LEFT OUTER JOIN"
             "     ( invcitem LEFT OUTER JOIN item"
             "       ON (invcitem_item_id=item_id) )"
             "     ON (invcitem_invchead_id=invchead_id) "
             "WHERE (invchead_invcnumber=:invoiceNumber) "
             "GROUP BY invchead_id, invchead_cust_id, invchead_ponumber,"
             "         invchead_shipdate, invchead_invcdate,"
             "         invchead_misc_amount, invchead_freight, invchead_tax,"
             "         invchead_billto_name, invchead_billto_address1,"
             "         invchead_billto_address2, invchead_billto_address3,"
             "         invchead_billto_city, invchead_billto_state, invchead_billto_zipcode,"
             "         invchead_shipto_name, invchead_shipto_address1,"
             "         invchead_shipto_address2, invchead_shipto_address3,"
             "         invchead_shipto_city, invchead_shipto_state, invchead_shipto_zipcode,"
             "         invchead_notes;" );
  q.bindValue(":invoiceNumber", _invoiceNumber->invoiceNumber());
  q.exec();
  if (q.first())
  {
    _print->setEnabled(TRUE);
    _view->setEnabled(TRUE);

    _invcheadid = q.value("invchead_id").toInt();

    _custPoNumber->setText(q.value("invchead_ponumber").toString());
    _cust->setId(q.value("invchead_cust_id").toInt());
    _invoiceDate->setDate(q.value("invchead_invcdate").toDate());
    _shipDate->setDate(q.value("invchead_shipdate").toDate());
    _invoiceAmount->setText(q.value("f_amount").toString());

    _billToName->setText(q.value("invchead_billto_name"));
    _billToAddress1->setText(q.value("invchead_billto_address1"));
    _billToAddress2->setText(q.value("invchead_billto_address2"));
    _billToAddress3->setText(q.value("invchead_billto_address3"));
    _billToCity->setText(q.value("invchead_billto_city"));
    _billToState->setText(q.value("invchead_billto_state"));
    _billToZip->setText(q.value("invchead_billto_zipcode"));

    _shipToName->setText(q.value("invchead_shipto_name"));
    _shipToAddress1->setText(q.value("invchead_shipto_address1"));
    _shipToAddress2->setText(q.value("invchead_shipto_address2"));
    _shipToAddress3->setText(q.value("invchead_shipto_address3"));
    _shipToCity->setText(q.value("invchead_shipto_city"));
    _shipToState->setText(q.value("invchead_shipto_state"));
    _shipToZip->setText(q.value("invchead_shipto_zipcode"));

    _notes->setText(q.value("invchead_notes").toString());

    q.prepare( "SELECT arapply_id,"
               "       CASE WHEN (arapply_source_doctype = 'C') THEN :creditMemo"
               "            WHEN (arapply_source_doctype = 'R') THEN :cashdeposit"
               "            WHEN (arapply_fundstype='C') THEN :check"
               "            WHEN (arapply_fundstype='T') THEN :certifiedCheck"
               "            WHEN (arapply_fundstype='M') THEN :masterCard"
               "            WHEN (arapply_fundstype='V') THEN :visa"
               "            WHEN (arapply_fundstype='A') THEN :americanExpress"
               "            WHEN (arapply_fundstype='D') THEN :discoverCard"
               "            WHEN (arapply_fundstype='R') THEN :otherCreditCard"
               "            WHEN (arapply_fundstype='K') THEN :cash"
               "            WHEN (arapply_fundstype='W') THEN :wireTransfer"
               "            WHEN (arapply_fundstype='O') THEN :other"
               "       END AS documenttype,"
               "       CASE WHEN (arapply_source_doctype IN ('C','R')) THEN arapply_source_docnumber"
               "            WHEN (arapply_source_doctype = 'K') THEN arapply_refnumber"
               "            ELSE :error"
               "       END AS docnumber,"
               "       arapply_postdate,"
               "       formatMoney(arapply_applied) AS f_amount "
               "FROM arapply "
               "WHERE ( (arapply_target_doctype='I') "
               " AND (arapply_target_docnumber=:aropen_docnumber) ) "
               "ORDER BY arapply_postdate;" );

    q.bindValue(":creditMemo", tr("C/M"));
    q.bindValue(":cashdeposit", tr("Cash Deposit"));
    q.bindValue(":error", tr("Error"));
    q.bindValue(":check", tr("Check"));
    q.bindValue(":certifiedCheck", tr("Certified Check"));
    q.bindValue(":masterCard", tr("Master Card"));
    q.bindValue(":visa", tr("Visa"));
    q.bindValue(":americanExpress", tr("American Express"));
    q.bindValue(":discoverCard", tr("Discover Card"));
    q.bindValue(":otherCreditCard", tr("Other Credit Card"));
    q.bindValue(":cash", tr("Cash"));
    q.bindValue(":wireTransfer", tr("Wire Transfer"));
    q.bindValue(":other", tr("Other"));
    q.bindValue(":aropen_docnumber", _invoiceNumber->invoiceNumber());
    q.exec();
    _arapply->clear();
    _arapply->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _print->setEnabled(FALSE);
    _view->setEnabled(FALSE);
    _invoiceNumber->clear();
    _arapply->clear();
    _invcheadid = -1;
  }
}

void dspInvoiceInformation::sPrint()
{
  ParameterList params;
  params.append("invchead_id", _invcheadid);

  orReport report("InvoiceInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInvoiceInformation::sViewDetails()
{
  invoice::viewInvoice(_invcheadid);
}

void dspInvoiceInformation::sInvoiceList()
{
  ParameterList params;
  params.append("invoiceNumber", _invoiceNumber->invoiceNumber());

  invoiceList newdlg(this, "", TRUE);
  newdlg.set(params);
  int invoiceid = newdlg.exec();

  if (invoiceid > 0)
    _invoiceNumber->setId(invoiceid);
}

