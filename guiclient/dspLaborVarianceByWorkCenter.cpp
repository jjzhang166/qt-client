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

#include "dspLaborVarianceByWorkCenter.h"

#include <QVariant>
#include <QStatusBar>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspLaborVarianceByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspLaborVarianceByWorkCenter::dspLaborVarianceByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_woopervar, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _wrkcnt->populate( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip) "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _woopervar->addColumn(tr("Post Date"),      _dateColumn, Qt::AlignCenter );
  _woopervar->addColumn(tr("Parent Item"),    -1,          Qt::AlignLeft   );
  _woopervar->addColumn(tr("Seq #"),          _seqColumn,  Qt::AlignRight  );
  _woopervar->addColumn(tr("Proj Setup"),     _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Proj. Run"),      _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Act. Setup"),     _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Act. Run"),       _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Setup Var."),     _timeColumn, Qt::AlignRight  );
  _woopervar->addColumn(tr("Run Var."),       _timeColumn, Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspLaborVarianceByWorkCenter::~dspLaborVarianceByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspLaborVarianceByWorkCenter::languageChange()
{
  retranslateUi(this);
}

void dspLaborVarianceByWorkCenter::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("wrkcnt_id", _wrkcnt->id());

  orReport report("LaborVarianceByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspLaborVarianceByWorkCenter::sPopulateMenu(QMenu *)
{
}

void dspLaborVarianceByWorkCenter::sFillList()
{
  if (_dates->allValid())
  {
    q.prepare( "SELECT woopervar_id, formatDate(woopervar_posted),"
               "       item_number, woopervar_seqnumber,"
               "       formatTime(woopervar_stdsutime),"
               "       formatTime(woopervar_stdrntime),"
               "       formatTime(woopervar_sutime),"
               "       formatTime(woopervar_rntime),"
               "       formattime(woopervar_sutime - woopervar_stdsutime),"
               "       formattime(woopervar_rntime - woopervar_stdrntime) "
               "FROM woopervar, itemsite, item "
               "WHERE ( (woopervar_parent_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (woopervar_posted BETWEEN :startDate AND :endDate)"
               " AND (woopervar_wrkcnt_id=:wrkcnt_id) ) "
               "ORDER BY woopervar_posted DESC, woopervar_seqnumber;" );
    _dates->bindValue(q);
    q.bindValue(":wrkcnt_id", _wrkcnt->id());
    q.exec();
    _woopervar->populate(q);
  }
  else
    _woopervar->clear();
}

