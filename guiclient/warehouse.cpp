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

#include "warehouse.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "warehouseZone.h"

warehouse::warehouse(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_code,       SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_delete,	 SIGNAL(clicked()), this, SLOT(sDeleteZone()));
  connect(_edit,	 SIGNAL(clicked()), this, SLOT(sEditZone()));
  connect(_new,		 SIGNAL(clicked()), this, SLOT(sNewZone()));
  connect(_save,	 SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_standard, SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));
  connect(_transit,  SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));

  _whsezone->addColumn(tr("Name"),        _itemColumn, Qt::AlignCenter );
  _whsezone->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  
  //Handle single warehouse scenario
  if (!_metrics->boolean("MultiWhs"))
  {
    ParameterList params;
    q.exec("SELECT warehous_id "
              "FROM whsinfo; ");
    if (q.first())
    {
      params.append("mode", "edit");
      params.append("warehous_id", q.value("warehous_id").toInt());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    else
      params.append("mode", "new");
    set(params);

    _active->setChecked(TRUE);
    _active->hide();
  }

  _standard->setVisible(_metrics->boolean("MultiWhs"));
  _transit->setVisible(_metrics->boolean("MultiWhs"));
}

warehouse::~warehouse()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouse::languageChange()
{
  retranslateUi(this);
}

