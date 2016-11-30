/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postStandardJournal.h"

#include <QVariant>
#include <QMessageBox>
#include "glSeries.h"
#include "errorReporter.h"

postStandardJournal::postStandardJournal(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  
  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  _captive = false;
  _doSubmit = false;

  _stdjrnl->setAllowNull(true);
  _stdjrnl->populate( "SELECT stdjrnl_id, stdjrnl_name "
                      "FROM stdjrnl "
                      "ORDER BY stdjrnl_name;" );
}

postStandardJournal::~postStandardJournal()
{
  // no need to delete child widgets, Qt does it all for us
}

void postStandardJournal::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postStandardJournal::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
  {
    _stdjrnl->setId(param.toInt());
    _stdjrnl->setEnabled(false);
  }

  return NoError;
}


void postStandardJournal::sPost()
{
  XSqlQuery postPost;
  if (!_distDate->isValid())
  {
    QMessageBox::critical( this, tr("Cannot Post Standard Journal"),
                           tr("You must enter a Distribution Date before you may post this Standard Journal.") );
    _distDate->setFocus();
    return;
  }

  postPost.prepare("SELECT postStandardJournal(:stdjrnl_id, :distDate, :reverse) AS result;");
  postPost.bindValue(":stdjrnl_id", _stdjrnl->id());
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
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Standard Journal"),
                       postPost, __FILE__, __LINE__);

  if (_captive)
    accept();
  {
    _stdjrnl->setNull();
    _close->setText(tr("&Close"));
    _stdjrnl->setFocus();
  }
}

void postStandardJournal::sSubmit()
{
  _doSubmit = true;
  sPost();
  _doSubmit = false;
}


