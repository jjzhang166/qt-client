/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspSummarizedTaxableSales.h"

#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>

#include "currdisplay.h"
#include "mqlutil.h"

dspSummarizedTaxableSales::dspSummarizedTaxableSales(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _dates->setStartCaption(tr("Start Ship Date:"));
  _dates->setEndCaption(tr("End Ship Date:"));

  _taxCode->setType(XComboBox::TaxTypes);

  QString base = CurrDisplay::baseCurrAbbr();
  _invchead->addColumn(tr("Tax Code"),      _itemColumn,  Qt::AlignLeft   );
  _invchead->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _invchead->addColumn(tr("Sales %1").arg(base),  _itemColumn,  Qt::AlignRight);
  _invchead->addColumn(tr("Freight %1").arg(base),_moneyColumn, Qt::AlignRight);
  _invchead->addColumn(tr("Freight Taxed"),       _itemColumn,  Qt::AlignCenter);
  _invchead->addColumn(tr("Tax %1").arg(base),    _moneyColumn, Qt::AlignRight);
  _invchead->addColumn(tr("Tax"),                 _moneyColumn, Qt::AlignRight);
  _invchead->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignRight);
}

dspSummarizedTaxableSales::~dspSummarizedTaxableSales()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSummarizedTaxableSales::languageChange()
{
  retranslateUi(this);
}

bool dspSummarizedTaxableSales::setParams(ParameterList &params)
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Start Date"),
                          tr("You must enter a valid Start Date to print this report.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Valid End Date"),
                          tr("You must enter a valid End Date to print this report.") );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);

  if (_selectedTaxCode->isChecked())
    params.append("tax_id", _taxCode->id());

  return true;
}

void dspSummarizedTaxableSales::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("SummarizedTaxableSales", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedTaxableSales::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  _invchead->clear();

  MetaSQLQuery mql = mqlLoad(":/so/displays/SummarizedTaxableSales/FillListDetail.mql");
  q = mql.toQuery(params);
  q.exec();
  _invchead->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
