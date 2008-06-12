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

#include "printInvoice.h"

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
 *  Constructs a printInvoice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printInvoice::printInvoice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
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
printInvoice::~printInvoice()
{
  if (_captive)	// see sPrint()
    orReport::endMultiPrint(&_printer);
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printInvoice::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>

void printInvoice::init()
{
  _captive = FALSE;
  _setup   = FALSE;
  _alert   = TRUE;

  _cust->setReadOnly(TRUE);

  _invoiceWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _invoiceWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _invoiceWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _invoiceNumOfCopies->setValue(_metrics->value("InvoiceCopies").toInt());
  if (_invoiceNumOfCopies->value())
  {
    for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++)
    {
      _invoiceWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("InvoiceWatermark%1").arg(i)));
      _invoiceWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("InvoiceShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  if(!_privileges->check("PostMiscInvoices"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }

  _print->setFocus();
}

enum SetResponse printInvoice::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("invchead_id", &valid);
  if (valid)
  {
    _invcheadid = param.toInt();
    populate();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  if (pParams.inList("persistentPrint"))
  {
    _alert = FALSE;

    if (_setup)
    {
      sPrint();
      return NoError_Print;
    }
    else
      return Error_NoSetup;
  }

  return NoError;
}

void printInvoice::sPrint()
{
  q.prepare( "SELECT invchead_invcnumber, invchead_printed, invchead_posted,"
             "       findCustomerForm(invchead_cust_id, 'I') AS reportname "
             "FROM invchead "
             "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    XSqlQuery xrate;
    xrate.prepare("SELECT curr_rate "
		  "FROM curr_rate, invchead "
		  "WHERE ((curr_id=invchead_curr_id)"
		  "  AND  (invchead_id=:invchead_id)"
		  "  AND  (invchead_invcdate BETWEEN curr_effective AND curr_expires));");
    // if SUM becomes dependent on curr_id then move XRATE before it in the loop
    XSqlQuery sum;
    sum.prepare("SELECT COALESCE(SUM(round((invcitem_billed * invcitem_qty_invuomratio) *"
                "                 (invcitem_price / "
                "                  CASE WHEN (item_id IS NULL) THEN 1"
                "                       ELSE invcitem_price_invuomratio"
                "                  END), 2)),0) + "
                "       invchead_freight + invchead_tax + "
                "       invchead_misc_amount AS subtotal "
                "  FROM invchead LEFT OUTER JOIN invcitem ON (invcitem_invchead_id=invchead_id) LEFT OUTER JOIN"
                "       item ON (invcitem_item_id=item_id) "
                " WHERE(invchead_id=:invchead_id) "
                " GROUP BY invchead_freight, invchead_tax, invchead_misc_amount;");
    QString reportname = q.value("reportname").toString();
    QString invchead_invcnumber = q.value("invchead_invcnumber").toString();

    bool dosetup = (!_setup);
    if (dosetup)
    {
      bool userCanceled = false;
      if (orReport::beginMultiPrint(&_printer, userCanceled) == false)
      {
	if(!userCanceled)
          systemError(this, tr("Could not initialize printing system for multiple reports."));
	return;
      }
    }

    // TODO: do we really want error checking & invoice posting INSIDE the loop?
    for (int i = 0; i < _invoiceWatermarks->topLevelItemCount(); i++ )
    {
      QTreeWidgetItem *cursor = _invoiceWatermarks->topLevelItem(i);
      ParameterList params;
      params.append("invchead_id", _invcheadid);
      params.append("showcosts", ((cursor->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
      params.append("watermark", cursor->text(1));

      orReport report(reportname, params);
      if (!report.isValid())
        QMessageBox::critical( this, tr("Cannot Find Invoice Form"),
                               tr( "The Invoice Form '%1' cannot be found for Invoice #%2.\n"
                                   "This Invoice cannot be printed until a Customer Form Assignment is updated to remove any\n"
                                   "references to this Invoice Form or this Invoice Form is created." )
                               .arg(reportname)
                               .arg(invchead_invcnumber) );
          
      else
      {
        if (report.print(&_printer, dosetup))
          dosetup = FALSE;
        else
        {
          systemError( this, tr("A Printing Error occurred at printInvoice::%1.")
                             .arg(__LINE__) );
	  if (!_captive)
	    orReport::endMultiPrint(&_printer);
          return;
        }

        q.prepare( "UPDATE invchead "
                   "SET invchead_printed=TRUE "
                   "WHERE (invchead_id=:invchead_id);" );
        q.bindValue(":invchead_id", _invcheadid);
        q.exec();

        if (_alert)
          omfgThis->sInvoicesUpdated(_invcheadid, TRUE);

        if (_post->isChecked())
        {
	  sum.bindValue(":invchead_id", _invcheadid);
	  if (sum.exec() && sum.first() && sum.value("subtotal").toDouble() == 0)
	  {
	    if (QMessageBox::question(this, tr("Invoice Has Value 0"),
				      tr("Invoice #%1 has a total value of 0.\n"
					 "Would you like to post it anyway?")
					.arg(invchead_invcnumber),
				      QMessageBox::Yes,
				      QMessageBox::No | QMessageBox::Default)
		  == QMessageBox::No)
	      continue;
	  }
	  else if (sum.lastError().type() != QSqlError::NoError)
	  {
	    systemError(this, sum.lastError().databaseText(), __FILE__, __LINE__);
	    continue;
	  }
	  else if (sum.value("subtotal").toInt() != 0)
	  {
	    xrate.bindValue(":invchead_id", _invcheadid);
	    xrate.exec();
	    if (xrate.lastError().type() != QSqlError::NoError)
	    {
	      systemError(this, tr("System Error posting Invoice #%1\n%2")
				  .arg(cursor->text(0))
				  .arg(xrate.lastError().databaseText()),
			  __FILE__, __LINE__);
	      continue;
	    }
	    else if (!xrate.first() || xrate.value("curr_rate").isNull())
	    {
	      systemError(this, tr("Could not post Invoice #%1 because of a missing exchange rate.")
				  .arg(cursor->text(0)));
	      continue;
	    }
	  }

          q.prepare("SELECT postInvoice(:invchead_id) AS result;");
          q.bindValue(":invchead_id", _invcheadid);
          q.exec();
	  if (q.lastError().type() != QSqlError::NoError)
	      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        }
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
      q.bindValue(":invchead_id", _invcheadid);
      q.exec();
      if(q.first())
      {
        if(q.value("result").toInt() == -1)
        {
          if(q.value("custom").toBool())
          {
            ParameterList params;
            params.append("invchead_id", _invcheadid);

            deliverInvoice newdlg(this, "", TRUE);
            newdlg.set(params);
            newdlg.exec();
          }
        }
        else
        {
          ParameterList params;
          params.append("action_name", "TransmitInvoice");
          params.append("invchead_id", _invcheadid);

          submitAction newdlg(this, "", TRUE);
          newdlg.set(params);
          newdlg.exec();
        }
      }
    }
    
    if (_captive)
      accept();
    else
      orReport::endMultiPrint(&_printer);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void printInvoice::sHandleCopies(int pValue)
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

void printInvoice::sEditWatermark()
{
  QTreeWidgetItem *cursor = _invoiceWatermarks->currentItem();
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

void printInvoice::populate()
{
  q.prepare( "SELECT invchead_invcnumber, invchead_cust_id, invchead_posted "
             "FROM invchead "
             "WHERE (invchead_id=:invchead_id);" );
  q.bindValue(":invchead_id", _invcheadid);
  q.exec();
  if (q.first())
  {
    _invoiceNum->setText(q.value("invchead_invcnumber").toString());
    _cust->setId(q.value("invchead_cust_id").toInt());
    if (q.value("invchead_posted").toBool())
    {
      _post->setChecked(FALSE);
      _post->hide();
    }
    else
      _post->show();
  }
}

bool printInvoice::isSetup()
{
  return _setup;
}

void printInvoice::setSetup(bool pSetup)
{
  _setup = pSetup;
}

