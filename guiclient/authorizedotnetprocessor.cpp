/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/* TODO:
   support Visa ECI, MasterCard UCAF for x_authentication_indicator?
   support Visa CAVV or MasterCard UCAF for x_cardholder_authentication_value?
   switch completely to the API @ http://developer.authorize.net/api/reference/
*/
/** @ingroup creditcards
    @class   AuthorizeDotNetProcessor
    @brief   The implementation of Authorize.Net-specific credit card handling
 */

#include <QDebug>
#include <QSqlError>

#include <currcluster.h>

#include "guiclient.h"
#include "authorizedotnetprocessor.h"

#define DEBUG false

// convenience macro to add Name and Content to the Request
// outbound delimiter is always &
#define APPENDFIELD(Request, Name, Content) \
        { \
	  if (! Request.isEmpty()) \
	    Request += "&"; \
	  Request += QString(Name) + "=" + QString(Content); \
	}

AuthorizeDotNetProcessor::AuthorizeDotNetProcessor() : CreditCardProcessor()
{
  _company = "Authorize.Net";
  _defaultLivePort   = 443;
  _defaultLiveServer = "https://secure2.authorize.net/gateway/transact.dll";
  _defaultTestPort   = 443;
  _defaultTestServer = "https://test.authorize.net/gateway/transact.dll";

  _msgHash.insert(-200, tr("A Login is required."));
  _msgHash.insert(-201, tr("The Login must be 20 characters or less."));
  _msgHash.insert(-202, tr("The Transaction Key must be 16 characters or less."));
  _msgHash.insert(-203, tr("The Delimiting Character must be exactly 1 character."));
  _msgHash.insert(-204, tr("The Delimiting Character must be exactly 1 character."));
  _msgHash.insert(-205, tr("The response from the Gateway appears to be "
			   "incorrectly formatted (could not find field %1 "
			   "as there are only %2 fields present)."));
  _msgHash.insert(-206, tr("The response from the Gateway failed the MD5 "
			   "security check."));
  _msgHash.insert( 206, tr("The response from the Gateway has failed the MD5 "
			   "security check but will be processed anyway."));
  _msgHash.insert(-207, tr("The Gateway returned the following error: %1"));
  _msgHash.insert(-208, tr("& (ampersand) is not a valid Delimiting Character."));
  _msgHash.insert(-209, tr("The Delimiting Character and the Encapsulating "
			   "Character cannot be the same. Please change one or "
			   "the other."));

  _ignoreSslErrors = _metrics->boolean("CCIgnoreSSLErrors") ||
                     _metrics->boolean("CCANIgnoreSSLErrors");

}

/** Build a basic outgoing request for an initial transaction - that is, one
    that can stand alone and does not depend on the success of a prior
    transaction.

    @see buildFollowup
 */
