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

#include "viewCheckRun.h"

#include <QSqlError>

#include <openreports.h>

#include "miscCheck.h"
#include "postCheck.h"
#include "printCheck.h"
#include "printChecks.h"
#include "storedProcErrorLookup.h"

viewCheckRun::viewCheckRun(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_check, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(sHandleItemSelection()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_postCheck, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_printCheck, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_printCheckRun, SIGNAL(clicked()), this, SLOT(sPrintCheckRun()));
  connect(_printEditList, SIGNAL(clicked()), this, SLOT(sPrintEditList()));
  connect(_replace, SIGNAL(clicked()), this, SLOT(sReplace()));
  connect(_replaceAll, SIGNAL(clicked()), this, SLOT(sReplaceAll()));
  connect(_void, SIGNAL(clicked()), this, SLOT(sVoid()));

  _check->setRootIsDecorated(TRUE);
  _check->addColumn(tr("Void"),             _ynColumn, Qt::AlignCenter );
  _check->addColumn(tr("Misc."),            _ynColumn, Qt::AlignCenter );
  _check->addColumn(tr("Prt'd"),            _ynColumn, Qt::AlignCenter );
  _check->addColumn(tr("Chk./Voucher/RA #"), _itemColumn, Qt::AlignCenter );
  _check->addColumn(tr("Recipient/Invc./CM #"),       -1, Qt::AlignLeft   );
  _check->addColumn(tr("Check Date") ,    _dateColumn, Qt::AlignCenter );
  _check->addColumn(tr("Amount"),        _moneyColumn, Qt::AlignRight  );
  _check->addColumn(tr("Currency"),   _currencyColumn, Qt::AlignLeft );

  if (omfgThis->singleCurrency())
      _check->hideColumn(7);

  connect(omfgThis, SIGNAL(checksUpdated(int, int, bool)), this, SLOT(sFillList(int)));

  sFillList();
}

viewCheckRun::~viewCheckRun()
{
  // no need to delete child widgets, Qt does it all for us
}

void viewCheckRun::languageChange()
{
  retranslateUi(this);
}

