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
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
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

#include "printInvoices.h"

#include <QVariant>
#include <QValidator>
#include <QMessageBox>
#include <QApplication>
#include <openreports.h>
#include <QStatusBar>
#include "editICMWatermark.h"
#include "deliverInvoice.h"
#include "submitAction.h"

/*
 *  Constructs a printInvoices as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printInvoices::printInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_invoiceNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_invoiceWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printInvoices::~printInvoices()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printInvoices::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>

void printInvoices::init()
{
  _invoiceWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _invoiceWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _invoiceWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _firstInvoiceNum->setValidator(omfgThis->orderVal());

  _invoiceNumOfCopies->setValue(_metrics->value("InvoiceCopies").toInt());
  if (_invoiceNumOfCopies->value())
  {
    for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++)
    {
      _invoiceWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(i)));
      _invoiceWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  _firstInvoiceNum->setEnabled(FALSE);
  q.exec( "SELECT orderseq_number "
          "FROM orderseq "
          "WHERE (orderseq_name='InvcNumber')" );
  if (q.first())
    _firstInvoiceNum->setText(q.value("orderseq_number").toString());

  if(!_privleges->check("PostMiscInvoices"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }

  _print->setFocus();
}

void printInvoices::sPrint()
{
  if ( (_metrics->value("InvcNumberGeneration") == "O") ||
       (_metrics->value("InvcNumberGeneration") == "M") )
  {
    q.prepare( "UPDATE orderseq "
               "SET orderseq_number=:invoicenumber "
               "WHERE (orderseq_name='InvcNumber');" );
    q.bindValue(":invoicenumber", _firstInvoiceNum->text().toInt());
    q.exec();
  }

  XSqlQuery invoices;
  invoices.prepare( "SELECT invchead_id, invchead_invcnumber, findCustomerForm(invchead_cust_id, 'I') AS reportname "
                    "FROM invchead " 
                    "WHERE ( (NOT invchead_printed)"
                    " AND (NOT invchead_posted) ) "
                    "ORDER BY invchead_ordernumber" );
  invoices.exec();
  if (invoices.first())
  {
    XSqlQuery local;
    QPrinter  printer(QPrinter::HighResolution);
    bool      setupPrinter = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      int invoiceNumber = invoices.value("invchead_invcnumber").toInt();
      if (invoiceNumber == 0)
      {
        local.prepare("SELECT fetchInvcNumber() AS invoicenumber;");
        local.exec();
        if (local.first())
        {
          invoiceNumber = local.value("invoicenumber").toInt();
          local.prepare( "UPDATE invchead "
                         "SET invchead_invcnumber=:invoicenumber "
                         "WHERE (invchead_id=:invchead_id);" );
          local.bindValue(":invoicenumber", invoiceNumber);
          local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
          local.exec();
        }
        else
          systemError(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__) );
      }

      message( tr("Printing Invoice #%1...")
               .arg(invoiceNumber) );

      for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++ )
      {
	QTreeWidgetItem *cursor = _invoiceWatermarks->topLevelItem(i);
        ParameterList params;
        params.append("invchead_id", invoices.value("invchead_id").toInt());
        params.append("showcosts", ((cursor->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
        params.append("watermark", cursor->text(1));

        orReport report(invoices.value("reportname").toString(), params);

        message( tr("Printing Invoice #%1...")
                 .arg(invoiceNumber) );

        if (!report.isValid())
        {
          QMessageBox::critical( this, tr("Cannot Find Invoice Form"),
                                 tr( "The Invoice Form '%1' cannot be found for Invoice #%2.\n"
                                     "This Invoice cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                     "references to this Invoice Form or this Invoice Form is created." )
                                 .arg(invoices.value("reportname").toString())
                                 .arg(invoiceNumber) );
          
          resetMessage();
        }
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printInvoices::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }

          if (_post->isChecked())
          {
            message( tr("Posting Invoice #%1...")
                     .arg(invoiceNumber) );

            local.prepare("SELECT postInvoice(:invchead_id) AS result;");
            local.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
            local.exec();
	    if (local.lastError().type() != QSqlError::NoError)
		systemError(this, local.lastError().databaseText(), __FILE__, __LINE__);
          }
        }
      }

      if (_metrics->boolean("EnableBatchManager"))
      {
        // TODO: Check for EDI and handle submission to Batch here
        XSqlQuery query;
        query.prepare("SELECT CASE WHEN (COALESCE(shipto_ediprofile_id, -2) = -2)"
                "              THEN COALESCE(cust_ediprofile_id,-1)"
                "            ELSE COALESCE(shipto_ediprofile_id,-2)"
                "       END AS result,"
                "       COALESCE(cust_emaildelivery, false) AS custom"
                "  FROM cust, invchead"
                "       LEFT OUTER JOIN shipto"
                "         ON (invchead_shipto_id=shipto_id)"
                "  WHERE ((invchead_cust_id=cust_id)"
                "    AND  (invchead_id=:invchead_id)); ");
        query.bindValue(":invchead_id", invoices.value("invchead_id").toInt());
        query.exec();
        if(query.first())
        {
          if(query.value("result").toInt() == -1)
          {
            if(query.value("custom").toBool())
            {
              ParameterList params;
              params.append("invchead_id", invoices.value("invchead_id").toInt());
  
              deliverInvoice newdlg(this, "", TRUE);
              newdlg.set(params);
              newdlg.exec();
            }
          }
          else
          {
            ParameterList params;
            params.append("action_name", "TransmitInvoice");
            params.append("invchead_id", invoices.value("invchead_id").toInt());

            submitAction newdlg(this, "", TRUE);
            newdlg.set(params);
            newdlg.exec();
          }
        }
      }

    }
    while (invoices.next());
    orReport::endMultiPrint(&printer);

    resetMessage();

    if (!_post->isChecked())
    {
      if ( QMessageBox::information( this, tr("Mark Invoices as Printed?"),
                                     tr("Did all of the Invoices print correctly?"),
                                     tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
        q.exec( "UPDATE invchead "
                "SET invchead_printed=TRUE "
                "WHERE (NOT invchead_printed);");

        omfgThis->sInvoicesUpdated(-1, TRUE);
    }
  }
  else if (invoices.lastError().type() != QSqlError::NoError)
    systemError(this, invoices.lastError().databaseText(), __FILE__, __LINE__);
  else
    QMessageBox::information( this, tr("No Invoices to Print"),
                              tr("There aren't any Invoices to print.") );

  accept();
}

void printInvoices::sHandleCopies(int pValue)
{
  if (_invoiceWatermarks->topLevelItemCount() > pValue)
    _invoiceWatermarks->takeTopLevelItem(_invoiceWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_invoiceWatermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_invoiceWatermarks,
			  _invoiceWatermarks->topLevelItem(_invoiceWatermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printInvoices::sEditWatermark()
{
  QTreeWidgetItem *cursor = _invoiceWatermarks->currentItem();
  ParameterList params;
  params.append("watermark", cursor->text(1));
  params.append("showPrices", (cursor->text(2) == tr("Yes")));

  editICMWatermark newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? tr("Yes") : tr("No")));
  }
}

