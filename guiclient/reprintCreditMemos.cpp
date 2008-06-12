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

#include "reprintCreditMemos.h"

#include <QVariant>
#include <openreports.h>
#include <QMessageBox>
#include "editICMWatermark.h"

/*
 *  Constructs a reprintCreditMemos as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reprintCreditMemos::reprintCreditMemos(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_numOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleCopies(int)));
    connect(_watermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));
    connect(_cmhead, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reprintCreditMemos::~reprintCreditMemos()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reprintCreditMemos::languageChange()
{
    retranslateUi(this);
}


void reprintCreditMemos::init()
{
  _cmhead->addColumn( tr("C/M #"),     _orderColumn, Qt::AlignRight  );
  _cmhead->addColumn( tr("Doc. Date"), _dateColumn,  Qt::AlignCenter );
  _cmhead->addColumn( tr("Customer"),  -1,           Qt::AlignLeft   );
  _cmhead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _watermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _watermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _watermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _numOfCopies->setValue(_metrics->value("CreditMemoCopies").toInt());
  if (_numOfCopies->value())
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      _watermarks->topLevelItem(i)->setText(1, _metrics->value(QString("CreditMemoWatermark%1").arg(i)));
      _watermarks->topLevelItem(i)->setText(2, ((_metrics->value(QString("CreditMemoShowPrices%1").arg(i)) == "t") ? tr("Yes") : tr("No")));
    }
  }

  _cmhead->populate( "SELECT cmhead_id, cust_id,"
                     "       cmhead_number, formatDate(cmhead_docdate),"
                     "       (TEXT(cust_number) || ' - ' || cust_name) "
                     "FROM cmhead, cust "
                     "WHERE (cmhead_cust_id=cust_id) "
                     "ORDER BY cmhead_docdate DESC;", TRUE );
}

void reprintCreditMemos::sPrint()
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
  QList<QTreeWidgetItem*> selected = _cmhead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    for (int j = 0; j < _watermarks->topLevelItemCount(); j++)
    {
      QTreeWidgetItem *watermark = _watermarks->topLevelItem(j);
      q.prepare("SELECT findCustomerForm(:cust_id, 'C') AS _reportname;");
      q.bindValue(":cust_id", cursor->altId());
      q.exec();
      if (q.first())
      {
	ParameterList params;
	params.append("cmhead_id", cursor->id());
	params.append("showcosts", ((watermark->text(2) == tr("Yes")) ? "TRUE" : "FALSE") );
	params.append("watermark", watermark->text(1));

	orReport report(q.value("_reportname").toString(), params);
	if (report.isValid())
	{
	  if (report.print(&printer, setupPrinter))
	      setupPrinter = FALSE;
	  else
	  {
	    orReport::endMultiPrint(&printer);
	    return;
	  }
	}
	else
	  QMessageBox::critical( this, tr("Cannot Find Credit Memo Form"),
				 tr( "The Invoice Form '%1' cannot be found.\n"
				     "One or more of the selected Credit Memos cannot be printed until a Customer Form Assignment\n"
				     "is updated to remove any references to this Credit Memo Form or this Credit Memo Form is created." )
				 .arg(q.value("_reportname").toString()) );
      }
    }
  }
  orReport::endMultiPrint(&printer);

  _cmhead->clearSelection();
  _close->setText(tr("&Close"));
  _print->setEnabled(FALSE);
}

void reprintCreditMemos::sHandleCopies(int pValue)
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

void reprintCreditMemos::sEditWatermark()
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
