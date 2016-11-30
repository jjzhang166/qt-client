/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createRecurringInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"

createRecurringInvoices::createRecurringInvoices(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
}

createRecurringInvoices::~createRecurringInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void createRecurringInvoices::languageChange()
{
  retranslateUi(this);
}

void createRecurringInvoices::sUpdate()
{
  XSqlQuery createUpdate;
  createUpdate.exec("SELECT createRecurringItems(NULL, 'I') AS result;");
  if (createUpdate.first())
  {
    int result = createUpdate.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Recurring Invoices"),
                             storedProcErrorLookup("createRecurringInvoices", result),
                             __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Recurring Invoices"),
                                createUpdate, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}
