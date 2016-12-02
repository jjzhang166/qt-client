/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "duplicateAccountNumbers.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "errorReporter.h"

/*
 *  Constructs a duplicateAccountNumbers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
duplicateAccountNumbers::duplicateAccountNumbers(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
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
  XSqlQuery duplicateDuplicate;
  if(!(_changeCompany->isChecked() || _changeProfit->isChecked() || _changeSub->isChecked()))
  {
    QMessageBox::warning( this, tr("No Segments Selected for Change"),
      tr("You have not selected any segments for changing. You must select at least one segment to be changed.") );
    return;
  }

  QString sql ("INSERT INTO accnt"
               "      (accnt_number, accnt_descrip,"
               "       accnt_comments, accnt_type, accnt_extref,"
               "       accnt_forwardupdate,"
               "       accnt_subaccnttype_code, accnt_curr_id,"
               "       accnt_company, accnt_profit, accnt_sub) "
               "SELECT accnt_number, (accnt_descrip||' '||:descrip),"
               "       accnt_comments, accnt_type, accnt_extref,"
               "       accnt_forwardupdate,"
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

  duplicateDuplicate.exec("BEGIN;");
  duplicateDuplicate.prepare(sql);

  QList<XTreeWidgetItem*> selected = _account->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    duplicateDuplicate.bindValue(":company",	_company->currentText());
    duplicateDuplicate.bindValue(":profit",	_profit->currentText());
    duplicateDuplicate.bindValue(":sub",		_sub->currentText());
    duplicateDuplicate.bindValue(":accnt_id",	((XTreeWidgetItem*)selected[i])->id());
    duplicateDuplicate.bindValue(":descrip",	_descrip->text());
    duplicateDuplicate.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Duplicating Account Numbers"),
                                  duplicateDuplicate, __FILE__, __LINE__))
    {
      duplicateDuplicate.exec("ROLLBACK;");
      return;
    }
  }

  duplicateDuplicate.exec("COMMIT;");
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
         "FROM ONLY accnt "
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
    _account->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");
  _changeCompany->setVisible(bComp);
  _company->setVisible(bComp);

  if(bProf)
    _account->addColumn(tr("Profit"), 50, Qt::AlignCenter, true, "accnt_profit");
  _changeProfit->setVisible(bProf);
  _profit->setVisible(bProf);

  _account->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if(bSub)
    _account->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  _changeSub->setVisible(bSub);
  _sub->setVisible(bSub);

  _account->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");
  _account->addColumn(tr("Type"), _ynColumn, Qt::AlignCenter, true, "accnt_type");

  sFillList();
}
