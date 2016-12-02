/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "uiformchooser.h"
#include "ui_uiformchooser.h"

uiformchooser::uiformchooser(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl) {

  setupUi(this);
  _list->setColumnCount(0);
  _list->addColumn(tr("Name"),         -1, Qt::AlignLeft, true, "uiform_name");
  _list->addColumn(tr("Grade"),        -1, Qt::AlignLeft, true, "uiform_order");
  _list->addColumn(tr("Enabled"),      -1, Qt::AlignLeft, true, "uiform_enabled");
  _list->addColumn(tr("Description"), 125, Qt::AlignLeft, true, "uiform_notes");
  connect(_select, SIGNAL(clicked()),         this,    SLOT(sSelect()));
  connect(_cancel, SIGNAL(clicked()),         this,    SLOT(sCancel()));
  connect(_list,   SIGNAL(itemSelected(int)), _select, SLOT(animateClick(int)));
}

void uiformchooser::languageChange() {
  retranslateUi(this);
}

void uiformchooser::populate(const XSqlQuery pQuery) {
  _list->populate(pQuery);
}

void uiformchooser::sSelect() {
  if (_list->id() > 0) {
    this->done(_list->id());
  }
}

void uiformchooser::sCancel() {
  this->reject();
}
