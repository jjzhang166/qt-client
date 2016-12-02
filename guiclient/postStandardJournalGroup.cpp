/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postStandardJournalGroup.h"

#include <QVariant>
#include <QMessageBox>
#include "glSeries.h"
#include "errorReporter.h"

postStandardJournalGroup::postStandardJournalGroup(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _captive = false;
  _doSubmit = false;

  _stdjrnlgrp->setAllowNull(true);
  _stdjrnlgrp->populate( "SELECT stdjrnlgrp_id, stdjrnlgrp_name "
                         "FROM stdjrnlgrp "
                         "ORDER BY stdjrnlgrp_name;" );
}

postStandardJournalGroup::~postStandardJournalGroup()
{
  // no need to delete child widgets, Qt does it all for us
}

void postStandardJournalGroup::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postStandardJournalGroup::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
  {
    _stdjrnlgrp->setId(param.toInt());
    _stdjrnlgrp->setEnabled(false);
  }

  return NoError;
}


void postStandardJournalGroup::sPost()
{
  XSqlQuery postPost;
  if (!_distDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Post Standard Journal Group"),
                           tr("You must enter a Distribution Date before you may post this Standard Journal Group.") );
    _distDate->setFocus();
    return;
  }

  postPost.prepare("SELECT postStandardJournalGroup(:stdjrnlgrp_id, :distDate, :reverse) AS result;");
  postPost.bindValue(":stdjrnlgrp_id", _stdjrnlgrp->id());
  postPost.bindValue(":distDate", _distDate->date());
  postPost.bindValue(":reverse", QVariant(_reverse->isChecked()));
  postPost.exec();
  if (postPost.first())
  {
    ParameterList params;
    params.append("mode", "postStandardJournal");
    params.append("glSequence", postPost.value("result"));
    if(_doSubmit)
      params.append("submit");

    glSeries newdlg(this, "", true);
    newdlg.set(params);
    newdlg.exec();
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Standard Journal Group"),
                       postPost, __FILE__, __LINE__);

  if (_captive)
    accept();
  {
    _stdjrnlgrp->setNull();
    _close->setText(tr("&Close"));
    _stdjrnlgrp->setFocus();
  }
}

void postStandardJournalGroup::sSubmit()
{
  _doSubmit = true;
  sPost();
  _doSubmit = false;
}
