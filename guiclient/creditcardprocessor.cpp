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

#include <QApplication>
#include <QMessageBox>
#include <QProcess>
#include <QSqlError>

#include <currcluster.h>
#include <metasql.h>
#include <openreports.h>

#include "guiclient.h"
#include "creditcardprocessor.h"
#include "storedProcErrorLookup.h"

#include "authorizedotnetprocessor.h"
#include "externalccprocessor.h"
#include "verisignprocessor.h"
#include "yourpayprocessor.h"

#define DEBUG false

/* NOTE TO SUBCLASSERS:
  It is your job to make sure that all of the configuration options available
  on the Credit Card Configuration window are implemented either here or in
  your subclass. An example of an option that /must/ be implemented in your
  subclass is CCTestResult, as requesting error responses from the credit card
  processor is different for every processor.
*/

QString			 CreditCardProcessor::_errorMsg = "";
QHash<int, QString>	 CreditCardProcessor::_msgHash;

/*
   > 0 => credit card transaction processing completed but there is a warning
   	  condition: the credit card processing company denied the transaction
	  or the post-processing generated an error

   = 0 => transaction succeeded in its entirety

   < 0 => preprocessing failed or the credit card company returned an error

   codes < -100 and codes > 100 are available for use by subclasses
   and should be loaded into _msgHash in the subclass' constructor.
 */
#define TR(a)	QObject::tr(a)
static struct {
    int		code;
    QString	text;
} messages[] = {

  {   0, TR("This transaction was approved.\n%1")			},

  {  -1, TR("Database Error")						},
  {  -2, TR("You don't have permission to process Credit Card transactions.") },
  {  -3, TR("The application is not set up to process credit cards.")	},
  {  -4, TR("The Bank Account is not set for Credit Card transactions.") },
  {  -5, TR("The encryption key is not defined.")			},
  {  -6, TR("The login for the proxy server is not defined.")		},
  {  -7, TR("The password for the proxy server is not defined.")	},
  {  -8, TR("The proxy server is not defined.")				},
  {  -9, TR("The port to use for the proxy server is not defined.")	},
  { -10, TR("Credit Card %1 is not active. Make it active or select "
	    "another Credit Card.")					},
  { -11, TR("Credit Card %1 has expired.")				},
  { -12, TR("The Credit Card Processing Company reported an error:\n%1")		},
  { -13, TR("The Credit Card configuration is inconsistent and the application "
	    "cannot determine whether to run in Test or Live mode.")	},
  { -14, TR("Could not figure out which Credit Card Processing Company "
	    "to set up (based on %1).")					},
  { -15, TR("The digital certificate (.pem file) is not set.")		},
  { -16, TR("Could not open digital certificate (.pem file) %1.")	},
  { -17, TR("Could not find a Credit Card with internal ID %1.")	},
  { -18, TR("Error with message transfer program:\n%1 %2\n\n%3")	},
  { -19, TR("%1 is not implemented.")					},
  { -20, TR("The application does not support either Credit Cards or "
	    "Checks with %1. Please choose a different company.")	}, 
  // preauthorizing charges
  { -21, TR("The amount to charge must be greater than 0.00.")		},
  { -24, TR("Could not generate a sequence number while preauthorizing.") },

  // processing charges based on a pre-authorization
  { -30, TR("Could not find the Credit Card preauthorization to charge.") },
  { -32, TR("You must select a preauthorization to charge.") 		},
  { -33, TR("The preauthorization (for %1) is not sufficient to cover "
		 "the desired transaction amount (%2).")		},
  { -34, TR("No Preauthorization found")				},
  { -35, TR("This preauthorization may not be charged. It was created "
	    "for a Sales Order which has been canceled.")	},

  // stand-alone charges
  { -40, TR("Inconsistent data passed to charge(): [%1] [%2]")	},
  { -44, TR("Could not generate a sequence number while charging.")	},

  // credits
  { -50, TR("Could not find original Credit Card payment to credit.")	},

  // voids
  { -60, TR("Could not find the Credit Card transaction to void.")	},

  // transaction was processed fine but was not successful
  { -90, TR("This Credit Card transaction was denied.\n%1")		},
  { -91, TR("This Credit Card transaction is a duplicate.\n%1")		},
  { -92, TR("This Credit Card transaction was declined.\n%1")		},
  { -93, TR("This Credit Card transaction was denied "
	    "because of possible fraud.\n%1")				},

  // other misc errors
  { -95, TR("The Credit Card Processor returned an error: %1")		},
  { -96, TR("This transaction failed the CVV check.")			},
  { -97, TR("This transaction failed the Address Verification check.")	},
  { -98, TR("You may not process this transaction without a CVV code. "
	    "Please enter one and try again.")				},
  { -99, TR("The CVV value is not valid.")				},
  {-100, TR("No approval code was received:\n%1\n%2\n%3")		},

  // positive values: warnings
  {   1, TR("Database Error")						},
  {   2, TR("Could not generate a unique key for the ccpay table.")	},
  {   3, TR("Stored Procedure Error")					},
  {   4, TR("The Credit Card transaction completed successfully but "
	    "it was not recorded correctly:\n%1")			},
  {   5, TR("The Server is %2 and is expected to be %3 in %1 mode, "
            "while the Port is %4 and is expected to be %5. Credit Card "
            "processing transactions may fail.")                        },
  {   6, TR("The Server is %2 and is expected to be %3 in %1 mode."
            "Credit Card processing transactions may fail.")            },
  {   7, TR("The Port is %2 and is expected to be %3 in %1 mode. "
            "Credit Card processing transactions may fail.")            },
  {  20, TR("User chose not to process the preauthorization.")		},
  {  30, TR("User chose not to post-authorize process the charge.")	},
  {  40, TR("User chose not to process the charge.")			},
  {  50, TR("User chose not to process the credit.")			},
  {  60, TR("User chose not to process the void.")			},
  {  94, TR("There was a problem printing the credit card receipt.")    },
  {  96, TR("This transaction failed the CVV check but will be "
	    "processed anyway.")					},
  {  97, TR("This transaction failed the Address Verification check "
	    "but will be processed anyway.")				},
  {  99, TR("User chose not to proceed without CVV code.")		},

};

