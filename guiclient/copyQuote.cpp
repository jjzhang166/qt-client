/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyQuote.h"

#include <QVariant>
#include "errorReporter.h"
#include "guiErrorCheck.h"

copyQuote::copyQuote(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // connect(_quoteid, SIGNAL(newId(int,QString)), this, SLOT(sPopulateQuInfo(int)));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sCopy()));

  _quoteid = -1;

  _cust->setType(CLineEdit::ActiveCustomersAndProspects);

}

copyQuote::~copyQuote()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyQuote::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyQuote::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    setId(param.toInt());
  }

  return NoError;
}

void copyQuote::setId(int p)
{
  if (_quoteid==p)
    return;

  _quoteid=p;
  emit newId(_quoteid);
}

void copyQuote::sCopy()
{
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(_cust->id() == -1, _cust,
                    tr("You must select a Customer/Prospect to copy the Quote to. "));

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Copy Quote"), errors))
    return;

  XSqlQuery copyCopy;
  copyCopy.prepare("SELECT copyQuoteToCustomer(:quhead_id, :customer, :scheddate) AS quhead_id;");
  copyCopy.bindValue(":quhead_id", _quoteid);
  copyCopy.bindValue(":customer", _cust->id());
  copyCopy.bindValue(":scheddate", _quoteDate->date());

  copyCopy.exec();

  if (copyCopy.first())
  {
    int quheadid = copyCopy.value("quhead_id").toInt();
    done(quheadid);
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Could not Copy Quote"),
                              copyCopy, __FILE__, __LINE__))
    return;
}
