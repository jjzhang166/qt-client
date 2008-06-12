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

#include "dspStandardJournalHistory.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "reverseGLSeries.h"

/*
 *  Constructs a dspStandardJournalHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspStandardJournalHistory::dspStandardJournalHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _gltrans->setRootIsDecorated(TRUE);
  _gltrans->addColumn(tr("Date"),      _dateColumn,     Qt::AlignCenter );
  _gltrans->addColumn(tr("Journal #"), _itemColumn,     Qt::AlignCenter );
  _gltrans->addColumn(tr("Journal Name"), _orderColumn,    Qt::AlignCenter );
  _gltrans->addColumn(tr("Account"),   -1,              Qt::AlignLeft   );
  _gltrans->addColumn(tr("Debit"),     _bigMoneyColumn, Qt::AlignRight  );
  _gltrans->addColumn(tr("Credit"),    _bigMoneyColumn, Qt::AlignRight  );
  _gltrans->addColumn(tr("Posted"),    _ynColumn,       Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspStandardJournalHistory::~dspStandardJournalHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspStandardJournalHistory::languageChange()
{
  retranslateUi(this);
}

void dspStandardJournalHistory::sPopulateMenu(QMenu * pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Reverse Journal..."), this, SLOT(sReverse()), 0);
  if (!_privileges->check("PostStandardJournals"))
    pMenu->setItemEnabled(menuItem, false);
}

void dspStandardJournalHistory::sPrint()
{
  ParameterList params;
  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());

  orReport report("StandardJournalHistory", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspStandardJournalHistory::sFillList()
{
  _gltrans->clear();

  QString sql( "SELECT gltrans_id, gltrans_sequence, formatDate(gltrans_date) AS transdate, gltrans_journalnumber,"
               "       gltrans_docnumber, (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               "       CASE WHEN (gltrans_amount < 0) THEN formatMoney(gltrans_amount * -1)"
               "            ELSE ''"
               "       END AS f_debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN formatMoney(gltrans_amount)"
               "            ELSE ''"
               "       END AS f_credit,"
               "       formatBoolYN(gltrans_posted) AS f_posted "
               "FROM gltrans, accnt "
               "WHERE ( (gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate)"
               " AND (gltrans_doctype='ST')" );

  sql += " ) "
         "ORDER BY gltrans_date, gltrans_sequence, gltrans_docnumber, gltrans_amount;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *parent  = NULL;
    XTreeWidgetItem *child   = 0;
    int           glSeries = -1;
    QString       docnum;

    do
    {
      if (glSeries != q.value("gltrans_sequence").toInt())
      {
        glSeries = q.value("gltrans_sequence").toInt();
	parent = new XTreeWidgetItem( _gltrans, parent, q.value("gltrans_sequence").toInt(), -1,
                                    q.value("transdate"), q.value("gltrans_journalnumber"),
                                    "", "", "", "", q.value("f_posted") );
        child = 0;
      }

      if(0==child || docnum!=q.value("gltrans_docnumber").toString())
      {
        QString doc = q.value("gltrans_docnumber").toString();
        if(doc.isEmpty())
          doc = tr("Unnamed");
        docnum = q.value("gltrans_docnumber").toString();
        child = new XTreeWidgetItem( parent, q.value("gltrans_sequence").toInt(), -1,
                                   "", "", doc,
                                   "", "", "");
      }

      new XTreeWidgetItem( child, q.value("gltrans_sequence").toInt(), q.value("gltrans_id").toInt(),
                         "", "", "",
                         q.value("account"), q.value("f_debit"),
                         q.value("f_credit") );
    }
    while (q.next());
  }
}

void dspStandardJournalHistory::sReverse()
{
  ParameterList params;
  params.append("glseries", _gltrans->id());  

  reverseGLSeries newdlg(this);
  if(newdlg.set(params) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

