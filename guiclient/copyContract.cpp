/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyContract.h"

#include <QSqlError>
#include <QVariant>

#include "inputManager.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

copyContract::copyContract(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sCopy()));

  _contrctid = -1;
  _vendid = -1;
  _captive = false;
}

copyContract::~copyContract()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyContract::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyContract::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("contrct_id", &valid);
  if (valid)
  {
    _captive = true;
    _contrctid = param.toInt();
    sPopulateContractInfo();
  }

  return NoError;
}

void copyContract::sPopulateContractInfo()
{
  XSqlQuery copyPopulate;
  if (_contrctid != -1)
  {
    copyPopulate.prepare( "SELECT contrct.*,  vend_name,"
                          "       (contrct_expires + 1) AS neweffective "
                          " FROM contrct "
                          " JOIN vendinfo ON (contrct_vend_id=vend_id)"
                          " WHERE (contrct_id=:contrct_id);" );
    copyPopulate.bindValue(":contrct_id", _contrctid);
    copyPopulate.exec();
    if (copyPopulate.first())
    {
      _oldContract->setText(copyPopulate.value("contrct_number").toString());
      _oldEffective->setDate(copyPopulate.value("contrct_effective").toDate());
      _oldExpires->setDate(copyPopulate.value("contrct_expires").toDate());
      _vendid = copyPopulate.value("contrct_vend_id").toInt();
      _vendName->setText(copyPopulate.value("vend_name").toString());
      _effective->setDate(copyPopulate.value("neweffective").toDate());
    }
    else if (copyPopulate.lastError().type() != QSqlError::NoError)
    {
      systemError(this, copyPopulate.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _contract->clear();
    _oldEffective->clear();
    _oldEffective->clear();
    _vendName->clear();
  }
}


void copyContract::sCopy()
{
  XSqlQuery copyq;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_effective->isValid(), _effective,
                          tr( "You must select an Effective Date before you may copy this Contract." ) )
         << GuiErrorCheck(!_expires->isValid(), _expires,
                          tr( "You must select an Expiration Date before you may copy this Contract." ) )
         << GuiErrorCheck(_expires->date() < _effective->date(), _effective,
                          tr("The expiration date cannot be earlier than the effective date.") )
         << GuiErrorCheck(_contract->isNull(), _contract,
                          tr( "You must enter a Contract Number before you may save this Contract." ) )
     ;

  copyq.prepare( "SELECT count(*) AS numberOfOverlaps "
                 "FROM contrct "
                 "WHERE (contrct_vend_id = :contrct_vend_id)"
                 "  AND (contrct_id != :contrct_id)"
                 "  AND ( (contrct_effective BETWEEN :contrct_effective AND :contrct_expires OR"
                 "         contrct_expires BETWEEN :contrct_effective AND :contrct_expires)"
                 "   OR   (contrct_effective <= :contrct_effective AND"
                 "         contrct_expires   >= :contrct_expires) );" );
  copyq.bindValue(":contrct_id", _contrctid);
  copyq.bindValue(":contrct_vend_id", _vendid);
  copyq.bindValue(":contrct_effective", _effective->date());
  copyq.bindValue(":contrct_expires", _expires->date());
  copyq.exec();
  if (copyq.first())
  {
    if (copyq.value("numberOfOverlaps").toInt() > 0)
    {
      errors << GuiErrorCheck(true, _effective,
                              tr("The date range overlaps with another date range.\n"
                                 "Please check the values of these dates."));
    }
  }
  else if (copyq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, copyq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  copyq.prepare( "SELECT contrct_id "
                 "  FROM contrct "
                 " WHERE ((contrct_vend_id=:vend_id) "
                 "   AND (contrct_number=:contrct_number));" );
  copyq.bindValue(":vend_id", _vendid);
  copyq.bindValue(":contrct_number", _contract->text());
  copyq.exec();
  if(copyq.first())
  {
    errors << GuiErrorCheck(true, _contract,
                            tr("A Contract already exists for the Vendor,\n"
                               "Contract Number you have specified."));
  }
  else if (copyq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, copyq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Contract"), errors))
    return;

  copyq.prepare("SELECT copyContract(:contrct_id, :contrct_number, :contrct_effective, :contrct_expires) AS contrct_id;");
  copyq.bindValue(":contrct_id", _contrctid);
  copyq.bindValue(":contrct_number", _contract->text());
  copyq.bindValue(":contrct_effective", _effective->date());
  copyq.bindValue(":contrct_expires", _expires->date());

  copyq.exec();

  if (_captive)
  {
    if (copyq.first())
    {
      int contrctid = copyq.value("contrct_id").toInt();
      done(contrctid);
    }
    else if (copyq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, copyq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
