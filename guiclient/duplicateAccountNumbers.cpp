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

#include "duplicateAccountNumbers.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a duplicateAccountNumbers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
duplicateAccountNumbers::duplicateAccountNumbers(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_duplicate, SIGNAL(clicked()), this, SLOT(sDuplicate()));

  sBuildList();
}


/*
 *  Destroys the object and frees any allocated resources
 */
duplicateAccountNumbers::~duplicateAccountNumbers()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void duplicateAccountNumbers::languageChange()
{
  retranslateUi(this);
}

void duplicateAccountNumbers::sDuplicate()
{
  if(!(_changeCompany->isChecked() || _changeProfit->isChecked() || _changeSub->isChecked()))
  {
    QMessageBox::warning( this, tr("No Segments Selected for Change"),
      tr("You have not selected any segments for changing. You must select at least one segment to be changed.") );
    return;
  }

  QString sql ("INSERT INTO accnt"
               "      (accnt_number, accnt_descrip,"
               "       accnt_comments, accnt_type, accnt_extref,"
               "       accnt_closedpost, accnt_forwardupdate,"
               "       accnt_subaccnttype_code, accnt_curr_id,"
               "       accnt_company, accnt_profit, accnt_sub) "
               "SELECT accnt_number, (accnt_descrip||' '||:descrip),"
               "       accnt_comments, accnt_type, accnt_extref,"
               "       accnt_closedpost, accnt_forwardupdate,"
               "       accnt_subaccnttype_code, accnt_curr_id,");
  if(_changeCompany->isChecked())
    sql +=     " :company,";
  else
    sql +=     " accnt_company,";

  if(_changeProfit->isChecked())
    sql +=     " :profit,";
  else
    sql +=     " accnt_profit,";

  if(_changeSub->isChecked())
    sql +=     " :sub";
  else
    sql +=     " accnt_sub";

  sql +=       "  FROM accnt"
               " WHERE (accnt_id=:accnt_id);";

  q.prepare(sql);

  QList<QTreeWidgetItem*> selected = _account->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    q.bindValue(":company",	_company->currentText());
    q.bindValue(":profit",	_profit->currentText());
    q.bindValue(":sub",		_sub->currentText());
    q.bindValue(":accnt_id",	((XTreeWidgetItem*)selected[i])->id());
    q.bindValue(":descrip",	_descrip->text());
    q.exec();
  }

  close();
}

void duplicateAccountNumbers::sFillList()
{
  QString sql("SELECT accnt_id, ");

  if (_metrics->value("GLCompanySize").toInt() > 0)
    sql += "accnt_company, ";

  if (_metrics->value("GLProfitSize").toInt() > 0)
    sql += "accnt_profit, ";

  sql += "accnt_number, ";

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
    sql += "accnt_sub, ";

  sql += " accnt_descrip,"
         " accnt_type "
         "FROM accnt "
         "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  _account->populate(sql);
}

void duplicateAccountNumbers::sBuildList()
{
  bool bComp = (_metrics->value("GLCompanySize").toInt() > 0);
  bool bProf = (_metrics->value("GLProfitSize").toInt() > 0);
  bool bSub  = (_metrics->value("GLSubaccountSize").toInt() > 0);

  _account->setColumnCount(0);

  if(bComp)
    _account->addColumn(tr("Company"), 50, Qt::AlignCenter);
  _changeCompany->setVisible(bComp);
  _company->setVisible(bComp);

  if(bProf)
    _account->addColumn(tr("Profit"), 50, Qt::AlignCenter);
  _changeProfit->setVisible(bProf);
  _profit->setVisible(bProf);

  _account->addColumn(tr("Account Number"), 100, Qt::AlignCenter);

  if(bSub)
    _account->addColumn(tr("Sub."), 50, Qt::AlignCenter);
  _changeSub->setVisible(bSub);
  _sub->setVisible(bSub);

  _account->addColumn(tr("Description"), -1, Qt::AlignLeft);
  _account->addColumn(tr("Type"), _ynColumn, Qt::AlignCenter);

  sFillList();
}
