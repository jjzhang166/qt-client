/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contacts.h"

#include <QMenu>
#include <QSqlError>
#include <QVariant>
#include <QMessageBox>

#include <openreports.h>
#include <metasql.h>

#include "contact.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

contacts::contacts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);
    
    _crmAccount->hide();
    _attach->hide();
    _detach->hide();

    _activeOnly->setChecked(true);

    connect(_contacts, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
    connect(_edit,		SIGNAL(clicked()),	this, SLOT(sEdit()));
    connect(_view,		SIGNAL(clicked()),	this, SLOT(sView()));
    connect(_delete,		SIGNAL(clicked()),	this, SLOT(sDelete()));
    connect(_print,		SIGNAL(clicked()),	this, SLOT(sPrint()));
    connect(_close,		SIGNAL(clicked()),	this, SLOT(close()));
    connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
    connect(_activeOnly,	SIGNAL(toggled(bool)),	this, SLOT(sFillList()));
    connect(_attach,            SIGNAL(clicked()),      this, SLOT(sAttach()));
    connect(_detach,            SIGNAL(clicked()),      this, SLOT(sDetach()));
    
    _contacts->addColumn(tr("First Name"),    50, Qt::AlignLeft, true, "cntct_first_name");
    _contacts->addColumn(tr("Last Name"),    100, Qt::AlignLeft, true, "cntct_last_name");
    _contacts->addColumn(tr("Account #"),     80, Qt::AlignLeft, true, "crmacct_number");
    _contacts->addColumn(tr("Account Name"), 160, Qt::AlignLeft, true, "crmacct_name");
    _contacts->addColumn(tr("Title"),         50, Qt::AlignLeft, true, "cntct_title");
    _contacts->addColumn(tr("Phone"),	     100, Qt::AlignLeft, true, "cntct_phone");
    _contacts->addColumn(tr("Alternate"),    100, Qt::AlignLeft, true, "cntct_phone2");
    _contacts->addColumn(tr("Fax"),	     100, Qt::AlignLeft, true, "cntct_fax");
    _contacts->addColumn(tr("E-Mail"),	     100, Qt::AlignLeft, true, "cntct_email");
    _contacts->addColumn(tr("Web Address"),  100, Qt::AlignLeft, true, "cntct_webaddr");

	_contacts->setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (_privileges->check("MaintainContacts"))
    {
      _attach->setEnabled(true);
      connect(_contacts, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
      connect(_contacts, SIGNAL(valid(bool)), _detach, SLOT(setEnabled(bool)));
      connect(_contacts, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
      _new->setEnabled(FALSE);
      connect(_contacts, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
}

contacts::~contacts()
{
    // no need to delete child widgets, Qt does it all for us
}

void contacts::languageChange()
{
    retranslateUi(this);
}

enum SetResponse contacts::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
    sFillList();

  return NoError;
}

void contacts::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem*)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainContacts"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  XSqlQuery chk;
  chk.prepare("SELECT cntctused(:cntct_id) AS inUse");
  chk.bindValue(":cntct_id", _contacts->id());
  chk.exec();
  if (chk.lastError().type() != QSqlError::NoError) {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (chk.first() && !chk.value("inUse").toBool()) {
    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    if (!_privileges->check("MaintainContacts"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void contacts::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  if (_crmAccount->isValid())
    params.append("crmacct_id", _crmAccount->id());

  contact newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void contacts::sEdit()
{
  QList<XTreeWidgetItem*> list = _contacts->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
	ParameterList params;
    params.append("mode", "edit");
	params.append("cntct_id", ((XTreeWidgetItem*)(list[i]))->id());

    contact newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  sFillList();
}

void contacts::sView()
{
  QList<XTreeWidgetItem*> list = _contacts->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
	  ParameterList params;
	  params.append("mode", "view");
	  params.append("cntct_id", ((XTreeWidgetItem*)(list[i]))->id());

	  contact newdlg(this, "", TRUE);
	  newdlg.set(params);
	  newdlg.exec();
  }
}

void contacts::sDelete()
{

  if ( QMessageBox::warning(this, tr("Delete Contact?"),
                            tr("<p>Are you sure that you want to completely "
			       "delete the selected contact?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteContact(:cntct_id) AS result;");

    QList<XTreeWidgetItem*> list = _contacts->selectedItems();
    for (int i = 0; i < list.size(); i++)
    {
	  q.bindValue(":cntct_id", ((XTreeWidgetItem*)(list[i]))->id());
      q.exec();

      if (q.first())
      {
        int result = q.value("result").toInt();
        if (result < 0)
        {
          systemError(this, storedProcErrorLookup("deleteContact", result), __FILE__, __LINE__);
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
  sFillList();
}

void contacts::setParams(ParameterList &params)
{
  if (_crmAccount->isValid())
    params.append("crmAccountId", _crmAccount->id());
  if (_activeOnly->isChecked())
    params.append("activeOnly");
}

void contacts::sPrint()
{
  ParameterList params;
  setParams(params);

  orReport report("ContactsMasterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void contacts::sFillList()
{
  MetaSQLQuery mql = mqlLoad("contacts", "detail");
  ParameterList params;
  setParams(params);
  q = mql.toQuery(params);
  _contacts->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contacts::sAttach()
{
  ContactCluster attached(this, "attached");
  attached.sEllipses();
  if (attached.id() > 0)
  {
    int answer = QMessageBox::Yes;

    if (attached.crmAcctId() > 0 && attached.crmAcctId() != _crmAccount->id())
      answer = QMessageBox::question(this, tr("Detach Contact?"),
			    tr("<p>This Contact is currently attached to a "
			       "different CRM Account. Are you sure you want "
			       "to change the CRM Account for this person?"),
			    QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
    if (answer == QMessageBox::Yes)
    {
      q.prepare("SELECT attachContact(:cntct_id, :crmacct_id) AS returnVal;");
      q.bindValue(":cntct_id", attached.id());
      q.bindValue(":crmacct_id", _crmAccount->id());
      q.exec();
      if (q.first())
      {
	int returnVal = q.value("returnVal").toInt();
	if (returnVal < 0)
	{
	  systemError(this, storedProcErrorLookup("attachContact", returnVal),
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
}

void contacts::sDetach()
{
  int answer = QMessageBox::question(this, tr("Detach Contact?"),
			tr("<p>Are you sure you want to detach this Contact "
			   "from this CRM Account?"),
			QMessageBox::Yes, QMessageBox::No | QMessageBox::Default);
  if (answer == QMessageBox::Yes)
  {
    int cntctId = _contacts->id();
    q.prepare("SELECT detachContact(:cntct_id, :crmacct_id) AS returnVal;");
    q.bindValue(":cntct_id", cntctId);
    q.bindValue(":crmacct_id", _crmAccount->id());
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("returnVal").toInt();
      if (returnVal < 0)
      {
	systemError(this, tr("Error detaching Contact from CRM Account (%1).")
			  .arg(returnVal), __FILE__, __LINE__);
	return;
      }
      emit cntctDetached(cntctId);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void contacts::sHandleButtons()
{
  if (_contacts->id() == -1) {
    _delete->setEnabled(false);
    return;
  }
  
  XSqlQuery chk;
  chk.prepare("SELECT cntctused(:cntct_id) AS inUse");
  chk.bindValue(":cntct_id", _contacts->id());
  chk.exec();
  chk.first();
  if (chk.lastError().type() != QSqlError::NoError) {
    systemError(this, chk.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _delete->setEnabled(_privileges->check("MaintainContacts") && !chk.value("inUse").toBool());
}