int AuthorizeDotNetProcessor::buildCommon(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &prequest, QString pordertype)
{
  // TODO: if check and not credit card transaction do something else
  XSqlQuery anq;
  anq.prepare(
    "SELECT ccard_active,"
    "  formatbytea(decrypt(setbytea(ccard_number),   setbytea(:key),'bf')) AS ccard_number,"
    "  formatccnumber(decrypt(setbytea(ccard_number),setbytea(:key),'bf')) AS ccard_number_x,"
    "  formatbytea(decrypt(setbytea(ccard_name),     setbytea(:key),'bf')) AS ccard_name,"
    "  formatbytea(decrypt(setbytea(ccard_address1), setbytea(:key),'bf')) AS ccard_address1,"
    "  formatbytea(decrypt(setbytea(ccard_address2), setbytea(:key),'bf')) AS ccard_address2,"
    "  formatbytea(decrypt(setbytea(ccard_city),     setbytea(:key),'bf')) AS ccard_city,"
    "  formatbytea(decrypt(setbytea(ccard_state),    setbytea(:key),'bf')) AS ccard_state,"
    "  formatbytea(decrypt(setbytea(ccard_zip),      setbytea(:key),'bf')) AS ccard_zip,"
    "  formatbytea(decrypt(setbytea(ccard_country),  setbytea(:key),'bf')) AS ccard_country,"
    "  formatbytea(decrypt(setbytea(ccard_month_expired),setbytea(:key),'bf')) AS ccard_month_expired,"
    "  formatbytea(decrypt(setbytea(ccard_year_expired),setbytea(:key), 'bf')) AS ccard_year_expired,"
    "  custinfo.*, cntct_phone, cntct_email"
    "  FROM ccard"
    "  JOIN custinfo ON (ccard_cust_id=cust_id)"
    "  LEFT OUTER JOIN cntct ON (cust_cntct_id=cntct_id)"
    "WHERE (ccard_id=:ccardid);");
  anq.bindValue(":ccardid", pccardid);
  anq.bindValue(":key",     omfgThis->_key);
  anq.exec();

  if (anq.first())
  {
    if (!anq.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10);
      return -10;
    }
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-17).arg(pccardid);
    return -17;
  }

  if (! _metrics->value("CCANDelim").isEmpty())
    APPENDFIELD(prequest, "x_delim_char", _metrics->value("CCANDelim"));

  APPENDFIELD(prequest, "x_version",      _metrics->value("CCANVer"));
  APPENDFIELD(prequest, "x_delim_data", "true");

  if (! _metrics->value("CCANEncap").isEmpty())
    APPENDFIELD(prequest, "x_encap_char", _metrics->value("CCANEncap"));

  APPENDFIELD(prequest, "x_login",    _metricsenc->value("CCLogin"));
  APPENDFIELD(prequest, "x_tran_key", _metricsenc->value("CCPassword"));
  APPENDFIELD(prequest, "x_amount",   QString::number(pamount, 'f', 2));
  // TODO: if check and not credit card transaction do something else
  APPENDFIELD(prequest, "x_card_num", anq.value("ccard_number").toString());

  if (_metrics->value("CCServer").contains("test.authorize.net")) {
    APPENDFIELD(prequest, "x_test_request", "false");
  } else {
    APPENDFIELD(prequest, "x_test_request", isLive() ? "false" : "true");
  }

  // TODO: if check and not credit card transaction do something else
  QString work_month;
  work_month.setNum(anq.value("ccard_month_expired").toDouble());
  if (work_month.length() == 1)
    work_month = "0" + work_month;
  APPENDFIELD(prequest, "x_exp_date",
	      work_month + anq.value("ccard_year_expired").toString().right(2));

  APPENDFIELD(prequest, "x_relay_response", "false");
  APPENDFIELD(prequest, "x_duplicate_window",
			_metrics->value("CCANDuplicateWindow"));

  QStringList name = anq.value("ccard_name").toString().split(QRegExp("\\s+"));
  APPENDFIELD(prequest, "x_first_name", name.at(0));
  APPENDFIELD(prequest, "x_last_name",  name.at(name.size() - 1));
  APPENDFIELD(prequest, "x_company",    anq.value("cust_name").toString());
  APPENDFIELD(prequest, "x_address",    anq.value("ccard_address1").toString());
  APPENDFIELD(prequest, "x_city",       anq.value("ccard_city").toString());
  APPENDFIELD(prequest, "x_state",      anq.value("ccard_state").toString());
  APPENDFIELD(prequest, "x_zip",        anq.value("ccard_zip").toString());
  APPENDFIELD(prequest, "x_country",    anq.value("ccard_country").toString());

  if (_metrics->boolean("CCANWellsFargoSecureSource"))
  {
    APPENDFIELD(prequest, "x_phone",    anq.value("cntct_phone").toString());
    APPENDFIELD(prequest, "x_email",    anq.value("cntct_email").toString());
  }

  anq.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=:currid);");
  anq.bindValue(":currid", pcurrid);
  anq.exec();
  if (anq.first())
  {
    APPENDFIELD(prequest, "x_currency_code", anq.value("curr_abbr").toString());
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }

  // TODO: if check and not credit card transaction do something else
  APPENDFIELD(prequest, "x_method",     "CC");
  APPENDFIELD(prequest, "x_type",       pordertype);

  if (! pcvv.isEmpty() && pcvv.toInt() != -2)
    APPENDFIELD(prequest, "x_card_code", pcvv);

  if (DEBUG)
    qDebug("AN:buildCommon built %s\n", prequest.toLatin1().data());
  return 0;
}

