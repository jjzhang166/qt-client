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

#include "postGLTransactionsToExternal.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <stdlib.h>

/*
 *  Constructs a postGLTransactionsToExternal as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postGLTransactionsToExternal::postGLTransactionsToExternal(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postGLTransactionsToExternal::~postGLTransactionsToExternal()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postGLTransactionsToExternal::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QTextStream>

void postGLTransactionsToExternal::init()
{
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));
}

void postGLTransactionsToExternal::sPost()
{
  if ( QMessageBox::critical( this, tr("Create New ICDIST File?"),
                              tr( "Creating a new Export File will delete the previous Export File.\n"
                                  "You should make sure that the previous Export Files have been imported into RW2000 before Proceeding.\n\n"
                                  "Are you sure that you want to Create a New Export File?" ),
                                  "&Yes", "&No", QString::null, 0, 1  ) != 0 )
    return;

  QFile distFile(_metrics->value("AccountingSystemExportPath") + "/ICDIST00.EXP");
  if (distFile.open(QIODevice::WriteOnly))
  {
    QString sql( "SELECT accnt_profit, accnt_number, accnt_sub,"
                 "       LPAD(SUBSTR(TEXT(gltrans_journalnumber), 1, 4), 4, '0') AS journalnumber,"
                 "       TO_CHAR(gltrans_date, 'YYYYMMDD') AS distdate,"
                 "       usr_initials,"
                 "       TRUNC((SUM(gltrans_amount) * 100), 0) AS amount "
                 "FROM accnt, usr, gltrans "
                 "WHERE ( (gltrans_accnt_id=accnt_id)"
                 " AND (NOT gltrans_exported)"
                 " AND (usr_username=CURRENT_USER)"
                 " AND (gltrans_date BETWEEN :startDate AND :endDate)" );

    if (_sourceModule->currentText() != tr("All"))
      sql += " AND (gltrans_source=:source)";

    sql += ") "
           "GROUP BY accnt_profit, accnt_number, accnt_sub, journalnumber,"
           "         distdate, usr_initials "
           "HAVING (SUM(gltrans_amount) <> 0);";

    q.prepare(sql);
    q.bindValue(":startDate", _dates->startDate());
    q.bindValue(":endDate", _dates->endDate());
    q.bindValue(":source", _sourceModule->currentText());
    q.exec();
    if (q.first())
    {
      QTextStream textStream(&distFile);
      QString     format;

      do
      {
        textStream << "8";                                                                                           //  IC-DIST-TYP (MISC-COSTS-APPLIED)
        if (_metrics->value("AccountingSystem") == "RealWorld91")
        {
          if (_metrics->value("GLProfitSize").toInt() == 0)
            textStream << "00000000";                                                                                //  IC-DIST-PFT-CTR-1
          else
          {
            format = "%0" + QString("%1").arg(_metrics->value("GLProfitSize").toInt()) + "d";
      
            textStream << QString().sprintf(format, q.value("accnt_profit").toInt());                               //  IC-DIST-PFT-CTR-1
            textStream << QString().fill('0', (8 - _metrics->value("GLProfitSize").toInt()));                        //  Filler '0's
          }

          textStream << "00000000";                                                                                  //  IC-DIST-PFT-CTR-2
        }

        format = "%0" + QString("%1").arg(_metrics->value("GLMainSize").toInt()) + "d";
        
        textStream << QString().sprintf(format, q.value("accnt_number").toInt());                                   //  IC-DIST-MAIN-ACCT-NO
        textStream << QString().fill('0', (8 - _metrics->value("GLMainSize").toInt()));                              //  Filler '0's

        if (_metrics->value("GLSubaccountSize").toInt() == 0)
          textStream << "00000000";                                                                                  //  IC-DIST-SUB-ACCT-NO
        else
        {
          format = "%0" + QString("%1").arg(_metrics->value("GLSubaccountSize").toInt()) + "d";
        
          textStream << QString().sprintf(format, q.value("accnt_sub").toInt());                                    //  IC-DIST-SUB-ACCT-NO
          textStream << QString().fill('0', (8 - _metrics->value("GLSubaccountSize").toInt()));                      //  Filler '0's
        }

        textStream << q.value("distdate").toString();                                                               //  IC-DIST-TRX-DAT
        textStream << "IC";                                                                                          //  Journal Code
        textStream << q.value("journalnumber").toString();                                                          //  IC-DIST-JRNL-NO

        if (q.value("usr_initials").toString().length())
          textStream << q.value("usr_initials").toString().leftJustify(3, ' ', TRUE);                               //  IC-DIST-USER-ID
        else
          textStream << "OMG";                                                                                       //  IC-DIST-USER-ID

        textStream << "N";                                                                                           //  IC-DIST-CORR-FLG

        if (q.value("amount").toInt() < 0)
          textStream << QString().sprintf("%014d+", abs(q.value("amount").toInt()));                                //  IC-DIST-AMT
        else
          textStream << QString().sprintf("%014d-", q.value("amount").toInt());                                     //  IC-DIST-AMT

        textStream << "N";                                                                                           //  IC-DIST-POST-TO-GL-FLG

        textStream << "\n";                                                                                          //  EOL
      }
      while (q.next());
    }

    distFile.close();
  }
  else
  {
    QMessageBox::critical( this, tr("Cannot Open Distribution File"),
                           tr( "The G/L Distribution Export file cannot be opened.  Please contact your Systems Adminstrator\n\n"
                               "G/L Transactions were NOT exported." ) );
    return;
  }

  if ( QMessageBox::information( this, tr("Mark Distributions as Posted"),
                                 tr( "A new ICDIST file has been generated in the RealWorld directory.\n"
                                     "You should now use the RealWorld icfu/icutil tool to import this file.\n"
                                     "After you have successfully imported the ICDIST file click the 'Post' button\n"
                                     "to mark these transactions as distributed.\n"
                                     "If, for any reason, you were unable to post the ICDIST file click on the\n"
                                     "'Do Not Post' button and re-export G/L Transactions to re-create the IDDIST file.\n" ),
                                     tr("&Post"), tr("Do &Not Post"), QString::null, 0, 1) == 0)
  {
    q.prepare( "UPDATE gltrans "
               "SET gltrans_exported=TRUE "
               "WHERE ( (NOT gltrans_exported)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate) );" );
    q.bindValue(":startDate", _dates->startDate());
    q.bindValue(":endDate", _dates->endDate());
    q.exec();
  }

  accept();
}

