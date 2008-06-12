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

#include "dspCheckRegister.h"

#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>
#include <xdateinputdialog.h>
#include "mqlutil.h"

#include "guiclient.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a dspCheckRegister as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCheckRegister::dspCheckRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_check, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _bankaccnt->setType(XComboBox::APBankAccounts);

  _check->addColumn(tr("Void"),        _ynColumn,    Qt::AlignCenter );
  _check->addColumn(tr("Misc."),       _ynColumn,    Qt::AlignCenter );
  _check->addColumn(tr("Prt'd"),       _ynColumn,    Qt::AlignCenter );
  _check->addColumn(tr("Posted"),      _ynColumn,    Qt::AlignCenter );
  _check->addColumn(tr("Chk./Vchr."),  _itemColumn,  Qt::AlignCenter );
  _check->addColumn(tr("Recipient"),   -1,           Qt::AlignLeft   );
  _check->addColumn(tr("Check Date") , _dateColumn,     Qt::AlignCenter );
  _check->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight  );
  _check->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignRight  );
  _check->sortByColumn(4);

  if (omfgThis->singleCurrency())
  {
    _check->hideColumn(8);
    _totalCurr->hide();
  }
}

dspCheckRegister::~dspCheckRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCheckRegister::languageChange()
{
  retranslateUi(this);
}

bool dspCheckRegister::setParams(ParameterList &pParams)
{
  if(!_dates->allValid())
  {
    QMessageBox::information( this, tr("Invalid Dates"),
			      tr("<p>Invalid dates specified. Please specify a "
				 "valid date range.") );
    _dates->setFocus();
    return false;
  }

  pParams.append("bankaccnt_id", _bankaccnt->id());
  _dates->appendValue(pParams);
  if(_showDetail->isChecked())
    pParams.append("showDetail");
  
  return true;
}

void dspCheckRegister::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CheckRegister", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCheckRegister::sFillList()
{
  MetaSQLQuery mql = mqlLoad(":/ap/displays/CheckRegister/FillListDetail.mql");

  ParameterList params;
  if (!setParams(params))
    return;

  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  
  _check->clear();
  XTreeWidgetItem *header = NULL;
  int           checkid = -1;
  while (q.next())
  {
    if (q.value("checkid").toInt() != checkid)
    {
      checkid = q.value("checkid").toInt();
      header = new XTreeWidgetItem( _check, header, checkid, q.value("extra").toInt(),
				  q.value("f_void"), q.value("f_misc"),
				  q.value("f_printed"), q.value("f_posted"), q.value("number"),
				  q.value("description"), q.value("f_checkdate"),
				  q.value("f_amount"), q.value("currAbbr"));
    }
    else if (header)
    {
      XTreeWidgetItem *item = new XTreeWidgetItem( header, checkid, 0);
      item->setText(4, q.value("number"));
      item->setText(5, q.value("description"));
      item->setText(7, q.value("f_amount"));
    }
  }
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(_showDetail->isChecked())
    _check->expandAll();

  QString tots("SELECT formatMoney(SUM(currToCurr(checkhead_curr_id,"
	       "                                  bankaccnt_curr_id,"
	       "	                          checkhead_amount,"
	       "                                  checkhead_checkdate))) AS f_amount,"
	       "       currConcat(bankaccnt_curr_id) AS currAbbr "
	       "FROM checkhead, bankaccnt "
	       "WHERE ((NOT checkhead_void)"
	       " AND (checkhead_checkdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?>) "
	       " AND (bankaccnt_id=checkhead_bankaccnt_id) "
	       " AND (checkhead_bankaccnt_id=<? value(\"bankaccnt_id\") ?>) )"
	       " GROUP BY bankaccnt_curr_id;" );
  MetaSQLQuery totm(tots);
  q = totm.toQuery(params);	// reused from above
  if(q.first())
  {
    _total->setText(q.value("f_amount").toString());
    _totalCurr->setText(q.value("currAbbr").toString());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspCheckRegister::sPopulateMenu( QMenu * pMenu )
{
  int menuItem;

  if(_check->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Void Posted Check"), this, SLOT(sVoidPosted()), 0);
    if(!_privileges->check("VoidPostedAPCheck"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspCheckRegister::sVoidPosted()
{
  ParameterList params;

  XDateInputDialog newdlg(this, "", TRUE);
  params.append("label", tr("On what date did you void this check?"));
  newdlg.set(params);
  int returnVal = newdlg.exec();
  if (returnVal == XDialog::Accepted)
  {
    QDate voidDate = newdlg.getDate();
    q.prepare("SELECT voidPostedCheck(:check_id, fetchJournalNumber('AP-CK'),"
	      "                         DATE :voidDate) AS result;");
    q.bindValue(":check_id", _check->id());
    q.bindValue(":voidDate", voidDate);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("voidPostedCheck", result),
			    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
}
