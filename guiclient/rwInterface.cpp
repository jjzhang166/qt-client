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

//  rwInterface.cpp
//  Created 10/28/2002, JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <stdlib.h>

#include <QMessageBox>
#include <QApplication>
#include <QCursor>
#include <QFile>
#include <QTextStream>

#include "guiclient.h"

#include "rwInterface.h"

bool rwInterface::exportArdist(QWidget *pParent)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  QFile distFile(_metrics->value("AccountingSystemExportPath") + "/ARDIST00.EXP");
  if (distFile.open(QIODevice::WriteOnly))
  {
    q.exec( "SELECT accnt_number, accnt_profit, accnt_sub,"
            "       to_char(gltrans_date, 'YYYYMMDD') AS distdate,"
            "       cust_number, isNumeric(cust_number) AS isnumeric,"
            "       LPAD(SUBSTR(TEXT(gltrans_journalnumber), 1, 4), 4, '0') AS journalnumber,"
            "       gltrans_docnumber, usr_initials,"
            "       (SUM(gltrans_amount) * 100)::INTEGER AS amount "
            "FROM gltrans, invchead, cust, accnt, usr "
            "WHERE ( (gltrans_accnt_id=accnt_id)"
            " AND (gltrans_docnumber=TEXT(invchead_invcnumber))"
            " AND (invchead_cust_id=cust_id)"
            " AND (NOT gltrans_exported)"
            " AND (gltrans_source='A/R')"
            " AND (gltrans_doctype='IN')"
            " AND (usr_username=CURRENT_USER) ) "
            "GROUP BY accnt_number, accnt_profit, accnt_sub,"
            "         journalnumber, distdate, cust_number, gltrans_docnumber,"
            "         usr_initials "

            "UNION SELECT accnt_number, accnt_profit, accnt_sub,"
            "             to_char(gltrans_date, 'YYYYMMDD') AS distdate,"
            "             cust_number, isNumeric(cust_number) AS isnumeric,"
            "             LPAD(SUBSTR(TEXT(gltrans_journalnumber), 1, 4), 4, '0') AS journalnumber,"
            "             gltrans_docnumber, usr_initials,"
            "             (SUM(gltrans_amount) * 100)::INTEGER AS amount "
            "FROM gltrans, cmhead, cust, accnt, usr "
            "WHERE ( (gltrans_accnt_id=accnt_id)"
            " AND (gltrans_docnumber=TEXT(cmhead_number))"
            " AND (cmhead_cust_id=cust_id)"
            " AND (NOT gltrans_exported)"
            " AND (gltrans_source='A/R')"
            " AND (gltrans_doctype='CM')"
            " AND (usr_username=CURRENT_USER) )"
            "GROUP BY accnt_number, accnt_profit, accnt_sub,"
            "         journalnumber, distdate, cust_number, gltrans_docnumber,"
            "         usr_initials " );

    if (q.first())
    {
      QTextStream textStream(&distFile);
      do
      {
        QString format;

        if (q.value("amount").toInt() < 0)
          textStream << "1";
        else
          textStream << "3";

        if (_metrics->value("AccountingSystem") == "RealWorld91")
        {

          if (_metrics->value("GLProfitSize").toInt() == 0)
            textStream << "00000000";                                                                               //  AR-DIST-PFT-CTR-1
          else
          {
            format = "%0" + QString("%1").arg(_metrics->value("GLProfitSize").toInt()) + "d";
        
            textStream << QString().sprintf(format, q.value("accnt_profit").toInt());                               //  AR-DIST-PFT-CTR-1
            textStream << QString().fill('0', (8 - _metrics->value("GLProfitSize").toInt()));                       //  Filler '0's
          }

          textStream << "00000000";                                                                                 //  AR-DIST-PFT-CTR-2
        }

        format = "%0" + QString("%1").arg(_metrics->value("GLMainSize").toInt()) + "d";
        
        textStream << QString().sprintf(format, q.value("accnt_number").toInt());                                   //  AR-DIST-MAIN-ACCT-NO
        textStream << QString().fill('0', (8 - _metrics->value("GLMainSize").toInt()));                             //  Filler '0's

        if (_metrics->value("GLSubaccountSize").toInt() == 0)
          textStream << "00000000";                                                                                 //  AR-DIST-SUB-ACCT-NO
        else
        {
          format = "%0" + QString("%1").arg(_metrics->value("GLSubaccountSize").toInt()) + "d";
        
          textStream << QString().sprintf(format, q.value("accnt_sub").toInt());                                    //  AR-DIST-SUB-ACCT-NO
          textStream << QString().fill('0', (8 - _metrics->value("GLSubaccountSize").toInt()));                     //  Filler '0's
        }

        textStream << q.value("distdate").toString();                                                               //  AR-DIST-DAT
        textStream << "AR";                                                                                         //  AR-DIST-JRNL-COD
        textStream << q.value("journalnumber").toString();                                                          //  AR-DIST-JRNL-RPT-NO

        if (q.value("isnumeric").toBool())
        {
          textStream << QString().fill('0', (12 - q.value("cust_number").toString().length()));                     //  Filler '0's
          textStream << q.value("cust_number").toString();                                                          //  AR-OPN-CUST-NO
        }
        else
          textStream << q.value("cust_number").toString().leftJustify(12, ' ', TRUE);                               //  AR-OPN-CUST-NO

        textStream << QString().sprintf("%08d", q.value("gltrans_docnumber").toInt());                              //  AR-DIST-DOC-NO

        if (q.value("usr_initials").toString().length())
          textStream << q.value("usr_initials").toString().leftJustify(3, ' ', TRUE);                               //  AR-DIST-USR-ID
        else
          textStream << "OMG";                                                                                      //  AR-DIST-USR-ID

        textStream << " ";                                                                                          //  AR-DIST-CORR-FLG

        if (q.value("amount").toInt() < 0)
          textStream << QString().sprintf("%014d+", abs(q.value("amount").toInt()));                                //  AR-DIST-AMT
        else
          textStream << QString().sprintf("%014d-", q.value("amount").toInt());                                     //  AR-DIST-AMT

        textStream << "N\n";                                                                                        //  AR-DIST-POST-TO-GL-FLG, EOL
      }
      while (q.next());
    }

    distFile.close();

    qApp->restoreOverrideCursor();

    return TRUE;
  }
  else
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot ARDIST File"),
                           QObject::tr( "The ARDIST Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                                        "A/R Distributions were NOT exported." ) );

    qApp->restoreOverrideCursor();

    return FALSE;
  }
}