/** @brief Build an outgoing request for a follow-on transaction -
           that is, one that requires a previous transaction to be meaningful.

    Some Authorize.Net transactions can only occur after being set
    up by a prior transaction. For example, @em capturing a
    pre-authorization requires an existing valid pre-authorization.
    These follow-on transactions pass less data and are somewhat
    more secure than full messages.

    @param         pccpayid the internal id of the @c ccpay record
    @param         ptransid the transaction id, typically the order number
    @param         pamount  in: the amount in the ERP transaction currency,
                            out: the amount in the Authorize.Net transaction
    @param[in,out] pcurrid  in: the currency of the ERP transaction,
                            out: the currency of the Authorize.Net transaction
    @param[in,out] prequest   The message to send to Authorize.Net
    @param         pordertype The type of Authorize.Net transaction

    @see buildCommon
 */
int AuthorizeDotNetProcessor::buildFollowup(const int pccpayid, const QString &ptransid, double &pamount, int &pcurrid, QString &prequest, QString pordertype)
{
  Q_UNUSED(pccpayid);
  int returnValue = 0;
  int currid      = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    pamount = currToCurr(pcurrid, currid, pamount, &returnValue);
    pcurrid = currid;
    if (returnValue < 0)
      return returnValue;
  }

  if (! _metrics->value("CCANDelim").isEmpty())
    APPENDFIELD(prequest, "x_delim_char", _metrics->value("CCANDelim"));

  if (! _metrics->value("CCANEncap").isEmpty())
    APPENDFIELD(prequest, "x_encap_char", _metrics->value("CCANEncap"));

  APPENDFIELD(prequest, "x_version",      _metrics->value("CCANVer"));
  APPENDFIELD(prequest, "x_delim_data", "true");
  APPENDFIELD(prequest, "x_login",    _metricsenc->value("CCLogin"));
  APPENDFIELD(prequest, "x_tran_key", _metricsenc->value("CCPassword"));
  APPENDFIELD(prequest, "x_trans_id", ptransid);
  APPENDFIELD(prequest, "x_amount",   QString::number(pamount, 'f', 2));
  APPENDFIELD(prequest, "x_method",   "CC");

  if (_metrics->value("CCServer").contains("test.authorize.net")) {
    APPENDFIELD(prequest, "x_test_request", "false");
  } else {
    APPENDFIELD(prequest, "x_test_request", isLive() ? "false" : "true");
  }

  APPENDFIELD(prequest, "x_relay_response", "false");
  APPENDFIELD(prequest, "x_duplicate_window",
			_metrics->value("CCANDuplicateWindow"));

  XSqlQuery anq;
  anq.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=:currid);");
  anq.bindValue(":currid", pcurrid);
  anq.exec();
  if (anq.first())
  {
    APPENDFIELD(prequest, "x_currency_code", anq.value("curr_abbr").toString());
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }

  // TODO: if check and not credit card transaction do something else
  APPENDFIELD(prequest, "x_method",     "CC");
  APPENDFIELD(prequest, "x_type",       pordertype);

  if (DEBUG)
    qDebug("AN:buildFollowup built %s\n", prequest.toLatin1().data());
  return 0;
}

