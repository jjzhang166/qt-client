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

#include "dspGLSeries.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "reverseGLSeries.h"

/*
 *  Constructs a dspGLSeries as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspGLSeries::dspGLSeries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedSource, SIGNAL(toggled(bool)), _source, SLOT(setEnabled(bool)));

  _gltrans->setRootIsDecorated(TRUE);
  _gltrans->addColumn(tr("Date"),      _dateColumn,     Qt::AlignCenter );
  _gltrans->addColumn(tr("Journal #"), _orderColumn,    Qt::AlignRight  );
  _gltrans->addColumn(tr("Source"),    _orderColumn,    Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. Type"), _itemColumn,     Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. Num."), _orderColumn,    Qt::AlignCenter );
  _gltrans->addColumn(tr("Notes/Account"),   -1,              Qt::AlignLeft   );
  _gltrans->addColumn(tr("Debit"),     _bigMoneyColumn, Qt::AlignRight  );
  _gltrans->addColumn(tr("Credit"),    _bigMoneyColumn, Qt::AlignRight  );
  _gltrans->addColumn(tr("Posted"),    _ynColumn,       Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspGLSeries::~dspGLSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspGLSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspGLSeries::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if(valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if(valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("journalnumber", &valid);
  if(valid)
  {
    _jrnlGroup->setChecked(true);
    _startJrnlnum->setText(param.toString());
    _endJrnlnum->setText(param.toString());
  }

  sFillList();

  return NoError;
}

void dspGLSeries::sPopulateMenu(QMenu * pMenu)
{
  int menuItem;

  bool reversible = false;
  XTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 != item)
  {
    if(item->altId() != -1)
      item = (XTreeWidgetItem*)item->parent();
    if(0 != item)
    {
      if(item->text(3) == "ST" || item->text(3) == "JE")
        reversible = true;
    }
  }

  menuItem = pMenu->insertItem(tr("Reverse Journal..."), this, SLOT(sReverse()), 0);
  if (!reversible || !_privileges->check("PostStandardJournals"))
    pMenu->setItemEnabled(menuItem, false);
}

void dspGLSeries::sPrint()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must first specify a valid date range.") );
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if(_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if(_jrnlGroup->isChecked())
  {
    params.append("startJrnlnum", _startJrnlnum->text().toInt());
    params.append("endJrnlnum", _endJrnlnum->text().toInt());
  }

  orReport report("GLSeries", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspGLSeries::sFillList()
{
  _gltrans->clear();

  QString sql( "SELECT gltrans_id, gltrans_sequence, formatDate(gltrans_date) AS transdate, gltrans_source, gltrans_journalnumber,"
               "       gltrans_doctype, gltrans_docnumber, (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
               "       CASE WHEN (gltrans_amount < 0) THEN formatMoney(gltrans_amount * -1)"
               "            ELSE ''"
               "       END AS f_debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN formatMoney(gltrans_amount)"
               "            ELSE ''"
               "       END AS f_credit,"
               "       formatBoolYN(gltrans_posted) AS f_posted,"
               "       firstLine(gltrans_notes) AS f_notes "
               "FROM gltrans, accnt "
               "WHERE ( (gltrans_accnt_id=accnt_id)"
               " AND (gltrans_date BETWEEN :startDate AND :endDate)" );

  if (_selectedSource->isChecked())
    sql += " AND (gltrans_source=:source)";

  if (_jrnlGroup->isChecked())
    sql += " AND (gltrans_journalnumber BETWEEN :startJrnlnum AND :endJrnlnum)";

  sql += " ) "
         "ORDER BY gltrans_date, gltrans_sequence, gltrans_amount;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":source", _source->currentText());
  if(_jrnlGroup->isChecked())
  {
    q.bindValue(":startJrnlnum", _startJrnlnum->text().toInt());
    q.bindValue(":endJrnlnum", _endJrnlnum->text().toInt());
  }
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *parent  = NULL;
    int           glSeries = -1;

    do
    {
      if (glSeries != q.value("gltrans_sequence").toInt())
      {
        glSeries = q.value("gltrans_sequence").toInt();
	parent = new XTreeWidgetItem( _gltrans, parent, q.value("gltrans_sequence").toInt(), -1,
                                    q.value("transdate"), q.value("gltrans_journalnumber"), q.value("gltrans_source"),
                                    q.value("gltrans_doctype"), q.value("gltrans_docnumber"),
                                    q.value("f_notes"), "", "", q.value("f_posted") );
      }

      new XTreeWidgetItem( parent, q.value("gltrans_sequence").toInt(), q.value("gltrans_id").toInt(),
                         "", "", "", "", "",
                         q.value("account"), q.value("f_debit"),
                         q.value("f_credit") );
    }
    while (q.next());
  }
}

void dspGLSeries::sReverse()
{
  ParameterList params;
  params.append("glseries", _gltrans->id());

  reverseGLSeries newdlg(this);
  if(newdlg.set(params) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