bool rwInterface::exportAropen(QWidget *pParent)
{
//  Check to see if any aropen items are for un-exported Customers
  qApp->setOverrideCursor(Qt::waitCursor);
  q.exec( "SELECT cust_id, cust_number, cust_name "
          "FROM aropen, cust "
          "WHERE ( (aropen_cust_id=cust_id)"
          " AND (NOT aropen_posted)"
          " AND (NOT cust_exported) ) "
          "LIMIT 1;" );
  qApp->restoreOverrideCursor();
  if (q.first())
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot Export A/R Open Items"),
                           QObject::tr( "A/R Open Items cannot be exported to RealWorld as one or more items have been\n"
                                        "posted against Customers that have not been exported to RealWorld.  Please make\n"
                                        "sure to export all new Customers from OpenMFG to RealWorld.\n" ) );
    return FALSE;
  }

  QFile openFile(_metrics->value("AccountingSystemExportPath") + "/AROPEN00.EXP");
  if (openFile.open(QIODevice::WriteOnly))
  {
    q.exec( "SELECT cust_number, isNumeric(cust_number) AS isnumeric,"
            "       TO_CHAR(aropen_docdate, 'YYYYMMDD') AS docdate,"
            "       TO_CHAR(aropen_duedate, 'YYYYMMDD') AS duedate, aropen_docnumber,"
            "       aropen_doctype, aropen_applyto, (aropen_amount * 100)::INTEGER AS amount,"
            "       terms_code, aropen_ponumber, aropen_ordernumber, salesrep_number,"
            "       (aropen_commission_due * 100)::INTEGER AS commission, aropen_commission_paid,"
            "       usr_initials "
            "FROM aropen, cust, terms, salesrep, usr "
            "WHERE ( (aropen_cust_id=cust_id)"
            " AND (aropen_terms_id=terms_id)"
            " AND (aropen_salesrep_id=salesrep_id)"
            " AND (aropen_doctype='I')"
            " AND (usr_username=CURRENT_USER)"
            " AND (NOT aropen_posted) ) "

            "UNION SELECT cust_number, isNumeric(cust_number) AS isnumeric,"
            "             TO_CHAR(aropen_docdate, 'YYYYMMDD') AS docdate,"
            "             TO_CHAR(aropen_duedate, 'YYYYMMDD') AS duedate, aropen_docnumber,"
            "             'C' AS aropen_doctype, aropen_applyto, (aropen_amount * -100)::INTEGER AS amount,"
            "             '   ' AS terms_code, aropen_ponumber, aropen_ordernumber, salesrep_number,"
            "             (aropen_commission_due * 100)::INTEGER AS commission, aropen_commission_paid,"
            "             usr_initials "
            "FROM aropen, cust, salesrep, usr "
            "WHERE ( (aropen_cust_id=cust_id)"
            " AND (aropen_salesrep_id=salesrep_id)"
            " AND (aropen_doctype IN ('C', 'R'))"
            " AND (usr_username=CURRENT_USER)"
            " AND (NOT aropen_posted) )" );
    if (q.first())
    {
      QTextStream textStream(&openFile);

      do
      {
        QString customerString;
        bool    flag;

        if (q.value("isnumeric").toBool())
        {
          customerString = QString().fill('0', (12 - q.value("cust_number").toString().length()));
          customerString += q.value("cust_number").toString();
        }
        else
          customerString = q.value("cust_number").toString().leftJustify(12, ' ', TRUE);

        textStream << customerString;                                                                                //  AR-OPN-CUST-NO
        textStream << q.value("docdate").toString();                                                                 //  AR-OPN-DOC-DAT
        textStream << QString().sprintf("%08d", q.value("aropen_docnumber").toInt());                                //  AR-OPN-DOC-NO
        textStream << q.value("aropen_doctype").toString();                                                          //  AR-OPN-DOC-TYP

        q.value("aropen_applyto").toInt(&flag);
        if (flag)
          textStream << QString().sprintf("%08d", q.value("aropen_applyto").toInt());                                //  AR-OPN-APPL-TO-NO (numeric)
        else
          textStream << q.value("aropen_applyto").toString().leftJustify(8, ' ', TRUE);                              //  AR-OPN-APPL-TO-NO (string)

        textStream << customerString;                                                                                //  AR-OPN-CUST-NO-ALT

        q.value("aropen_applyto").toInt(&flag);
        if (flag)
          textStream << QString().sprintf("%08d", q.value("aropen_applyto").toInt());                                //  AR-OPN-APPL-TO-NO-ALT (numeric)
        else
          textStream << q.value("aropen_applyto").toString().leftJustify(8, ' ', TRUE);                              //  AR-OPN-APPL-TO-NO-ALT (string)

        textStream << q.value("docdate").toString();                                                                 //  AR-OPN-DOC-DAT-ALT
        textStream << QString().sprintf("%08d", q.value("aropen_docnumber").toInt());                                //  AR-OPN-DOC-NO-ALT
        textStream << q.value("aropen_doctype").toString();                                                          //  AR-OPN-DOC-TYP-ALT
        textStream << q.value("duedate").toString();                                                                 //  AR-OPN-DOC-DUE-DAT

        if (_metrics->value("AccountingSystem") == "RW2000")
        {
          if (q.value("amount").toInt() > 0)
            textStream << QString().sprintf("%012d+", q.value("amount").toInt());                                    //  AR-OPN-AMT-1
          else
            textStream << QString().sprintf("%012d-", (q.value("amount").toInt() * -1));                             //  AR-OPN-AMT-1

          textStream << "00000000000+";                                                                              //  AR-OPN-AMT-2
        }
        else if (_metrics->value("AccountingSystem") == "RealWorld91")
        {
          if (q.value("amount").toInt() > 0)
            textStream << QString().sprintf("%014d+", q.value("amount").toInt());                                    //  AR-OPN-AMT-1
          else
            textStream << QString().sprintf("%014d-", (q.value("amount").toInt() * -1));                             //  AR-OPN-AMT-1

          textStream << "00000000000000+";                                                                           //  AR-OPN-AMT-2
          textStream << "00000000000000+";                                                                           //  AR-OPN-AMT-3
        }

        textStream << q.value("terms_code").toString().leftJustify(3, ' ', TRUE);                                    //  AR-OPN-TERMS-COD
        textStream << q.value("aropen_ponumber").toString().leftJustify(15, ' ', TRUE);                              //  AR-OPN-PO-NO

        textStream << "From our order: ";
        textStream << q.value("aropen_ordernumber").toString().leftJustify(8, ' ', TRUE);
        textStream << " ";                                                                                           //  AR-OPN-REF

        textStream << q.value("salesrep_number").toString().leftJustify(3, ' ', TRUE);                               //  AR-OPN-SLS-REP
        textStream << "0000000000+";                                                                                 //  AR-OPN-COMMIS-AMT

        if (q.value("usr_initials").toString().length())
          textStream << q.value("usr_initials").toString().leftJustify(3, ' ', TRUE);                                //  AR-OPN-USER-ID
        else
          textStream << "OMG";                                                                                       //  AR-OPN-USER-ID

        textStream << "N";                                                                                           //  AR-OPN-COMMIS-PD-FLG

        if (_metrics->value("AccountingSystem") == "RealWorld91")
        {
          textStream << "00000000";                                                                                  //  AR-OPN-CASH-PFT-CTR-1
          textStream << "00000000";                                                                                  //  AR-OPN-CASH-PFT-CTR-2
          textStream << "00000000";                                                                                  //  AR-OPN-CASH-MAIN
          textStream << "00000000";                                                                                  //  AR-OPN-CASH-SUB-ACCT-NO
          textStream << "00000000";                                                                                  //  AR-OPN-ALLOW-PFT-CTR-1
          textStream << "00000000";                                                                                  //  AR-OPN-ALLOW-PFT-CTR-2
          textStream << "00000000";                                                                                  //  AR-OPN-ALLOW-MAIN
          textStream << "00000000";                                                                                  //  AR-OPN-ALLOW-SUB-ACCT-NO
          textStream << "00000000000000+";                                                                           //  AR-OPN-CURRENCY-AMT
          textStream << "00000000000+";                                                                              //  AR-OPN-CURRENCY-RATE
        }

        textStream << "\n";                                                                                          //  EOL
      }
      while (q.next());
    }
    openFile.close();

    return TRUE;
  }
  else
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot Open AROPEN File"),
                           QObject::tr( "The AROPEN Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                                        "A/R Open Items were NOT exported." ) );

    return FALSE;
  }
}

