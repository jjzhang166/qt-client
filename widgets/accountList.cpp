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


#include "accountList.h"

#include <qvariant.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <parameter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <q3header.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include "xtreewidget.h"

#include "OpenMFGWidgets.h"

#include "glcluster.h"

accountList::accountList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
  QDialog(parent, name, modal, fl)
{
  if ( !name )
    setName( "accountList" );

  _accntid = -1;
  _type = GLCluster::cUndefined;

  setCaption(tr("Account Numbers"));

  Q3HBoxLayout *accountListLayout = new Q3HBoxLayout( this, 5, 7, "accountListLayout"); 
  Q3VBoxLayout *Layout68 = new Q3VBoxLayout( 0, 0, 0, "Layout68"); 
  Q3VBoxLayout *layout305 = new Q3VBoxLayout( 0, 0, 5, "layout305"); 

  QLabel *accountsLit = new QLabel(tr("Chart of Accounts:"), this, "accountsLit");
  Layout68->addWidget(accountsLit);

  _accnt = new XTreeWidget(this);
  _accnt->setName("_accnt");
  Layout68->addWidget( _accnt );
  accountListLayout->addLayout( Layout68 );

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  layout305->addWidget( _close );

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setAutoDefault( TRUE );
  _select->setDefault( TRUE );
  layout305->addWidget( _select );

  _clear = new QPushButton(tr("C&lear"), this, "_clear");
  layout305->addWidget( _clear );
  QSpacerItem* spacer = new QSpacerItem( 20, 269, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout305->addItem( spacer );
  accountListLayout->addLayout( layout305 );

  resize( QSize(571, 351).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _accnt, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _clear, SIGNAL( clicked() ), this, SLOT( sClear() ) );

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

void accountList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
    _type = param.toUInt();

  sFillList();
}

void accountList::sClose()
{
  done(_accntid);
}

void accountList::sSelect()
{
  done(_accnt->id());
}

void accountList::sClear()
{
  done(-1);
}

void accountList::sFillList()
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

  QStringList types;
  if(_type > 0)
  {
    if(_type & GLCluster::cAsset)
      types << ("'A'");
    if(_type & GLCluster::cLiability)
      types << ("'L'");
    if(_type & GLCluster::cExpense)
      types << ("'E'");
    if(_type & GLCluster::cRevenue)
      types << ("'R'");
    if(_type & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
    sql += "WHERE (accnt_type IN (" + types.join(",") + ")) ";

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  _accnt->populate(sql, _accntid);
}