CreditCardProcessor::CreditCardProcessor()
{
  if (DEBUG)
    qDebug("CCP:CreditCardProcessor()");
  _defaultLivePort    = 0;
  _defaultLiveServer  = "live.creditcardprocessor.com";
  _defaultTestPort    = 0;
  _defaultTestServer  = "test.creditcardprocessor.com";
  _errorMsg       = "";

  for (unsigned int i = 0; i < sizeof(messages) / sizeof(messages[0]); i++)
    _msgHash.insert(messages[i].code, messages[i].text);
}

CreditCardProcessor::~CreditCardProcessor()
{
}

// pcompany should be "" except when checking for errors in configCC
CreditCardProcessor * CreditCardProcessor::getProcessor(const QString pcompany)
{
  if (DEBUG)
    qDebug("CCP:getProcessor(%s)", pcompany.toAscii().data());

  if (pcompany == "Authorize.Net")
    return new AuthorizeDotNetProcessor();

  else if (pcompany == "Verisign")
    return new VerisignProcessor();

  else if (pcompany == "YourPay")
    return new YourPayProcessor();

  else if (pcompany == "External")
    return new ExternalCCProcessor();

  else if (! pcompany.isEmpty())
  {
    _errorMsg = errorMsg(-14).arg(pcompany);
    return 0;
  }

  CreditCardProcessor *processor = 0;

  if (_metrics->value("CCCompany") == "Authorize.Net")
    processor = new AuthorizeDotNetProcessor();

  else if (_metrics->value("CCCompany") == "Verisign")
    processor = new VerisignProcessor();

  else if ((_metrics->value("CCCompany") == "YourPay"))
    processor = new YourPayProcessor();

  else if ((_metrics->value("CCCompany") == "External"))
    processor = new ExternalCCProcessor();

  else
    _errorMsg = errorMsg(-14).arg(_metrics->value("CCServer"));

  // reset to 0 if the config is bad and we're not configuring the system
  if (processor && processor->testConfiguration() < 0 && pcompany.isEmpty())
  {
    delete processor;
    processor = 0;
  }

  if (processor)
    processor->reset();

  return processor;
}

