/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printWoPickList.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>

printWoPickList::printWoPickList(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_wo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = false;

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);

  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
}

printWoPickList::~printWoPickList()
{
  // no need to delete child widgets, Qt does it all for us
}

void printWoPickList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printWoPickList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = true;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void printWoPickList::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);

  ParameterList params;
  params.append("wo_id", _wo->id());

  orReport report("PickList", params);
  bool userCanceled = false;
  if (orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("Could not initialize printing system for multiple reports."));
    return;
  }
  if (report.isValid())
  {
    for (int counter = 0; counter < _copies->value(); counter++)
      if (!report.print(&printer, (counter == 0)))
      {
	report.reportError(this);
        break;
      }
  }
  orReport::endMultiPrint(&printer);
}

