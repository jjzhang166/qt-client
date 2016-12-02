/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyBOM.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

copyBOM::copyBOM(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_source, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_target, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = false;

  _source->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                   ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                   ItemLineEdit::cPlanning);
  _target->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                   ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                   ItemLineEdit::cPlanning);
}

copyBOM::~copyBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyBOM::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(false);
  }

  return NoError;
}

void copyBOM::sCopy()
{
  XSqlQuery copyCopy;
  copyCopy.prepare("SELECT bomitem_id "
                   "FROM bomitem(:item_id) "
                   "WHERE ( (bomitem_booitem_seq_id != -1) "
                   " AND (bomitem_booitem_seq_id IS NOT NULL) ) "
                   "LIMIT 1;" );
  copyCopy.bindValue(":item_id", _source->id());
  copyCopy.exec();
  if (copyCopy.first())
    QMessageBox::information( this, tr("Dependent BOO Data"),
      tr("One or more of the components for this Bill of Materials make reference to a\n"
         "Bill of Operations. These references cannot be copied and must be added manually.") );
      
  copyCopy.prepare("SELECT copyBOM(:sourceid, :targetid) AS result;");
  copyCopy.bindValue(":sourceid", _source->id());
  copyCopy.bindValue(":targetid", _target->id());
  copyCopy.exec();
  if (copyCopy.first())
  {
    int result = copyCopy.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying BOM"),
                             storedProcErrorLookup("copyBOM", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Copying BOM"),
                                copyCopy, __FILE__, __LINE__))
  {
    return;
  }
  omfgThis->sBOMsUpdated(_target->id(), true);
  if (_captive)
    close();
  else
  {
    _source->setId(-1);
    _target->setId(-1);
    _source->setFocus();
  }
}

void copyBOM::sHandleButtons()
{
  _copy->setEnabled(_source->isValid() && _target->isValid());
}
