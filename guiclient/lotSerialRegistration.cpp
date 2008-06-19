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

#include "lotSerialRegistration.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QMenu>
#include <QSqlRecord>

#include "storedProcErrorLookup.h"
#include "characteristicAssignment.h"

/*
 *  Constructs a lotSerialRegistration as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
lotSerialRegistration::lotSerialRegistration(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _lsregid = -1;

  // signals and slots connections
  connect(_save,	SIGNAL(clicked()),              this, SLOT(sSave()));
  connect(_soldDate,    SIGNAL(newDate(const QDate&)),  this, SLOT(sDateUpdated()));
  connect(_crmacct,     SIGNAL(newId(int)),             this, SLOT(sSetSoCustId()));
  connect(_deleteChar,  SIGNAL(clicked()),              this, SLOT(sDeleteCharass()));
  connect(_editChar,    SIGNAL(clicked()),              this, SLOT(sEditCharass()));
  connect(_newChar,     SIGNAL(clicked()),              this, SLOT(sNewCharass()));
  
  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft );
 
  _lotSerial->setStrict(true);
  _cntct->setAccountVisible(FALSE);
  _cntct->setActiveVisible(FALSE);

  _so->setType(cSoReleased);

  _qty->setValidator(omfgThis->qtyVal());
  
  resize(minimumSize());
}

/*
 *  Destroys the object and frees any allocated resources
 */
lotSerialRegistration::~lotSerialRegistration()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void lotSerialRegistration::languageChange()
{
  retranslateUi(this);
}

enum SetResponse lotSerialRegistration::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacct->setId(param.toInt());
  
  param = pParams.value("ls_id", &valid);
  if (valid)
    _lotSerial->setId(param.toInt());
  
  param = pParams.value("lsreg_id", &valid);
  if (valid)
    _lsregid=param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = cNew;

    if (param.toString() == "new")
    {
      _mode = cNew;
      q.exec ("SELECT fetchlsregnumber() AS number;");
      if (q.first())
	_regNumber->setText(q.value("number").toString());
      else if(q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        reject();
      }
      q.exec("SELECT NEXTVAL('lsreg_lsreg_id_seq') AS _lsreg_id;");
      if (q.first())
        _lsregid = q.value("_lsreg_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        reject();
      }
      _qty->setText("1");
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      populate();
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _regDate->setEnabled(false);
      _soldDate->setEnabled(false);
      _expireDate->setEnabled(false);
      _crmacct->setEnabled(false);
      _cntct->setEnabled(false);
      _type->setEnabled(false);
      _item->setReadOnly(true);
      _lotSerial->setEnabled(false);
      _newChar->setEnabled(false);
      _editChar->setEnabled(false);
      _deleteChar->setEnabled(false);
      _notes->setEnabled(false);
      _save->hide();
      _cancel->setText(tr("&Close"));
      _cancel->setFocus();
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }
  return NoError;
}

void lotSerialRegistration::sNewCharass()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("lsreg_id", _lsregid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerialRegistration::sEditCharass()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerialRegistration::sDeleteCharass()
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

  sFillList();
}

