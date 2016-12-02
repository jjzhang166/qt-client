/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "registrationKeyDialog.h"
#include "xtupleproductkey.h"
#include <QCloseEvent>
#include <QVariant>

registrationKeyDialog::registrationKeyDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  Q_UNUSED(name);
  setupUi(this);

  // signals and slots connections
  connect(_key, SIGNAL(textChanged(const QString&)), this, SLOT(sCheckKey()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
}

registrationKeyDialog::~registrationKeyDialog()
{
  // no need to delete child widgets, Qt does it all for us
}

void registrationKeyDialog::languageChange()
{
  retranslateUi(this);
}

enum SetResponse registrationKeyDialog::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("invalid", &valid);
  if (valid)
    setWindowTitle(tr("Invalid Registration Key"));

  return NoError;
}

void registrationKeyDialog::sCheckKey()
{
  XTupleProductKey pkey(_key->text());
  if(pkey.valid() && pkey.expiration() > QDate::currentDate())
    _select->setEnabled(true);
  else
    _select->setEnabled(false);
}

void registrationKeyDialog::sSelect()
{
  XTupleProductKey pk(_key->text());
  if(pk.valid())
  {
    XSqlQuery keyq;
    keyq.prepare("UPDATE metric SET metric_value=UPPER(:key) WHERE metric_name='RegistrationKey';");
    keyq.bindValue(":key", _key->text());
    keyq.exec();
  }
  done(0);
}

void registrationKeyDialog::sClose()
{
  done(-1);
}

void registrationKeyDialog::closeEvent(QCloseEvent *pEvent)
{
  Q_UNUSED(pEvent);
  done(-1);
}