enum SetResponse warehouse::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _warehousid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();

      q.exec("SELECT NEXTVAL('warehous_warehous_id_seq') AS warehous_id");
      if (q.first())
        _warehousid = q.value("warehous_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _taxauth->setId(_metrics->value("DefaultTaxAuthority").toInt());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _defaultFOB->setEnabled(FALSE);
      _bolPrefix->setEnabled(FALSE);
      _bolNumber->setEnabled(FALSE);
      _shipping->setEnabled(FALSE);
      _countTagPrefix->setEnabled(FALSE);
      _countTagNumber->setEnabled(FALSE);
      _useSlips->setEnabled(FALSE);
      _arblGroup->setEnabled(FALSE);
      _useZones->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _account->setEnabled(FALSE);
      _shipcomm->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _transit->setEnabled(FALSE);
      _shipform->setEnabled(FALSE);
      _shipvia->setEnabled(FALSE);
      _shipcomments->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

// similar code in address, customer, shipto, vendor, vendorAddress
int warehouse::saveContact(ContactCluster* pContact)
{
  //pContact->setAccount(_crmacctid);

  int answer = 2;	// Cancel
  int saveResult = pContact->save(AddressCluster::CHECK);

  if (-1 == saveResult)
    systemError(this, tr("There was an error saving a Contact (%1, %2).\n"
			 "Check the database server log for errors.")
		      .arg(pContact->label()).arg(saveResult),
		__FILE__, __LINE__);
  else if (-2 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this address (%1).\n"
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Contact %1").arg(pContact->label()),
		    tr("Would you like to update the existing Contact or "
		       "create a new one?"),
		    tr("Create New"),
		    tr("Change Existing"),
		    tr("Cancel"),
		    2, 2);
  if (0 == answer)
    return pContact->save(AddressCluster::CHANGEONE);
  else if (1 == answer)
    return pContact->save(AddressCluster::CHANGEALL);

  return saveResult;
}

void warehouse::sSave()
{
  //  Make sure that at least a warehouse code has been entered
  if (_code->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Warehouse"),
                              tr("<p>You must enter a code for this Warehouse "
				 "before saving it.")  );
    _code->setFocus();
    return;
  }

  //  Make sure that the Count Tag prefix is unique
  if (_mode != cNew)
  {
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE ( (warehous_counttag_prefix=:prefix)"
               " AND (warehous_id<>:warehous_id) );" );

    q.bindValue(":warehous_id", _warehousid);
  }
  else
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE (warehous_counttag_prefix=:prefix)" );

  q.bindValue(":prefix", _countTagPrefix->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Count Tag Prefix not Unique"),
                           tr("<p>The Count Tag prefix entered has been used "
			      "in another Warehouse. To enable Count Tag "
			      "audits, the application requires a unique Count "
			      "Tag prefix for each Warehouse. Please enter a "
			      "different Count Tag prefix." ) );
    _countTagPrefix->clear();
    _countTagPrefix->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT warehous_id "
	     "FROM warehous "
	     "WHERE ( (warehous_id<>:warehous_id)"
	     " AND (UPPER(warehous_code)=UPPER(:warehous_code)) );" );
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":warehous_code", _code->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Warehouse"),
			   tr( "<p>The new Warehouse information cannot be "
			      "saved as the new Warehouse Code that you "
			      "entered conflicts with an existing Warehouse. "
			      "You must uniquely name this Warehouse before "
			      "you may save it." ) );
    _code->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  //  Make sure that a default G/L Account has been entered
  if (_account->id() == -1)
  {
    QMessageBox::information( this, tr("Cannot Save Warehouse"),
                              tr("<p>You must enter a default Account for this "
				 "Warehouse before saving it.")  );
    _account->setFocus();
    return;
  }

  if (_transit->isChecked() && ! _costcat->isValid())
  {
    QMessageBox::information(this, tr("Cannot Save Warehouse"),
			     tr("<p>You must select a Cost Category for this "
				"Transit Warehouse before saving it.") );
    _costcat->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (saveContact(_contact) < 0)
  {
    rollback.exec();
    _contact->setFocus();
    return;
  }

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple uses of this Warehouse "
		       "Address.\nWhat would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    systemError(this, tr("There was an error saving this address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    _address->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO whsinfo "
               "( warehous_id, warehous_code, warehous_descrip,"
               "  warehous_cntct_id, warehous_fob, warehous_active,"
               "  warehous_bol_prefix, warehous_bol_number, warehous_shipping,"
               "  warehous_counttag_prefix, warehous_counttag_number, warehous_useslips,"
               "  warehous_aislesize, warehous_aislealpha,"
               "  warehous_racksize, warehous_rackalpha,"
               "  warehous_binsize, warehous_binalpha,"
               "  warehous_locationsize, warehous_locationalpha,"
               "  warehous_enforcearbl, warehous_usezones, "
	       "  warehous_default_accnt_id, warehous_shipping_commission, "
	       "  warehous_addr_id, warehous_taxauth_id, warehous_transit,"
	       "  warehous_shipform_id, warehous_shipvia_id,"
	       "  warehous_shipcomments, warehous_costcat_id ) "
               "VALUES "
               "( :warehous_id, :warehous_code, :warehous_descrip,"
               "  :warehous_cntct_id, :warehous_fob, :warehous_active,"
               "  :warehous_bol_prefix, :warehous_bol_number, :warehous_shipping,"
               "  :warehous_counttag_prefix, :warehous_counttag_number, :warehous_useslips,"
               "  :warehous_aislesize, :warehous_aislealpha,"
               "  :warehous_racksize, :warehous_rackalpha,"
               "  :warehous_binsize, :warehous_binalpha,"
               "  :warehous_locationsize, :warehous_locationalpha,"
               "  :warehous_enforcearbl, :warehous_usezones, "
	       "  :warehous_default_accnt_id, :warehous_shipping_commission, "
	       "  :warehous_addr_id, :warehous_taxauth_id, :warehous_transit,"
	       "  :warehous_shipform_id, :warehous_shipvia_id,"
	       "  :warehous_shipcomments, :warehous_costcat_id );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE whsinfo "
               "SET warehous_code=:warehous_code,"
	       "    warehous_descrip=:warehous_descrip,"
               "    warehous_cntct_id=:warehous_cntct_id, "
               "    warehous_fob=:warehous_fob,"
	       "    warehous_active=:warehous_active,"
               "    warehous_bol_prefix=:warehous_bol_prefix,"
	       "    warehous_bol_number=:warehous_bol_number,"
               "    warehous_shipping=:warehous_shipping,"
               "    warehous_counttag_prefix=:warehous_counttag_prefix,"
	       "    warehous_counttag_number=:warehous_counttag_number,"
               "    warehous_useslips=:warehous_useslips,"
               "    warehous_aislesize=:warehous_aislesize,"
	       "    warehous_aislealpha=:warehous_aislealpha,"
               "    warehous_racksize=:warehous_racksize,"
	       "    warehous_rackalpha=:warehous_rackalpha,"
               "    warehous_binsize=:warehous_binsize,"
	       "    warehous_binalpha=:warehous_binalpha,"
               "    warehous_locationsize=:warehous_locationsize,"
	       "    warehous_locationalpha=:warehous_locationalpha,"
               "    warehous_enforcearbl=:warehous_enforcearbl,"
	       "    warehous_usezones=:warehous_usezones,"
               "    warehous_default_accnt_id=:warehous_default_accnt_id, "
	       "    warehous_shipping_commission=:warehous_shipping_commission,"
	       "    warehous_addr_id=:warehous_addr_id,"
	       "    warehous_taxauth_id=:warehous_taxauth_id,"
	       "    warehous_transit=:warehous_transit,"
	       "    warehous_shipform_id=:warehous_shipform_id,"
	       "    warehous_shipvia_id=:warehous_shipvia_id,"
	       "    warehous_shipcomments=:warehous_shipcomments,"
	       "    warehous_costcat_id=:warehous_costcat_id "
               "WHERE (warehous_id=:warehous_id);" );
  
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":warehous_code", _code->text().stripWhiteSpace().upper());
  q.bindValue(":warehous_descrip", _description->text());
  if (_contact->id() > 0)
    q.bindValue(":warehous_cntct_id", _contact->id());	// else NULL
  if (_address->id() > 0)
    q.bindValue(":warehous_addr_id", _address->id());	// else NULL

  q.bindValue(":warehous_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":warehous_default_accnt_id", _account->id());

  if (_standard->isChecked())
  {
    q.bindValue(":warehous_fob",	     _defaultFOB->text());
    q.bindValue(":warehous_bol_prefix",	     _bolPrefix->text());
    q.bindValue(":warehous_bol_number",	     _bolNumber->text().toInt());
    q.bindValue(":warehous_counttag_prefix", _countTagPrefix->text());
    q.bindValue(":warehous_counttag_number", _countTagNumber->text().toInt());
    q.bindValue(":warehous_shipping",	QVariant(_shipping->isChecked(), 0));
    q.bindValue(":warehous_useslips",	QVariant(_useSlips->isChecked(), 0));
    q.bindValue(":warehous_enforcearbl",QVariant(_arblGroup->isChecked(), 0));
    q.bindValue(":warehous_aislesize",	_aisleSize->value());
    q.bindValue(":warehous_aislealpha",	QVariant(_aisleAlpha->isChecked(), 0));
    q.bindValue(":warehous_racksize",	_rackSize->value());
    q.bindValue(":warehous_rackalpha",	QVariant(_rackAlpha->isChecked(), 0));
    q.bindValue(":warehous_binsize",	_binSize->value());
    q.bindValue(":warehous_binalpha",	QVariant(_binAlpha->isChecked(), 0));
    q.bindValue(":warehous_locationsize",  _locationSize->value());
    q.bindValue(":warehous_locationalpha", QVariant(_locationAlpha->isChecked(), 0));
    q.bindValue(":warehous_usezones",	   QVariant(_useZones->isChecked(), 0));
    q.bindValue(":warehous_shipping_commission", (_shipcomm->toDouble() / 100));
    if(_taxauth->isValid())
      q.bindValue(":warehous_taxauth_id",	_taxauth->id());
  }

  q.bindValue(":warehous_transit",	QVariant(_transit->isChecked(), 0));
  if (_transit->isChecked())
  {
    if (_shipform->isValid())
      q.bindValue(":warehous_shipform_id",	_shipform->id());
    if (_shipvia->isValid())
      q.bindValue(":warehous_shipvia_id",	_shipvia->id());
    q.bindValue(":warehous_shipcomments",	_shipcomments->text());
    if (_costcat->isValid())
      q.bindValue(":warehous_costcat_id",	_costcat->id());
  }

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  omfgThis->sWarehousesUpdated();
  done(_warehousid);
}

