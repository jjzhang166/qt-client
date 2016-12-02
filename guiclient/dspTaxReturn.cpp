/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTaxReturn.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "currdisplay.h"
#include "mqlutil.h"

dspTaxReturn::dspTaxReturn(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  
  QString base = CurrDisplay::baseCurrAbbr();
  _taxsum->addColumn(tr("Source"), _itemColumn,  Qt::AlignLeft, true, "source" );
  _taxsum->addColumn(tr("Sales %1").arg(base),        _bigMoneyColumn,    Qt::AlignRight,  true,  "salesbase"  );
//  _taxsum->addColumn(tr("Zero Rated Sales %1").arg(base),        _bigMoneyColumn,    Qt::AlignRight,  true,  "salesbaseexempt"  );
  _taxsum->addColumn(tr("Output Tax %1").arg(base),    _bigMoneyColumn,    Qt::AlignRight,  true,  "salestaxbase"  );
  _taxsum->addColumn(tr("Purchases %1").arg(base),    _bigMoneyColumn,    Qt::AlignRight,  true,  "purchasebase"  );
//  _taxsum->addColumn(tr("Zero Rated Purchases %1").arg(base),_bigMoneyColumn,    Qt::AlignRight,  true,  "purchasebaseexempt"  );
  _taxsum->addColumn(tr("Input Tax %1").arg(base), _bigMoneyColumn,    Qt::AlignRight,  true,  "purchasetaxbase"  );
  _taxsum->addColumn(tr("Rev. Charge %1").arg(base), _bigMoneyColumn,    Qt::AlignRight,  true,  "reversechargebase"  );
  _taxsum->addColumn(tr("Rev. Charge Tax %1").arg(base), _bigMoneyColumn,    Qt::AlignRight,  true,  "reversechargetaxbase"  );
  _taxsum->addColumn(tr("Net Tax %1").arg(base),      _bigMoneyColumn,    Qt::AlignRight,  true,  "nettaxbase"  );

  _taxsum->sortItems(0,Qt::AscendingOrder);
}

dspTaxReturn::~dspTaxReturn()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspTaxReturn::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspTaxReturn::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;
  
  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());
    
  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspTaxReturn::setParams(ParameterList &params)
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_taxAuth->isValid(), _taxAuth,
                          tr("You must select a Tax Authority.") );
  errors << GuiErrorCheck(!_dates->startDate().isValid(), _dates,
                          tr("You must enter a valid Start Date to print this report.") );
  errors << GuiErrorCheck(!_dates->endDate().isValid(), _dates,
                          tr("You must enter a valid End Date to print this report.") );

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Execute Tax Return"), errors))
      return false;
  
  if (_metrics->boolean("CashBasedTax"))
    params.append("cashbasedtax", true);
    
  if (_distDate->isChecked())
    params.append("distDate");

  _dates->appendValue(params);
 
  params.append("taxauth_id",_taxAuth->id());
  params.append("taxauth", _taxAuth->currentText());
    
  params.append("sales_taxable",tr("Sales Taxable"));
  params.append("sales_nontaxable",tr("Sales Zero-Rated"));
  params.append("purchases_taxable",tr("Purchases Taxable "));
  params.append("purchases_nontaxable",tr("Purchases Zero-Rated"));
  params.append("reversecharges", tr("Reverse Charge"));

  return true;
}

void dspTaxReturn::sPrint()
{
  QString name;
  ParameterList params;
  if (! setParams(params))
    return;
    
  orReport report("TaxReturn", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspTaxReturn::sFillList()
{
  XSqlQuery dspFillList;
  ParameterList params;
  if (! setParams(params))
    return;

  _taxsum->clear();

  MetaSQLQuery mql = mqlLoad("taxReturn", "detail");
  dspFillList = mql.toQuery(params);
  dspFillList.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Tax Return detail"),
                           dspFillList, __FILE__, __LINE__))
  {
    return;
  } 
  else 
  {
    _taxsum->populate(dspFillList);
  }
}
