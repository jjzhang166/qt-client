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


#include "plannedOrderList.h"

#include <qvariant.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <parameter.h>
#include <qpushbutton.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <q3header.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include "xtreewidget.h"
#include "warehouseCluster.h"

#include <xsqlquery.h>

plannedOrderList::plannedOrderList(QWidget *parent, const char *name, bool modal, Qt::WFlags fl) :
  QDialog( parent, name, modal, fl )
{
  setCaption(tr("Planned Orders"));

  _planordid = -1;

  if ( !name )
    setName( "plannedOrderList" );

    Q3VBoxLayout *plannedOrderListLayout = new Q3VBoxLayout( this, 5, 5, "plannedOrderListLayout"); 
    Q3HBoxLayout *layout6387 = new Q3HBoxLayout( 0, 0, 7, "layout6387"); 
    Q3VBoxLayout *layout6385 = new Q3VBoxLayout( 0, 0, 0, "layout6385"); 
    Q3HBoxLayout *layout6384 = new Q3HBoxLayout( 0, 0, 5, "layout6384"); 
    Q3VBoxLayout *Layout183 = new Q3VBoxLayout( 0, 0, 5, "Layout183"); 
    Q3VBoxLayout *Layout185 = new Q3VBoxLayout( 0, 0, 0, "Layout185"); 
    Q3HBoxLayout *layout6386 = new Q3HBoxLayout( 0, 0, 0, "layout6386"); 

    _warehouseGroup = new QGroupBox( this );
    QVBoxLayout *_warehouseGroupLayout = new QVBoxLayout( _warehouseGroup );
    _warehouseGroupLayout->setSpacing( 0 );
    _warehouseGroupLayout->setMargin( 5 );
    _warehouseGroupLayout->setAlignment( Qt::AlignTop );

    _allWarehouses = new QRadioButton(tr("&All Warehouses"), _warehouseGroup, "_allWarehouses");
    _allWarehouses->setChecked( TRUE );
    _warehouseGroupLayout->addWidget( _allWarehouses );

    _selectedWarehouse = new QRadioButton(tr("Warehouse:"), _warehouseGroup, "_selectedWarehouse");
    _selectedWarehouse->setChecked( FALSE );
    layout6384->addWidget( _selectedWarehouse );

    _warehouse = new WComboBox( _warehouseGroup, "_warehouse" );
    _warehouse->setEnabled( FALSE );
    layout6384->addWidget( _warehouse );
    _warehouseGroupLayout->addLayout( layout6384 );
    layout6385->addWidget( _warehouseGroup );
    _warehouseGroup->setLayout( _warehouseGroupLayout );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    layout6385->addItem( spacer );
    layout6387->addLayout( layout6385 );

    QSpacerItem* spacer_2 = new QSpacerItem( 223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6386->addItem( spacer_2 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout183->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setAutoDefault( TRUE );
    _select->setDefault( TRUE );
    Layout183->addWidget( _select );
    layout6386->addLayout( Layout183 );
    layout6387->addLayout( layout6386 );
    plannedOrderListLayout->addLayout( layout6387 );

    QLabel *_plannedOrdersLit = new QLabel(tr("Planned Orders:"), this, "_plannedOrdersLit");
    Layout185->addWidget( _plannedOrdersLit );

    _planord = new XTreeWidget( this);
    _planord->setName("_planord" );
    Layout185->addWidget( _planord );
    plannedOrderListLayout->addLayout( Layout185 );

    resize( QSize(496, 360).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _planord, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
    connect( _selectedWarehouse, SIGNAL( toggled(bool) ), this, SLOT( sFillList() ) );
    connect( _warehouse, SIGNAL( newID(int) ), this, SLOT( sFillList() ) );
    connect( _selectedWarehouse, SIGNAL( toggled(bool) ), _warehouse, SLOT( setEnabled(bool) ) );

  setTabOrder(_allWarehouses, _selectedWarehouse);
  setTabOrder(_selectedWarehouse, _warehouse);
  setTabOrder(_warehouse, _planord);
  setTabOrder(_planord, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _allWarehouses);
  _planord->setFocus();

  _planord->addColumn(tr("PL/O #"),      _orderColumn, Qt::AlignLeft   );
  _planord->addColumn(tr("Firmed"),      40,           Qt::AlignCenter );
  _planord->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _planord->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft   );
  _planord->addColumn(tr("Description"), -1,           Qt::AlignLeft   );

  sFillList();
}

void plannedOrderList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  sFillList();
}

void plannedOrderList::sClose()
{
  done(_planordid);
}

void plannedOrderList::sSelect()
{
  done(_planord->id());
}

void plannedOrderList::sFillList()
{
  QString sql( "SELECT planord_id,"
               "       formatPloNumber(planord_id),"
               "       formatBoolYN(planord_firm),"
               "       warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) "
               "FROM planord, itemsite, warehous, item "
               "WHERE ((planord_itemsite_id=itemsite_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=item_id)" );

  if (_selectedWarehouse->isChecked())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY planord_number, planord_subnumber,"
         "         warehous_code, item_number;";

  XSqlQuery planord;
  planord.prepare(sql);
  planord.bindValue(":warheous_id", _warehouse->id());
  planord.exec();

  _planord->populate(planord, _planordid);
}
