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

#include "dspCheckRegister.h"

#include <Q3PopupMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <parameter.h>
#include <xdateinputdialog.h>
#include "mqlutil.h"

#include "OpenMFGGUIClient.h"
#include "rptCheckRegister.h"
#include "storedProcErrorLookup.h"

/*
 *  Constructs a dspCheckRegister as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCheckRegister::dspCheckRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_apchk, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));

  statusBar()->hide();
  
  _bankaccnt->setType(XComboBox::APBankAccounts);

  _apchk->addColumn(tr("Void"),        _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Misc."),       _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Prt'd"),       _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Posted"),      _ynColumn,    Qt::AlignCenter );
  _apchk->addColumn(tr("Chk./Vchr."),  _itemColumn,  Qt::AlignCenter );
  _apchk->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft   );
  _apchk->addColumn(tr("Check Date") , _dateColumn,     Qt::AlignCenter );
  _apchk->addColumn(tr("Amount"),      _moneyColumn,    Qt::AlignRight  );
  _apchk->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignRight  );
  _apchk->setSortColumn(4);

  if (omfgThis->singleCurrency())
  {
    _apchk->hideColumn(8);
    _totalCurr->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCheckRegister::~dspCheckRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCheckRegister::languageChange()
{
  retranslateUi(this);
}

void dspCheckRegister::sPrint()
{
  if(!checkParams())
    return;
  
  ParameterList params;
  
  params.append("bankaccnt_id", _bankaccnt->id());
  _dates->appendValue(params);
  params.append("print");
  
  rptCheckRegister newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspCheckRegister::sFillList()
{
  if(!checkParams())
    return;
  
  MetaSQLQuery mql = mqlLoad(":/ap/displays/CheckRegister/FillListDetail.mql");

  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  _dates->appendValue(params);
  if(_showDetail->isChecked())
    params.append("showDetail");

  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  
  _apchk->clear();
  if (q.first())
  {
    XListViewItem *header = NULL;
    int           apchkid = -1;

    do
    {
      if (q.value("apchkid").toInt() != apchkid)
      {
        apchkid = q.value("apchkid").toInt();
        header = new XListViewItem( _apchk, _apchk->lastItem(), apchkid, q.value("extra").toInt(),
                                    q.value("f_void"), q.value("f_misc"),
                                    q.value("f_printed"), q.value("f_posted"), q.value("number"),
                                    q.value("description"), q.value("f_checkdate"),
                                    q.value("f_amount"), q.value("currAbbr"));
      }
      else if (header)
      {
        XListViewItem *item = new XListViewItem( header, apchkid, 0);
        item->setText(4, q.value("number"));
        item->setText(5, q.value("description"));
        item->setText(7, q.value("f_amount"));
      }
    }
    while (q.next());
  }

  if(_showDetail->isChecked())
    _apchk->openAll();

  q.prepare( "SELECT formatMoney(SUM(currToCurr(apchk_curr_id, bankaccnt_curr_id,"
	     "	                                apchk_amount, apchk_checkdate))) AS f_amount,"
	     "       currConcat(bankaccnt_curr_id) AS currAbbr "
             "FROM apchk, vend, bankaccnt "
             "WHERE ( (apchk_vend_id=vend_id) "
             "  AND   (NOT apchk_void)"
             " AND (apchk_checkdate BETWEEN :startDate AND :endDate) "
	     " AND (bankaccnt_id=apchk_bankaccnt_id) "
             " AND (apchk_bankaccnt_id=:bankaccnt_id) )"
             " GROUP BY bankaccnt_curr_id;" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  _dates->bindValue(q);
  q.exec();
  if(q.first())
  {
    _total->setText(q.value("f_amount").toString());
    _totalCurr->setText(q.value("currAbbr").toString());
  }
}

bool dspCheckRegister::checkParams()
{
  if(!_dates->allValid())
  {
    QMessageBox::information( this, tr("Invalid Dates"), tr("Invalid dates specified. Please specify a valid date range.") );
    _dates->setFocus();
    return false;
  }
  
  return true;
}


void dspCheckRegister::sPopulateMenu( Q3PopupMenu * pMenu )
{
  int menuItem;

  if(_apchk->altId() == 1)
  {
    menuItem = pMenu->insertItem(tr("Void Posted Check"), this, SLOT(sVoidPosted()), 0);
    if(!_privleges->check("VoidPostedAPCheck"))
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
  if (returnVal == QDialog::Accepted)
  {
    QDate voidDate = newdlg.getDate();
    q.prepare("SELECT voidPostedAPCheck(:apchk_id, fetchJournalNumber('AP-CK'),"
	      "                         DATE :voidDate) AS result;");
    q.bindValue(":apchk_id", _apchk->id());
    q.bindValue(":voidDate", voidDate);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("voidPostedAPCheck", result),
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
