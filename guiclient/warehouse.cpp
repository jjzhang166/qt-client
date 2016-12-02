/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouse.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "warehouseZone.h"

warehouse::warehouse(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl),
    _mode(cView),
    _warehousid(-1)
{
  setupUi(this);

  connect(_code,       SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_delete,       SIGNAL(clicked()), this, SLOT(sDeleteZone()));
  connect(_edit,         SIGNAL(clicked()), this, SLOT(sEditZone()));
  connect(_new,          SIGNAL(clicked()), this, SLOT(sNewZone()));
  connect(_save,         SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_standard, SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));
  connect(_transit,  SIGNAL(toggled(bool)), this, SLOT(sHandleWhsType()));

  connect(_address, SIGNAL(addressChanged(QString,QString,QString,QString,QString,QString, QString)),
          _contact, SLOT(setNewAddr(QString,QString,QString,QString,QString,QString, QString)));

  _whsezone->addColumn(tr("Name"), _itemColumn, Qt::AlignCenter, true, "whsezone_name");
  _whsezone->addColumn(tr("Description"),   -1, Qt::AlignLeft,   true, "whsezone_descrip");

  if (!_metrics->boolean("MultiWhs"))
  {
    _active->setChecked(true);
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
  XSqlQuery warehouseet;
  XDialog::set(pParams);
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
      warehouseet.exec("SELECT NEXTVAL('warehous_warehous_id_seq') AS warehous_id");
      if (warehouseet.first())
        _warehousid = warehouseet.value("warehous_id").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Warehouse Information"),
                                    warehouseet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      emit newId(_warehousid);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_whsezone, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_whsezone, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(false);
      _sitetype->setEnabled(false);
      _active->setEnabled(false);
      _description->setEnabled(false);
      _contact->setEnabled(false);
      _address->setEnabled(false);
      _defaultFOB->setEnabled(false);
      _bolPrefix->setEnabled(false);
      _bolNumber->setEnabled(false);
      _shipping->setEnabled(false);
      _countTagPrefix->setEnabled(false);
      _countTagNumber->setEnabled(false);
      _sequence->setEnabled(false);
      _useSlips->setEnabled(false);
      _arblGroup->setEnabled(false);
      _useZones->setEnabled(false);
      _new->setEnabled(false);
      _account->setEnabled(false);
      _shipcomm->setEnabled(false);
      _taxzone->setEnabled(false);
      _comments->setReadOnly(true);
      _transit->setEnabled(false);
      _shipform->setEnabled(false);
      _pickList->setEnabled(false);
      _shipvia->setEnabled(false);
      _shipcomments->setEnabled(false);

      _close->setText(tr("&Close"));
      _save->hide();
    }

    emit newMode(_mode);
  }

  return NoError;
}

int warehouse::id() const
{
  return _warehousid;
}

int warehouse::mode() const
{
  return _mode;
}