void lotSerialRegistration::sFillList()
{
  q.prepare( "SELECT charass_id, char_name, charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='LSR')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:lsreg_id) ) "
             "ORDER BY char_name;" );
  q.bindValue(":lsreg_id", _lsregid);
  q.exec();
  _charass->clear();
  _charass->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void lotSerialRegistration::populate()
{
  q.prepare("SELECT *"
            "  FROM lsreg LEFT OUTER JOIN ls ON (lsreg_ls_id=ls_id)"
            " WHERE(lsreg_id=:lsreg_id);");
  q.bindValue(":lsreg_id", _lsregid);
  q.exec();
  if(q.first())
  {
    _lsregid = q.value("lsreg_id").toInt();
    _regNumber->setText(q.value("lsreg_number").toString());
    _type->setId(q.value("lsreg_regtype_id").toInt());
    _lotSerial->setId(q.value("lsreg_ls_id").toInt());
    _qty->setDouble(q.value("lsreg_qty").toDouble());
    _item->setId(q.value("ls_item_id").toInt());
    _regDate->setDate(q.value("lsreg_regdate").toDate());
    _soldDate->setDate(q.value("lsreg_solddate").toDate());
    _expireDate->setDate(q.value("lsreg_expiredate").toDate());
    _crmacct->setId(q.value("lsreg_crmacct_id").toInt());
    _cntct->setId(q.value("lsreg_cntct_id").toInt());
    _notes->setText(q.value("lsreg_notes").toString());
    if(!q.value("lsreg_cohead_id").isNull())
      _so->setId(q.value("lsreg_cohead_id").toInt());
    if(!q.value("lsreg_shiphead_id").isNull())
      _shipment->setId(q.value("lsreg_shiphead_id").toInt());
    sFillList();
  }
  else if(q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void lotSerialRegistration::sSave()
{
  if(cView == _mode)
    return;

  if (!_regDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a registration date.");
    _regDate->setFocus();
    return;
  }
  
  if (!_soldDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a sold date.");
    _soldDate->setFocus();
    return;
  }

  if (!_expireDate->isValid())
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a expiration date.");
    _expireDate->setFocus();
    return;
  }
  
  if (_lotSerial->id() == -1)
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a lot/serial number.");
    _lotSerial->setFocus();
    return;
  }
  
  if (!(_qty->toDouble() > 0))
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a quantity greater than zero.");
    _qty->setFocus();
    return;
  }
  
  _cntct->check();
  // TODO: start explicit transaction?
  // TODO: make contactcluster smart enough to check its own change() state?
  if (_cntct->change() == "CHANGEONE")
    _cntct->save(AddressCluster::CHANGEONE);
  else if (_cntct->change() == "CHANGEALL")
    _cntct->save(AddressCluster::CHANGEALL);
  else
    _cntct->save();

  if (_cntct->id() == -1)
  {
    QMessageBox::warning(this, windowTitle(), "You must provide a contact.");
    _cntct->setFocus();
    return;
  }

  if(cNew == _mode)
  {
    q.prepare("INSERT INTO lsreg"
              "      (lsreg_id, lsreg_number, lsreg_regtype_id,"
              "       lsreg_ls_id, lsreg_qty, lsreg_regdate, lsreg_solddate,"
              "       lsreg_expiredate, lsreg_crmacct_id, lsreg_cntct_id,"
              "       lsreg_notes, lsreg_cohead_id, lsreg_shiphead_id) "
              "VALUES(:lsreg_id, :lsreg_number, :lsreg_regtype_id,"
              "       :lsreg_ls_id, :lsreg_qty, :lsreg_regdate, :lsreg_solddate,"
              "       :lsreg_expiredate, :lsreg_crmacct_id, :lsreg_cntct_id,"
              "       :lsreg_notes, :lsreg_cohead_id, :lsreg_shiphead_id);");
  }
  else if(cEdit == _mode)
    q.prepare("UPDATE lsreg"
              "   SET lsreg_number=:lsreg_number,"
              "       lsreg_regtype_id=:lsreg_regtype_id,"
              "       lsreg_ls_id=:lsreg_ls_id,"
              "       lsreg_qty=:lsreg_qty,"
              "       lsreg_regdate=:lsreg_regdate,"
              "       lsreg_solddate=:lsreg_solddate,"
              "       lsreg_expiredate=:lsreg_expiredate,"
              "       lsreg_crmacct_id=:lsreg_crmacct_id,"
              "       lsreg_cntct_id=:lsreg_cntct_id,"
              "       lsreg_notes=:lsreg_notes,"
              "       lsreg_cohead_id=:lsreg_cohead_id,"
              "       lsreg_shiphead_id=:lsreg_shiphead_id"
              " WHERE(lsreg_id=:lsreg_id);");
  
  q.bindValue(":lsreg_id", _lsregid);
  q.bindValue(":lsreg_number", _regNumber->text().stripWhiteSpace());
  q.bindValue(":lsreg_regtype_id", _type->id());
  q.bindValue(":lsreg_ls_id", _lotSerial->id());
  q.bindValue(":lsreg_qty", _qty->toDouble());
  q.bindValue(":lsreg_regdate", _regDate->date());
  q.bindValue(":lsreg_solddate", _soldDate->date());
  q.bindValue(":lsreg_expiredate", _expireDate->date());
  q.bindValue(":lsreg_crmacct_id", _crmacct->id());
  q.bindValue(":lsreg_cntct_id", _cntct->id());
  q.bindValue(":lsreg_notes", _notes->text());
  if(_so->id() != -1)
    q.bindValue(":lsreg_cohead_id", _so->id());
  if(_shipment->id() != -1)
    q.bindValue(":lsreg_shiphead_id", _shipment->id());

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  close();
}

void lotSerialRegistration::sDateUpdated()
{
  if(cView == _mode)
    return;

  QDate date = _soldDate->date();
  if(date.isNull())
    return;

  XSqlQuery dq;
  dq.prepare("SELECT item_warrdays"
             "  FROM item"
             " WHERE(item_id=:item_id);");
  dq.bindValue(":item_id", _item->id());
  dq.exec();
  if(dq.first())
  {
    date.addDays(dq.value("item_warrdays").toInt());
    _expireDate->setDate(date);
  }
}

void lotSerialRegistration::sSetSoCustId()
{
  if (_crmacct->id() != -1)
  {
    XSqlQuery cq;
    cq.prepare("SELECT crmacct_cust_id "
               "FROM crmacct "
	       "WHERE (crmacct_id=:crmacct_id);");
    cq.bindValue(":crmacct_id", _crmacct->id());
    cq.exec();
    if (cq.first())
    {
      _so->setType(cSoCustomer);
      _so->setCustId(cq.value("crmacct_cust_id").toInt()); 
    }
    else if(cq.lastError().type() != QSqlError::None)
    { 
      systemError(this, cq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }     
  }
  else
  {
    _so->setCustId(-1);
    _so->setType(cSoReleased);
  }
}
