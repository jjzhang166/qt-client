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
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>
#include <xdateinputdialog.h>

#include "guiclient.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

dspCheckRegister::dspCheckRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_check, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_vendRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_taxauthRB, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_custRB,    SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));

  _bankaccnt->setType(XComboBox::APBankAccounts);

  _check->addColumn(tr("Void"),        _ynColumn,       Qt::AlignCenter, true,  "checkhead_void" );
  _check->addColumn(tr("Misc."),       _ynColumn,       Qt::AlignCenter, true,  "checkhead_misc" );
  _check->addColumn(tr("Prt'd"),       _ynColumn,       Qt::AlignCenter, true,  "checkhead_printed" );
  _check->addColumn(tr("Posted"),      _ynColumn,       Qt::AlignCenter, true,  "checkhead_posted" );
  _check->addColumn(tr("Chk./Vchr."),  _itemColumn,     Qt::AlignCenter, true,  "number" );
  _check->addColumn(tr("Recipient"),   -1,              Qt::AlignLeft,   true,  "description"   );
  _check->addColumn(tr("Check Date") , _dateColumn,     Qt::AlignCenter, true,  "checkdate" );
  _check->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight,  true,  "amount"  );
  _check->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );
  _check->addColumn(tr("Base Amount"), _bigMoneyColumn, Qt::AlignRight,  true,  "base_amount"  );
  if (_metrics->boolean("ACHEnabled"))
    _check->addColumn(tr("ACH Batch"), _orderColumn,    Qt::AlignRight,  true,  "checkhead_ach_batch");

  _check->sortByColumn(4);

  sHandleButtons();
  _recipGroup->setChecked(false);

  _total->setPrecision(omfgThis->moneyVal());
  if (omfgThis->singleCurrency())
  {
    _check->hideColumn("currAbbr");
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
  
  if(_recipGroup->isChecked())
  {
    pParams.append("recip", 100);
    if(_vendRB->isChecked())
    {
      pParams.append("recip_type_v", 100);
      if(_vend->isValid())
      {
        pParams.append("recip_id", _vend->id());
      }
    }
    if(_custRB->isChecked())
    {
      pParams.append("recip_type_c", 100);
      if(_cust->isValid())
      {
        pParams.append("recip_id", _cust->id());
      }
    }
    if(_taxauthRB->isChecked())
    {
      pParams.append("recip_type_t", 100);
      pParams.append("recip_id", _taxauth_2->id());
    }
  }

  if(_checkNumber->text() != "")
  {
    pParams.append("check_number", _checkNumber->text());
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
  MetaSQLQuery mql = mqlLoad("checkRegister", "detail");

  ParameterList params;
  if (!setParams(params))
    return;
  
  q = mql.toQuery(params);
  _check->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if(_showDetail->isChecked())
    _check->expandAll();
	
  QString tots("SELECT SUM(currToCurr(checkhead_curr_id, bankaccnt_curr_id,"
               "           checkhead_amount, checkhead_checkdate)) AS amount, "
               "       currConcat(bankaccnt_curr_id) AS currAbbr "
               "  FROM bankaccnt LEFT OUTER JOIN"
               "       checkhead ON ((bankaccnt_id=checkhead_bankaccnt_id)"
               "                AND  (checkhead_checkdate BETWEEN <? value(\"startDate\") ?> AND <? value(\"endDate\") ?> ) "
               "                AND  (NOT checkhead_void)"
               " <? if exists(\"check_number\") ?>"
               " AND   (CAST(checkhead_number AS text) ~ <? value(\"check_number\") ?>)"
               " <? endif ?>"
               " <? if exists(\"recip\") ?>"
               " <? if exists(\"recip_type_v\") ?>"
               " AND   (checkhead_recip_type = 'V' )"
               " <? endif ?>"
               " <? if exists(\"recip_type_c\") ?>"
               " AND   (checkhead_recip_type = 'C' )"
               " <? endif ?>"
               " <? if exists(\"recip_type_t\") ?>"
               " AND   (checkhead_recip_type = 'T' )"
               " <? endif ?>"
               " <? if exists(\"recip_id\") ?>"
               " AND   (checkhead_recip_id = <? value(\"recip_id\") ?> )"
               " <? endif ?>"
               " <? endif ?>)"
               "       LEFT OUTER JOIN "
               "       checkrecip ON ((checkhead_recip_id=checkrecip_id) "
               "                 AND  (checkhead_recip_type=checkrecip_type)) "
               " WHERE(bankaccnt_id=<? value(\"bankaccnt_id\") ?>)  "
               "GROUP BY bankaccnt_curr_id;" );
  MetaSQLQuery totm(tots);
  q = totm.toQuery(params);	// reused from above
  if(q.first())
  {
    _total->setDouble(q.value("amount").toDouble());
    _totalCurr->setText(q.value("currAbbr").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
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
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  sFillList();
}

void dspCheckRegister::sHandleButtons()
{
  if (_vendRB->isChecked())
  {
    _widgetStack->setCurrentIndex(0);
  }
  else if (_custRB->isChecked())
  {
    _widgetStack->setCurrentIndex(1);
  }
  else
  {
    _widgetStack->setCurrentIndex(2);
  }
}