void warehouse::populate()
{
  q.prepare( "SELECT *, formatScrap(warehous_shipping_commission) AS f_commission "
             "FROM whsinfo "
             "WHERE (warehous_id=:warehous_id);" );
  q.bindValue(":warehous_id", _warehousid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("warehous_code"));
    _active->setChecked(q.value("warehous_active").toBool());
    _description->setText(q.value("warehous_descrip"));
    _contact->setId(q.value("warehous_cntct_id").toInt());
    _address->setId(q.value("warehous_addr_id").toInt());
    _defaultFOB->setText(q.value("warehous_fob"));
    _bolPrefix->setText(q.value("warehous_bol_prefix"));
    _bolNumber->setText(q.value("warehous_bol_number"));
    _shipping->setChecked(q.value("warehous_shipping").toBool());
    _countTagPrefix->setText(q.value("warehous_counttag_prefix"));
    _countTagNumber->setText(q.value("warehous_counttag_number"));
    _useSlips->setChecked(q.value("warehous_useslips").toBool());
    _arblGroup->setChecked(q.value("warehous_enforceARBL").toBool());
    _aisleSize->setValue(q.value("warehous_aislesize").toInt());
    _aisleAlpha->setChecked(q.value("warehous_aislealpha").toBool());
    _rackSize->setValue(q.value("warehous_racksize").toInt());
    _rackAlpha->setChecked(q.value("warehous_rackalpha").toBool());
    _binSize->setValue(q.value("warehous_binsize").toInt());
    _binAlpha->setChecked(q.value("warehous_binalpha").toBool());
    _locationSize->setValue(q.value("warehous_locationsize").toInt());
    _locationAlpha->setChecked(q.value("warehous_locationalpha").toBool());
    _useZones->setChecked(q.value("warehous_usezones").toBool());
    _account->setId(q.value("warehous_default_accnt_id").toInt());
    _shipcomm->setText(q.value("f_commission").toString());
    _taxauth->setId(q.value("warehous_taxauth_id").toInt());
    _transit->setChecked(q.value("warehous_transit").toBool());
    _shipform->setId(q.value("warehous_shipform_id").toInt());
    _shipvia->setId(q.value("warehous_shipvia_id").toInt());
    _shipcomments->setText(q.value("warehous_shipcomments").toString());
    _costcat->setId(q.value("warehous_costcat_id").toInt());

    sFillList();
    _comments->setId(_warehousid);
  }
}

