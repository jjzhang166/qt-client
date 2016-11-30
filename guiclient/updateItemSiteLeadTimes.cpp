/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateItemSiteLeadTimes.h"

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

updateItemSiteLeadTimes::updateItemSiteLeadTimes(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  _classCode->setType(ParameterGroup::ClassCode);
}

updateItemSiteLeadTimes::~updateItemSiteLeadTimes()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateItemSiteLeadTimes::languageChange()
{
  retranslateUi(this);
}

void updateItemSiteLeadTimes::sUpdate()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _classCode->appendValue(params);

  XSqlQuery updateUpdate;
  QProgressDialog progress;
  progress.setWindowModality(Qt::ApplicationModal);
  
  MetaSQLQuery mql = mqlLoad("updateItemsiteLeadTimes", "load");
  updateUpdate = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Loading Item Site Lead Times"),
                           updateUpdate, __FILE__, __LINE__))
  {
    return;
  }
  
  int count=0;
  progress.setMaximum(updateUpdate.size());
  XSqlQuery update;
  while (updateUpdate.next())
  {
    progress.setLabelText(tr("Site: %1\n"
                             "Item: %2 - %3")
                          .arg(updateUpdate.value("warehous_code").toString())
                          .arg(updateUpdate.value("item_number").toString())
                          .arg(updateUpdate.value("item_descrip1").toString()));
    
    ParameterList rparams = params;
    rparams.append("itemsite_id", updateUpdate.value("itemsite_id"));
    rparams.append("leadTimePad", _leadTimePad->value());
    MetaSQLQuery mql2 = mqlLoad("updateItemsiteLeadTimes", "update");
    update = mql2.toQuery(rparams);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Item Site Lead Times"),
                             update, __FILE__, __LINE__))
    {
      return;
    }
    
    if (progress.wasCanceled())
      break;
    
    count++;
    progress.setValue(count);
  }

  accept();
}
