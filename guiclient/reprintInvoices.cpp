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

#include "reprintInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>

#include "editICMWatermark.h"
#include "deliverInvoice.h"
#include "submitAction.h"

reprintInvoices::reprintInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));
  connect(_numOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleInvoiceCopies(int)));
  connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));

  _invoice->addColumn( tr("Invoice #"),    _orderColumn,    Qt::AlignRight, true, "invchead_invcnumber");
  _invoice->addColumn( tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter,true, "invchead_invcdate");
  _invoice->addColumn( tr("Customer"),     -1,              Qt::AlignLeft,  true, "customer");
  _invoice->addColumn( tr("Total Amount"), _bigMoneyColumn, Qt::AlignRight, true, "extprice" );
  _invoice->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _numOfCopies->setValue(_metrics->value("InvoiceCopies").toInt());
  if (_numOfCopies->value())
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      _watermarks->topLevelItem(i)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(i)));
      _watermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }
}

reprintInvoices::~reprintInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintInvoices::languageChange()
{
  retranslateUi(this);
}

void reprintInvoices::sQuery()
{
  MetaSQLQuery mql = mqlLoad("invoices", "detail");
  ParameterList params;
  params.append("invc_pattern", _invoicePattern->text().trimmed());
  _dates->appendValue(params);
  q = mql.toQuery(params);
  _invoice->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void reprintInvoices::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool     setupPrinter = TRUE;

  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  QList<QTreeWidgetItem*> selected = _invoice->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];

    for (int j = 0; j < _watermarks->topLevelItemCount(); j++)
    {
      q.prepare("SELECT findCustomerForm(:cust_id, 'I') AS _reportname;");
      q.bindValue(":cust_id", cursor->altId());
      q.exec();
      if (q.first())
      {
	ParameterList params;
	params.append("invchead_id", cursor->id());
	params.append("showcosts", ((_watermarks->topLevelItem(j)->text(2) == tr("Yes")) ? "TRUE" : "FALSE") );
	params.append("watermark", _watermarks->topLevelItem(j)->text(1));

	orReport report(q.value("_reportname").toString(), params);
	if (report.isValid())
	{
	  if (report.print(&printer, setupPrinter))
		 setupPrinter = FALSE;
	      else 
	      {
		report.reportError(this);
		orReport::endMultiPrint(&printer);
		return;
	      }
	}
	else
	  QMessageBox::critical(this, tr("Cannot Find Invoice Form"),
				tr("<p>The Invoice Form '%1' cannot be found. "
                                   "One or more of the selected Invoices "
                                   "cannot be printed until a Customer Form "
                                   "Assignment is updated to remove any "
                                   "references to this Invoice Form or this "
                                   "Invoice Form is created.")
                                  .arg(q.value("_reportname").toString()) );
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }
    if (_metrics->boolean("EnableBatchManager"))
    {
      // TODO: Check for EDI and handle submission to Batch here
      q.prepare("SELECT CASE WHEN (COALESCE(shipto_ediprofile_id, -2) = -2)"
	      "              THEN COALESCE(cust_ediprofile_id,-1)"
	      "            ELSE COALESCE(shipto_ediprofile_id,-2)"
	      "       END AS result,"
	      "       COALESCE(cust_emaildelivery, false) AS custom"
	      "  FROM cust, invchead"
	      "       LEFT OUTER JOIN shipto"
	      "         ON (invchead_shipto_id=shipto_id)"
	      "  WHERE ((invchead_cust_id=cust_id)"
	      "    AND  (invchead_id=:invchead_id)); ");
      q.bindValue(":invchead_id", cursor->id());
      q.exec();
      if(q.first())
      {
	if(q.value("result").toInt() == -1)
	{
	  if(q.value("custom").toBool())
	  {
	    ParameterList params;
	    params.append("invchead_id", cursor->id());
  
	    deliverInvoice newdlg(this, "", TRUE);
	    newdlg.set(params);
	    newdlg.exec();
	  }
	}
	else
	{
	  ParameterList params;
	  params.append("action_name", "TransmitInvoice");
	  params.append("invchead_id", cursor->id());
  
	  submitAction newdlg(this, "", TRUE);
	  newdlg.set(params);
	  newdlg.exec();
	}
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  orReport::endMultiPrint(&printer);

  _invoice->clearSelection();
  _close->setText(tr("&Close"));
  _print->setEnabled(FALSE);
}

void reprintInvoices::sHandleInvoiceCopies(int pValue)
{
  if (_watermarks->topLevelItemCount() > pValue)
    _watermarks->takeTopLevelItem(_watermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_watermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_watermarks,
			  _watermarks->topLevelItem(_watermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void reprintInvoices::sEditWatermark()
{
  QTreeWidgetItem *cursor = _watermarks->currentItem();
  ParameterList params;
  params.append("watermark", cursor->text(1));
  params.append("showPrices", (cursor->text(2) == tr("Yes")));

  editICMWatermark newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == XDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
  }
}