void viewCheckRun::sVoid()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, voidCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sDelete()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, deleteCheck(checkhead_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:checkhead_id);" );
  q.bindValue(":checkhead_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(q.value("checkhead_bankaccnt_id").toInt(), _check->id(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sEdit()
{
  ParameterList params;
  params.append("edit");
  params.append("check_id", _check->id());

  miscCheck *newdlg = new miscCheck();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void viewCheckRun::sReplace()
{
  q.prepare( "SELECT checkhead_bankaccnt_id, replaceVoidedCheck(:check_id) AS result "
             "FROM checkhead "
             "WHERE (checkhead_id=:check_id);" );
  q.bindValue(":check_id", _check->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceVoidedCheck", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated( q.value("checkhead_bankaccnt_id").toInt(),
                                q.value("result").toInt(), TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sReplaceAll()
{
  q.prepare("SELECT replaceAllVoidedChecks(:bankaccnt_id) AS result;");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("replaceAllVoidedChecks", result), __FILE__, __LINE__);
      return;
    }
    omfgThis->sChecksUpdated(_bankaccnt->id(), -1, TRUE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrint()
{
  ParameterList params;
  params.append("check_id", _check->id());

  printCheck newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void viewCheckRun::sPost()
{
  ParameterList params;
  params.append("check_id", _check->id());

  postCheck newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void viewCheckRun::sHandleItemSelection()
{
  QTreeWidgetItem *selected = _check->currentItem();

  if (! selected)
    return;

  if (selected->text(0) == tr("Yes"))
  {
    _void->setEnabled(FALSE);
    _delete->setEnabled(TRUE);
    _replace->setEnabled(TRUE);
    _printCheck->setEnabled(FALSE);

    _edit->setEnabled(FALSE);
    _postCheck->setEnabled(FALSE);
  }
  else if (selected->text(0) == tr("No"))
  {
    _void->setEnabled(TRUE);
    _delete->setEnabled(FALSE);
    _replace->setEnabled(FALSE);
    _printCheck->setEnabled(TRUE);

    _edit->setEnabled((selected->text(1) == tr("Yes")) && (selected->text(2) == tr("No")));
    _postCheck->setEnabled((selected->text(2) == tr("Yes")) && (_privileges->check("PostPayments")));
  }
}

void viewCheckRun::sFillList(int pBankaccntid)
{
  if (pBankaccntid == _bankaccnt->id())
    sFillList();
}

void viewCheckRun::sFillList()
{
  _check->clear();

  QString sql( "SELECT checkhead_id AS checkid, -1 AS checkitem_id,"
               "       formatBoolYN(checkhead_void) AS f_void,"
               "       formatBoolYN(checkhead_misc) AS f_misc,"
               "       formatBoolYN(checkhead_printed) AS f_printed,"
               "       CASE WHEN(checkhead_number=-1) THEN TEXT('Unspecified')"
               "            ELSE TEXT(checkhead_number)"
               "       END AS number,"
               "       (checkrecip_number || '-' || checkrecip_name) AS description,"
               "       formatDate(checkhead_checkdate) AS f_checkdate,"
               "       formatMoney(checkhead_amount) AS f_amount,"
               "       CASE WHEN (checkhead_misc) THEN 1"
               "            ELSE 0"
               "       END AS misc,"
               "       checkhead_number, currConcat(checkhead_curr_id) AS curr_concat, "
	       "       1 AS orderby "
               "FROM checkhead LEFT OUTER JOIN"
	       "     checkrecip ON ((checkrecip_id=checkhead_recip_id)"
	       "               AND  (checkrecip_type=checkhead_recip_type))"
               "WHERE ((checkhead_bankaccnt_id=:bankaccnt_id) "
               "  AND  (NOT checkhead_posted)"
               "  AND  (NOT checkhead_replaced)"
               "  AND  (NOT checkhead_deleted) ) "

               "UNION SELECT checkitem_checkhead_id AS checkid, checkitem_id,"
               "             '' AS f_void, '' AS f_misc, '' AS f_printed,"
               "             CASE WHEN (checkitem_ranumber IS NOT NULL) THEN"
	       "                        checkitem_ranumber::TEXT"
	       "                  ELSE checkitem_vouchernumber"
	       "             END AS number,"
	       "             CASE WHEN (checkitem_cmnumber IS NOT NULL) THEN"
	       "                        checkitem_cmnumber::TEXT"
	       "                  ELSE checkitem_invcnumber"
               "             END AS description,"
               "             '' AS f_checkdate,"
               "             formatMoney(checkitem_amount) AS f_amount,"
               "             0 AS misc, checkhead_number, "
	       "             currConcat(checkitem_curr_id) AS curr_concat, "
	       "             2 AS orderby "
               "FROM checkitem, checkhead "
               "WHERE ( (checkitem_checkhead_id=checkhead_id)"
               " AND (checkhead_bankaccnt_id=:bankaccnt_id) "
               " AND (NOT checkhead_posted)"
               " AND (NOT checkhead_replaced)"
               " AND (NOT checkhead_deleted) ) "

               "ORDER BY checkhead_number, checkid, orderby;" );

  q.prepare(sql);
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *header = NULL;
    int           checkid = -1;

    do
    {
      if (q.value("checkid").toInt() != checkid)
      {
        checkid = q.value("checkid").toInt();
        header = new XTreeWidgetItem( _check, header, checkid, q.value("misc").toInt(),
                                    q.value("f_void"), q.value("f_misc"),
                                    q.value("f_printed"), q.value("number"),
                                    q.value("description"), q.value("f_checkdate"),
                                    q.value("f_amount"), q.value("curr_concat"));
      }
      else if (header)
      {
        XTreeWidgetItem *item = new XTreeWidgetItem( header, checkid, 0);
        item->setText(3, q.value("number"));
        item->setText(4, q.value("description"));
        item->setText(6, q.value("f_amount"));
	item->setText(7, q.value("curr_concat"));
      }
    }
    while (q.next());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void viewCheckRun::sPrintEditList()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 
    
  orReport report("ViewAPCheckRunEditList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void viewCheckRun::sPrintCheckRun()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id()); 

  printChecks newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
