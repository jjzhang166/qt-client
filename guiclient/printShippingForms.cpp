/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printShippingForms.h"

#include "guiErrorCheck.h"

printShippingForms::printShippingForms(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printMulticopyDocument("ShippingFormCopies",     "ShippingFormWatermark",
                             "ShippingFormShowPrices", "",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  setDoctype("P");
  setReportKey("shiphead_id");
  _distributeInventory = false;

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _printNew->setChecked(true);
    _printDirty->setChecked(true);
  }

  _docinfoQueryString =
             "SELECT shiphead_id               AS docid,"
             "       shiphead_number           AS docnumber,"
             "       (shiphead_sfstatus = 'P') AS printed,"
             "       NULL                      AS posted,"
             "       shipform_report_name      AS reportname,"
             "       shiphead_id, shiphead_shipchrg_id"
             "  FROM shiphead"
             "     JOIN cohead   ON (shiphead_order_id=cohead_id)"
             "                  AND (shiphead_order_type='SO')"
             "     JOIN shipform ON (shipform_id=COALESCE(shiphead_shipform_id,"
             "                                            cohead_shipform_id))"
             " WHERE NOT shiphead_shipped"
             " <? if exists('printFlags') ?>"
             "   AND (shiphead_sfstatus IN ("
             "    <? foreach('printFlags') ?>"
             "      <? if not isfirst('printFlags') ?> , <? endif ?>"
             "      <? value('printFlags') ?>"
             "    <? endforeach ?>"
             "    ))"
             " <? endif ?> "
             " UNION "
             "SELECT shiphead_id               AS docid,"
             "       shiphead_number           AS docnumber,"
             "       (shiphead_sfstatus = 'P') AS printed,"
             "       NULL                      AS posted,"
             "       shipform_report_name      AS reportname,"
             "       shiphead_id, shiphead_shipchrg_id"
             "  FROM shiphead"
             "     JOIN tohead   ON (shiphead_order_id=tohead_id)"
             "                  AND (shiphead_order_type='TO')"
             "     JOIN shipform ON (shipform_id=COALESCE(shiphead_shipform_id,"
             "                                            tohead_shipform_id))"
             " WHERE NOT shiphead_shipped"
             " <? if exists('printFlags') ?>"
             "   AND (shiphead_sfstatus IN ("
             "    <? foreach('printFlags') ?>"
             "      <? if not isfirst('printFlags') ?> , <? endif ?>"
             "      <? value('printFlags') ?>"
             "    <? endforeach ?>"
             "    ))"
             " <? endif ?> "
             " ORDER BY shiphead_id;" ;

  _markOnePrintedQry = "UPDATE shiphead"
                       "   SET shiphead_sfstatus='P'"
                       " WHERE (shiphead_id=<? value('docid') ?>);" ;
}

printShippingForms::~printShippingForms()
{
  // no need to delete child widgets, Qt does it all for us
}

void printShippingForms::languageChange()
{
  retranslateUi(this);
}

ParameterList printShippingForms::getParamsDocList()
{
  ParameterList params = printMulticopyDocument::getParamsDocList();

  QList<QVariant> printFlags;
  if (_printNew->isChecked())
    printFlags << "N";

  if (_printDirty->isChecked())
    printFlags << "D";

  if (printFlags.size() > 0)
    params.append("printFlags", printFlags);

  return params;
}

ParameterList printShippingForms::getParamsOneCopy(const int row, XSqlQuery *qry)
{
  ParameterList params = printMulticopyDocument::getParamsOneCopy(row, qry);

  params.append("shiphead_id", qry->value("shiphead_id"));
  params.append("shipchrg_id", qry->value("shiphead_shipchrg_id"));

  if (_metrics->boolean("MultiWhs"))
    params.append("MultiWhs");

  if (copies()->showCosts(row))
    params.append("showcosts");

  return params;
}

bool printShippingForms::isOkToPrint()
{
  QList<GuiErrorCheck> errors;
  errors<< GuiErrorCheck(!_printNew->isChecked() && !_printDirty->isChecked(),
                         _printNew,
                         tr("You must indicate if you wish to print Shipping "
                            "Forms for New and/or Changed Shipments."))
    ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Print Shipping Forms"),
                                  errors))
    return false;

  return true;
}
