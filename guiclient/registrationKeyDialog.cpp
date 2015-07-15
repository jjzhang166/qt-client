/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "registrationKeyDialog.h"
#include "xtupleproductkey.h"

#include <QVariant>

registrationKeyDialog::registrationKeyDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
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

void registrationKeyDialog::sCheckKey()
{
  XTupleProductKey pkey(_key->text());
  if(pkey.valid() && pkey.expiration() > QDate::currentDate())
    _select->setEnabled(true);
}

void registrationKeyDialog::sSelect()
{
  XTupleProductKey pk(_key->text());
  if(pk.valid())
  {
    XSqlQuery keyq;
    keyq.prepare("UPDATE metric SET metric_value=:key WHERE metric_name='RegistrationKey';");
    keyq.bindValue(":key", _key->text());
    keyq.exec();
  }
  done(0);
}

void registrationKeyDialog::sClose()
{
  done(-1);
}

