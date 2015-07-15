/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournalGroupItem.h"

#include <QVariant>
#include <QMessageBox>

standardJournalGroupItem::standardJournalGroupItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_limited, SIGNAL(toggled(bool)), _toApply, SLOT(setEnabled(bool)));
  connect(_stdjrnl, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Expires"));

  _stdjrnl->populate( "SELECT stdjrnl_id, stdjrnl_name "
                      "FROM stdjrnl "
                      "ORDER BY stdjrnl_name;" );
}

standardJournalGroupItem::~standardJournalGroupItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void standardJournalGroupItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse standardJournalGroupItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
    _stdjrnlgrpid = param.toInt();

  param = pParams.value("stdjrnlgrpitem_id", &valid);
  if (valid)
  {
    _stdjrnlgrpitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _stdjrnl->setEnabled(false);
      _dates->setEnabled(false);
      _applyGroup->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void standardJournalGroupItem::sSave()
{
  XSqlQuery standardSave;
  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Effective Date"),
                           tr("You must enter an effective date for this Standard Journal Group Item.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("You must enter an expiration date for this Standard Journal Group Item.") );
    _dates->setFocus();
    return;
  }

  if (_dates->endDate() < _dates->startDate())
  {
    QMessageBox::critical( this, tr("Invalid Expiration Date"),
                           tr("The expiration date cannot be earlier than the effective date.") );
    _dates->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    standardSave.exec("SELECT NEXTVAL('stdjrnlgrpitem_stdjrnlgrpitem_id_seq') AS stdjrnlgrpitem_id;");
    if (standardSave.first())
      _stdjrnlgrpitemid = standardSave.value("stdjrnlgrpitem_id").toInt();
//  ToDo

    standardSave.prepare( "INSERT INTO stdjrnlgrpitem "
               "( stdjrnlgrpitem_id, stdjrnlgrpitem_stdjrnlgrp_id, stdjrnlgrpitem_stdjrnl_id,"
               "  stdjrnlgrpitem_toapply, stdjrnlgrpitem_applied,"
               "  stdjrnlgrpitem_effective, stdjrnlgrpitem_expires )"
               "VALUES "
               "( :stdjrnlgrpitem_id, :stdjrnlgrpitem_stdjrnlgrp_id, :stdjrnlgrpitem_stdjrnl_id,"
               "  :stdjrnlgrpitem_toapply, 0,"
               "  :stdjrnlgrpitem_effective, :stdjrnlgrpitem_expires );" );
  }
  else if (_mode == cEdit)
    standardSave.prepare( "UPDATE stdjrnlgrpitem "
               "SET stdjrnlgrpitem_toapply=:stdjrnlgrpitem_toapply,"
               "    stdjrnlgrpitem_effective=:stdjrnlgrpitem_effective, stdjrnlgrpitem_expires=:stdjrnlgrpitem_expires "
               "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );

  standardSave.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitemid);
  standardSave.bindValue(":stdjrnlgrpitem_stdjrnlgrp_id", _stdjrnlgrpid);
  standardSave.bindValue(":stdjrnlgrpitem_stdjrnl_id", _stdjrnl->id());
  standardSave.bindValue(":stdjrnlgrpitem_toapply", ((_limited->isChecked()) ? _toApply->value() : -1));
  standardSave.bindValue(":stdjrnlgrpitem_effective", _dates->startDate());
  standardSave.bindValue(":stdjrnlgrpitem_expires", _dates->endDate());
  standardSave.exec();

  done(_stdjrnlgrpitemid);
}

void standardJournalGroupItem::sCheck()
{
}

void standardJournalGroupItem::populate()
{
  XSqlQuery standardpopulate;
  standardpopulate.prepare( "SELECT stdjrnlgrpitem_stdjrnl_id, stdjrnlgrpitem_toapply,"
             "       stdjrnlgrpitem_effective, stdjrnlgrpitem_expires "
             "FROM stdjrnlgrpitem "
             "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );
  standardpopulate.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitemid);
  standardpopulate.exec();
  if (standardpopulate.first())
  {
    _stdjrnl->setId(standardpopulate.value("stdjrnlgrpitem_stdjrnl_id").toInt());
    _dates->setStartDate(standardpopulate.value("stdjrnlgrpitem_effective").toDate());
    _dates->setEndDate(standardpopulate.value("stdjrnlgrpitem_expires").toDate());

    if (standardpopulate.value("stdjrnlgrpitem_toapply").toInt() == -1)
      _unlimited->setChecked(true);
    else
    {
      _limited->setChecked(true);
      _toApply->setValue(standardpopulate.value("stdjrnlgrpitem_toapply").toInt());
    }
  }
}

