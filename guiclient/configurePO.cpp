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

#include "configurePO.h"

#include <QVariant>
#include <QValidator>

/*
 *  Constructs a configurePO as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configurePO::configurePO(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(released()), this, SLOT(reject()));
  connect(_internalCopy, SIGNAL(toggled(bool)), _numOfCopies, SLOT(setEnabled(bool)));

  if (_metrics->value("PONumberGeneration") == "M")
    _orderNumGeneration->setCurrentItem(0);
  else if (_metrics->value("PONumberGeneration") == "A")
    _orderNumGeneration->setCurrentItem(1);
  else if (_metrics->value("PONumberGeneration") == "O")
    _orderNumGeneration->setCurrentItem(2);

#if 0
  if (_metrics->value("VoucherNumberGeneration") == "M")
    _voucherNumGeneration->setCurrentItem(0);
  else if (_metrics->value("VoucherNumberGeneration") == "A")
    _voucherNumGeneration->setCurrentItem(1);
  else if (_metrics->value("VoucherNumberGeneration") == "O")
    _voucherNumGeneration->setCurrentItem(2);
#endif
//  ToDo
 
  if (_metrics->value("PrNumberGeneration") == "M")
    _prNumberGeneration->setCurrentItem(0);
  else if (_metrics->value("PrNumberGeneration") == "A")
    _prNumberGeneration->setCurrentItem(1);
  else if (_metrics->value("PrNumberGeneration") == "O")
    _prNumberGeneration->setCurrentItem(2);

  _nextPoNumber->setValidator(omfgThis->orderVal());
  _nextVcNumber->setValidator(omfgThis->orderVal());
  _nextPrNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT ponumber.orderseq_number AS ponumber,"
          "       vcnumber.orderseq_number AS vcnumber,"
          "       prnumber.orderseq_number AS prnumber "
          "FROM orderseq AS ponumber,"
          "     orderseq AS vcnumber,"
          "     orderseq AS prnumber "
          "WHERE ( (ponumber.orderseq_name='PoNumber')"
          " AND (vcnumber.orderseq_name='VcNumber')"
          " AND (prnumber.orderseq_name='PrNumber') );" );
  if (q.first())
  {
    _nextPoNumber->setText(q.value("ponumber").toString());
    _nextVcNumber->setText(q.value("vcnumber").toString());
    _nextPrNumber->setText(q.value("prnumber").toString());
  }

  _poChangeLog->setChecked(_metrics->boolean("POChangeLog"));
  _vendorChangeLog->setChecked(_metrics->boolean("VendorChangeLog"));
  _earliestPO->setChecked(_metrics->boolean("UseEarliestAvailDateOnPOItem"));
  _printPO->setChecked(_metrics->boolean("DefaultPrintPOOnSave"));

  _vendorCopy->setChecked(_metrics->boolean("POVendor"));
  _requirePoitemStdCost->setChecked(_metrics->boolean("RequireStdCostForPOItem"));

  if (_metrics->value("POInternal").toInt() > 0)
  {
    _internalCopy->setChecked(TRUE);
    _numOfCopies->setValue(_metrics->value("POInternal").toInt());
  }
  else
  {
    _internalCopy->setChecked(FALSE);
    _numOfCopies->setEnabled(FALSE);
  }

  _defaultShipVia->setText(_metrics->value("DefaultPOShipVia"));

  this->setCaption("Purchase Configuration");
}

/*
 *  Destroys the object and frees any allocated resources
 */
configurePO::~configurePO()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configurePO::languageChange()
{
  retranslateUi(this);
}

void configurePO::sSave()
{
  if (_orderNumGeneration->currentItem() == 0)
    _metrics->set("PONumberGeneration", QString("M"));
  else if (_orderNumGeneration->currentItem() == 1)
    _metrics->set("PONumberGeneration", QString("A"));
  else if (_orderNumGeneration->currentItem() == 2)
    _metrics->set("PONumberGeneration", QString("O"));

  _metrics->set("VoucherNumberGeneration", QString("A"));

  if (_prNumberGeneration->currentItem() == 0)
    _metrics->set("PrNumberGeneration", QString("M"));
  if (_prNumberGeneration->currentItem() == 1)
    _metrics->set("PrNumberGeneration", QString("A"));
  if (_prNumberGeneration->currentItem() == 2)
    _metrics->set("PrNumberGeneration", QString("O"));

  _metrics->set("POChangeLog", _poChangeLog->isChecked());
  _metrics->set("VendorChangeLog", _vendorChangeLog->isChecked());
  _metrics->set("UseEarliestAvailDateOnPOItem", _earliestPO->isChecked());
  _metrics->set("DefaultPrintPOOnSave", _printPO->isChecked());
 
  _metrics->set("POVendor", _vendorCopy->isChecked());
  _metrics->set("RequireStdCostForPOItem", _requirePoitemStdCost->isChecked());
  _metrics->set("POInternal", ((_internalCopy->isChecked()) ? _numOfCopies->value() : 0) );
  _metrics->set("DefaultPOShipVia", _defaultShipVia->text().stripWhiteSpace());

  q.prepare("SELECT setNextPoNumber(:poNumber), setNextVcNumber(:vcNumber), setNextPrNumber(:prNumber);");
  q.bindValue(":poNumber", _nextPoNumber->text().toInt());
  q.bindValue(":vcNumber", _nextVcNumber->text().toInt());
  q.bindValue(":prNumber", _nextPrNumber->text().toInt());
  q.exec();

  _metrics->load();

  accept();
}

