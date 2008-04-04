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

#include "printChecks.h"

#include <QList>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QPrintDialog>

#include <orprerender.h>
#include <orprintrender.h>
#include <renderobjects.h>

#include "printChecksReview.h"
#include "storedProcErrorLookup.h"

printChecks::printChecks(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sPopulate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _setCheckNumber = -1;

  _bankaccnt->setAllowNull(TRUE);
  _bankaccnt->setType(XComboBox::APBankAccounts);
}

printChecks::~printChecks()
{
  // no need to delete child widgets, Qt does it all for us
}

void printChecks::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printChecks::set(const ParameterList pParams )
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
    _bankaccnt->setId(param.toInt());

  return NoError;
}

void printChecks::sPrint()
{
  QList<int> printedChecks;

  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());

  bool setup = true;

  QList<ORODocument*> singleCheckPrerendered;
  XSqlQuery checks;
  checks.prepare( "SELECT checkhead_id, checkhead_number, report_name "
                  "FROM checkhead, bankaccnt, form, report "
                  "WHERE ( (checkhead_bankaccnt_id=bankaccnt_id)"
                  " AND (bankaccnt_check_form_id=form_id)"
                  " AND (form_report_id=report_id)"
                  " AND (NOT checkhead_printed) "
                  " AND (NOT checkhead_void) "
                  " AND (bankaccnt_id=:bankaccnt_id) ) "
                  "ORDER BY checkhead_number "
                  "LIMIT :numtoprint; " );
  checks.bindValue(":bankaccnt_id", _bankaccnt->id());
  checks.bindValue(":numtoprint", _numberOfChecks->value());
  checks.exec();
  QDomDocument docReport;

  while (checks.next())
  {
    if (setup)
    {
      // get the report definition out of the database
      // this should somehow be condensed into a common code call or something
      // in the near future to facilitate easier conversion in other places
      // of the application to use the new rendering engine directly
      XSqlQuery report;
      report.prepare( "SELECT report_source "
                      "  FROM report "
                      " WHERE (report_name=:report_name) "
                      "ORDER BY report_grade DESC LIMIT 1;" );
      report.bindValue(":report_name", checks.value("report_name").toString());
      report.exec();
      if (report.first())
      {
        QString errorMessage;
        int     errorLine;

        if (!docReport.setContent(report.value("report_source").toString(), &errorMessage, &errorLine))
        {
          systemError(this, errorMessage, __FILE__, __LINE__);
          return;
        }
      }
      else
      {
        systemError(this, report.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
      // end getting the report definition out the database

      if(_setCheckNumber != -1 && _setCheckNumber != _nextCheckNum->text().toInt())
      {
        q.prepare("SELECT setNextCheckNumber(:bankaccnt_id, :nextCheckNumber) AS result;");
        q.bindValue(":bankaccnt_id", _bankaccnt->id());
        q.bindValue(":nextCheckNumber", _nextCheckNum->text().toInt());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("setNextCheckNumber", result),
                        __FILE__, __LINE__);
            return;
          }
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
    }

    q.prepare("UPDATE checkhead SET checkhead_number=fetchNextCheckNumber(checkhead_bankaccnt_id)"
              " WHERE(checkhead_id=:checkhead_id);");
    q.bindValue(":checkhead_id", checks.value("checkhead_id").toInt());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    ParameterList params;
    params.append("checkhead_id", checks.value("checkhead_id").toInt());
    params.append("apchk_id", checks.value("checkhead_id").toInt());

    ORPreRender pre;
    pre.setDom(docReport);
    pre.setParamList(params);
    ORODocument * doc = pre.generate();

    printedChecks.append(checks.value("checkhead_id").toInt());

    int page_num = 1;
    while(page_num < doc->pages())
    {
      page_num++;

      XSqlQuery qq;
      qq.prepare("INSERT INTO checkhead"
                 "      (checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, checkhead_printed,"
                 "       checkhead_checkdate, checkhead_number,"
                 "       checkhead_amount, checkhead_void,"
                 "       checkhead_misc,"
                 "       checkhead_for, checkhead_notes,"
                 "       checkhead_curr_id, checkhead_deleted) "
                 "SELECT checkhead_recip_id, checkhead_recip_type,"
                 "       checkhead_bankaccnt_id, TRUE,"
                 "       checkhead_checkdate, fetchNextCheckNumber(checkhead_bankaccnt_id),"
                 "       checkhead_amount, TRUE, TRUE,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       'Continuation of Check #'||checkhead_number,"
                 "       checkhead_curr_id, TRUE"
                 "  FROM checkhead"
                 " WHERE(checkhead_id=:checkhead_id);");
      qq.bindValue(":checkhead_id", checks.value("checkhead_id").toInt());
      if(!qq.exec())
      {
        systemError(this, "Received error but will continue anyway:\n"+qq.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
    singleCheckPrerendered.append(doc);
  }
  if (checks.lastError().type() != QSqlError::None)
  {
    systemError(this, checks.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(!printedChecks.empty())
  {
    QPrinter printer(QPrinter::HighResolution);
    ORODocument entireCheckRunPrerendered;
    for (int j = 0; j < singleCheckPrerendered.size(); j++)
    {
      for (int i = 0; i < singleCheckPrerendered.at(j)->pages(); i++)
      {
        entireCheckRunPrerendered.addPage(singleCheckPrerendered.at(j)->page(i));
      }
    }

    ORPrintRender render;
    render.setupPrinter(&entireCheckRunPrerendered, &printer);

    QPrintDialog pd(&printer);
    pd.setMinMax(1, entireCheckRunPrerendered.pages());
    if(pd.exec() != QDialog::Accepted)
      return;
    render.setPrinter(&printer);
    render.render(&entireCheckRunPrerendered);

    QList<int>::iterator it;

    if ( QMessageBox::question(this, tr("All Checks Printed"),
			       tr("<p>Did all the Checks print successfully?"),
				QMessageBox::Yes,
				QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
    {
      XSqlQuery postCheck;
      postCheck.prepare( "SELECT markCheckAsPrinted(:checkhead_id) AS result;");
      for( it = printedChecks.begin(); it != printedChecks.end(); ++it)
      {
        postCheck.bindValue(":checkhead_id", (*it));
        postCheck.exec();
	if (postCheck.first())
	{
	  int result = postCheck.value("result").toInt();
	  if (result < 0)
	  {
	    systemError(this, storedProcErrorLookup("markCheckAsPrinted",
						   result), __FILE__, __LINE__);
	    return;
	  }
	}
	else if (postCheck.lastError().type() != QSqlError::None)
	{
	  systemError(this, postCheck.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
      }
    }
    else
    {
      printChecksReview newdlg(this, "", TRUE);
      QString query = QString( "SELECT checkhead_id, checkhead_number"
                               "  FROM checkhead"
                               " WHERE (checkhead_id IN (" );
      bool first = true;
      for( it = printedChecks.begin(); it != printedChecks.end(); ++it)
      {
        if(!first)
          query += ",";
        query += QString::number((*it)); 
        first = false;
      }
      query += ") ); ";
      newdlg._checks->populate(query);
      newdlg.sSelectAll();
      newdlg.sMarkPrinted();
      newdlg._checks->clearSelection();
      newdlg.exec();
    }
  }
  else
    QMessageBox::information( this, tr("No Checks Printed"),
			     tr("<p>No Checks were printed for the selected "
				"Bank Account.") );

  omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);
  sHandleBankAccount(_bankaccnt->id());
}

void printChecks::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT bankaccnt_nextchknum,"
             "       COUNT(*) AS numofchecks "
             "  FROM checkhead, bankaccnt "
             " WHERE((NOT checkhead_void)"
             "   AND (NOT checkhead_printed)"
             "   AND (checkhead_bankaccnt_id=bankaccnt_id)"
             "   AND (checkhead_bankaccnt_id=:bankaccnt_id))"
             " GROUP BY bankaccnt_nextchknum;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  if (q.first())
  {
    _nextCheckNum->setText(q.value("bankaccnt_nextchknum").toString());
    _numberOfChecks->setMaxValue(q.value("numofchecks").toInt());
    _numberOfChecks->setValue(q.value("numofchecks").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void printChecks::sPopulate()
{
}