void warehouse::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_code->text().trimmed().isEmpty(), _code,
                          tr("<p>You must enter a code for this Site "
                                 "before saving it."))
         << GuiErrorCheck(!_sitetype->isValid(), _sitetype,
                          tr("<p>You must enter a Type for this "
                                 "Site before saving it."))
         << GuiErrorCheck(_account->id() == -1, _account,
                          tr("<p>You must enter a default Account for this "
                             "Site before saving it."))
         << GuiErrorCheck(_transit->isChecked() && ! _costcat->isValid(),
                          _costcat,
                          tr("<p>You must select a Cost Category for this "
                             "Transit Site before saving it."))
         ;

  XSqlQuery ctq;
  if (_mode != cNew)
  {
    ctq.prepare( "SELECT warehous_id "
               "FROM whsinfo "
               "WHERE ( (warehous_counttag_prefix=:prefix)"
               " AND (warehous_id<>:warehous_id) );" );
    ctq.bindValue(":warehous_id", _warehousid);
  }
  else
    ctq.prepare( "SELECT warehous_id "
               "FROM whsinfo "
               "WHERE (warehous_counttag_prefix=:prefix)" );
  ctq.bindValue(":prefix", _countTagPrefix->text());
  ctq.exec();
  if (ctq.first())
  {
    errors << GuiErrorCheck(true, _countTagPrefix,
                           tr("<p>The Count Tag prefix entered has been used "
                              "in another Site. To enable Count Tag "
                              "audits, the application requires a unique Count "
                              "Tag prefix for each Site. Please enter a "
                              "different Count Tag prefix." ) );
    _countTagPrefix->clear();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Count Tag"),
                                ctq, __FILE__, __LINE__))
    return;

  XSqlQuery uniq;
  uniq.prepare("SELECT warehous_id "
               "FROM whsinfo "
               "WHERE ( (warehous_id<>:warehous_id)"
               " AND (UPPER(warehous_code)=UPPER(:warehous_code)) );" );
  uniq.bindValue(":warehous_id", _warehousid);
  uniq.bindValue(":warehous_code", _code->text());
  uniq.exec();
  if (uniq.first())
  {
    errors << GuiErrorCheck(true, _code,
                            tr("<p>The new Site information cannot be "
                               "saved as the new Site Code that you "
                               "entered conflicts with an existing Site. "
                               "You must uniquely name this Site before "
                               "you may save it." ));
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Site Code"),
                                uniq, __FILE__, __LINE__))
    return;

  XSqlQuery activeq;
  if (!_active->isChecked())
  {
    activeq.prepare( "SELECT itemsite_id "
                     "FROM itemsite "
                     "WHERE ( (itemsite_active)"
                     " AND (itemsite_warehous_id=:warehous_id) );" );
    activeq.bindValue(":warehous_id", _warehousid);
    activeq.exec();
    if (activeq.first())
    {
      errors << GuiErrorCheck(true, _active,
                              tr("This Site is used in an active Item Site and must be marked as active. "
                              "Deactivate the Item Sites to allow this Site to not be active.") );
      _active->setFocus();
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Active Itemsites"),
                                  activeq, __FILE__, __LINE__))
      return;
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Site"), errors))
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  XSqlQuery begin("BEGIN");
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                           begin, __FILE__, __LINE__))
    return;

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this, tr("Question Saving Address"),
                    tr("<p>There are multiple uses of this Site Address.</p>"
                       "<p>What would you like to do?</p>"),
                    tr("Change This One"),
                    tr("Change Address for All"),
                    tr("Cancel"),
                    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)   // not else-if: this is error check for CHANGE{ONE,ALL}
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: There was an error saving this address (%2).\n"
                            "Check the database server log for errors.")
                         .arg(windowTitle())
                         .arg(saveResult),__FILE__,__LINE__);
    _address->setFocus();
    return;
  }

  XSqlQuery upsq;
  if (_mode == cNew)
    upsq.prepare( "INSERT INTO whsinfo "
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
               "  warehous_addr_id, warehous_taxzone_id, warehous_transit,"
               "  warehous_shipform_id, warehous_picklist_shipform_id, warehous_shipvia_id,"
               "  warehous_shipcomments, warehous_costcat_id, warehous_sitetype_id,"
               "  warehous_sequence ) "
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
               "  :warehous_addr_id, :warehous_taxzone_id, :warehous_transit,"
               "  :warehous_shipform_id, :warehous_picklist_shipform_id, :warehous_shipvia_id,"
               "  :warehous_shipcomments, :warehous_costcat_id, :warehous_sitetype_id,"
               "  :warehous_sequence );" );
  else if (_mode == cEdit)
    upsq.prepare( "UPDATE whsinfo "
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
               "    warehous_taxzone_id=:warehous_taxzone_id,"
               "    warehous_transit=:warehous_transit,"
               "    warehous_shipform_id=:warehous_shipform_id,"
               "    warehous_picklist_shipform_id=:warehous_picklist_shipform_id,"
               "    warehous_shipvia_id=:warehous_shipvia_id,"
               "    warehous_shipcomments=:warehous_shipcomments,"
               "    warehous_costcat_id=:warehous_costcat_id, "
               "    warehous_sitetype_id=:warehous_sitetype_id,"
               "    warehous_sequence=:warehous_sequence "
               "WHERE (warehous_id=:warehous_id);" );

  upsq.bindValue(":warehous_id", _warehousid);
  upsq.bindValue(":warehous_code", _code->text().trimmed().toUpper());
  upsq.bindValue(":warehous_descrip", _description->text());
  if (_contact->id() > 0)
    upsq.bindValue(":warehous_cntct_id", _contact->id());  // else NULL
  if (_address->id() > 0)
    upsq.bindValue(":warehous_addr_id", _address->id());   // else NULL

  upsq.bindValue(":warehous_active", QVariant(_active->isChecked()));
  upsq.bindValue(":warehous_default_accnt_id", _account->id());
  if(_sitetype->isValid())
    upsq.bindValue(":warehous_sitetype_id", _sitetype->id());
  upsq.bindValue(":warehous_sequence",     _sequence->value());

  if (_standard->isChecked())
  {
    upsq.bindValue(":warehous_fob",             _defaultFOB->text());
    upsq.bindValue(":warehous_bol_prefix",      _bolPrefix->text());
    upsq.bindValue(":warehous_bol_number",      _bolNumber->text().toInt());
    upsq.bindValue(":warehous_counttag_prefix", _countTagPrefix->text());
    upsq.bindValue(":warehous_counttag_number", _countTagNumber->text().toInt());
    upsq.bindValue(":warehous_shipping",   QVariant(_shipping->isChecked()));
    upsq.bindValue(":warehous_useslips",   QVariant(_useSlips->isChecked()));
    upsq.bindValue(":warehous_enforcearbl",QVariant(_arblGroup->isChecked()));
    upsq.bindValue(":warehous_aislesize",  _aisleSize->value());
    upsq.bindValue(":warehous_aislealpha", QVariant(_aisleAlpha->isChecked()));
    upsq.bindValue(":warehous_racksize",   _rackSize->value());
    upsq.bindValue(":warehous_rackalpha",  QVariant(_rackAlpha->isChecked()));
    upsq.bindValue(":warehous_binsize",    _binSize->value());
    upsq.bindValue(":warehous_binalpha",   QVariant(_binAlpha->isChecked()));
    upsq.bindValue(":warehous_locationsize",  _locationSize->value());
    upsq.bindValue(":warehous_locationalpha", QVariant(_locationAlpha->isChecked()));
    upsq.bindValue(":warehous_usezones",      QVariant(_useZones->isChecked()));
    upsq.bindValue(":warehous_shipping_commission", (_shipcomm->toDouble() / 100));
    if(_taxzone->isValid())
      upsq.bindValue(":warehous_taxzone_id",       _taxzone->id());
  }

  upsq.bindValue(":warehous_transit",      QVariant(_transit->isChecked()));
  if (_transit->isChecked())
  {
    if (_shipform->isValid())
      upsq.bindValue(":warehous_shipform_id",      _shipform->id());
    if (_pickList->isValid())
      upsq.bindValue(":warehous_picklist_shipform_id",  _pickList->id()); 
    if (_shipvia->isValid())
      upsq.bindValue(":warehous_shipvia_id",       _shipvia->id());
    upsq.bindValue(":warehous_shipcomments",       _shipcomments->toPlainText());
    if (_costcat->isValid())
      upsq.bindValue(":warehous_costcat_id",       _costcat->id());
  }

  upsq.exec();
  if (upsq.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                         upsq, __FILE__, __LINE__);
    return;
  }

  XSqlQuery commit("COMMIT;");

  omfgThis->sWarehousesUpdated();
  emit saved(_warehousid);
  done(_warehousid);
}

