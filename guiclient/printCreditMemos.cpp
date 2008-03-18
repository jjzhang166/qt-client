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

#include "printCreditMemos.h"

#include <QVariant>
#include <QValidator>
#include <QMessageBox>
#include <QApplication>
#include <parameter.h>
#include <openreports.h>
#include "OpenMFGGUIClient.h"
#include "editICMWatermark.h"

/*
 *  Constructs a printCreditMemos as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printCreditMemos::printCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_creditMemoNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_creditMemoWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printCreditMemos::~printCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printCreditMemos::languageChange()
{
    retranslateUi(this);
}


void printCreditMemos::init()
{
  _creditMemoWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _creditMemoWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _creditMemoWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );
  
  _creditMemoNumOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt());
  if (_creditMemoNumOfCopies->value())
  {
    for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++)
    {
      _creditMemoWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
      _creditMemoWatermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  if(!_privleges->check("PostARDocuments"))
  {
    _post->setChecked(false);
    _post->setEnabled(false);
  }
  
  _print->setFocus();
}

void printCreditMemos::sPrint()
{
  XSqlQuery cmhead( "SELECT cmhead_id, cmhead_number,"
                    "       findCustomerForm(cmhead_cust_id, 'C') AS _reportname "
                    "FROM cmhead "
                    "WHERE ( (NOT cmhead_hold)"
                    " AND (NOT cmhead_printed) );");
  if (cmhead.first())
  {
    QPrinter printer(QPrinter::HighResolution);
    bool     setupPrinter  = TRUE;
    int      itemlocSeries = 0;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      for (int i = 0; i < _creditMemoWatermarks->topLevelItemCount(); i++ )
      {
        ParameterList params;

        params.append("cmhead_id", cmhead.value("cmhead_id").toInt());
        params.append("showcosts", ((_creditMemoWatermarks->topLevelItem(i)->text(2) == tr("Yes")) ? "TRUE" : "FALSE"));
        params.append("watermark", _creditMemoWatermarks->topLevelItem(i)->text(1));

        orReport report(cmhead.value("_reportname").toString(), params);
        if (!report.isValid())
          QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
                                 tr( "The Credit Memo Form '%1' for Credit Memo #%2 cannot be found.\n"
                                     "This Credit Memo cannot be printed until Customer Form Assignments are updated to remove\n"
                                     "any references to this Credit Memo Form or this Credit Memo Form is created." )
                                 .arg(cmhead.value("_reportname").toString())
                                 .arg(cmhead.value("cmhead_number").toString()) );
        else
        {
          if (report.print(&printer, setupPrinter))
            setupPrinter = FALSE;
          else
          {
            systemError( this, tr("A Printing Error occurred at printCreditMemos::%1.")
                               .arg(__LINE__) );
	    orReport::endMultiPrint(&printer);
            return;
          }
        }

        if (_post->isChecked())
        {
          XSqlQuery postCreditMemo;
          postCreditMemo.prepare("SELECT postCreditMemo(:cmhead_id, :itemlocSeries) AS result;");
          postCreditMemo.bindValue(":cmhead_id", cmhead.value("cmhead_id").toInt());
          postCreditMemo.bindValue(":itemlocSeries", itemlocSeries);
          postCreditMemo.exec();
          if (postCreditMemo.first())
            itemlocSeries = postCreditMemo.value("result").toInt();
          else
            systemError(this, tr("A System Error occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__) );
        }
      }
    }
    while (cmhead.next());
    orReport::endMultiPrint(&printer);

    if ( QMessageBox::information( this, tr("Mark Credit Memos as Printed?"),
                                   tr("Did all of the Credit Memos print correctly?"),
                                   tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
      XSqlQuery().exec( "UPDATE cmhead "
                        "SET cmhead_printed=TRUE "
                        "WHERE (NOT cmhead_printed);" );

    omfgThis->sCreditMemosUpdated();
  }
  else
    QMessageBox::information( this, tr("No Credit Memos to Print"),
                              tr("There aren't any Credit Memos to print.") );

  accept();
}

void printCreditMemos::sHandleCopies(int pValue)
{
  if (_creditMemoWatermarks->topLevelItemCount() > pValue)
    _creditMemoWatermarks->takeTopLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_creditMemoWatermarks->topLevelItemCount() + 1); i <= pValue; i++)
      new XTreeWidgetItem(_creditMemoWatermarks,
			  _creditMemoWatermarks->topLevelItem(_creditMemoWatermarks->topLevelItemCount() - 1),
			  i, i,
			  tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printCreditMemos::sEditWatermark()
{
  QTreeWidgetItem *cursor = _creditMemoWatermarks->currentItem();
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

