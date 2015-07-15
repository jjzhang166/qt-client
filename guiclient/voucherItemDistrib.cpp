/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "voucherItemDistrib.h"

#include <QVariant>

voucherItemDistrib::voucherItemDistrib(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_costelem, SIGNAL(newID(int)), this, SLOT(sCheck()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _costelem->populate( QString( "SELECT costelem_id, costelem_type, 1 AS orderby "
                                "FROM costelem "
                                "WHERE (costelem_type='Material') "

                                "UNION SELECT 0, '%1' AS costelem_type, 2 AS orderby "

                                "UNION SELECT costelem_id, costelem_type, 3 AS orderby "
                                "FROM costelem "
                                "WHERE ( (costelem_active)"
                                " AND (NOT costelem_sys)"
                                " AND (costelem_po) ) "
                                "ORDER BY orderby, costelem_type;" )
                       .arg(tr("None")) );
}

voucherItemDistrib::~voucherItemDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

void voucherItemDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse voucherItemDistrib::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("poitem_id", &valid);
  if (valid)
    _poitemid = param.toInt();

  param = pParams.value("vohead_id", &valid);
  if (valid)
    _voheadid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _amount->setId(param.toInt());

  param = pParams.value("effective", &valid);
  if (valid)
    _amount->setEffective(param.toDate());

  param = pParams.value("vodist_id", &valid);
  if (valid)
  {
    _vodistid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      param = pParams.value("amount", &valid);
      if (valid)
        _amount->setLocalValue(param.toDouble());
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _costelem->setEnabled(false);
    }
  }

  return NoError;
}

void voucherItemDistrib::populate()
{
  XSqlQuery voucherpopulate;
  voucherpopulate.prepare( "SELECT * "
             "FROM vodist "
             "WHERE (vodist_id=:vodist_id);" ) ;
  voucherpopulate.bindValue(":vodist_id", _vodistid);
  voucherpopulate.exec();
  if (voucherpopulate.first())
  {
    _costelem->setId(voucherpopulate.value("vodist_costelem_id").toInt());
    _amount->setLocalValue(voucherpopulate.value("vodist_amount").toDouble());
    _discountable->setChecked(voucherpopulate.value("vodist_discountable").toBool());
    _notes->setText(voucherpopulate.value("vodist_notes").toString());
  }
}

bool voucherItemDistrib::sCheck()
{
  XSqlQuery voucherCheck;
  if (_mode == cNew)
  {
    voucherCheck.prepare( "SELECT vodist_id "
               "FROM vodist "
               "WHERE ( (vodist_vohead_id=:vodist_vohead_id)"
               "  AND   (vodist_poitem_id=:vodist_poitem_id)"
               "  AND   (vodist_costelem_id=:vodist_costelem_id) );" );
    voucherCheck.bindValue(":vodist_vohead_id", _voheadid);
    voucherCheck.bindValue(":vodist_poitem_id", _poitemid);
    voucherCheck.bindValue(":vodist_costelem_id", _costelem->id());
    voucherCheck.exec();
    if (voucherCheck.first())
    {
      _vodistid = voucherCheck.value("vodist_id").toInt();
      _mode = cEdit;
      populate();

      _costelem->setEnabled(false);
      return false;
    }
  }
  return true;
}

void voucherItemDistrib::sSave()
{
  XSqlQuery voucherSave;
  if (_mode == cNew)
  {
    if (!sCheck())
      return;
      
    voucherSave.exec("SELECT NEXTVAL('vodist_vodist_id_seq') AS _vodistid;");
    if (voucherSave.first())
      _vodistid = voucherSave.value("_vodistid").toInt();

    voucherSave.prepare( "INSERT INTO vodist "
               "( vodist_id, vodist_vohead_id, vodist_poitem_id,"
               "  vodist_costelem_id, vodist_amount, vodist_discountable,"
			   "  vodist_notes ) "
               "VALUES "
               "( :vodist_id, :vodist_vohead_id, :vodist_poitem_id,"
               "  :vodist_costelem_id, :vodist_amount, :vodist_discountable,"
			   "  :vodist_notes );" );
  }
  if (_mode == cEdit)
    voucherSave.prepare( "UPDATE vodist "
               "SET vodist_costelem_id=:vodist_costelem_id,"
               "    vodist_amount=:vodist_amount,"
               "    vodist_discountable=:vodist_discountable,"
			   "    vodist_notes=:vodist_notes "
               "WHERE (vodist_id=:vodist_id);" );

  voucherSave.bindValue(":vodist_id", _vodistid);
  voucherSave.bindValue(":vodist_vohead_id", _voheadid);
  voucherSave.bindValue(":vodist_poitem_id", _poitemid);
  voucherSave.bindValue(":vodist_costelem_id", _costelem->id());
  voucherSave.bindValue(":vodist_amount", _amount->localValue());
  voucherSave.bindValue(":vodist_discountable", QVariant(_discountable->isChecked()));
  voucherSave.bindValue(":vodist_notes", _notes->toPlainText().trimmed());
  voucherSave.exec();

  done(_vodistid);
}
  