void warehouse::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace().upper());

  if (_mode == cNew)
  {
    q.prepare( "SELECT warehous_id "
               "FROM warehous "
               "WHERE (UPPER(warehous_code)=UPPER(:warehous_code));" );
    q.bindValue(":warehous_code", _code->text());
    q.exec();
    if (q.first())
    {
      _warehousid = q.value("warehous_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
    else
    {
      if (_countTagPrefix->text().stripWhiteSpace().length() == 0)
        _countTagPrefix->setText(_code->text());

      if (_bolPrefix->text().stripWhiteSpace().length() == 0)
        _bolPrefix->setText(_code->text());
    }
  }
}

void warehouse::sNewZone()
{
  ParameterList params;
  params.append("warehous_id", _warehousid);
  params.append("mode", "new");

  warehouseZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sEditZone()
{
  ParameterList params;
  params.append("whsezone_id", _whsezone->id());
  params.append("mode", "edit");

  warehouseZone newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sDeleteZone()
{
  q.prepare( "SELECT location_id "
             "FROM location "
             "WHERE (location_whsezone_id=:whsezone_id);" );
  q.bindValue(":whsezone_id", _whsezone->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::warning( this, tr("Cannot Delete Warehouse Zone"),
                          tr( "<p>The selected Warehouse Zone cannot be "
			     "deleted as one or more Warehouse Locations have "
			     "been assigned to it. You must delete or reassign "
			     "these Warehouse Location before you may delete "
			     "the selected Warehouse Zone." ) );
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "DELETE FROM whsezone "
             "WHERE (whsezone_id=:whsezone_id);" );
  q.bindValue(":whsezone_id", _whsezone->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void warehouse::sFillList()
{
  q.prepare( "SELECT whsezone_id, whsezone_name, whsezone_descrip "
             "FROM whsezone "
             "WHERE (whsezone_warehous_id=:warehous_id) "
             "ORDER BY whsezone_name;" );
  q.bindValue(":warehous_id", _warehousid);
  q.exec();
  _whsezone->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void warehouse::sHandleWhsType()
{
  if (_standard->isChecked())
  {
    _whsTypeStack->setCurrentIndex(0);
    _locationsTab->setEnabled(true);
  }
  else if (_transit->isChecked())
  {
    _whsTypeStack->setCurrentIndex(1);
    _locationsTab->setEnabled(false);
  }
}