int  AuthorizeDotNetProcessor::doAuthorize(const int pccardid, const QString &pcvv, double &pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("AN:doAuthorize(%d, pcvv, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toLatin1().data(), preforder.toLatin1().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  double tax     = ptax;
  double freight = pfreight;
  double duty    = pduty;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount,   &returnValue);
    if (returnValue < 0)
      return returnValue;
    tax     = currToCurr(pcurrid, currid, ptax,     &returnValue);
    if (returnValue < 0)
      return returnValue;
    freight = currToCurr(pcurrid, currid, pfreight, &returnValue);
    if (returnValue < 0)
      return returnValue;
    duty    = currToCurr(pcurrid, currid, pduty,    &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;
  returnValue = buildCommon(pccardid, pcvv, amount, currid, request, "AUTH_ONLY");
  if (returnValue != 0)
    return returnValue;

  APPENDFIELD(request, "x_tax",        QString::number(tax, 'f', 2));
  APPENDFIELD(request, "x_tax_exempt", ptaxexempt ? "true" : "false");
  APPENDFIELD(request, "x_freight",    QString::number(freight, 'f', 2));
  APPENDFIELD(request, "x_duty",       QString::number(duty,    'f', 2));

  if (! preforder.isEmpty())
    APPENDFIELD(request, "x_po_num", preforder.left(20));

  QString response;

  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "A", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int  AuthorizeDotNetProcessor::doCharge(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString& pneworder, QString& preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("AN:doCharge(%d, pcvv, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pamount,  ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toLatin1().data(), preforder.toLatin1().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  double tax     = ptax;
  double freight = pfreight;
  double duty    = pduty;
  int    currid  = pcurrid;

  if (_metrics->value("CCANCurrency") != "TRANS")
  {
    currid = _metrics->value("CCANCurrency").toInt();
    amount = currToCurr(pcurrid, currid, pamount,   &returnValue);
    if (returnValue < 0)
      return returnValue;
    tax     = currToCurr(pcurrid, currid, ptax,     &returnValue);
    if (returnValue < 0)
      return returnValue;
    freight = currToCurr(pcurrid, currid, pfreight, &returnValue);
    if (returnValue < 0)
      return returnValue;
    duty    = currToCurr(pcurrid, currid, pduty,    &returnValue);
    if (returnValue < 0)
      return returnValue;
  }

  QString request;

  returnValue = buildCommon(pccardid, pcvv, amount, currid, request, "AUTH_CAPTURE");
  if (returnValue !=  0)
    return returnValue;

  APPENDFIELD(request, "x_tax",        QString::number(tax, 'f', 2));
  APPENDFIELD(request, "x_tax_exempt", ptaxexempt ? "true" : "false");
  APPENDFIELD(request, "x_freight",    QString::number(freight, 'f', 2));
  APPENDFIELD(request, "x_duty",       QString::number(duty,    'f', 2));

  if (! preforder.isEmpty())
    APPENDFIELD(request, "x_po_num",   preforder);

  QString response;

  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "C", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int AuthorizeDotNetProcessor::doChargePreauthorized(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  Q_UNUSED(pcvv);
  if (DEBUG)
    qDebug("AN:doChargePreauthorized(%d, pcvv, %f, %d, %s, %s, %d)",
	   pccardid, pamount,  pcurrid,
	   pneworder.toLatin1().data(), preforder.toLatin1().data(), pccpayid);

  int    returnValue = 0;
  double amount  = pamount;
  int    currid  = pcurrid;

  QString request;
  returnValue = buildFollowup(pccpayid, preforder, amount, currid, request, "PRIOR_AUTH_CAPTURE");
  if (returnValue != 0)
    return returnValue;

  QString response;
  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "CP", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  return returnValue;
}

int AuthorizeDotNetProcessor::doCredit(const int pccardid, const QString &pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  Q_UNUSED(pcvv);
  if (DEBUG)
    qDebug("AN:doCredit(%d, pcvv, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pamount, ptax, ptaxexempt,  pfreight,  pduty, pcurrid,
	   pneworder.toLatin1().data(), preforder.toLatin1().data(), pccpayid);

  int     returnValue = 0;
  double  amount  = pamount;
  int     currid  = pcurrid;
  bool    tryVoid = false;
  QString approvalCode;
  QString request;

  returnValue = buildFollowup(pccpayid, preforder, amount, currid, request, "CREDIT");
  if (returnValue != 0)
    return returnValue;

  XSqlQuery anq;
  anq.prepare("SELECT ccpay_card_pan_trunc,"
              "  (ccpay_transaction_datetime > CURRENT_DATE"
              "  AND ccpay_amount = :amount) AS tryVoid,"
              "  ccpay_r_code,"
              "  formatbytea(decrypt(setbytea(ccard_number),"
              "              setbytea(:key),'bf')) AS ccard_number"
              "  FROM ccpay LEFT OUTER JOIN ccard ON (ccpay_ccard_id=ccard_id)"
              " WHERE (ccpay_id=:ccpayid);");
  anq.bindValue(":ccpayid", pccpayid);
  anq.bindValue(":key",     omfgThis->_key);
  anq.bindValue(":now",     QDateTime::currentDateTime());
  anq.bindValue(":amount",  amount);
  anq.exec();
  if (anq.first())
  {
    QString cardnum = anq.value("ccpay_card_pan_trunc").toString();
    if (cardnum.isEmpty())
      cardnum = anq.value("ccard_number").toString();
    APPENDFIELD(request, "x_card_num", cardnum.right(4));
    tryVoid = anq.value("tryVoid").toBool();
    approvalCode = anq.value("ccpay_r_code").toString();
  }
  else if (anq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = anq.lastError().databaseText();
    return -1;
  }

  QString response;
  returnValue = sendViaHTTP(request, response);
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "R", amount, currid,
			       pneworder, preforder, pccpayid, pparams);

  // TODO: make more precise - look for return code 54
  if (returnValue < 0 && tryVoid) {
    int voidResult = 0;
    QString tmpErrorMsg = _errorMsg;
    ParameterList voidParams;
    _errorMsg.clear();
    voidResult = doVoidPrevious(pccardid,   pcvv,      amount,       currid,
                                pneworder,  preforder, approvalCode, pccpayid,
                                voidParams);
    if (voidResult >= 0) {
      returnValue = voidResult;
      pparams.clear();
      while (! voidParams.isEmpty())
        pparams.append(voidParams.takeFirst());
    }
    else
      _errorMsg = tmpErrorMsg;
  }

  return returnValue;
}

int AuthorizeDotNetProcessor::doVoidPrevious(const int pccardid, const QString &pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &pparams)
{
  Q_UNUSED(pcvv);
  if (DEBUG)
    qDebug("AN:doVoidPrevious(%d, pcvv, %f, %d, %s, %s, %s, %d)",
	   pccardid, pamount, pcurrid,
	   pneworder.toLatin1().data(), preforder.toLatin1().data(),
	   papproval.toLatin1().data(), pccpayid);

  QString tmpErrorMsg = _errorMsg;

  int     returnValue = 0;
  double  amount = pamount;
  int     currid = pcurrid;
  QString request;

  returnValue = buildFollowup(pccpayid, preforder, amount, currid, request, "VOID");

  QString response;
  returnValue = sendViaHTTP(request, response);
  _errorMsg = tmpErrorMsg;
  if (returnValue < 0)
    return returnValue;

  returnValue = handleResponse(response, pccardid, "V", amount, currid,
			       pneworder, preforder, pccpayid, pparams);
  if (! tmpErrorMsg.isEmpty())
    _errorMsg = tmpErrorMsg;
  return returnValue;
}

int AuthorizeDotNetProcessor::fieldValue(const QStringList plist, const int pindex, QString &pvalue)
{
  if (plist.size() < pindex)
  {
    _errorMsg = errorMsg(-205).arg(pindex).arg(plist.size());
    return -205;
  }

  if (_metrics->value("CCANEncap").isEmpty())
    pvalue = plist.at(pindex - 1);
  else
  {
    pvalue = plist.at(pindex - 1);
    // now strip of the encapsulating char from front and back
    int firstPos = plist.at(pindex - 1).indexOf(_metrics->value("CCANEncap"));
    int lastPos  = plist.at(pindex - 1).lastIndexOf(_metrics->value("CCANEncap"));
    pvalue = pvalue.mid(firstPos + 1, lastPos - firstPos - 1);
  }
  if (DEBUG)
    qDebug("AN:fieldValue of %d is %s", pindex, pvalue.toLatin1().data());
  return 0;
}

int AuthorizeDotNetProcessor::handleResponse(const QString &presponse, const int pccardid, const QString &ptype, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug() << "AN::handleResponse(" << presponse << pccardid << ptype << pamount
             << pcurrid << preforder << pccpayid << "pparams)";

  // if we got an error msg very early on
  if (presponse.startsWith("<HTML>"))
  {
    _errorMsg = errorMsg(-207).arg(presponse);
    return -207;
  }

  QString delim = _metrics->value("CCANDelim").isEmpty() ? "," :
						  _metrics->value("CCANDelim");
  QString encap = _metrics->value("CCANEncap");

  QString r_approved;
  QString r_avs;
  QString r_code;
  QString r_cvv;
  QString r_error;
  QString r_message;
  QString r_ordernum;
  QString r_reason;     // not stored
  QString r_ref;
  QString r_shipping;
  QString r_tax;
  QString r_pantrunc;
  QString r_cardtype;
  QString r_hash;

  QString status;
  bool    isCP=false; // looks like an (unsupported) card-present transaction

  // TODO: explore using encap here and code from CSV Import to properly split

  /* add an extra field at the beginning. otherwise we'll be off by one
     because the Advanced Integration Method (AIM) Implementation Guide
     numbers fields starting at 1 but QString::split() creates a list
     starting at 0.
   */
  QStringList responseFields = presponse.split(delim);

  QString r_response;
  int returnValue = fieldValue(responseFields, 1, r_response);
  if (returnValue < 0)
    return returnValue;

  if (! QRegExp("[1-4]").exactMatch(r_response) &&
      QRegExp("\\d+\\.\\d+").exactMatch(r_response))
  {
    qDebug() << "Looks like A.N returned a unsupported Card-Present response:" << r_response;
    isCP = true;
    returnValue = fieldValue(responseFields, 2, r_response);
    if (returnValue < 0)
      return returnValue;
  }

  if (r_response.toInt() == 1)
    r_approved = "APPROVED";
  else if (r_response.toInt() == 2)
    r_approved = "DECLINED";
  else if (r_response.toInt() == 3)
    r_approved = "ERROR";
  else if (r_response.toInt() == 4)
    r_approved = "HELDFORREVIEW";

  // fieldValue(responseFields, 2);				// subcode

  returnValue = fieldValue(responseFields, 3, r_reason);	// reason code
  if (returnValue < 0)
    return returnValue;
  returnValue = fieldValue(responseFields, 4, r_message);	// reason text
  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 5, r_code);	 	// approval code
  if (returnValue < 0)
    return returnValue;
  returnValue = fieldValue(responseFields, 6, r_avs);	 	// avs result
  if (returnValue < 0)
    return returnValue;

  if (isCP)
  {
    returnValue = fieldValue(responseFields, 8, r_ordernum);	// transaction id
    if (returnValue < 0)
      return returnValue;

    returnValue = fieldValue(responseFields, 7, r_cvv); // ccv response code
    if (returnValue < 0 && ptype == "CP") // may not get cvv on preauth capture
      returnValue = 0;
    else if (returnValue < 0)
      return returnValue;

    returnValue = fieldValue(responseFields, 9, r_hash);	// md5 hash

  } else {
  returnValue = fieldValue(responseFields, 7, r_ordernum);	// transaction id

  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 33, r_tax);		// echo x_tax
  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 35, r_shipping);	// echo x_freight
  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 38, r_hash);	// md5 hash
  if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 39, r_cvv); // ccv response code
  if (returnValue < 0 && ptype == "CP") // may not get cvv on preauth capture
    returnValue = 0;
  else if (returnValue < 0)
    return returnValue;

  returnValue = fieldValue(responseFields, 51, r_pantrunc);
  if (returnValue < 0)
    return returnValue;
  r_pantrunc = r_pantrunc.right(4);

  returnValue = fieldValue(responseFields, 52, r_cardtype);
  if (returnValue < 0)
    return returnValue;
  }

  if (r_cardtype == "Discover" || r_cardtype == "MasterCard"
      || r_cardtype == "Visa"  || r_cardtype == "American Express")
    r_cardtype.remove(1, r_cardtype.length());
  else
    r_cardtype = "O";

  /* treat heldforreview as approved because the AIM doc says response
     reason codes 252 and 253 are both approved but being reviewed.
     the intent of the other heldforreview, 193, is ambiguous.
   */
  if (r_approved == "APPROVED" || r_approved == "HELDFORREVIEW")
  {
    _errorMsg = errorMsg(0).arg(r_code);
    if (ptype == "A")
      status = "A";	// Authorized
    else if (ptype == "V")
      status = "V";	// Voided
    else
      status = "C";	// Completed/Charged
  }
  else if (r_approved == "DECLINED")
  {
    _errorMsg = errorMsg(-92).arg(r_message);
    returnValue = -92;
    status = "D";
  }
  else if (r_approved == "ERROR")
  {
    r_error = r_message;
    _errorMsg = errorMsg(-12).arg(r_error).arg(_company);
    returnValue = -12;
    status = "X";
  }

  else if (r_approved.isEmpty() && ! r_message.isEmpty())
  {
    _errorMsg = errorMsg(-95).arg(r_message).arg(_company);
    returnValue = -95;
    status = "X";
  }

  else if (r_approved.isEmpty())
  {
    _errorMsg = errorMsg(-100).arg(r_error).arg(r_message).arg(presponse);
    returnValue = -100;
    status = "X";
  }

  // always use the AVS checking configured on the gateway
  _passedAvs = ((r_reason.toInt() != 27) &&
	        (r_reason.toInt() != 127));

  // always use the CVV checking configured on the gateway
  _passedCvv = (r_reason.toInt() != 78);

  if (DEBUG)
    qDebug("AN:%s _passedAvs %d\t%s _passedCvv %d",
	    qPrintable(r_avs), _passedAvs, qPrintable(r_cvv), _passedCvv);

  pparams.append("ccard_id",    pccardid);
  pparams.append("currid",      pcurrid);
  pparams.append("auth_charge", ptype);
  pparams.append("type",        ptype);
  pparams.append("reforder",    (preforder.isEmpty()) ? pneworder : preforder);
  pparams.append("status",      status);
  pparams.append("avs",         r_avs);
  pparams.append("ordernum",    pneworder);
  pparams.append("xactionid",   r_ordernum);
  pparams.append("error",       r_error);
  pparams.append("approved",    r_approved);
  pparams.append("code",        r_code);
  pparams.append("shipping",    r_shipping);
  pparams.append("tax",         r_tax);
  pparams.append("ref",         r_ref);
  pparams.append("message",     r_message);
  pparams.append("pantrunc",    r_pantrunc);
  pparams.append("cardtype",    r_cardtype);

  pparams.append("auth", QVariant(ptype == "A"));

  if (DEBUG)
    qDebug("AN:r_error.isEmpty() = %d", r_error.isEmpty());

  if (returnValue == 0)
    pparams.append("amount",   pamount);
  else
    pparams.append("amount",   0);	// no money changed hands this attempt

  // don't bother checking MD5 if we hit a bigger problem
  if (returnValue == 0 && _metrics->boolean("CCANMD5HashSetOnGateway"))
  {
    QString expected_hash;
    XSqlQuery anq;
    anq.prepare("SELECT UPPER(MD5(:inputstr)) AS expected;");
    anq.bindValue(":inputstr", _metricsenc->value("CCANMD5Hash") +
			       _metricsenc->value("CCLogin") +
			       r_ordernum +
			       QString::number(pamount, 'f', 2));
    anq.exec();
    if (anq.first())
      expected_hash = anq.value("expected").toString();
    else if (anq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(-1).arg(anq.lastError().databaseText());
      returnValue = -1;
    }
    if (DEBUG)
      qDebug("AN:handleResponse expected md5 %s and got %s",
	      expected_hash.toLatin1().data(), r_hash.toLatin1().data());

    if (_metrics->value("CCANMD5HashAction") == "F" && expected_hash != r_hash)
    {
      _errorMsg = errorMsg(-206);
      returnValue = -206;
    }
    else if (_metrics->value("CCANMD5HashAction") == "W" && expected_hash != r_hash)
    {
      _errorMsg = errorMsg(206);
      returnValue = 206;
    }
  }

  if (DEBUG)
    qDebug("AN::handleResponse returning %d %s",
           returnValue, errorMsg().toLatin1().data());
  return returnValue;
}

int AuthorizeDotNetProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("AN:doTestConfiguration()");

  int returnValue = 0;

  if (_metricsenc->value("CCLogin").isEmpty())
  {
    _errorMsg = errorMsg(-200);
    returnValue = -200;
  }
  else if (_metricsenc->value("CCLogin").size() > 20)
  {
    _errorMsg = errorMsg(-201);
    returnValue = -201;
  }
  else if (_metricsenc->value("CCPassword").size() > 16)
  {
    _errorMsg = errorMsg(-202);
    returnValue = -202;
  }
  else if (_metrics->value("CCANDelim").size() > 1)
  {
    _errorMsg = errorMsg(-203);
    returnValue = -203;
  }
  else if (_metrics->value("CCANDelim") == "&")
  {
    _errorMsg = errorMsg(-208);
    returnValue = -208;
  }
  else if (_metrics->value("CCANDelim") == _metrics->value("CCANEncap") &&
	   ! _metrics->value("CCANDelim").isEmpty())
  {
    _errorMsg = errorMsg(-209);
    returnValue = -209;
  }

  return returnValue;
}

bool AuthorizeDotNetProcessor::handlesCreditCards()
{
  return true;
}