bool rwInterface::exportApdist(QWidget *pParent)
{
  qApp->setOverrideCursor(Qt::waitCursor);

  QFile distFile(_metrics->value("AccountingSystemExportPath") + "/APDIST00.EXP");
  if (distFile.open(QIODevice::WriteOnly))
  {
    q.exec( "SELECT accnt_number, accnt_profit, accnt_sub,"
            "       TO_CHAR(gltrans_date, 'YYYYMMDD') AS distdate,"
            "       vend_number, isNumeric(vend_number) AS isnumeric,"
            "       vend_name, vohead_reference, vohead_invcnumber,"
            "       LPAD(SUBSTR(TEXT(gltrans_journalnumber), 1, 4), 4, '0') AS journalnumber,"
            "       gltrans_docnumber, usr_initials,"
            "       (gltrans_amount * 100)::INTEGER AS amount "
            "FROM gltrans, vohead, pohead, vend, accnt, usr "
            "WHERE ( (gltrans_accnt_id=accnt_id)"
            " AND (gltrans_docnumber=TEXT(vohead_number))"
            " AND (vohead_pohead_id=pohead_id)"
            " AND (pohead_vend_id=vend_id)"
            " AND (NOT gltrans_exported)"
            " AND (gltrans_source='A/P')"
            " AND (gltrans_doctype='VO')"
            " AND (usr_username=CURRENT_USER) );" );

    if (q.first())
    {
      QTextStream textStream(&distFile);

      do
      {
        QString format;

        textStream << "1";                                                                                          //  AP-DIST-TYP

        if (_metrics->value("AccountingSystem") == "RealWorld91")
        {

          if (_metrics->value("GLProfitSize").toInt() == 0)
            textStream << "00000000";                                                                               //  AP-DIST-PFT-CTR-1
          else
          {
            format = "%0" + QString("%1").arg(_metrics->value("GLProfitSize").toInt()) + "d";
        
            textStream << QString().sprintf(format, q.value("accnt_profit").toInt());                               //  AP-DIST-PFT-CTR-1
            textStream << QString().fill('0', (8 - _metrics->value("GLProfitSize").toInt()));                       //  Filler '0's
          }

          textStream << "00000000";                                                                                 //  AP-DIST-PFT-CTR-2
        }

        format = "%0" + QString("%1").arg(_metrics->value("GLMainSize").toInt()) + "d";
        
        textStream << QString().sprintf(format, q.value("accnt_number").toInt());                                   //  AP-DIST-MAIN-ACCT-NO
        textStream << QString().fill('0', (8 - _metrics->value("GLMainSize").toInt()));                             //  Filler '0's

        if (_metrics->value("GLSubaccountSize").toInt() == 0)
          textStream << "00000000";                                                                                 //  AP-DIST-SUB-ACCT-NO
        else
        {
          format = "%0" + QString("%1").arg(_metrics->value("GLSubaccountSize").toInt()) + "d";
        
          textStream << QString().sprintf(format, q.value("accnt_sub").toInt());                                    //  AP-DIST-SUB-ACCT-NO
          textStream << QString().fill('0', (8 - _metrics->value("GLSubaccountSize").toInt()));                     //  Filler '0's
        }

        textStream << q.value("distdate").toString();                                                               //  AP-DIST-DAT
        textStream << QString().fill(' ', 32);                                                                      //  AP-DIST-CASH-ACCT-NO
        textStream << QString().sprintf("%06d", q.value("gltrans_docnumber").toInt());                              //  AP-DIST-DOC-NO

        if (q.value("usr_initials").toString().length())
          textStream << q.value("usr_initials").toString().leftJustify(3, ' ', TRUE);                               //  AP-DIST-USR-ID
        else
          textStream << "OMG";                                                                                      //  AP-DIST-USR-ID

        textStream << "AP";                                                                                         //  AP-DIST-JRNL-COD
        textStream << q.value("journalnumber").toString();                                                          //  AP-DIST-JRNL-RPT-NO
        textStream << " ";                                                                                          //  AP-DIST-CORR-FLG

        if (q.value("isnumeric").toBool())
        {
          textStream << QString().fill('0', (6 - q.value("vend_number").toString().length()));                      //  Filler '0's
          textStream << q.value("vend_number").toString();                                                          //  AP-DIST-VEND-NO
        }
        else
          textStream << q.value("vend_number").toString().leftJustify(6, ' ', TRUE);                                //  AP-DIST-VEND-NO

        if (q.value("vend_name").toString().length() > 25)
          textStream << q.value("vend_name").toString().left(25);                                                   //  AP-DIST-VEND-NAM
        else
        {
          textStream << q.value("vend_name").toString();                                                            //  AP-DIST-VEND-NAM
          textStream << QString().fill(' ', (25 - q.value("vend_name").toString().length()));                       //  Filler ' 's
        }

        if (q.value("vohead_reference").toString().length() > 25)
          textStream << q.value("vohead_reference").toString().left(25);                                            //  AP-DIST-TRX-REF
        else
        {
          textStream << q.value("vohead_reference").toString();                                                     //  AP-DIST-TRX-REF
          textStream << QString().fill(' ', (25 - q.value("vohead_reference").toString().length()));                //  Filler ' 's
        }

        if (q.value("amount").toInt() < 0)
          textStream << QString().sprintf("%014d+", abs(q.value("amount").toInt()));                                //  AP-DIST-AMT
        else
          textStream << QString().sprintf("%014d-", q.value("amount").toInt());                                     //  AP-DIST-AMT

        if (q.value("vohead_invcnumber").toString().length() > 15)
          textStream << q.value("vohead_invcnumber").toString().left(15);                                           //  AP-DIST-INVC-NO
        else
        {
          textStream << q.value("vohead_invcnumber").toString();                                                    //  AP-DIST-INVC-NO
          textStream << QString().fill(' ', (15 - q.value("vohead_invcnumber").toString().length()));               //  Filler ' 's
        }

        textStream << "N";                                                                                          //  AP-DIST-POST-TO-GL-FLG
        textStream << "\n";                                                                                         //  EOL
      }
      while (q.next());
    }

    distFile.close();

    qApp->restoreOverrideCursor();

    return TRUE;
  }
  else
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot ARDIST File"),
                           QObject::tr( "The ARDIST Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                                        "A/R Distributions were NOT exported." ) );

    qApp->restoreOverrideCursor();

    return FALSE;
  }
}

