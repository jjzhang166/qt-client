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

#include "openVouchers.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QStatusBar>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "failedPostList.h"
#include "getGLDistDate.h"
#include "miscVoucher.h"
#include "storedProcErrorLookup.h"
#include "voucher.h"

openVouchers::openVouchers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vohead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_newMisc, SIGNAL(clicked()), this, SLOT(sNewMisc()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _vohead->addColumn(tr("Vchr. #"),        _orderColumn, Qt::AlignRight  );
  _vohead->addColumn(tr("P/O #"),          _orderColumn, Qt::AlignRight  );
  _vohead->addColumn(tr("Vendor"),         -1,           Qt::AlignLeft   );
  _vohead->addColumn(tr("Vend. Type"),     _itemColumn,  Qt::AlignLeft   );
  _vohead->addColumn(tr("Vendor Invc. #"), _itemColumn,  Qt::AlignRight  );
  _vohead->addColumn(tr("Dist. Date"),     _dateColumn,  Qt::AlignCenter );
  _vohead->addColumn(tr("G/L Post Date"),  _dateColumn,  Qt::AlignCenter );

  if (! _privileges->check("ChangeVOPostDate"))
    _vohead->hideColumn(6);

  if (_privileges->check("MaintainVouchers"))
  {
    connect(_vohead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_vohead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_vohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    _newMisc->setEnabled(FALSE);
    connect(_vohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  if (_privileges->check("PostVouchers"))
    connect(_vohead, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(vouchersUpdated()), this, SLOT(sFillList()));

  sFillList();
}

openVouchers::~openVouchers()
{
  // no need to delete child widgets, Qt does it all for us
}

void openVouchers::languageChange()
{
  retranslateUi(this);
}

void openVouchers::sPrint()
{
  orReport report("VoucheringEditList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void openVouchers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  voucher *newdlg = new voucher();
  omfgThis->handleNewWindow(newdlg);
  newdlg->set(params);
}

void openVouchers::sNewMisc()
{
  ParameterList params;
  params.append("mode", "new");

  miscVoucher *newdlg = new miscVoucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openVouchers::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vohead_id", _vohead->id());

  if (_vohead->altId() == -1)
  {
    miscVoucher *newdlg = new miscVoucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    voucher *newdlg = new voucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void openVouchers::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vohead_id", _vohead->id());

  if (_vohead->altId() == -1)
  {
    miscVoucher *newdlg = new miscVoucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else
  {
    voucher *newdlg = new voucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void openVouchers::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Vouchers"),
			    tr("<p>Are you sure that you want to delete the "
			       "selected Vouchers?"),
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteVoucher(:vohead_id) AS result;");

    QList<QTreeWidgetItem*>selected = _vohead->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      q.bindValue(":vohead_id", _vohead->id());
      q.exec();
      if (q.first())
      {
	int result = q.value("result").toInt();
	if (result < 0)
	{
	  systemError(this, storedProcErrorLookup("deleteVoucher", result),
		      __FILE__, __LINE__);
	}
      }
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    omfgThis->sVouchersUpdated();
  }
}

void openVouchers::sPost()
{
  bool changeDate = false;
  QDate newDate = QDate::currentDate();

  if (_privileges->check("ChangeVOPostDate"))
  {
    getGLDistDate newdlg(this, "", TRUE);
    newdlg.sSetDefaultLit(tr("Distribution Date"));
    if (newdlg.exec() == XDialog::Accepted)
    {
      newDate = newdlg.date();
      changeDate = (newDate.isValid());
    }
    else
      return;
  }

  XSqlQuery setDate;
  setDate.prepare("UPDATE vohead SET vohead_gldistdate=:distdate "
		  "WHERE vohead_id=:vohead_id;");

  QList<QTreeWidgetItem*>selected = _vohead->selectedItems();
  QList<QTreeWidgetItem*>triedToClosed;

  for (int i = 0; i < selected.size(); i++)
  {
    int id = ((XTreeWidgetItem*)(selected[i]))->id();

    if (changeDate)
    {
      setDate.bindValue(":distdate",  newDate);
      setDate.bindValue(":vohead_id", id);
      setDate.exec();
      if (setDate.lastError().type() != QSqlError::None)
      {
	systemError(this, setDate.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
  }
  
  XSqlQuery post;
  post.prepare("SELECT fetchJournalNumber('AP-VO') AS result;");
  post.exec();
  int journalNumber = 0;
  if(post.first())
    journalNumber = post.value("result").toInt();
  else
  {
    systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  post.prepare("SELECT postVoucher(:vohead_id, :journalNumber, FALSE) AS result;");

  bool tryagain = false;
  do {
    for (int i = 0; i < selected.size(); i++)
    {
      int id = ((XTreeWidgetItem*)(selected[i]))->id();

      post.bindValue(":vohead_id", id);
      post.bindValue(":journalNumber", journalNumber);
      post.exec();
      if (post.first())
      {
	int result = post.value("result").toInt();
	if (result < 0)
	  systemError(this, storedProcErrorLookup("postVoucher", result),
		      __FILE__, __LINE__);
      }
      // contains() string is hard-coded in stored procedure
      else if (post.lastError().databaseText().contains("posted to closed period"))
      {
	if (changeDate)
	{
	  triedToClosed = selected;
	  break;
	}
	else
	  triedToClosed.append(selected[i]);
      }
      else if (post.lastError().type() != QSqlError::None)
      {
	systemError(this, post.lastError().databaseText(), __FILE__, __LINE__);
      }
    }

    if (triedToClosed.size() > 0)
    {
      failedPostList newdlg(this, "", true);
      newdlg.sSetList(triedToClosed, _vohead->headerItem(), _vohead->header());
      tryagain = (newdlg.exec() == XDialog::Accepted);
      selected = triedToClosed;
      triedToClosed.clear();
    }
  } while (tryagain);

  if (_printJournal->isChecked())
  {
    ParameterList params;
    params.append("journalNumber", journalNumber);

    orReport report("PayablesJournal", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }

  omfgThis->sVouchersUpdated();
}

void openVouchers::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit Voucher..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainVouchers"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Voucher..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete Voucher..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainVouchers"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Post Voucher..."), this, SLOT(sPost()), 0);
  if (!_privileges->check("PostVouchers"))
    pMenu->setItemEnabled(menuItem, FALSE);
}


void openVouchers::sFillList()
{
  q.prepare( "SELECT vohead_id, COALESCE(pohead_id, -1), vohead_number,"
             "       COALESCE(TEXT(pohead_number), TEXT(:misc)),"
             "       (vend_number || '-' || vend_name), vendtype_code, vohead_invcnumber,"
             "       formatDate(vohead_distdate), "
	     "       formatDate(COALESCE(vohead_gldistdate, vohead_distdate)) "
             "  FROM vendinfo, vendtype, vohead LEFT OUTER JOIN pohead ON (vohead_pohead_id=pohead_id) "
             " WHERE ((vohead_vend_id=vend_id)"
             "   AND  (vend_vendtype_id=vendtype_id)"
             "   AND  (NOT vohead_posted)) "
             " ORDER BY vohead_number;" );
  q.bindValue(":misc", tr("Misc."));
  q.exec();
  _vohead->clear();
  _vohead->populate(q, TRUE);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