int CreditCardProcessor::authorize(const int pccardid, const int pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:authorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmPreauth") &&
      QMessageBox::question(0,
		    tr("Confirm Preauthorization of Credit Card Purchase"),
		    tr("<p>Are you sure that you want to preauthorize "
		       "a charge to credit card %1 in the amount of %2 %3?")
		       .arg(ccard_x)
		       .arg(CurrDisplay::currSymbol(pcurrid))
		       .arg(pamount),
		    QMessageBox::Yes | QMessageBox::Default,
		    QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(20);
    return 20;
  }

  ParameterList dbupdateinfo;
  returnVal = doAuthorize(pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == 20)
    return 20;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    XSqlQuery cashq;
    if (preftype == "cashrcpt")
    {
      if (prefid <= 0)
      {
	cashq.exec("SELECT NEXTVAL('cashrcpt_cashrcpt_id_seq') AS cashrcpt_id;");
	if (cashq.first())
	  prefid = cashq.value("cashrcpt_id").toInt();
	else if (q.lastError().type() != QSqlError::None)
	{
	  _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	  // TODO: log an event?
	  return 1;
	}

	cashq.prepare("INSERT INTO cashrcpt (cashrcpt_id,"
		  "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
		  "  cashrcpt_fundstype, cashrcpt_docnumber,"
		  "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate) "
		  "SELECT :cashrcptid,"
		  "       ccpay_cust_id, :amount, :curr_id,"
		  "       ccard_type, ccpay_r_ordernum,"
		  "       :bankaccnt_id, :notes, current_date"
		  "  FROM ccpay, ccard "
		  "WHERE (ccpay_ccard_id=ccard_id);");
      }
      else
	cashq.prepare( "UPDATE cashrcpt "
		       "SET cashrcpt_cust_id=ccard_cust_id,"
		       "    cashrcpt_amount=:amount,"
		       "    cashrcpt_fundstype=ccard_type,"
		       "    cashrcpt_bankaccnt_id=:bankaccnt_id,"
		       "    cashrcpt_distdate=CURRENT_DATE,"
		       "    cashrcpt_notes=:notes, "
		       "    cashrcpt_curr_id=:curr_id "
		       "FROM ccard "
		       "WHERE ((cashrcpt_id=:cashrcptid)"
		       "  AND  (ccard_id=:ccardid));" );

      cashq.bindValue(":cashrcptid",   prefid);
      cashq.bindValue(":ccardid",      pccardid);
      cashq.bindValue(":amount",       pamount);
      cashq.bindValue(":curr_id",      pcurrid);
      cashq.bindValue(":bankaccnt_id", _metrics->value("CCDefaultBank").toInt());
      cashq.bindValue(":notes",        "Credit Card Pre-Authorization");
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::None)
      {
	_errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	// TODO: log an event?
	returnVal = 1;
      }
    }
    else if (preftype == "cohead")
    {
      cashq.prepare("INSERT INTO payco VALUES"
		    " (:payco_ccpay_id, :payco_cohead_id,"
		    "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_ccpay_id",  pccpayid);
      cashq.bindValue(":payco_cohead_id", prefid);
      cashq.bindValue(":payco_amount",    pamount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
	// TODO: log an event?
	returnVal = 1;
      }
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

int CreditCardProcessor::charge(const int pccardid, const int pcvv, const double pamount, double ptax, bool ptaxexempt, double pfreight, double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:charge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  if (preftype == "cohead" && prefid < 0)
  {
    _errorMsg = errorMsg(-40).arg(preftype).arg(prefid);
    return -40;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmCharge") &&
      QMessageBox::question(0, tr("Confirm Charge of Credit Card Purchase"),
	      tr("Are you sure that you want to charge credit card %1 "
		 "in the amount of %2 %3?")
		 .arg(ccard_x)
		 .arg(CurrDisplay::currSymbol(pcurrid))
		 .arg(pamount),
	      QMessageBox::Yes | QMessageBox::Default,
	      QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(40);
    return 40;
  }

  ParameterList dbupdateinfo;
  returnVal = doCharge(pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == 40)
    return 40;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    XSqlQuery cashq;
    cashq.prepare("SELECT postCCcashReceipt(:ccpayid,"
                  "                         :docid, :doctype) AS cm_id;");
    cashq.bindValue(":ccpayid",   pccpayid); 
    cashq.bindValue(":doctype",   preftype);
    cashq.bindValue(":docid",     prefid);
    cashq.exec();
    if (cashq.first())
    {
      int cm_id = cashq.value("cm_id").toInt();
      if (cm_id < 0)
      {
        _errorMsg = "<p>" + errorMsg(4)
                      .arg(storedProcErrorLookup("postCCcashReceipt", cm_id));
        returnVal = 3;
      }
    }
    else if (cashq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }

    if (preftype == "cohead")
    {
      XSqlQuery cashq;

      cashq.prepare("INSERT INTO payco VALUES"
                " (:payco_ccpay_id, :payco_cohead_id,"
                "  :payco_amount, :payco_curr_id);");
      cashq.bindValue(":payco_ccpay_id",  pccpayid);
      cashq.bindValue(":payco_cohead_id", prefid);
      cashq.bindValue(":payco_amount",    pamount);
      cashq.bindValue(":payco_curr_id",   pcurrid);
      cashq.exec();
      if (cashq.lastError().type() != QSqlError::NoError)
      {
        _errorMsg = errorMsg(4).arg(cashq.lastError().databaseText());
        // TODO: log an event?
        returnVal = 1;
      }
    }
  }

  if (_metrics->boolean("CCPrintReceipt"))
  {
    int receiptReturn = printReceipt(pccpayid);
    if (receiptReturn != 0 && returnVal == 0)
      returnVal = receiptReturn;
  }

  return returnVal;
}

int CreditCardProcessor::chargePreauthorized(const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:chargePreauthorized(%d, %f, %d, %s, %s, %d)",
	   pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  reset();

  int ccValidDays = _metrics->value("CCValidDays").toInt();
  if (ccValidDays < 1)
    ccValidDays = 7;

  if (pamount <= 0)
  {
    _errorMsg = errorMsg(-21);
    return -21;
  }

  if (pccpayid < 0)
  {
    _errorMsg = errorMsg(-32);
    return -32;
  }

  XSqlQuery ccq;
  ccq.prepare("SELECT *,"
	    "       currToCurr(ccpay_curr_id, :curr_id, ccpay_amount,"
	    "                  CURRENT_DATE) AS ccpay_amount_converted "
	    "FROM ccpay "
	    "WHERE ((ccpay_status = 'A')"
	    "  AND  (date_part('day', CURRENT_TIMESTAMP - "
	    "                         ccpay_transaction_datetime) < "
	    "        :ccValidDays)"
	    "  AND  (ccpay_id=:id));");
  ccq.bindValue(":id", pccpayid);
  ccq.bindValue(":curr_id",     pcurrid);
  ccq.bindValue(":ccValidDays", ccValidDays);
  ccq.exec();
  if (ccq.first())
  {
    if (pamount > ccq.value("ccpay_amount_converted").toDouble())
    {
      _errorMsg = errorMsg(-33)
		  .arg(ccq.value("ccpay_amount").toString())
		  .arg(pamount);
      return -33;
    }

  }
  else if (ccq.lastError().type() != QSqlError::None)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-34);
    return -34;
  }

  int ccardid = ccq.value("ccpay_ccard_id").toInt();
  preforder = ccq.value("ccpay_r_ordernum").toString();

  ccq.prepare("SELECT * FROM payco WHERE (payco_ccpay_id=:ccpayid)");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();
  if (ccq.first())
  {
    int coheadid = ccq.value("payco_cohead_id").toInt();
    ccq.prepare("SELECT COUNT(*) AS linecount "
	      "FROM coitem "
	      "WHERE ((coitem_status IN ('O', 'C'))"
	      "  AND  (coitem_cohead_id=:coheadid));");
    ccq.bindValue(":coheadid", coheadid);
    ccq.exec();
    if (ccq.first() && ccq.value("linecount").toInt() <= 0)
    {
      _errorMsg = errorMsg(-35);
      return -35;
    }
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
  }
  else if (ccq.lastError().type() != QSqlError::None)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(ccardid, pcvv, ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmChargePreauth") &&
      QMessageBox::question(0,
	      tr("Confirm Post-authorization of Credit Card Purchase"),
              tr("Are you sure that you want to charge a pre-authorized "
                 "transaction to credit card %1 in the amount of %2 %3?")
		 .arg(ccard_x)
		 .arg(CurrDisplay::currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(30);
    return 30;
  }

  ParameterList dbupdateinfo;
  returnVal = doChargePreauthorized(ccardid, pcvv, pamount, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == 30)
    return 30;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    ccq.prepare("INSERT INTO cashrcpt ("
	      "  cashrcpt_cust_id, cashrcpt_amount, cashrcpt_curr_id,"
	      "  cashrcpt_fundstype, cashrcpt_docnumber,"
	      "  cashrcpt_bankaccnt_id, cashrcpt_notes, cashrcpt_distdate) "
	      "SELECT ccpay_cust_id, :amount, :curr_id,"
	      "       ccard_type, ccpay_r_ordernum,"
	      "       :bankaccnt_id, :notes, current_date"
	      "  FROM ccpay, ccard "
	      "WHERE ((ccpay_ccard_id=ccard_id)"
	      "  AND  (ccpay_id=:pccpayid));");
    ccq.bindValue(":pccpayid",     pccpayid);
    ccq.bindValue(":amount",       pamount);
    ccq.bindValue(":curr_id",      pcurrid);
    ccq.bindValue(":bankaccnt_id", _metrics->value("CCDefaultBank").toInt());
    ccq.bindValue(":notes",        "Converted Pre-auth");
    ccq.exec();
    if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
      // TODO: log an event?
      returnVal = 1;
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

int CreditCardProcessor::testConfiguration()
{
  if (DEBUG)
    qDebug("CCP:testConfiguration()");
  reset();

  if (!_privileges->check("ProcessCreditCards"))
  {
    _errorMsg = errorMsg(-2);
    return -2;
  }

  if(!_metrics->boolean("CCAccept"))
  {
    _errorMsg = errorMsg(-3);
    return -3;
  }

  if (_metrics->value("CCDefaultBank").isEmpty())
  {
    _errorMsg = errorMsg(-4);
    return -4;
  }

  if (omfgThis->_key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  if(_metrics->boolean("CCUseProxyServer"))
  {
    _plogin = _metricsenc->value("CCProxyLogin");
    if (_plogin.isEmpty())
    {
      _errorMsg = errorMsg(-6);
      return -6;
    }

    _ppassword = _metricsenc->value("CCPassword");
    if (_ppassword.isEmpty())
    {
      _errorMsg = errorMsg(-7);
      return -7;
    }

    _pserver = _metrics->value("CCProxyServer");
    if (_pserver.isEmpty())
    {
      _errorMsg = errorMsg(-8);
      return -8;
    }

    _pport = _metrics->value("CCProxyPort");
    if (_pport.isEmpty())
    {
      _errorMsg = errorMsg(-9);
      return -9;
    }
  }

  if (! handlesChecks() && ! handlesCreditCards())
  {
    _errorMsg = errorMsg(-20);
    return -20;
  }

  if (isLive() == isTest())	// if both true or both false
  {
    _errorMsg = errorMsg(-13);
    return -13;
  }

  int returnValue = doTestConfiguration();

  // now handle warnings
  if (returnValue >= 0)
  {
    QString serverStr = buildURL(_metrics->value("CCServer"), _metrics->value("CCPort"), false);

    if (serverStr != defaultServer() && ! _metrics->value("CCPort").isEmpty() &&
        _metrics->value("CCPort").toInt() != defaultPort(isLive()) &&
        _metrics->value("CCPort").toInt() != 0)
    {
      _errorMsg = errorMsg(5).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCServer"))
                             .arg(defaultServer())
                             .arg(_metrics->value("CCPort"))
                             .arg(defaultPort(isLive()));
      returnValue = 5;
    }
    else if (serverStr != defaultServer())
    {
      _errorMsg = errorMsg(6).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCServer"))
                             .arg(defaultServer());
      returnValue = 6;
    }
    else if (_metrics->value("CCPort").toInt() != defaultPort(isLive()) &&
             _metrics->value("CCPort").toInt() != 0 &&
            ! _metrics->value("CCPort").isEmpty())
    {
      _errorMsg = errorMsg(7).arg(isLive() ? tr("Live") : tr("Test"))
                             .arg(_metrics->value("CCPort"))
                             .arg(defaultPort(isLive()));
      returnValue = 7;
    }
  }

  return returnValue;
}

int CreditCardProcessor::credit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, QString preftype, int &prefid)
{
  if (DEBUG)
    qDebug("CCP:credit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid,
	   preftype.toAscii().data(), prefid);
  reset();

  if (preftype == "cohead" && prefid < 0)
  {
    _errorMsg = errorMsg(-40).arg(preftype).arg(prefid);
    return -40;
  }

  QString ccard_x;
  int returnVal = checkCreditCard(pccardid, pcvv,  ccard_x);
  if (returnVal < 0)
    return returnVal;

  if (_metrics->boolean("CCConfirmCredit") &&
      QMessageBox::question(0,
	      tr("Confirm Credit Card Credit"),
              tr("Are you sure that you want to refund %2 %3 to credit card %1?")
		 .arg(ccard_x)
                 .arg(CurrDisplay::currSymbol(pcurrid))
                 .arg(pamount),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
  {
    _errorMsg = errorMsg(50);
    return 50;
  }

  if (pccpayid > 0)
  {
    int oldccpayid = pccpayid;

    XSqlQuery ccq;
    ccq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ccq.first())
      pccpayid = ccq.value("ccpay_id").toInt();
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
    else // no rows found is fatal because we haven't processed the credit yet
    {
      _errorMsg = errorMsg(2);
      return -1;
    }

    int next_seq = -1;
    ccq.prepare("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		"       AS next_seq "
		"  FROM ccpay "
		" WHERE (ccpay_order_number=:ccpay_order_number);");
    ccq.bindValue(":ccpay_order_number", preforder);
    ccq.exec();
    if (ccq.first())
      next_seq = ccq.value("next_seq").toInt();
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }

    ccq.prepare( "INSERT INTO ccpay ("
		 "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    ccpay_amount,"
		 "    ccpay_curr_id, ccpay_type, ccpay_status,"
		 "    ccpay_order_number, ccpay_order_number_seq"
		 ") SELECT "
		 "    :newccpayid, ccpay_ccard_id, ccpay_cust_id,"
		 "    ccpay_auth_charge, ccpay_auth,"
		 "    :amount, :currid, 'R', 'X',"
		 "    ccpay_order_number, :nextseq "
		 "FROM ccpay "
		 "WHERE (ccpay_id=:oldccpayid);");
    ccq.bindValue(":newccpayid", pccpayid);
    ccq.bindValue(":currid",     pcurrid);
    ccq.bindValue(":amount",     pamount);
    ccq.bindValue(":nextseq",    next_seq);
    ccq.bindValue(":oldccpayid", oldccpayid);
    ccq.exec();
    if (ccq.lastError().type() != QSqlError::NoError)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }

    ccq.prepare("SELECT ccpay_r_ordernum FROM ccpay WHERE (ccpay_id=:ccpayid);");
    ccq.bindValue(":ccpayid", oldccpayid);
    ccq.exec();
    if (ccq.first())
      preforder = ccq.value("ccpay_r_ordernum").toString();
    else if (ccq.lastError().type() != QSqlError::None)
    {
      _errorMsg = ccq.lastError().databaseText();
      return -1;
    }
    else
    {
      _errorMsg = errorMsg(-50);
      return -50;
    }
  }

  ParameterList dbupdateinfo;
  returnVal = doCredit(pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid, pneworder, preforder, pccpayid, dbupdateinfo);
  if (returnVal == 50)
    return 50;
  else if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  int ccpayReturn = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal == 0 && ccpayReturn != 0)
    returnVal = ccpayReturn;

  if (returnVal >= 0)
  {
    returnVal = fraudChecks();
    if (returnVal < 0)
    {
      int voidReturnVal = voidPrevious(pccpayid);
      return (voidReturnVal < 0) ? voidReturnVal : returnVal;
    }

    if (pccpayid > 0)
    {
      XSqlQuery cq;
      cq.prepare("SELECT postCCCredit(:ccpayid, :reftype, :refid) AS result;");
      cq.bindValue(":ccpayid", pccpayid);
      cq.bindValue(":reftype", preftype);
      cq.bindValue(":refid",   prefid);
      cq.exec();
      if (cq.first())
      {
	int result = cq.value("result").toInt();
	if (result < 0)
	{
	  _errorMsg = "<p>" +
		      errorMsg(4).arg(storedProcErrorLookup("postCCCredit",
								   result));
	  returnVal = 1;
	}
      }
      else if (cq.lastError().type() != QSqlError::NoError)
      {
	_errorMsg = errorMsg(4).arg(cq.lastError().databaseText());
	returnVal = 1;
      }
    }

    if (_metrics->boolean("CCPrintReceipt"))
    {
      int receiptReturn = printReceipt(pccpayid);
      if (receiptReturn != 0 && returnVal == 0)
        returnVal = receiptReturn;
    }
  }

  return returnVal;
}

