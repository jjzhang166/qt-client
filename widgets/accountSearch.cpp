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

#include "accountSearch.h"

#include <QVariant>
#include <QMessageBox>

#include <parameter.h>

#include "glcluster.h"
#include "xsqlquery.h"

/*
 *  Constructs a accountSearch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
accountSearch::accountSearch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  _type->addItem(tr("ALL"), QVariant(""));
  _type->addItem(tr("Asset"), QVariant("A"));
  _type->addItem(tr("Liability"), QVariant("L"));
  _type->addItem(tr("Expense"), QVariant("E"));
  _type->addItem(tr("Revenue"), QVariant("R"));
  _type->addItem(tr("Equity"), QVariant("Q"));

  // signals and slots connections
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _accnt, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _clear, SIGNAL( clicked() ), this, SLOT( sClear() ) );
  connect(_type, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillList()));
  connect(_descrip, SIGNAL(textChanged(const QString &)), this, SLOT(sFillList()));

  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _accnt->addColumn(tr("Company"), 50, Qt::AlignCenter);

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _accnt->addColumn(tr("Profit"), 50, Qt::AlignCenter);
  }

  _accnt->addColumn(tr("Account Number"), 100, Qt::AlignCenter);

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _accnt->addColumn(tr("Sub."), 50, Qt::AlignCenter);
  }

  _accnt->addColumn(tr("Description"), -1, Qt::AlignLeft);
}

/*
 *  Destroys the object and frees any allocated resources
 */
accountSearch::~accountSearch()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void accountSearch::languageChange()
{
  retranslateUi(this);
}

void accountSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
  {
    _typeval = param.toUInt();
    if(_typeval > 0)
    {
      if(!(_typeval & GLCluster::cEquity))
        _type->removeItem(5);
      if(!(_typeval & GLCluster::cRevenue))
        _type->removeItem(4);
      if(!(_typeval & GLCluster::cExpense))
        _type->removeItem(3);
      if(!(_typeval & GLCluster::cLiability))
        _type->removeItem(2);
      if(!(_typeval & GLCluster::cAsset))
        _type->removeItem(1);
    }
  }

  sFillList();
}

void accountSearch::sClose()
{
  done(_accntid);
}

void accountSearch::sSelect()
{
  done(_accnt->id());
}

void accountSearch::sClear()
{
  done(-1);
}

void accountSearch::sFillList()
{
  QString sql("SELECT accnt_id, ");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      sql += "accnt_company, ";

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      sql += "accnt_profit, ";
  }

  sql += "accnt_number, ";

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      sql += "accnt_sub, ";
  }

  sql += " accnt_descrip "
         "FROM accnt ";

  bool where = false;
  QStringList types;
  if(_type->currentIndex() > 0)
  {
    where = true;
    sql += "WHERE (accnt_type = '" + _type->itemData(_type->currentItem()).toString() + "') ";
  }
  else if(_typeval > 0)
  {
    if(_typeval & GLCluster::cAsset)
      types << ("'A'");
    if(_typeval & GLCluster::cLiability)
      types << ("'L'");
    if(_typeval & GLCluster::cExpense)
      types << ("'E'");
    if(_typeval & GLCluster::cRevenue)
      types << ("'R'");
    if(_typeval & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
  {
    where = true;
    sql += "WHERE (accnt_type IN (" + types.join(",") + ")) ";
  }

  if(!_descrip->text().isEmpty())
  {
    if(where)
      sql += " AND ";
    else
      sql += "WHERE ";
    sql += "((accnt_descrip ~* :descrip) OR (accnt_extref ~* :descrip)) ";
  }

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  XSqlQuery qry;
  qry.prepare(sql);
  qry.bindValue(":descrip", _descrip->text());
  qry.exec();
  _accnt->populate(qry, _accntid);
}

