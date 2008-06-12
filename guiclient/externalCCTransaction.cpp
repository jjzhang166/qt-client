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

#include "externalCCTransaction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QTextStream>
#include <QFileDialog>
#include <QFile>

externalCCTransaction::externalCCTransaction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
}

externalCCTransaction::~externalCCTransaction()
{
  // no need to delete child widgets, Qt does it all for us
}

void externalCCTransaction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse externalCCTransaction::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("ccard_number_x", &valid);
  if (valid)
    _ccard->setText(param.toString());

  param = pParams.value("currid", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("amount", &valid);
  if (valid)
    _amount->setLocalValue(param.toDouble());

  param = pParams.value("type", &valid);
  if (valid)
  {
    if (param.toString() == "A")
      _transType->setCurrentIndex(0);
    else if (param.toString() == "C")
      _transType->setCurrentIndex(1);
    else if (param.toString() == "CP")
      _transType->setCurrentIndex(2);
    else if (param.toString() == "R")
      _transType->setCurrentIndex(3);
    else if (param.toString() == "V")
      _transType->setCurrentIndex(4);
  }

  param = pParams.value("reforder", &valid);
  if (valid)
    _order->setText(param.toString());

  param = pParams.value("ordernum", &valid);
  if (valid)
    _transactionId->setText(param.toString());

  return NoError;
}

void externalCCTransaction::getResults(ParameterList &pParams)
{
  QVariant amount = pParams.value("amount");
  int amountIndex;
  for (amountIndex = 0; amountIndex < pParams.size(); amountIndex++)
    if (pParams.name(amountIndex) == "amount")
      break;

  switch (_approved->currentIndex())
  {
    case 0:
      pParams.append("approved", "APPROVED");
      if (_transType->currentIndex() == 0)
        pParams.append("status", "A");
      else if (_transType->currentIndex() == 4)
        pParams.append("status", "V");
      else
        pParams.append("status", "C");
      break;
    case 1:
      pParams.append("approved", "DECLINED");
      pParams.append("status",   "D");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",   "0");
      break;
    case 2:
      pParams.append("approved", "ERROR");
      pParams.append("status",   "X");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",   "0");
      break;
    case 3:
      pParams.append("approved", "HELDFORREVIEW");
      if (_transType->currentIndex() == 0)
        pParams.append("status", "A");
      else if (_transType->currentIndex() == 4)
        pParams.append("status", "V");
      else
        pParams.append("status", "C");
      break;
    default:
      pParams.append("status",  "X");
      if (amountIndex < pParams.size())
        pParams.removeAt(amountIndex);
      pParams.append("amount",  "0");
  }

  pParams.append("code",     _approvalCode->text());
  pParams.append("avs",      QVariant(_passedAVS->isChecked()));
  pParams.append("xactionid",_transactionId->text());
  pParams.append("cvv",      QVariant(_passedCVV->isChecked()));
  //pParams.append("error",    );
  //pParams.append("shipping",    );
  //pParams.append("tax",     );
  //pParams.append("ref",     );
  //pParams.append("message", );
}
