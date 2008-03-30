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

#include "configureSR.h"

#include <QSqlError>
#include <QVariant>

#include "editICMWatermark.h"

configureSR::configureSR(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_shipformWatermarks, SIGNAL(itemSelected(int)), this, SLOT(sEditShippingFormWatermark()));
    connect(_shipformNumOfCopies, SIGNAL(valueChanged(int)), this, SLOT(sHandleShippingFormCopies(int)));

    QString metric = _metrics->value("ShipmentNumberGeneration");
    if (metric == "A")
      _shipmentNumGeneration->setCurrentItem(0);

    _nextShipmentNum->setValidator(omfgThis->orderVal());
    q.exec("SELECT setval('shipment_number_seq', nextval('shipment_number_seq') -1); "
           "SELECT currval('shipment_number_seq') AS shipment_number;");
    if (q.first())
      _nextShipmentNum->setText(q.value("shipment_number"));
    else if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    _shipformWatermarks->addColumn( tr("Copy #"),      _dateColumn, Qt::AlignCenter );
    _shipformWatermarks->addColumn( tr("Watermark"),   -1,          Qt::AlignLeft   );
    _shipformWatermarks->addColumn( tr("Show Prices"), _dateColumn, Qt::AlignCenter );

    _shipformNumOfCopies->setValue(_metrics->value("ShippingFormCopies").toInt());

    if (_shipformNumOfCopies->value())
    {
      for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
      {
	QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
	cursor->setText(1, _metrics->value(QString("ShippingFormWatermark%1").arg(counter)));
	cursor->setText(2, ((_metrics->boolean(QString("ShippingFormShowPrices%1").arg(counter))) ? tr("Yes") : tr("No")));
      }
    }

    _disallowReceiptExcess->setChecked(_metrics->boolean("DisallowReceiptExcessQty"));
    _warnIfReceiptDiffers->setChecked(_metrics->boolean("WarnIfReceiptQtyDiffers"));

    _tolerance->setValidator(omfgThis->percentVal());
    _tolerance->setText(_metrics->value("ReceiptQtyTolerancePct"));
}

configureSR::~configureSR()
{
    // no need to delete child widgets, Qt does it all for us
}

void configureSR::languageChange()
{
    retranslateUi(this);
}

void configureSR::sSave()
{
  char *numberGenerationTypes[] = { "A" };

  _metrics->set("ShipmentNumberGeneration", QString(numberGenerationTypes[_shipmentNumGeneration->currentItem()]));

  _metrics->set("ShippingFormCopies", _shipformNumOfCopies->value());

  if (_shipformNumOfCopies->value())
  {
    for (int counter = 0; counter < _shipformWatermarks->topLevelItemCount(); counter++)
    {
      QTreeWidgetItem *cursor = _shipformWatermarks->topLevelItem(counter);
      _metrics->set(QString("ShippingFormWatermark%1").arg(counter), cursor->text(1));
      _metrics->set(QString("ShippingFormShowPrices%1").arg(counter), (cursor->text(2) == tr("Yes")));
    }
  }

  _metrics->set("DisallowReceiptExcessQty", _disallowReceiptExcess->isChecked());
  _metrics->set("WarnIfReceiptQtyDiffers", _warnIfReceiptDiffers->isChecked());
  _metrics->set("ReceiptQtyTolerancePct", _tolerance->text());

  q.prepare("SELECT setval('shipment_number_seq', :shipmentnumber);");
  q.bindValue(":shipmentnumber", _nextShipmentNum->text().toInt());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _metrics->load();

  accept();
}

void configureSR::sHandleShippingFormCopies(int pValue)
{
  if (_shipformWatermarks->topLevelItemCount() > pValue)
    _shipformWatermarks->takeTopLevelItem(_shipformWatermarks->topLevelItemCount() - 1);
  else
  {
    for (unsigned int counter = (_shipformWatermarks->topLevelItemCount() + 1); counter <= (unsigned int)pValue; counter++)
      new XTreeWidgetItem(_shipformWatermarks,
			  (XTreeWidgetItem*)(_shipformWatermarks->topLevelItem(_shipformWatermarks->topLevelItemCount() - 1)),
			  counter,
			  QVariant(tr("Copy #%1").arg(counter)), "", tr("Yes"));
  }
}

void configureSR::sEditShippingFormWatermark()
{
  XTreeWidgetItem *cursor = (XTreeWidgetItem*)(_shipformWatermarks->currentItem());

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
