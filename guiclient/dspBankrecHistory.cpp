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

#include "dspBankrecHistory.h"

#include <QVariant>
#include <parameter.h>
#include <QMessageBox>
#include <QStatusBar>
#include <openreports.h>
#include "guiclient.h"

/*
 *  Constructs a dspBankrecHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBankrecHistory::dspBankrecHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankaccntChanged()));
  connect(_bankrec, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _details->addColumn(tr("Date"), _dateColumn, Qt::AlignCenter );
  _details->addColumn(tr("Doc Number/Notes"), -1, Qt::AlignLeft );
  _details->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight );
  
  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");
  
  sBankaccntChanged();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBankrecHistory::~dspBankrecHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBankrecHistory::languageChange()
{
  retranslateUi(this);
}

void dspBankrecHistory::sPrint()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("bankrec_id", _bankrec->id());

  orReport report("BankrecHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBankrecHistory::sBankaccntChanged()
{
  q.prepare( "SELECT bankrec_id, (formatDate(bankrec_opendate) || '-' || formatDate(bankrec_enddate)) "
             "FROM bankrec "
             "WHERE (bankrec_bankaccnt_id=:bankaccnt_id) "
             "ORDER BY bankrec_opendate, bankrec_enddate; ");
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  _bankrec->populate(q);

  sFillList();
}

void dspBankrecHistory::sFillList()
{
  _details->clear();

  q.prepare( "SELECT bankrec_username, formatDate(bankrec_created) AS f_created,"
             "       formatDate(bankrec_opendate) AS f_opendate,"
             "       formatDate(bankrec_enddate) AS f_enddate,"
             "       formatMoney(bankrec_openbal) AS f_openbal,"
             "       formatMoney(bankrec_endbal) AS f_endbal "
             "FROM bankrec "
             "WHERE (bankrec_id=:bankrecid);" );
  q.bindValue(":bankrecid", _bankrec->id());
  q.exec();
  if(q.first())
  {
    _poster->setText(q.value("bankrec_username").toString());
    _postdate->setText(q.value("f_created").toString());

    QString opendate = q.value("f_opendate").toString();
    QString openbal  = q.value("f_openbal").toString();
    QString enddate  = q.value("f_enddate").toString();
    QString endbal   = q.value("f_endbal").toString();

    q.prepare( "SELECT gltrans_id, formatDate(gltrans_date) AS f_date,"
               "       gltrans_docnumber,"
               "       formatMoney(gltrans_amount * -1) AS f_amount "
               "FROM gltrans, bankrecitem "
               "WHERE ((bankrecitem_bankrec_id=:bankrecid)"
               "  AND (bankrecitem_source='GL')"
               "  AND (bankrecitem_source_id=gltrans_id) ) "
               "ORDER BY gltrans_date, gltrans_id;" );
    q.bindValue(":bankrecid", _bankrec->id());
    q.exec();

    XTreeWidgetItem *last = 0;
    last = new XTreeWidgetItem( _details, last, -1, QVariant(opendate), tr("Opening Balance"), openbal);

    while (q.next())
      last = new XTreeWidgetItem( _details, last, q.value("gltrans_id").toInt(),
				 q.value("f_date"), q.value("gltrans_docnumber"), q.value("f_amount") );

    last = new XTreeWidgetItem( _details, last, -1, QVariant(enddate), tr("Ending Balance"), endbal);
  }
}
