/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printChecksReviewEdit.h"

#include <QSqlError>
#include <QVariant>

#include "guiclient.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

printChecksReviewEdit::printChecksReviewEdit(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _id = -1;

  connect(_save,  SIGNAL(clicked()), this, SLOT(sSave()));
}

printChecksReviewEdit::~printChecksReviewEdit()
{
  // no need to delete child widgets, Qt does it all for us
}

void printChecksReviewEdit::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printChecksReviewEdit::set(const ParameterList &pParams)
{
  QVariant param;
  bool valid;
  param = pParams.value("checkhead_id", &valid);
  if (valid)
  {
    _id = param.toInt();
    XSqlQuery checkNumber;
    checkNumber.prepare( "SELECT checkhead_number "
                         "FROM checkhead "
                         "WHERE checkhead_id=:checkhead_id;");
    checkNumber.bindValue(":checkhead_id", _id);
    checkNumber.exec();
    if(checkNumber.first())
    {
      _number->setText(checkNumber.value("checkhead_number").toString());
    }
    else if (checkNumber.lastError().type() != QSqlError::NoError)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error fetching Check Number"),
                             checkNumber, __FILE__, __LINE__);
        return UndefinedError;
    }
  }

  return NoError;
}

void printChecksReviewEdit::sSave()
{
  XSqlQuery checkNumber;
  checkNumber.prepare( "UPDATE checkhead "
                       "SET checkhead_number=:checkhead_number "
                       "WHERE checkhead_id=:checkhead_id;");
  checkNumber.bindValue(":checkhead_number", _number->text());
  checkNumber.bindValue(":checkhead_id", _id);
  checkNumber.exec();
  if (checkNumber.lastError().type() != QSqlError::NoError)
    ErrorReporter::error(QtCriticalMsg, this, tr("Error setting Check Number"),
                         checkNumber, __FILE__, __LINE__);

  close();
}
