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

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "characteristicAssignment.h"
#include "employee.h"
#include "empGroup.h"
#include "empgroupcluster.h"
#include "salesRep.h"
#include "storedProcErrorLookup.h"
#include "user.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns FALSE
bool employee::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("employee::userHasPriv(%d)", pMode);
  bool retval = false;
  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewEmployees") ||
               _privileges->check("MaintainEmployees");
      break;
    case cNew:
    case cEdit:
      retval = _privileges->check("MaintainEmployees");
      break;
    default:
      retval = false;
      break;
  }
  if (DEBUG)
    qDebug("employee::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XDialog
void employee::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("employee::setVisible(%d) called with mode() == %d",
           visible, _mode);
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

employee::employee(QWidget* parent, Qt::WindowFlags fl)
    : XDialog(parent, fl)
{
  setupUi(this);

  connect(_attachGroup,   SIGNAL(clicked()), this, SLOT(sAttachGroup()));
  connect(_deleteCharass, SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
  connect(_detachGroup,   SIGNAL(clicked()), this, SLOT(sDetachGroup()));
  connect(_editCharass,   SIGNAL(clicked()), this, SLOT(sEditCharass()));
  connect(_editGroup,     SIGNAL(clicked()), this, SLOT(sEditGroup()));
  connect(_newCharass,    SIGNAL(clicked()), this, SLOT(sNewCharass()));
  connect(_salesrepButton,SIGNAL(clicked()), this, SLOT(sSalesrep()));
  connect(_save,          SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_userButton,    SIGNAL(clicked()), this, SLOT(sUser()));
  connect(_viewGroup,     SIGNAL(clicked()), this, SLOT(sViewGroup()));

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");

  _groups->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "empgrp_name");
  _groups->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "empgrp_descrip");

  q.prepare("SELECT curr_abbr FROM curr_symbol WHERE (curr_id=baseCurrId());");
  q.exec();
  if (q.first())
    _currabbr = q.value("curr_abbr").toString();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _createUsers = false;
  q.exec("SELECT userCanCreateUsers(CURRENT_USER) AS enabled;");
  if (q.first())
    _createUsers = q.value("enabled").toBool();
  else if (q.lastError().type() != QSqlError::None)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  if (_privileges->check("MaintainSalesReps") ||
      _privileges->check("ViewSalesReps"))
    connect(_salesrep, SIGNAL(toggled(bool)), _salesrepButton, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainUsers"))
    connect(_user, SIGNAL(toggled(bool)), _userButton, SLOT(setEnabled(bool)));

  _comments->setId(-1);

  _empid = -1;
  _mode = cView;
  _origmode = cView;
}

employee::~employee()
{
  // no need to delete child widgets, Qt does it all for us
}

void employee::languageChange()
{
  retranslateUi(this);
}

enum SetResponse employee::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("emp_id", &valid);
  if (valid)
  {
    q.prepare("SELECT emp_code FROM emp WHERE (emp_id=:empid);");
    q.bindValue(":empid", param.toInt());
    q.exec();
    if (q.first())
    {
      _empid   = param.toInt();
      _empcode = q.value("emp_code").toString();
      _comments->setId(_empid);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    sPopulate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _user->setEnabled(_createUsers);
      _salesrep->setEnabled(_privileges->check("MaintainSalesReps"));
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  bool editing = (_mode == cNew || _mode == cEdit);
  if (editing)
  {
    connect(_charass,SIGNAL(valid(bool)), _deleteCharass, SLOT(setEnabled(bool)));
    connect(_charass,SIGNAL(valid(bool)), _editCharass, SLOT(setEnabled(bool)));
    connect(_groups, SIGNAL(valid(bool)), _detachGroup, SLOT(setEnabled(bool)));
    _attachGroup->setEnabled(true);
    if (empGroup::userHasPriv(cEdit))
    {
      connect(_groups, SIGNAL(valid(bool)), _editGroup,   SLOT(setEnabled(bool)));
    }
    if (empGroup::userHasPriv(cView))
      connect(_groups, SIGNAL(valid(bool)), _viewGroup,   SLOT(setEnabled(bool)));
  }

  _code->setEnabled(editing);
  _number->setEnabled(editing);
  _active->setEnabled(editing);
  _contact->setEnabled(editing);
  _site->setEnabled(editing);
  _mgr->setEnabled(editing);
  _wagetype->setEnabled(editing);
  _rate->setEnabled(editing);
  _per->setEnabled(editing);
  _dept->setEnabled(editing);
  _shift->setEnabled(editing);
  _notes->setEnabled(editing);
  _image->setEnabled(editing);
  _comments->setEnabled(editing);
  _save->setEnabled(editing);
  _newCharass->setEnabled(editing);

  _origmode = _mode;
  if (DEBUG)
    qDebug("employee::set() returning with _mode %d and _origmode %d",
           _mode, _origmode);

  return NoError;
}

void employee::sSave(const bool pClose)
{
  if (_user->isChecked() && pClose)
  {
    q.prepare("SELECT usr_id FROM usr WHERE usr_username=:username;");
    q.bindValue(":username", _code->text());
    q.exec();
    if (q.first())
    {
      // OK
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_createUsers) // not found
    {
      if (QMessageBox::question(this, tr("No Corresponding User"),
                            tr("There is no User named %1. Would you like to "
                               "create one now?<p>If you answer 'No' then you "
                               "should either Cancel creating this Employee, "
                               "uncheck the User check box, or use a different "
                               "Employee Code.")
                            .arg(_code->text()),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::Yes)
      {
        // don't use sUser() because it asks too many questions
        ParameterList params;
        params.append("mode",     "new");
        params.append("username", _code->text());
        user newdlg(this, "", TRUE);
        newdlg.set(params);
        newdlg.exec();
      }
      return;
    }
    else // not found
    {
      systemError(this, tr("There is no User named %1. Either Cancel creating "
                           "this Employee or use a different Employee Code.")
                         .arg(_code->text()));
      return;
    }
  }
  
  if (_salesrep->isChecked() && pClose)
  {
    q.prepare("SELECT salesrep_id FROM salesrep WHERE salesrep_number=:username;");
    q.bindValue(":username", _code->text());
    q.exec();
    if (q.first())
    {
      // OK
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else if (_privileges->check("MaintainSalesReps")) // not found
    {
      if (QMessageBox::question(this, tr("No Corresponding SalesRep"),
                            tr("There is no Sales Rep. named %1. Would you "
                               "like to create one now?<p>If you answer 'No' "
                               "then you should either Cancel creating this "
                               "Employee, uncheck the Sales Rep check box, or "
                               "use a different Employee Code.")
                            .arg(_code->text()),
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No) == QMessageBox::Yes)
      {
        // don't use sSalesrep() because it asks too many questions
        ParameterList params;
        params.append("mode",     "new");
        params.append("emp_id",   _empid);
        salesRep newdlg(this, "", TRUE);
        newdlg.set(params);
        newdlg.exec();
      }
      return;
    }
    else // not found
    {
      systemError(this, tr("There is no User named %1. Either Cancel creating "
                           "this Employee or use a different Employee Code.")
                         .arg(_code->text()));
      return;
    }
  }

  _contact->check();

  if (_mode == cNew)
    q.prepare("INSERT INTO api.employee ("
              " code, number, active,"
              " contact_number, honorific, first, last, job_title,"
              " voice, alternate, fax, email,"
              " web, contact_change,"
              " address_number, address1, address2, address3,"
              " city, state, postalcode,"
              " country, address_change,"
              " site, manager_code,"
              " wage_type, wage, wage_currency, wage_period,"
              " department, shift, is_user, is_salesrep, notes, image"
              ") VALUES ("
              " :code, :number, :active,"
              " :cntctnumber, :hnfc, :first, :last, :jobtitle,"
              " :voice, :alt, :fax, :email,"
              " :web, :cntctmode,"
              " :addrnumber, :addr1, :addr2, :addr3,"
              " :city, :state, :zip,"
              " :country, :addrmode,"
              " :site, :mgrcode,"
              " :wagetype, :wage, :wagecurr, :wageper,"
              " :dept, :shift, :isusr, :isrep, :notes, :image);");

  else if (_mode == cEdit)
    q.prepare("UPDATE api.employee SET"
              " code=:code,"
              " number=:number,"
              " active=:active,"
              " contact_number=:cntctnumber,"
              " honorific=:hnfc,"
              " first=:first,"
              " last=:last,"
              " job_title=:jobtitle,"
              " voice=:voice,"
              " alternate=:alt,"
              " fax=:fax,"
              " email=:email,"
              " web=:web,"
              " contact_change=:cntctmode,"
              " address_number=:addrnumber,"
              " address1=:addr1,"
              " address2=:addr2,"
              " address3=:addr3,"
              " city=:city,"
              " state=:state,"
              " postalcode=:zip,"
              " country=:country,"
              " address_change=:addrmode,"
              " site=:site,"
              " manager_code=:mgrcode,"
              " wage_type=:wagetype,"
              " wage=:wage,"
              " wage_currency=:wagecurr,"
              " wage_period=:wageper,"
              " department=:dept,"
              " shift=:shift,"
              " is_user=:isusr,"
              " is_salesrep=:isrep,"
              " notes=:notes,"
              " image=:image"
              " WHERE (code=:origcode);" );

  q.bindValue(":code",        _code->text());
  q.bindValue(":number",      _number->text());
  q.bindValue(":active",      _active->isChecked());
  q.bindValue(":cntctnumber", _contact->number());
  q.bindValue(":hnfc",        _contact->honorific());
  q.bindValue(":first",       _contact->first());
  q.bindValue(":last",        _contact->last());
  q.bindValue(":jobtitle",    _contact->title());
  q.bindValue(":voice",       _contact->phone());
  q.bindValue(":alt",         _contact->phone2());
  q.bindValue(":fax",         _contact->fax());
  q.bindValue(":email",       _contact->emailAddress());
  q.bindValue(":web",         _contact->webAddress());
  q.bindValue(":cntctmode",   _contact->change());
  q.bindValue(":addrnumber",  _contact->addressWidget()->number());
  q.bindValue(":addr1",       _contact->addressWidget()->line1());
  q.bindValue(":addr2",       _contact->addressWidget()->line2());
  q.bindValue(":addr3",       _contact->addressWidget()->line3());
  q.bindValue(":city",        _contact->addressWidget()->city());
  q.bindValue(":state",       _contact->addressWidget()->state());
  q.bindValue(":zip",         _contact->addressWidget()->postalCode());
  q.bindValue(":country",     _contact->addressWidget()->country());
  q.bindValue(":addrmode",    _contact->addressWidget()->addrChange());
  q.bindValue(":site",        _site->code());
  q.bindValue(":mgrcode",     _mgr->number());
  q.bindValue(":wagetype",    _wagetype->code());
  q.bindValue(":wage",        _rate->localValue());
  q.bindValue(":wagecurr",    _currabbr);
  q.bindValue(":wageper",     _per->code());
  q.bindValue(":dept",        _dept->number());
  q.bindValue(":shift",       _shift->number());
  q.bindValue(":isusr",       _user->isChecked());
  q.bindValue(":isrep",       _salesrep->isChecked());
  q.bindValue(":notes",       _notes->text());
  q.bindValue(":origcode",    _empcode);
  q.bindValue(":image",       _image->number());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT CURRVAL('emp_emp_id_seq') AS result;");
    if (q.first())
    {
      _empid = q.value("result").toInt();
      _mode  = cEdit;
      _comments->setId(_empid);
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (pClose)
    done(_empid);
}

void employee::reject()
{
  if (DEBUG)
    qDebug("employee::reject() entered with _mode %d and _origmode %d",
           _mode, _origmode);
  if (_origmode == cNew)
  {
    q.prepare("SELECT deleteEmp(:empid) AS result;");
    q.bindValue(":empid", _empid);
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
        systemError(this, storedProcErrorLookup("deleteEmp", result),
                    __FILE__, __LINE__);
    }
    else if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
  }

  XDialog::reject();
}

void employee::sPopulate()
{
  q.prepare("SELECT *, curr_id, curr_abbr "
            "FROM api.employee LEFT OUTER JOIN"
            "     curr_symbol ON (wage_currency = curr_abbr) "
            "WHERE (code=:code);");
  q.bindValue(":code", _empcode);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("code").toString());
    _number->setText(q.value("number").toString());
    _active->setChecked(q.value("active").toBool());
    _contact->setNumber(q.value("contact_number").toString());
    _contact->setHonorific(q.value("honorific").toString());
    _contact->setFirst(q.value("first").toString());
    _contact->setLast(q.value("last").toString());
    _contact->setTitle(q.value("job_title").toString());
    _contact->setPhone(q.value("voice").toString());
    _contact->setPhone2(q.value("alternate").toString());
    _contact->setFax(q.value("fax").toString());
    _contact->setEmailAddress(q.value("email").toString());
    _contact->setWebAddress(q.value("web").toString());
    _contact->addressWidget()->setLine1(q.value("address1").toString());
    _contact->addressWidget()->setLine2(q.value("address2").toString());
    _contact->addressWidget()->setLine3(q.value("address3").toString());
    _contact->addressWidget()->setCity(q.value("city").toString());
    _contact->addressWidget()->setState(q.value("state").toString());
    _contact->addressWidget()->setPostalCode(q.value("postalcode").toString());
    _contact->addressWidget()->setCountry(q.value("country").toString());
    _site->setCode(q.value("site").toString());
    _mgr->setNumber(q.value("manager_code").toString());
    _wagetype->setCode(q.value("wage_type").toString());
    _rate->set(q.value("wage").toDouble(),
               q.value("curr_id").toInt(),
               QDate::currentDate());
    _currabbr = q.value("curr_abbr").toString();
    _per->setCode(q.value("wage_period").toString());
    _dept->setNumber(q.value("department").toString());
    _shift->setNumber(q.value("shift").toString());

    _user->setChecked(q.value("is_user").toBool());
    _salesrep->setChecked(q.value("is_salesrep").toBool());
    _notes->setText(q.value("notes").toString());
    _image->setNumber(q.value("image").toString());

    _user->setEnabled(_createUsers && ! _user->isChecked());
    _userButton->setEnabled(_privileges->check("MaintainUsers") &&
                            _user->isChecked());

    _salesrep->setEnabled(_privileges->check("MaintainSalesReps") &&
                          ! _salesrep->isChecked());
    _salesrepButton->setEnabled((_privileges->check("MaintainSalesReps") ||
                                 _privileges->check("ViewSalesReps")) &&
                                _salesrep->isChecked());

    if (DEBUG)
      qDebug("image %s and %s",
             qPrintable(q.value("image").toString()),
             qPrintable(_image->number()));

    sFillCharassList();
    sFillGroupsList();
    _comments->setId(_empid);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void employee::sDeleteCharass()
{
  q.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charass->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillCharassList();
}

void employee::sEditCharass()
{
  ParameterList params;
  params.append("mode",       "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharassList();
}

void employee::sNewCharass()
{
  ParameterList params;
  params.append("mode",   "new");
  params.append("emp_id", _empid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharassList();
}

void employee::sFillCharassList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='EMP')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:emp_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":emp_id", _empid);
  q.exec();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void employee::sFillGroupsList()
{
  q.prepare( "SELECT empgrp.* "
             "FROM empgrp, empgrpitem "
             "WHERE ((empgrp_id=empgrpitem_empgrp_id)"
             "  AND  (empgrpitem_emp_id=:emp_id) ) "
             "ORDER BY empgrp_name;" );
  q.bindValue(":emp_id", _empid);
  q.exec();
  _groups->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void employee::sSalesrep()
{
  XSqlQuery srq;
  if (_empid < 0 && _code->text().isEmpty())
  {
    QMessageBox::warning(this, tr("Specify an Employee Code"),
                         tr("<p>You must either be editing an existing "
                            "Employee or have at least given an Employee Code "
                            "before trying to associate this Employee with a "
                            "Sales Rep."));
    return;
  }
  else if (_empid >= 0)
  {
    srq.prepare("SELECT salesrep_id "
                "FROM salesrep "
                "WHERE (salesrep_emp_id=:id);");
    srq.bindValue(":id", _empid);
  }
  else
  {
    srq.prepare("SELECT salesrep_id "
                "FROM salesrep "
                "WHERE (salesrep_number=:code);");
    srq.bindValue(":code", _code->text());
  }

  srq.exec();
  if (srq.first() &&
      (_privileges->check("MaintainSalesReps") ||
       _privileges->check("ViewSalesReps"))
      )
  {
    sSave(false);
    ParameterList params;
    if (_mode == cView || ! _privileges->check("MaintainSalesReps"))
      params.append("mode", "view");
    else
      params.append("mode", "edit");
    params.append("salesrep_id", srq.value("salesrep_id"));
    params.append("emp_id",      _empid);
    salesRep newdlg(this, "", TRUE);
    newdlg.set(params);
    _salesrep->setEnabled(newdlg.exec() == QDialog::Rejected);
  }
  else if (srq.lastError().type() != QSqlError::None)
  {
    systemError(this, srq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (_privileges->check("MaintainSalesReps") &&
           (_mode == cEdit || _mode == cNew))
  {
    if (QMessageBox::question(this, tr("Create Sales Rep?"),
                              tr("<p>There does not appear to be a Sales "
                                 "Rep associated with this Employee. "
                                 "Would you like to create a new Sales "
                                 "Rep?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No) == QMessageBox::Yes)
    {
      sSave(false);
      ParameterList params;
      params.append("mode",     "new");
      params.append("emp_id",   _empid);
      salesRep newdlg(this, "", TRUE);
      newdlg.set(params);
      _salesrep->setEnabled(newdlg.exec() == QDialog::Rejected);
    }
  }
}

void employee::sUser()
{
  XSqlQuery usrq;
  usrq.prepare("SELECT usr_username "
              "FROM usr JOIN emp ON (emp_usr_id=usr_id) "
              "WHERE (emp_id=:id);");
  usrq.bindValue(":id", _empid);
  usrq.exec();
  if (usrq.first() && _privileges->check("MaintainUsers"))
  {
    ParameterList params;
    params.append("mode",     (_mode == cView) ? "view" : "edit");
    params.append("username", usrq.value("usr_username"));
    user newdlg(this, "", TRUE);
    newdlg.set(params);
    _user->setEnabled(newdlg.exec() != QDialog::Rejected);
  }
  else if (usrq.lastError().type() != QSqlError::None)
  {
    systemError(this, usrq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else if (_createUsers && (_mode == cEdit || _mode == cNew))
  {
    if (QMessageBox::question(this, tr("Create User?"),
                              tr("<p>There does not appear to be a User "
                                 "associated with this Employee. "
                                 "Would you like to create a new User?"),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No) == QMessageBox::Yes)
    {
      sSave(false);
      ParameterList params;
      params.append("mode",     "new");
      params.append("username", _code->text());
      user newdlg(this, "", TRUE);
      newdlg.set(params);
      _user->setEnabled(newdlg.exec() == QDialog::Rejected);
    }
  }
  else
  {
    QMessageBox::warning(this, tr("Cannot Create User"),
                         tr("<p>There does not appear to be a User associated "
                            "with this Employee and you do not have permission "
                            "to create a User."));
    return;
  }
}

void employee::sAttachGroup()
{
  int empgrpid = EmpGroupClusterLineEdit::idFromList(this);
  if (empgrpid != XDialog::Rejected && empgrpid != -1)
  {
    q.prepare("INSERT INTO empgrpitem (empgrpitem_empgrp_id, empgrpitem_emp_id)"
              " VALUES "
              "(:empgrpid, :empid);");
    q.bindValue(":empgrpid", empgrpid);
    q.bindValue(":empid",    _empid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillGroupsList();
  }
}

void employee::sDetachGroup()
{
  q.prepare("DELETE FROM empgrpitem "
            "WHERE ((empgrpitem_empgrp_id=:grpid)"
            "  AND  (empgrpitem_emp_id=:empid));");
  q.bindValue(":grpid", _groups->id());
  q.bindValue(":empid", _empid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillGroupsList();
}

void employee::sEditGroup()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("empgrp_id", _groups->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillGroupsList();
}

void employee::sViewGroup()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("empgrp_id", _groups->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}
