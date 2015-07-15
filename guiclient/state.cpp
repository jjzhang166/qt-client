/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its state.
 */

#include "state.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

state::state(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));

  _mode = cNew;
  setWindowModified(false);
}

state::~state()
{
  // no need to delete child widgets, Qt does it all for us
}

void state::languageChange()
{
  retranslateUi(this);
}

enum SetResponse state::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("country_id", &valid);
  if (valid)
  {
    _country->setId(param.toInt());
    if (_country->isValid())
      _country->setEnabled(false);
  }

  param = pParams.value("state_id", &valid);
  if (valid)
  {
    _stateid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      enableWindowModifiedSetting();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      enableWindowModifiedSetting();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _country->setEnabled(false);
      _abbr->setEnabled(false);
      _name->setEnabled(false);

      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}


void state::sSave()
{
  XSqlQuery stateSave;
  _abbr->setText(_abbr->text().trimmed());
  _name->setText(_name->text().trimmed());

  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { (! _country->isValid()), tr("<p>You must select a country."), _country },
    { _name->text().isEmpty(),
            tr("<p>You must enter a name for the state/province."), _name    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save State/Province"),
                         error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  XSqlQuery stateq;
  stateq.prepare("SELECT state_id "
                 "FROM state "
                 "WHERE ((state_country_id=:country_id)"
                 "   AND ((UPPER(state_abbr)=UPPER(:state_abbr) AND :state_abbr != '')"
                 "        OR (UPPER(state_name)=UPPER(:state_name) AND :state_name != ''))"
                 "   AND (state_id != :state_id));");
  stateq.bindValue(":country_id", _country->id());
  stateq.bindValue(":state_abbr", _abbr->text());
  stateq.bindValue(":state_name", _name->text());
  stateq.bindValue(":state_id",   _stateid);
  stateq.exec();
  if (stateq.first())
  {
    QMessageBox::warning(this, tr("Cannot Save State/Province"),
                         tr("There is already another state/province for %1 "
                            "with the same name or abbreviation.")
                           .arg(_country->currentText()));
    _abbr->setFocus();
    return;

  }
  else if (stateq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, stateq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
    stateSave.prepare("INSERT INTO state ("
              "  state_id, state_abbr, state_name, state_country_id"
              ") VALUES ("
              "  DEFAULT, :state_abbr, :state_name, :country_id"
              ") RETURNING state_id;");
  else if (_mode == cEdit)
  {
    stateSave.prepare("UPDATE state "
              "SET state_abbr=:state_abbr, state_name=:state_name,"
              "    state_country_id=:country_id "
              "WHERE (state_id=:state_id) RETURNING state_id;" );
    stateSave.bindValue(":state_id", _stateid);
  }

  stateSave.bindValue(":state_abbr", _abbr->text().trimmed());
  stateSave.bindValue(":state_name", _name->text().trimmed());
  stateSave.bindValue(":country_id", _country->id());
  stateSave.exec();
  if (stateSave.first())
    _stateid = stateSave.value("state_id").toInt();
  else if (stateSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, stateSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_stateid);
}

void state::reject()
{
  if (isWindowModified() && (cNew == _mode || cEdit == _mode) &&
      QMessageBox::question(this, tr("Save?"),
                            tr("<p>Do you want to try saving this State/Province?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes) == QMessageBox::Yes)
    sSave();

  XDialog::reject();
}

void state::populate()
{
  XSqlQuery statepopulate;
  statepopulate.prepare("SELECT state_abbr, state_name, state_country_id"
            "  FROM state"
            " WHERE (state_id=:state_id);");
  statepopulate.bindValue(":state_id", _stateid);
  statepopulate.exec();
  if (statepopulate.first())
  {
    _abbr->setText(statepopulate.value("state_abbr").toString());
    _name->setText(statepopulate.value("state_name").toString());
    _country->setId(statepopulate.value("state_country_id").toInt());
  }
  else if (statepopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, statepopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void state::enableWindowModifiedSetting()
{
  connect(_abbr,SIGNAL(textEdited(const QString&)), this, SLOT(sDataChanged()));
  connect(_country,             SIGNAL(newID(int)), this, SLOT(sDataChanged()));
  connect(_name,SIGNAL(textEdited(const QString&)), this, SLOT(sDataChanged()));
}

void state::sDataChanged()
{
  setWindowModified(true);
}
