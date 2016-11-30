/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "ledgerControl.h"
#include "xtreewidget.h"

ledgerControl::ledgerControl(QWidget *parent, const char *name, Qt::WindowFlags fl)
  : display(parent, name, fl)
{
  if (! name) setObjectName("ledgerControl");

  setWindowTitle(tr("Ledger Control"));
  setListLabel(tr("Ledger Control"));
  setMetaSQLOptions("ledgercontrol", "detail");

  list()->addColumn(tr("Ledger"),            -1, Qt::AlignLeft,  true, "ledger");
  list()->addColumn(tr("GL Amount"),         -1, Qt::AlignRight, true, "glamount");
  list()->addColumn(tr("Sub Ledger Amount"), -1, Qt::AlignRight, true, "subledger");
  list()->addColumn(tr("Difference"),        -1, Qt::AlignRight, true, "varamt");
}

bool ledgerControl::setParams(ParameterList &params)
{
  params.append("fromdate", "always today");

  return true;
}