int CreditCardProcessor::voidPrevious(int &pccpayid)
{
  if (DEBUG)
    qDebug("CCP:voidPrevious(%d)", pccpayid);
  // don't reset(); because we're probably voiding because of a previous error

  int ccv = -2;

  XSqlQuery ccq;
  ccq.prepare("SELECT * FROM ccpay WHERE (ccpay_id=:ccpayid);");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();

  int ccardid;
  if (ccq.first())
    ccardid = ccq.value("ccpay_ccard_id").toInt();
  else if (ccq.lastError().type() != QSqlError::None)
  {
    _errorMsg = ccq.lastError().databaseText();
    return -1;
  }
  else
  {
    _errorMsg = errorMsg(-60);
    return -60;
  }

  // don't checkCreditCard because we want to void the transaction regardless

  QString neworder = ccq.value("ccpay_order_number").toString();
  QString reforder = ccq.value("ccpay_r_ordernum").toString();
  QString approval = ccq.value("ccpay_r_ref").toString();
  ParameterList dbupdateinfo;
  int returnVal = doVoidPrevious(ccardid, ccv,
				 ccq.value("ccpay_amount").toDouble(),
				 ccq.value("ccpay_curr_id").toInt(),
				 neworder, reforder, approval,
                                 pccpayid, dbupdateinfo);
  if (returnVal == 60)
    return 60;
  else if (returnVal < 0)
    return returnVal;
  else if (returnVal > 0)
    _errorMsg = errorMsg(4).arg(_errorMsg);

  returnVal = updateCCPay(pccpayid, dbupdateinfo);
  if (returnVal < 0)
    return returnVal;

  ccq.prepare("SELECT postCCVoid(:ccpayid) AS result;");
  ccq.bindValue(":ccpayid", pccpayid);
  ccq.exec();
  if (ccq.first())
  {
    int result = ccq.value("result").toInt();
    if (result < 0)
    {
      _errorMsg = "<p>" +
		  errorMsg(4).arg(storedProcErrorLookup("postCCVoid",
							       result));
      returnVal = 1;
    }
  }
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
    returnVal = 1;
  }

  if (_metrics->boolean("CCPrintReceipt"))
  {
    int receiptReturn = printReceipt(pccpayid);
    if (receiptReturn != 0 && returnVal == 0)
      returnVal = receiptReturn;
  }

  return returnVal;
}

