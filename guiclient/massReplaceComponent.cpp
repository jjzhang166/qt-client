/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "massReplaceComponent.h"

#include <QSqlError>
#include <QMessageBox>
#include <QVariant>

#include "errorReporter.h"
#include "storedProcErrorLookup.h"

massReplaceComponent::massReplaceComponent(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_replace, SIGNAL(clicked()), this, SLOT(sReplace()));

  _captive = FALSE;

  _original->setType(ItemLineEdit::cGeneralComponents);
  _replacement->setType(ItemLineEdit::cGeneralComponents);

  _effective->setNullString(tr("Immediate"));
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();
}

massReplaceComponent::~massReplaceComponent()
{
  // no need to delete child widgets, Qt does it all for us
}

void massReplaceComponent::languageChange()
{
  retranslateUi(this);
}

enum SetResponse massReplaceComponent::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _original->setId(param.toInt());

  return NoError;
}

void massReplaceComponent::sReplace()
{
  XSqlQuery massReplace;
  if (_original->isValid() && _replacement->isValid() && _effective->isValid())
  {
    if (_original->uom() != _replacement->uom())
    {
      QMessageBox::critical( this, tr("Mass Replace"),
        tr("The original and replacement Items must have the same Unit of Measure.") );
      return;
    }
    if (_metrics->boolean("RevControl"))
    {
      massReplace.prepare("SELECT 1"
                          "  FROM bomitem"
                          "  JOIN rev ON (bomitem_rev_id=rev_id)"
                          " WHERE ((rev_status='P') "
                          "    AND (bomitem_item_id=:item_id))"
                          " LIMIT 1;");
      massReplace.bindValue(":item_id", _original->id());
      massReplace.exec();
      if (massReplace.first())
        QMessageBox::information(this, tr("Mass Replace"),
                                 tr("<p>This process will only affect active "
                                    "revisions. Items on pending revisions "
                                    "must be replaced manually."));
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Checking Revisions"),
                                    massReplace, __FILE__, __LINE__))
        return;
    }
    massReplace.prepare("SELECT massReplaceBomitem(:replacement_item_id,"
              "                          :original_item_id, :effective_date,"
              "                          :ecn) AS result;");
    massReplace.bindValue(":replacement_item_id", _replacement->id());
    massReplace.bindValue(":original_item_id", _original->id());
    massReplace.bindValue(":ecn", _ecn->text());

    if (!_effective->isNull())
      massReplace.bindValue(":effective_date", _effective->date());

    massReplace.exec();
    if (massReplace.first())
    {
      int result = massReplace.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("massReplaceBomitem", result),
                    __FILE__, __LINE__);
        return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Replacing BOM Items"),
                                  massReplace, __FILE__, __LINE__))
      return;

    _original->setId(-1);
    _replacement->setId(-1);
    _ecn->clear();
    _close->setText(tr("&Close"));
    _original->setFocus();
  }
}
