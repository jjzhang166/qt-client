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

#include "dspCapacityBufferStatusByWorkCenter.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>


/*
 *  Constructs a dspCapacityBufferStatusByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspCapacityBufferStatusByWorkCenter::dspCapacityBufferStatusByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_selectedWorkCenter, SIGNAL(toggled(bool)), _wrkcnt, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));

  statusBar()->hide();

  _wrkcnt->populate( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip) "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );

  _roughCut->addColumn(tr("Whs."),          _whsColumn,  Qt::AlignCenter );
  _roughCut->addColumn(tr("Work Center"),   -1,          Qt::AlignLeft   );
  _roughCut->addColumn(tr("Total Setup"),   _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Total Run"),     _timeColumn, Qt::AlignRight  );
  _roughCut->addColumn(tr("Daily Capacity"),_uomColumn,  Qt::AlignRight  );
  _roughCut->addColumn(tr("Days Load"),     _uomColumn,  Qt::AlignRight  );
  _roughCut->addColumn(tr("Buffer Status"), _uomColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspCapacityBufferStatusByWorkCenter::~dspCapacityBufferStatusByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspCapacityBufferStatusByWorkCenter::languageChange()
{
    retranslateUi(this);
}

void dspCapacityBufferStatusByWorkCenter::sPrint()
{
  ParameterList params;

  if (_selectedWorkCenter->isChecked())
    params.append("wrkcnt_id", _wrkcnt->id());

  _warehouse->appendValue(params);
  params.append("maxdaysload", _maxdaysload->value());

  orReport report("CapacityBufferStatusByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCapacityBufferStatusByWorkCenter::sQuery()
{
  _roughCut->clear();

  QString sql( "SELECT wrkcnt_id, warehous_code, wrkcnt_code,"
               "       formatTime(SUM("
               "         CASE WHEN wooper_sucomplete = False THEN NoNeg((wooper_sutime-wooper_suconsumed))"
               "         ELSE 0"
               "         END)) AS f_sutime,"
               "         formatTime(SUM(NoNeg(wooper_rntime-wooper_rnconsumed))) AS f_rntime,"
               "         formatTime(wrkcnt_dailycap*CASE WHEN(COALESCE(wrkcnt_efficfactor,0.0)=0.0) THEN 1.0 ELSE wrkcnt_efficfactor END) as f_dailycap,"
               "         formatTime((SUM(("
               "             (CASE WHEN wooper_sucomplete = False THEN NoNeg((wooper_sutime-wooper_suconsumed))"
               "                   ELSE 0"
               "              END)+NoNeg(wooper_rntime-wooper_rnconsumed)))/(CASE WHEN(COALESCE(wrkcnt_dailycap,0.0)=0.0) THEN 1.0 ELSE wrkcnt_dailycap END*CASE WHEN(COALESCE(wrkcnt_efficfactor,0.0)=0.0) THEN 1.0 ELSE wrkcnt_efficfactor END))) AS daysload,"
               "         CASE WHEN (:maxdaysload <= 0) THEN 'N/A'"
               "              ELSE CAST(calcbufferstatus(:maxdaysload,:maxdaysload-(SUM("
               "                           (CASE WHEN (wooper_sucomplete=false) THEN NoNeg(wooper_sutime-wooper_suconsumed)"
               "                                 ELSE 0"
               "                            END)"
               "                          +NoNeg(wooper_rntime-wooper_rnconsumed)"
               "                         ) / (CASE WHEN(COALESCE(wrkcnt_dailycap,0.0)=0.0) THEN 1.0 ELSE wrkcnt_dailycap END*CASE WHEN(COALESCE(wrkcnt_efficfactor,0.0)=0.0) THEN 1.0 ELSE wrkcnt_efficfactor END))) AS text)"
               "         END AS bufrsts,"
               "         CASE WHEN (:maxdaysload <= 0) THEN 0"
               "              ELSE calcbufferstatus(:maxdaysload,:maxdaysload-(SUM("
               "                       (CASE WHEN (wooper_sucomplete=false) THEN NoNeg(wooper_sutime-wooper_suconsumed)"
               "                             ELSE 0"
               "                        END)"
               "                      +NoNeg(wooper_rntime-wooper_rnconsumed)"
               "                     ) / (CASE WHEN(COALESCE(wrkcnt_dailycap,0.0)=0.0) THEN 1.0 ELSE wrkcnt_dailycap END*CASE WHEN(COALESCE(wrkcnt_efficfactor,0.0)=0.0) THEN 1.0 ELSE wrkcnt_efficfactor END)))"
               "         END AS bufrsts_sort"
               "  FROM wooper, wo, wrkcnt, warehous "
               " WHERE ( (wooper_wo_id=wo_id)"
               "   AND   (wooper_wrkcnt_id=wrkcnt_id)"
               "   AND   (wrkcnt_warehous_id=warehous_id)"
               "   AND   (wo_status<>'C')"
               "   AND   (wooper_rncomplete=False)"
               "   AND   (((CASE WHEN wooper_sucomplete = False THEN NoNeg((wooper_sutime-wooper_suconsumed))"
               "                 ELSE 0"
               "            END) + NoNeg(wooper_rntime-wooper_rnconsumed)) > 0)");

  if (_selectedWorkCenter->isChecked())
    sql += " AND (wrkcnt_id=:wrkcnt_id)";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         " GROUP BY wrkcnt_id, warehous_code, wrkcnt_code, wrkcnt_dailycap, wrkcnt_efficfactor"
         " ORDER BY bufrsts_sort DESC, warehous_code, wrkcnt_code";

  q.prepare(sql);
  q.bindValue(":maxdaysload", _maxdaysload->value());
  _warehouse->bindValue(q);
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  XTreeWidgetItem * last = 0;
  while(q.next())
  {
    last = new XTreeWidgetItem(_roughCut, last,
                                q.value("wrkcnt_id").toInt(), -1,
                                q.value("warehous_code"), q.value("wrkcnt_code"),
                                q.value("f_sutime"), q.value("f_rntime"), q.value("f_dailycap"),
                                q.value("daysload"), q.value("bufrsts"));
    if(q.value("bufrsts_sort").toInt() > 66)
      last->setTextColor(6, "red");
  }
}