bool CreditCardProcessor::isLive()
{
  return (!_metrics->boolean("CCTest"));
}

bool CreditCardProcessor::isTest()
{
  return (_metrics->boolean("CCTest"));
}

QString CreditCardProcessor::errorMsg()
{
  return _errorMsg;
}

QString CreditCardProcessor::errorMsg(const int pcode)
{
  return _msgHash.value(pcode);
}

int CreditCardProcessor::checkCreditCard(int pccid, int pcvv, QString &pccard_x)
{
  if (DEBUG)
    qDebug("CCP:checkCreditCard(%d, %d)", pccid, pcvv);
  reset();

  if(omfgThis->_key.isEmpty())
  {
    _errorMsg = errorMsg(-5);
    return -5;
  }

  q.prepare( "SELECT ccard_active, ccard_cust_id, "
             "       formatbytea(decrypt(setbytea(ccard_month_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_month_expired,"
             "       formatbytea(decrypt(setbytea(ccard_year_expired),"
	     "               setbytea(:key), 'bf')) AS ccard_year_expired,"
             "       formatccnumber(decrypt(setbytea(ccard_number),"
	     "               setbytea(:key), 'bf')) AS ccard_number_x,"
             "       ccard_type "
             "FROM ccard "
             "WHERE (ccard_id=:ccardid);");
  q.bindValue(":ccardid", pccid);
  q.bindValue(":key",     omfgThis->_key);
  q.exec();
  if (q.first())
  {
    pccard_x = q.value("ccard_number_x").toString();

    if (!q.value("ccard_active").toBool())
    {
      _errorMsg = errorMsg(-10).arg(pccard_x);
      return -10;
    }

    if (QDate(q.value("ccard_year_expired").toInt(),
	      q.value("ccard_month_expired").toInt(),
	      1) < QDate::currentDate())
    {
      XSqlQuery xpq;
      xpq.prepare("SELECT expireCreditCard(:custid, setbytea(:key)) AS result;");
      xpq.bindValue(":custid", q.value("ccard_cust_id"));
      xpq.bindValue(":key", omfgThis->_key);
      xpq.exec();
      // ignore errors from expirecreditcard()
      _errorMsg = errorMsg(-11).arg(pccard_x);
      return -11;
    }
  }

  if (pcvv == -1 && _metrics->boolean("CCRequireCVV"))
  {
    _errorMsg = errorMsg(-98);
    return -98;
  }
  else if (pcvv == -1)
  {
    if (QMessageBox::question(0,
	      tr("Confirm No CVV Code"),
              tr("<p>You must confirm that you wish to proceed "
                 "without a CVV code. Would you like to continue?"),
              QMessageBox::Yes | QMessageBox::Default,
              QMessageBox::No  | QMessageBox::Escape ) == QMessageBox::No)
    {
      _errorMsg = errorMsg(99);
      return 99;
    }
  }
  else if (pcvv == -2)
    ; // the caller knows that this transaction doesn't have cvv available
  else if (pcvv > 9999) // YP docs are not consistent - 000-999 or 3-4 digits?
  {
    _errorMsg = errorMsg(-99);
    return -99;
  }

  return 0;
}

