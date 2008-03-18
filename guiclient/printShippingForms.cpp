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

#include "printShippingForms.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include "salesOrderList.h"
#include "editICMWatermark.h"

printShippingForms::printShippingForms(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));
  connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));

  _captive = FALSE;

  _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
  _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
  _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

  _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());
  if (_shipformNumOfCopies->value())
  {
    for (int i = 0; i < _shipformWatermarks->topLevelItemCount(); i++)
    {
      _shipformWatermarks->topLevelItem(i)->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(i)));
      _shipformWatermarks->topLevelItem(i)->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(i))) ? tr("Yes") : tr("No")));
    }
  }

  Preferences _pref = Preferences(omfgThis->username());
  if (_pref.boolean("XCheckBox/forgetful"))
  {
    _printNew->setChecked(true);
    _printDirty->setChecked(true);
  }
}

printShippingForms::~printShippingForms()
{
    // no need to delete child widgets, Qt does it all for us
}

void printShippingForms::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printShippingForms::set(const ParameterList &)
{
  _captive = TRUE;
  return NoError;
}

void printShippingForms::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    for (int i = (_shipformWatermarks->topLevelItemCount() + 1); i <= pValue; i++ )
      new XTreeWidgetItem(_shipformWatermarks,
			  _shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1),
			  i, i, tr("Copy #%1").arg(i), "", tr("Yes"));
  }
}

void printShippingForms::sEditShippingFormWatermark()
{
  QTreeWidgetItem *cursor = _shipformWatermarks->currentItem();
  if (cursor)
  {
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
}

void printShippingForms::sPrint()
{
  if (!_printNew->isChecked() && !_printDirty->isChecked())
  {
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("You must indicate if you wish to print Shipping Forms for New and/or Changed Shipments.") );
    return;
  }

  QString sql( "SELECT shiphead_id, report_name "
               "FROM shiphead, cohead, shipform, report "
               "WHERE ( (NOT shiphead_shipped)"
               " AND (shiphead_order_id=cohead_id)"
	       " AND (shiphead_order_type='SO')"
               " AND (shipform_report_id=report_id)"
               " AND (shipform_id=COALESCE(shiphead_shipform_id, cohead_shipform_id))"
               " AND (shiphead_sfstatus IN (" );

  if (_printNew->isChecked())
  {
    sql += "'N'";

    if (_printDirty->isChecked())
      sql += ", 'D'";
  }
  else if (_printDirty->isChecked())
    sql += "'D'";
  sql += ")) ) "
         "ORDER BY shiphead_id;";

  XSqlQuery reports;
  reports.exec(sql);
  if (reports.first())
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

    do
    {
      for (int i = 0; i < _shipformWatermarks->topLevelItemCount(); i++ )
      {
        ParameterList params;

        params.append("cosmisc_id", reports.value("shiphead_id").toInt());
        params.append("shiphead_id", reports.value("shiphead_id").toInt());

        params.append("watermark", _shipformWatermarks->topLevelItem(i)->text(1));

#if 0
        params.append("shipchrg_id", _shipchrg->id());

#endif

        if (_shipformWatermarks->topLevelItem(i)->text(2) == tr("Yes"))
          params.append("showcosts");

        orReport report(reports.value("report_name").toString(), params);
        if (report.print(&printer, setupPrinter))
          setupPrinter = FALSE;
        else
        {
          report.reportError(this);
	  orReport::endMultiPrint(&printer);
          return;
        }
      }

      XSqlQuery setStatus;
      setStatus.prepare( "UPDATE shiphead "
                         "SET shiphead_sfstatus='P' "
                         "WHERE (shiphead_id=:shiphead_id);" );
      setStatus.bindValue(":shiphead_id", reports.value("shiphead_id").toInt());
      setStatus.exec();
    }
    while (reports.next());
    orReport::endMultiPrint(&printer);

    if (_captive)
      accept();
  }
  else
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("There are no New or Changed Shipments for which Shipping Forms should be printed.") );

}