bool rwInterface::exportApopen(QWidget *pParent)
{
//  Check to see if any apopen items are for un-exported Vendors
  qApp->setOverrideCursor(Qt::waitCursor);
  q.exec( "SELECT vend_id, vend_number, vend_name "
          "FROM apopen, vend "
          "WHERE ( (apopen_vend_id=vend_id)"
          " AND (NOT apopen_posted)"
          " AND (NOT vend_exported) ) "
          "LIMIT 1;" );
  qApp->restoreOverrideCursor();
  if (q.first())
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot Export A/P Open Items"),
                           QObject::tr( "A/P Open Items cannot be exported to RealWorld as one or more items have been\n"
                                        "posted against Vendors that have not been exported to RealWorld.  Please make\n"
                                        "sure to export all new Vendors from OpenMFG to RealWorld.\n" ) );
    return FALSE;
  }

  QFile openFile(_metrics->value("AccountingSystemExportPath") + "/APOPEN00.EXP");
  if (openFile.open(QIODevice::WriteOnly))
  {
    q.exec( "SELECT vend_number, isNumeric(vend_number) AS isnumeric,"
            "       TO_CHAR(apopen_docdate, 'YYYYMMDD') AS distdate,"
            "       TO_CHAR(apopen_docdate, 'YYYYMMDD') AS docdate,"
            "       TO_CHAR(apopen_duedate, 'YYYYMMDD') AS duedate,"
            "       apopen_reference, apopen_docnumber, apopen_ponumber, apopen_invcnumber,"
            "       (apopen_amount * 100)::INTEGER AS amount,"
            "       accnt_number, accnt_profit, accnt_sub "
            "FROM apopen, vend, accnt "
            "WHERE ( (apopen_vend_id=vend_id)"
            " AND (findAPAccount(vend_id)=accnt_id)"
            " AND (NOT apopen_posted) ) "
            "ORDER BY vend_number;" );
    if (q.first())
    {
      QTextStream textStream(&openFile);

      do
      {
        QString vendorString;

        textStream << "0";                                                                                           //  AP-OPN-SEL-STAT
        textStream << " ";                                                                                           //  AP-OPN-ITEM-OK-TO-PURGE-FLG

        if (q.value("isnumeric").toBool())
        {
          vendorString = QString().fill('0', (6 - q.value("vend_number").toString().length()));
          vendorString += q.value("vend_number").toString();
        }
        else
          vendorString = q.value("vend_number").toString().leftJustify(6, ' ', TRUE);

        textStream << vendorString;                                                                                  //  AP-OPN-VEND-NO
        textStream << QString().sprintf("%06d", q.value("apopen_docnumber").toInt());                                //  AP-OPN-VCHR-NO
        textStream << "000000";                                                                                      //  AP-OPN-ITEM-ALT-BATCH-NO
        textStream << vendorString;                                                                                  //  AP-OPN-ITEM-ALT-VEND-NO
        textStream << QString().sprintf("%06d", q.value("apopen_docnumber").toInt());                                //  AP-OPN-ITEM-ALT-VCHR-NO
        textStream << "R";                                                                                           //  AP-OPN-VCHR-TYP
        textStream << QString().sprintf("%015d", q.value("apopen_ponumber").toInt());                                //  AP-OPN-PRCH-ORD-NO
        textStream << q.value("distdate").toString();                                                                //  AR-OPN-DIST-DAT
        textStream << q.value("apopen_invcnumber").toString().leftJustify(15, ' ', TRUE);                            //  AP-OPN-INVC-NO (string)
        textStream << q.value("docdate").toString();                                                                 //  AR-OPN-INVC-DAT
        textStream << q.value("duedate").toString();                                                                 //  AP-OPN-DUE-DAT
        textStream << q.value("duedate").toString();                                                                 //  AP-OPN-DISC-DAT
        textStream << QString().sprintf("%011d+", q.value("amount").toInt());                                        //  AP-OPN-ORIG-INVC-AMT
        textStream << "0000000000+";                                                                                 //  AP-OPN-ORIG-DISC-AMT
        textStream << "00000000000+";                                                                                //  AP-OPN-ORIG-RETNGE-AMT
        textStream << QString().sprintf("%011d+", q.value("amount").toInt());                                        //  AP-OPN-INVC-BAL
        textStream << "0000000000+";                                                                                 //  AP-OPN-DISC-BAL
        textStream << "00000000000+";                                                                                //  AP-OPN-CURR-PART-PMT
        textStream << "0000000000+";                                                                                 //  AP-OPN-CURR-DISC-AMT
        textStream << "        ";                                                                                    //  AP-OPN-DISC-PFT-CTR-1
        textStream << "        ";                                                                                    //  AP-OPN-DISC-PFT-CTR-2
        textStream << "        ";                                                                                    //  AP-OPN-DISC-MAIN-ACCT-NO
        textStream << "        ";                                                                                    //  AP-OPN-DISC-SUB-ACCNT-NO
        textStream << q.value("apopen_reference").toString().leftJustify(25, ' ', TRUE);                             //  AP-OPN-REF
        textStream << QString().fill('0', 6);                                                                        //  AP-OPN-CHK-NO
        textStream << QString().fill('0', 8);                                                                        //  AP-OPN-CHK-DAT
        textStream << "0";                                                                                           //  AP-OPN-CHK-PRT-STAT
        textStream << " ";                                                                                           //  AP-OPN-RETNGE-FLG
        textStream << "N";                                                                                           //  AP-OPN-1099-FLG
        textStream << "OMG";                                                                                         //  AP-OPN-USER-ID
        textStream << "00000000000000+";                                                                             //  AP-OPN-ORIG-CURRENCY-AMT
        textStream << "00000000000+";                                                                                //  AP-OPN-ORIG-CURRENCY-RATE
        textStream << "\n";                                                                                          //  EOL
      }
      while (q.next());
    }
    openFile.close();

    return TRUE;
  }
  else
  {
    QMessageBox::critical( pParent, QObject::tr("Cannot Open APOPEN File"),
                           QObject::tr( "The APOPEN Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                                        "A/P Open Items were NOT exported." ) );

    return FALSE;
  }
}



