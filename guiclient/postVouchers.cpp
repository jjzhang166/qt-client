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

#include "postVouchers.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include "rwInterface.h"

postVouchers::postVouchers(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _printJournal->setChecked(true);

  _post->setFocus();
}

postVouchers::~postVouchers()
{
    // no need to delete child widgets, Qt does it all for us
}

void postVouchers::languageChange()
{
    retranslateUi(this);
}

void postVouchers::sPost()
{
  q.prepare("SELECT count(*) AS unposted FROM vohead WHERE (NOT vohead_posted)");
  q.exec();
  if(q.first() && q.value("unposted").toInt()==0)
  {
    QMessageBox::critical( this, tr("No Vouchers to Post"),
                           tr("There are no Vouchers to post.") );
    return;
  }

  q.prepare("SELECT postVouchers(FALSE) AS result;");
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result == -5)
    {
      QMessageBox::critical( this, tr("Cannot Post Voucher"),
                             tr( "The Cost Category(s) for one or more Item Sites for the Purchase Order covered by one or more\n"
                                 "of the Vouchers that you are trying to post is not configured with Purchase Price Variance or\n"
                                 "P/O Liability Clearing Account Numbers or the Vendor of these Vouchers is not configured with an\n"
                                 "A/P Account Number.  Because of this, G/L Transactions cannot be posted for these Vouchers.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may\n"
                                 "post Vouchers." ) );
      return;
    }
    else if (result < 0)
    {
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(result) );
      return;
    }

    omfgThis->sVouchersUpdated();
 
    if (_printJournal->isChecked())
    {
      ParameterList params;
      params.append("journalNumber", result);

      orReport report("PayablesJournal", params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }
  }
  else
  {
    systemError( this, tr("A System Error occurred at %1::%2.\n%3")
                       .arg(__FILE__)
                       .arg(__LINE__)
                       .arg(q.lastError().databaseText()) );
    return;
  }

// START_RW
  if(_metrics->boolean("EnableExternalAccountingInterface"))
  {
    if ( (_metrics->value("AccountingSystem") == "RW2000") ||
         (_metrics->value("AccountingSystem") == "RealWorld91") )
    {
  //  Check to see if any apopen items are available to export
      q.exec( "SELECT apopen_id "
              "FROM apopen "
              "WHERE (NOT apopen_posted) "
              "LIMIT 1;" );
      if (!q.first())
        QMessageBox::information( this, tr("No A/P Open Items"),
                                  tr("There are no A/P Open Items to post.  This means that there are no Vouchers posted to A/R.") );
      else
      {
        if (QMessageBox::critical( this, tr("Create New APOPEN and APDIST Files?"),
                                   tr( "Creating new Export Files will delete the previous Export Files.\n"
                                       "You should make sure that the previous Export Files have been\n"
                                       "imported into RealWorld before Proceeding.\n\n"
                                       "Are you sure that you want to Create New Export Files?" ),
                                       "&Yes", "&No", QString::null, 0, 1  ) == 0)
        {
          if (rwInterface::exportApopen(this))
          {
            rwInterface::exportApdist(this);
  
            if ( QMessageBox::information( this, tr("Mark Distributions as Posted"),
                                           tr( "New APDIST and APOPEN files have been generated in the RealWorld directory.\n"
                                               "You should now use the RealWorld apfu/aputil tool to import these files.\n"
                                               "After you have successfully imported the APDIST and APOPEN files click the 'Post' button\n"
                                               "to mark these items as distributed.\n"
                                               "If, for any reason, you were unable to post the APDIST and APOPEN files click\n"
                                               "on the 'Do Not Post' button and Re-Post Vouchers to re-create the APDIST and APOPEN files.\n" ),
                                           tr("&Post"), tr("Do &Not Post"), QString::null, 0, 1) == 0)
              q.exec( "SELECT postPoGLTransactions();"
                      "SELECT postApopenItems();" );
          }
        }
      }
    }
  }
// END_RW

  accept();
}