void warehouse::populate()
{
  XSqlQuery getq;
  getq.prepare( "SELECT *, formatScrap(warehous_shipping_commission) AS f_commission "
             "FROM whsinfo "
             "WHERE (warehous_id=:warehous_id);" );
  getq.bindValue(":warehous_id", _warehousid);
  getq.exec();
  if (getq.first())
  {
    _code->setText(getq.value("warehous_code"));
    _sitetype->setId(getq.value("warehous_sitetype_id").toInt());
    _active->setChecked(getq.value("warehous_active").toBool());
    _description->setText(getq.value("warehous_descrip"));
    _contact->setId(getq.value("warehous_cntct_id").toInt());
    _address->setId(getq.value("warehous_addr_id").toInt());
    _defaultFOB->setText(getq.value("warehous_fob"));
    _bolPrefix->setText(getq.value("warehous_bol_prefix"));
    _bolNumber->setText(getq.value("warehous_bol_number"));
    _shipping->setChecked(getq.value("warehous_shipping").toBool());
    _countTagPrefix->setText(getq.value("warehous_counttag_prefix"));
    _countTagNumber->setText(getq.value("warehous_counttag_number"));
    _useSlips->setChecked(getq.value("warehous_useslips").toBool());
    _arblGroup->setChecked(getq.value("warehous_enforceARBL").toBool());
    _aisleSize->setValue(getq.value("warehous_aislesize").toInt());
    _aisleAlpha->setChecked(getq.value("warehous_aislealpha").toBool());
    _rackSize->setValue(getq.value("warehous_racksize").toInt());
    _rackAlpha->setChecked(getq.value("warehous_rackalpha").toBool());
    _binSize->setValue(getq.value("warehous_binsize").toInt());
    _binAlpha->setChecked(getq.value("warehous_binalpha").toBool());
    _locationSize->setValue(getq.value("warehous_locationsize").toInt());
    _locationAlpha->setChecked(getq.value("warehous_locationalpha").toBool());
    _useZones->setChecked(getq.value("warehous_usezones").toBool());
    _account->setId(getq.value("warehous_default_accnt_id").toInt());
    _shipcomm->setText(getq.value("f_commission").toString());
    _taxzone->setId(getq.value("warehous_taxzone_id").toInt());
    _transit->setChecked(getq.value("warehous_transit").toBool());
    _shipform->setId(getq.value("warehous_shipform_id").toInt());
    _pickList->setId(getq.value("warehous_picklist_shipform_id").toInt());
    _shipvia->setId(getq.value("warehous_shipvia_id").toInt());
    _shipcomments->setText(getq.value("warehous_shipcomments").toString());
    _costcat->setId(getq.value("warehous_costcat_id").toInt());
    _sequence->setValue(getq.value("warehous_sequence").toInt());

    sFillList();
    _comments->setId(_warehousid);

    emit populated();
    emit newId(_warehousid);
  }

}

