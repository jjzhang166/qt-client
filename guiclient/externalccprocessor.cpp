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

#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "externalccprocessor.h"
#include "externalCCTransaction.h"

#define DEBUG true

ExternalCCProcessor::ExternalCCProcessor() : CreditCardProcessor()
{
  _defaultLivePort   = 0;
  _defaultLiveServer = "";
  _defaultTestPort   = 0;
  _defaultTestServer = "";
}

int  ExternalCCProcessor::doAuthorize(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "A", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  ExternalCCProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "C", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount,  pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "CP", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder), pccpayid);

  int returnValue = handleTrans(pccardid, "R", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   qPrintable(pneworder), qPrintable(preforder),
	   qPrintable(papproval), pccpayid);

  int returnValue = handleTrans(pccardid, "V", pcvv, pamount, pcurrid,
                                pneworder, preforder, pccpayid, pparams);
  return returnValue;
}

int ExternalCCProcessor::handleTrans(const int pccardid, const QString &ptype, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("ExtCC::handleTrans(%d, %s, %d, %f, %d, %s, %s, %d, pparams)",
	   pccardid, qPrintable(ptype), pcvv, pamount, pcurrid,
           qPrintable(pneworder), qPrintable(preforder), pccpayid);

  // TODO: if check and not credit card transaction do something else
  XSqlQuery extq;
  extq.prepare(
    "SELECT ccard_active,"
    "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(:key),'bf')) AS ccard_number_x,"
    "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(:key),'bf')) AS ccard_month_expired,"
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired,"
    "  ccard_cust_id "
    "  FROM ccard "
    "WHERE (ccard_id=:ccardid);");
  extq.bindValue(":ccardid", pccardid);
  extq.bindValue(":key",     omfgThis->_key);
  extq.exec();

  if (extq.first())
  {
    if (!extq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (extq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = extq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  pparams.append("cust_id",        extq.value("ccard_cust_id"));
  pparams.append("ccard_number_x", extq.value("ccard_number_x"));
  pparams.append("ccard_exp",      QDate(extq.value("ccard_year_expired").toInt(),
                                   extq.value("ccard_month_expired").toInt(), 1));
  pparams.append("ccard_id",   pccardid);
  pparams.append("currid",     pcurrid);
  pparams.append("amount",     pamount);
  pparams.append("auth_charge",ptype);
  pparams.append("type",       ptype);
  pparams.append("reforder",   (preforder.isEmpty()) ? pneworder : preforder);
  pparams.append("ordernum",   pneworder);
  pparams.append("auth",       QVariant(ptype == "A"));

  externalCCTransaction newdlg(0, "", true);
  newdlg.set(pparams);
  int returnValue = 0;
  if (newdlg.exec() == QDialog::Rejected)
  {
    if (ptype == "A")
      returnValue = 20;
    else if (ptype == "C")
      returnValue = 40;
    else if (ptype == "CP")
      returnValue = 30;
    else if (ptype == "R")
      returnValue = 50;
    else if (ptype == "V")
      returnValue = 60;
    else
      returnValue = -19;

    _errorMsg = errorMsg(returnValue).arg(ptype);
  }
  else
  {
    newdlg.getResults(pparams);

    QVariant param;
    bool valid;

    param = pparams.value("avs", &valid);
    if (valid)
      _passedAvs = param.toBool();

    param = pparams.value("cvv", &valid);
    if (valid)
      _passedCvv = param.toBool();

    param = pparams.value("approved", &valid);
    if (valid && param.toString() == "ERROR")
    {
      _errorMsg = errorMsg(-12).arg(tr("User reported that an error occurred."));
      returnValue = -12;
    }
  }

  if (DEBUG)
    qDebug("ExtCC::handleTrans returning %d %s",
           returnValue, qPrintable(errorMsg()));
  return returnValue;
}

bool ExternalCCProcessor::handlesCreditCards()
{
  return true;
}