int CreditCardProcessor::doAuthorize(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doAuthorize(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doAuthorize");
  return -19;
}

int CreditCardProcessor::doCharge(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doCharge(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	    pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCharge");
  return -19;
}

int CreditCardProcessor::doChargePreauthorized(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doChargePreauthorized(%d, %d, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	    pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doChargePreauthorized");
  return -19;
}

int CreditCardProcessor::doTestConfiguration()
{
  if (DEBUG)
    qDebug("CCP:doTestConfiguration()");
  return 0;	// assume that subclasses will override IFF they need to
}

int CreditCardProcessor::doCredit(const int pccardid, const int pcvv, const double pamount, const double ptax, const bool ptaxexempt, const double pfreight, const double pduty, const int pcurrid, QString &pneworder, QString &preforder, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doCredit(%d, %d, %f, %f, %d, %f, %f, %d, %s, %s, %d)",
	   pccardid, pcvv, pamount, ptax, ptaxexempt, pfreight, pduty, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doCredit");
  return -19;
}

int CreditCardProcessor::doVoidPrevious(const int pccardid, const int pcvv, const double pamount, const int pcurrid, QString &pneworder, QString &preforder, QString &papproval, int &pccpayid, ParameterList &)
{
  if (DEBUG)
    qDebug("CCP:doVoidPrevious(%d, %d, %f, %d, %s, %s, %s, %d)",
	   pccardid, pcvv, pamount, pcurrid,
	   pneworder.toAscii().data(), preforder.toAscii().data(),
	   papproval.toAscii().data(), pccpayid);
  _errorMsg = errorMsg(-19).arg("doVoidPrevious");
  return -19;
}

int CreditCardProcessor::sendViaHTTP(const QString &prequest,
				     QString &presponse)
{
  if (DEBUG)
    qDebug("CCP:sendViaHTTP(input, output) with input:\n%s",
	   prequest.toAscii().data());

  // TODO: find a better place to save this
  if (isTest())
    _metrics->set("CCOrder", prequest);

  // TODO: why have a hard-coded path to curl?
  QProcess proc(this);
  QString curl_path;
#ifdef Q_WS_WIN
  curl_path = qApp->applicationDirPath() + "\\curl";
#elif defined Q_WS_MACX
  curl_path = "/usr/bin/curl";
#elif defined Q_WS_X11
  curl_path = "/usr/bin/curl";
#endif

  QStringList curl_args;
  curl_args.append( "-k" );
  curl_args.append( "-d" );
  curl_args.append( prequest );

#ifdef Q_WS_WIN
  QString pemfile = _metrics->value("CCYPWinPathPEM");
#elif defined Q_WS_MACX
  QString pemfile = _metrics->value("CCYPMacPathPEM");
#elif defined Q_WS_X11
  QString pemfile = _metrics->value("CCYPLinPathPEM");
#else
  QString pemfile;
#endif
  if (! pemfile.isEmpty())
  {
    curl_args.append( "-E" );
    curl_args.append(pemfile);
  }

  curl_args.append(buildURL(_metrics->value("CCServer"), _metrics->value("CCPort"), true));

  if(_metrics->boolean("CCUseProxyServer"))
  {
    curl_args.append( "-x" );
    curl_args.append(_metrics->value("CCProxyServer") +
		     QString(_metrics->value("CCProxyPort").toInt() == 0 ? "" :
					(":" + _metrics->value("CCProxyPort"))));
    curl_args.append( "-U" );
    curl_args.append(_metricsenc->value("CCProxyLogin") + ":" +
		     _metricsenc->value("CCPassword"));
  }

  QString curlCmd = curl_path + ((DEBUG) ? (" " + curl_args.join(" ")) : "");
  if (DEBUG)
    qDebug("%s", curlCmd.toAscii().data());

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  /* TODO: consider changing to the original implementation:
	      start the proc in a separate thread
	      capture the output as it's generated
	      while (proc->isRunning())
		qApp->processEvents();
   */
  proc.start(curl_path, curl_args);
  if ( !proc.waitForStarted() )
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (! proc.waitForFinished())
  {
    QApplication::restoreOverrideCursor();
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  QApplication::restoreOverrideCursor();

  if (proc.exitStatus() != QProcess::NormalExit)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg("")
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  if (proc.exitCode() != 0)
  {
    _errorMsg = errorMsg(-18)
		  .arg(curlCmd)
		  .arg(proc.exitCode())
		  .arg(QString(proc.readAllStandardError()));
    return -18;
  }

  presponse = proc.readAllStandardOutput();
  if (isTest())
    _metrics->set("CCTestMe", presponse);

  return 0;
}

int CreditCardProcessor::updateCCPay(int &pccpayid, ParameterList &pparams)
{
  if (DEBUG)
    qDebug("updateCCPay(%d, %d params)", pccpayid, pparams.size());

  QString sql;
  XSqlQuery ccq;

  bool valid;
  QVariant param;
  QString r_error;
  param = pparams.value("r_error", &valid);
  if (valid)
    r_error = param.toString();

  if (pccpayid > 0)
  {
    param = pparams.value("status", &valid);
    QString r_status;
    if (valid)
      r_status = param.toString();
    if (r_status == "X" || r_status == "D")
    {
      // ccpay_r_ordernum because it's used to build postauth/capture messages
      ccq.prepare("INSERT INTO ccpay ("
                  "    ccpay_ccard_id, ccpay_cust_id, ccpay_amount,"
                  "    ccpay_status, ccpay_type, ccpay_auth_charge,"
                  "    ccpay_order_number, ccpay_order_number_seq,"
                  "    ccpay_by_username, ccpay_curr_id, ccpay_r_ordernum "
                  ") SELECT "
                  "    ccpay_ccard_id, ccpay_cust_id, ccpay_amount,"
                  "    ccpay_status, ccpay_type, ccpay_auth_charge,"
                  "    ccpay_order_number, ccpay_order_number_seq + 1,"
                  "    ccpay_by_username, ccpay_curr_id, ccpay_r_ordernum "
                  "  FROM ccpay"
                  "  WHERE ((ccpay_id=:id)"
                  "     AND (ccpay_type='A'));");
      ccq.bindValue(":id", pccpayid);
      ccq.exec();
    }

    sql =  "UPDATE ccpay SET"
	   "<? if exists(\"fromcurr\") ?>"
	   "      ccpay_amount=ROUND(currToCurr(<? value(\"fromcurr\") ?>,"
	   "                                    <? value(\"tocurr\") ?>,"
	   "                                    <? value(\"amount\") ?>,"
	   "                                    CURRENT_DATE), 2),"
	   "       ccpay_curr_id=<? value(\"currid\") ?>,"
	   "<? else ?>"
	   "       ccpay_amount=ROUND(<? value(\"amount\") ?>, 2),"
	   "       ccpay_curr_id=<? value(\"currid\") ?>,"
	   "<? endif ?>"
	   "       ccpay_auth=<? value(\"auth\") ?>,"
	   "       ccpay_r_approved=<? value(\"approved\") ?>,"
	   "       ccpay_r_avs=<? value(\"avs\") ?>,"
	   "       ccpay_r_code=<? value(\"code\") ?>,"
	   "       ccpay_r_error=<? value(\"error\") ?>,"
	   "       ccpay_r_message=<? value(\"message\") ?>,"
	   "       ccpay_r_ordernum=<? value(\"ordernum\") ?>,"
	   "       ccpay_r_ref=<? value(\"ref\") ?>,"
	   "<? if exists(\"shipping\") ?>"
	   "       ccpay_r_shipping=<? value(\"shipping\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"score\") ?>"
	   "       ccpay_yp_r_score=<? value(\"score\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"tax\") ?>"
	   "       ccpay_r_tax=<? value(\"tax\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"tdate\") ?>"
	   "       ccpay_yp_r_tdate=<? value(\"tdate\") ?>,"
	   "<? endif ?>"
	   "<? if exists(\"time\") ?>"
	   "       ccpay_yp_r_time=<? value(\"time\")?>,"
	   "<? endif ?>"
	   "       ccpay_status=<? value(\"status\") ?>"
	   " WHERE (ccpay_id=<? value(\"ccpay_id\") ?>);" ;
  }
  else
  {
    ccq.exec("SELECT NEXTVAL('ccpay_ccpay_id_seq') AS ccpay_id;");
    if (ccq.first())
      pccpayid = ccq.value("ccpay_id").toInt();
    else if (ccq.lastError().type() != QSqlError::None && r_error.isEmpty())
    {
      _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
      return 1;
    }
    else if (ccq.lastError().type() == QSqlError::None && r_error.isEmpty())
    {
      _errorMsg = errorMsg(4).arg(errorMsg(2));
      return 2;
    }
    else	// no rows found and YP reported an error
    {
      _errorMsg = errorMsg(-12).arg(r_error);
      return -12;
    }

    if (pparams.inList("ordernum") && pparams.value("ordernum").toInt() > 0)
    {
      QString maxs("SELECT MAX(COALESCE(ccpay_order_number_seq, -1)) + 1"
		   "       AS next_seq "
		   "  FROM ccpay "
		   " WHERE (ccpay_order_number=<? value(\"ordernum\") ?>);");
      MetaSQLQuery maxm(maxs);
      ccq = maxm.toQuery(pparams);
      if (ccq.first())
	pparams.append("next_seq", ccq.value("next_seq"));
      else
	pparams.append("next_seq", 0);
    }
    else
	pparams.append("next_seq", 0);

    sql =  "INSERT INTO ccpay ("
	   "    ccpay_id, ccpay_ccard_id, ccpay_cust_id,"
	   "    ccpay_type,"
	   "    ccpay_amount,"
	   "    ccpay_curr_id,"
	   "    ccpay_auth, ccpay_auth_charge,"
	   "    ccpay_order_number,"
	   "    ccpay_order_number_seq,"
	   "    ccpay_r_approved, ccpay_r_avs,"
	   "    ccpay_r_code,    ccpay_r_error,"
	   "    ccpay_r_message, ccpay_r_ordernum,"
	   "    ccpay_r_ref,"
	   "<? if exists(\"score\") ?>    ccpay_yp_r_score, <? endif ?>"
	   "<? if exists(\"shipping\") ?> ccpay_r_shipping, <? endif ?>"
	   "<? if exists(\"tax\") ?>      ccpay_r_tax,   <? endif ?>"
	   "<? if exists(\"tdate\") ?>    ccpay_yp_r_tdate, <? endif ?>"
	   "<? if exists(\"time\") ?>     ccpay_yp_r_time,  <? endif ?>"
	   "    ccpay_status"
	   ") SELECT <? value(\"ccpay_id\") ?>, ccard_id, cust_id,"
	   "    <? value(\"type\") ?>,"
	   "<? if exists(\"fromcurr\") ?>"
	   "    ROUND(currToCurr(<? value(\"fromcurr\") ?>,"
	   "                     <? value(\"tocurr\") ?>,"
	   "                     <? value(\"amount\") ?>,CURRENT_DATE), 2),"
	   "    <? value(\"tocurr\") ?>,"
	   "<? else ?>"
	   "    ROUND(<? value(\"amount\") ?>, 2),"
	   "    <? value(\"currid\") ?>,"
	   "<? endif ?>"
	   "    <? value(\"auth\") ?>,     <? value(\"auth_charge\") ?>,"
	   "    <? value(\"ordernum\") ?>,"
	   "    COALESCE(<? value(\"next_seq\") ?>, 1),"
	   "    <? value(\"approved\") ?>, <? value(\"avs\") ?>,"
	   "    <? value(\"code\") ?>,     <? value(\"error\") ?>,"
	   "    <? value(\"message\") ?>,  <? value(\"xactionid\") ?>,"
	   "    <? value(\"ref\") ?>,"
	   "<? if exists(\"score\") ?>    <? value(\"score\") ?>,   <? endif ?>"
	   "<? if exists(\"shipping\") ?> <? value(\"shipping\") ?>,<? endif ?>"
	   "<? if exists(\"tax\") ?>      <? value(\"tax\") ?>,     <? endif ?>"
	   "<? if exists(\"tdate\") ?>    <? value(\"tdate\") ?>,   <? endif ?>"
	   "<? if exists(\"time\") ?>     <? value(\"time\") ?>,    <? endif ?>"
	   "    <? value(\"status\") ?>"
	   "  FROM ccard, custinfo"
	   "  WHERE ((ccard_cust_id=cust_id)"
	   "    AND  (ccard_id=<? value(\"ccard_id\") ?>));";
  }

  pparams.append("ccpay_id", pccpayid);

  MetaSQLQuery mql(sql);
  ccq = mql.toQuery(pparams);

  if (ccq.lastError().type() != QSqlError::None)
  {
    pccpayid = -1;
    _errorMsg = errorMsg(4).arg(ccq.lastError().databaseText());
    return 1;
  }

  return 0;
}

int CreditCardProcessor::defaultPort(bool ptestmode)
{
  if (ptestmode)
    return _defaultTestPort;
  else
    return _defaultLivePort;
}

QString CreditCardProcessor::defaultServer()
{
  if (isTest())
    return _defaultTestServer;
  else if (isLive())
    return _defaultLiveServer;
  else
    return "";
}

void CreditCardProcessor::reset()
{
  _errorMsg = "";
  _passedAvs = true;
  _passedCvv = true;
}

int CreditCardProcessor::fraudChecks()
{
  if (DEBUG)
    qDebug("CCP:fraudChecks()");

  int returnValue = 0;

  if (! _passedAvs && _metrics->value("CCAvsCheck") == "F")
  {
    _errorMsg = errorMsg(-97);
    returnValue = -97;
  }
  else if (! _passedAvs && _metrics->value("CCAvsCheck") == "W")
  {
    _errorMsg = errorMsg(97);
    returnValue = 97;
  }

  // not "else if" - maybe this next check will be fatal
  if (! _passedCvv && _metrics->value("CCCVVCheck") == "F")
  {
    _errorMsg = errorMsg(-96);
    returnValue = -96;
  }
  else if (! _passedCvv && _metrics->value("CCCVVCheck") == "W")
  {
    _errorMsg = errorMsg(96);
    returnValue = 96;
  }

  if (DEBUG)
    qDebug("CCP:fraudChecks() returning %d with _errorMsg %s",
	   returnValue, _errorMsg.toAscii().data());

  return returnValue;
}

int CreditCardProcessor::printReceipt(const int pccpayid)
{
  if (DEBUG)
    qDebug("CCP:printReceipt()");

  int returnValue = 0;

  ParameterList params;
  params.append("ccpay_id",  pccpayid);
  params.append("preauth",   tr("Preauthorization"));
  params.append("charge",    tr("Charge"));
  params.append("refund",    tr("Refund"));
  params.append("authorized",tr("Authorized"));
  params.append("approved",  tr("Approved"));
  params.append("declined",  tr("Declined"));
  params.append("voided",    tr("Voided"));
  params.append("noapproval",tr("No Approval Code"));
  params.append("key",       omfgThis->_key);

  orReport report("CCReceipt", params);

  if (report.isValid())
    report.print();
  else
  {
    report.reportError(0);
    _errorMsg = errorMsg(94);
    returnValue = 94;
  }

  if (DEBUG)
    qDebug("CCP:printReceipt() returning %d with _errorMsg %s",
	   returnValue, _errorMsg.toAscii().data());

  return returnValue;
}

// use a different version here than in CurrDisplay because we have special
// error reporting needs
double CreditCardProcessor::currToCurr(const int pfrom, const int pto, const double pamount, int *perror)
{
  if (pfrom == pto)
    return pamount;

  XSqlQuery ccq;
  ccq.prepare("SELECT ROUND(currToCurr(:fromid, :toid,"
	      "                :amount, CURRENT_DATE), 2) AS result;");
  ccq.bindValue(":fromid", pfrom);
  ccq.bindValue(":toid",   pto);
  ccq.bindValue(":amount", pamount);
  ccq.exec();

  if (ccq.first())
    return ccq.value("result").toDouble();
  else if (ccq.lastError().type() != QSqlError::NoError)
  {
    _errorMsg = ccq.lastError().databaseText();
    if (perror)
      *perror = -1;
  }

  return 0;
}

bool CreditCardProcessor::handlesChecks()
{
  return false;
}

bool CreditCardProcessor::handlesCreditCards()
{
  return false;
}

/* take apart the url we got from the user and rebuild it to make sure it's complete.
   if the user did not specify a URL to start with then build it using defaults.
   the third argument tells us whether to build with or without the port.
*/
QString CreditCardProcessor::buildURL(const QString pserver, const QString pport, const bool pinclport)
{
  if (DEBUG)
    qDebug("buildURL(%s, %s, %d)", pserver.toAscii().data(),
           pport.toAscii().data(), pinclport);

  QString defaultprotocol = "https";

  QString serverStr = pserver.isEmpty() ?  defaultServer() : pserver;
  QString protocol  = serverStr;
  protocol.remove(QRegExp("://.*"));
  if (protocol == serverStr)
    protocol = "";
  if (DEBUG) qDebug("protocol: %s", protocol.toAscii().data());
  if (protocol.isEmpty())
    protocol = defaultprotocol;

  QString host      = serverStr;
  host.remove(QRegExp("^.*://")).remove(QRegExp("[/:].*"));
  if (DEBUG) qDebug("host: %s", host.toAscii().data());

  QString port      = serverStr;
  port.remove(QRegExp("^([^:/]+://)?[^:]*:")).remove(QRegExp("/.*"));
  if (port == serverStr || port == host)
    port = "";
  if (DEBUG) qDebug("port: %s", port.toAscii().data());
  if (! pinclport)
    port = "";
  else if (port.isEmpty())
    port = pport.toInt() == 0 ?  QString::number(defaultPort()) : pport;

  QString remainder = serverStr;
  remainder.remove(QRegExp("^([^:/]+://)?[^:/]*(:[0-9]+)?/"));
  if (remainder == serverStr)
    remainder = "";
  if (DEBUG) qDebug("remainder: %s", remainder.toAscii().data());

  serverStr = protocol + "://" + host;

  if (! port.isEmpty() && ! (protocol == "https" && port == "443"))
    serverStr += ":" + port;

  if (! remainder.isEmpty())
    serverStr += "/" + remainder;

  if (DEBUG) qDebug("buildURL: returning %s", serverStr.toAscii().data());
  return serverStr;
}