void warehouse::sCheck()
{
  XSqlQuery warehouseCheck;
  _code->setText(_code->text().trimmed().toUpper());

  if (_mode == cNew)
  {
    warehouseCheck.prepare( "SELECT warehous_id "
               "FROM whsinfo "
               "WHERE (UPPER(warehous_code)=UPPER(:warehous_code));" );
    warehouseCheck.bindValue(":warehous_code", _code->text());
    warehouseCheck.exec();
    if (warehouseCheck.first())
    {
      _warehousid = warehouseCheck.value("warehous_id").toInt();
      _mode = cEdit;
      populate();
      emit newMode(_mode);

      _code->setEnabled(false);
    }
    else
    {
      if (_countTagPrefix->text().trimmed().length() == 0)
        _countTagPrefix->setText(_code->text());

      if (_bolPrefix->text().trimmed().length() == 0)
        _bolPrefix->setText(_code->text());
    }
  }
}

void warehouse::sNewZone()
{
  ParameterList params;
  params.append("warehous_id", _warehousid);
  params.append("mode", "new");

  warehouseZone newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sEditZone()
{
  ParameterList params;
  params.append("whsezone_id", _whsezone->id());
  params.append("mode", "edit");

  warehouseZone newdlg(this, "", true);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void warehouse::sDeleteZone()
{
  XSqlQuery warehouseDeleteZone;
  warehouseDeleteZone.prepare( "SELECT location_id "
             "FROM location "
             "WHERE (location_whsezone_id=:whsezone_id);" );
  warehouseDeleteZone.bindValue(":whsezone_id", _whsezone->id());
  warehouseDeleteZone.exec();
  if (warehouseDeleteZone.first())
  {
    QMessageBox::warning( this, tr("Cannot Delete Site Zone"),
                          tr( "<p>The selected Site Zone cannot be "
                             "deleted as one or more Site Locations have "
                             "been assigned to it. You must delete or reassign "
                             "these Site Location before you may delete "
                             "the selected Site Zone." ) );
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Site Zone Information"),
                                warehouseDeleteZone, __FILE__, __LINE__))
  {
    return;
  }

  warehouseDeleteZone.prepare( "DELETE FROM whsezone "
             "WHERE (whsezone_id=:whsezone_id);" );
  warehouseDeleteZone.bindValue(":whsezone_id", _whsezone->id());
  warehouseDeleteZone.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Site Zone Information"),
                                warehouseDeleteZone, __FILE__, __LINE__))
  {
    return;
  }
  sFillList();
}

void warehouse::sFillList()
{
  XSqlQuery warehouseFillList;
  warehouseFillList.prepare( "SELECT whsezone_id, whsezone_name, whsezone_descrip "
             "FROM whsezone "
             "WHERE (whsezone_warehous_id=:warehous_id) "
             "ORDER BY whsezone_name;" );
  warehouseFillList.bindValue(":warehous_id", _warehousid);
  warehouseFillList.exec();
  _whsezone->populate(warehouseFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Warehouse Information"),
                                warehouseFillList, __FILE__, __LINE__))
  {
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
