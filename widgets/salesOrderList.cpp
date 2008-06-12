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


#include "salesOrderList.h"

#include <parameter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include "xtreewidget.h"
#include "warehousegroup.h"
#include "socluster.h"
#include "xsqlquery.h"

salesOrderList::salesOrderList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setName( "salesOrderList" );

  _soheadid = -1;
  _type = (cSoOpen | cSoClosed | cSoReleased);

  setCaption(tr("Sales Orders"));

  Q3VBoxLayout *mainLayout = new Q3VBoxLayout(this, 5, 5, "mainLayout"); 
  Q3VBoxLayout *warehouseLayout = new Q3VBoxLayout(0, 0, 0, "warehouseLayout"); 
  Q3HBoxLayout *topLayout = new Q3HBoxLayout( 0, 0, 7, "topLayout"); 
  Q3VBoxLayout *buttonsLayout = new Q3VBoxLayout(0, 0, 5, "buttonsLayout");
  Q3VBoxLayout *listLayout = new Q3VBoxLayout( 0, 0, 0, "listLayout"); 

  _warehouse = new WarehouseGroup(this, "_warehouse");
  warehouseLayout->addWidget(_warehouse);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
  warehouseLayout->addItem(spacer);
  topLayout->addLayout(warehouseLayout);

  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  topLayout->addItem(spacer_2);

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonsLayout->addWidget(_select);
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_salesOrdersLit = new QLabel(tr("Sales Orders:"), this, "_salesOrdersLit");
  listLayout->addWidget(_salesOrdersLit);

  _so = new XTreeWidget(this);
  _so->setName("_so");
  listLayout->addWidget(_so);
  mainLayout->addLayout(listLayout);

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _so, SIGNAL( itemSelected(int) ), _select, SLOT( animateClick() ) );
  connect( _so, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
  connect( _warehouse, SIGNAL(updated()), this, SLOT( sFillList() ) );

  _so->addColumn(tr("Order #"),   _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Customer"),  -1,           Qt::AlignLeft   );
  _so->addColumn(tr("P/O #"),     _orderColumn, Qt::AlignLeft   );
  _so->addColumn(tr("Ordered"),   _dateColumn,  Qt::AlignCenter );
  _so->addColumn(tr("Scheduled"), _dateColumn,  Qt::AlignCenter );

  setTabOrder(_warehouse, _so);
  setTabOrder(_so, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _warehouse);
  _warehouse->setFocus();
}

void salesOrderList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
    _soheadid = param.toInt();
    
  param = pParams.value("soType", &valid);
  if (valid)
    _type = param.toInt();
    
  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();
 
  sFillList();
}

void salesOrderList::sClose()
{
  done(_soheadid);
}

void salesOrderList::sSelect()
{
  done(_so->id());
}

void salesOrderList::sFillList()
{
  QString sql;


  if (_type == cSoAtShipping)
  {
    sql = "SELECT DISTINCT cohead_id, cohead_number, cust_name, cohead_custponumber,"
          "                formatDate(cohead_orderdate),"
          "                formatDate(MIN(coitem_scheddate)) "
          "FROM cosmisc, coship, cohead, coitem, itemsite, cust "
          "WHERE ((cohead_cust_id=cust_id)"
          " AND (coitem_cohead_id=cohead_id)"
          " AND (coitem_itemsite_id=itemsite_id)"
          " AND (coship_coitem_id=coitem_id)"
          " AND (coship_cosmisc_id=cosmisc_id)"
          " AND (NOT cosmisc_shipped)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "GROUP BY cohead_id, cohead_number, cust_name,"
         "         cohead_custponumber, cohead_orderdate "
         "ORDER BY cohead_number;";
  }
  else
  {
    bool statusCheck = FALSE;

    sql = "SELECT DISTINCT cohead_id, cohead_number, cust_name, cohead_custponumber,"
          "                formatDate(cohead_orderdate) AS orderdate,"
          "                formatDate(MIN(coitem_scheddate)) "
          "FROM cohead, coitem, itemsite, cust "
          "WHERE ((cohead_cust_id=cust_id)"
          " AND (coitem_status <> 'X')"
          " AND (coitem_cohead_id=cohead_id)"
          " AND (coitem_itemsite_id=itemsite_id)";

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += "AND (";

    if (_type & cSoOpen)
    {
      sql += "(coitem_status='O')";
      statusCheck = TRUE;
    }

    if (_type & cSoClosed)
    {
      if (statusCheck)
        sql += " OR ";
      sql += "(coitem_status='C')";
      statusCheck = TRUE;
    }

    if (_type & cSoReleased)
    {
      if (statusCheck)
        sql += " AND ";
      sql += "(cohead_holdtype='N')";
      statusCheck = TRUE;
    }
    
    if (_type & cSoCustomer)
    {
      if (statusCheck)
        sql += " AND ";
      sql += "(cohead_cust_id=:cust_id)";
    }

    sql += ")) "
           "GROUP BY cohead_id, cohead_number, cust_name,"
           "         cohead_custponumber, cohead_orderdate "
           "ORDER BY cohead_number;";
  }

  XSqlQuery q;
  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":cust_id", _custid);
  q.exec();
  _so->populate(q, _soheadid);
}

