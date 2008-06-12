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


#include "shipToList.h"

#include <qvariant.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <parameter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <q3header.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <xsqlquery.h>

#include "xtreewidget.h"

shipToList::shipToList(QWidget * parent, const char *name, bool modal, Qt::WFlags fl) :
  QDialog( parent, name, modal, fl )
{
    setCaption(tr("Ship To's"));

  _shiptoid = -1;

  if (!name)
    setName("shipToList");

    Q3VBoxLayout *shipToListLayout = new Q3VBoxLayout( this, 5, 5, "shipToListLayout"); 
    Q3HBoxLayout *Layout54 = new Q3HBoxLayout( 0, 0, 7, "Layout54"); 
    Q3HBoxLayout *Layout53 = new Q3HBoxLayout( 0, 0, 5, "Layout53"); 
    Q3VBoxLayout *Layout51 = new Q3VBoxLayout( 0, 0, 5, "Layout51"); 
    Q3VBoxLayout *Layout49 = new Q3VBoxLayout( 0, 0, 0, "Layout49"); 
    Q3VBoxLayout *Layout52 = new Q3VBoxLayout( 0, 0, 5, "Layout52"); 
    Q3VBoxLayout *Layout50 = new Q3VBoxLayout( 0, 0, 0, "Layout50"); 
    Q3HBoxLayout *Layout6 = new Q3HBoxLayout( 0, 0, 0, "Layout6"); 
    Q3VBoxLayout *Layout2 = new Q3VBoxLayout( 0, 0, 5, "Layout2"); 
    Q3VBoxLayout *Layout55 = new Q3VBoxLayout( 0, 0, 0, "Layout55"); 

    QLabel *_custNumberLit = new QLabel(tr("Customer #:"), this, "_custNumberLit");
    _custNumberLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custNumberLit );

    QLabel *_custNameLit = new QLabel(tr("Cust Name:"), this, "_custNameLit");
    _custNameLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custNameLit );

    QLabel *_custAddressLit = new QLabel(tr("Cust Address:"), this, "_custAddressLit");
    _custAddressLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custAddressLit );
    Layout51->addLayout( Layout49 );

    QLabel *_searchForLit = new QLabel(tr("Search For:"), this, "_searchForLit");
    _searchForLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout51->addWidget( _searchForLit );
    Layout53->addLayout( Layout51 );

    _custNumber = new QLabel( this, "_custNumber" );
    Layout6->addWidget( _custNumber );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout6->addItem( spacer );
    Layout50->addLayout( Layout6 );

    _custName = new QLabel( this, "_custName" );
    Layout50->addWidget( _custName );

    _custAddress = new QLabel( this, "_custAddress" );
    Layout50->addWidget( _custAddress );
    Layout52->addLayout( Layout50 );

    _searchFor = new QLineEdit( this, "_searchFor" );
    Layout52->addWidget( _searchFor );
    Layout53->addLayout( Layout52 );
    Layout54->addLayout( Layout53 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout2->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setDefault( TRUE );
    Layout2->addWidget( _select );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    Layout2->addItem( spacer_2 );
    Layout54->addLayout( Layout2 );
    shipToListLayout->addLayout( Layout54 );

    _shipTosList = new QLabel(tr("Ship To's:"), this, "_shipTosList");
    Layout55->addWidget( _shipTosList );

    _shipto = new XTreeWidget(this);
    _shipto->setName("_shipto");
    Layout55->addWidget( _shipto );
    shipToListLayout->addLayout( Layout55 );

    resize( QSize(580, 365).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
    connect( _shipto, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );

  _selected = NULL;

  _shipto->addColumn(tr("Number"),  70,           Qt::AlignLeft );
  _shipto->addColumn(tr("Name"),    150,          Qt::AlignLeft );
  _shipto->addColumn(tr("Address"), 150,          Qt::AlignLeft );
  _shipto->addColumn(tr("City, State, Zip"), -1,  Qt::AlignLeft );
  _shipto->sortByColumn(1, Qt::AscendingOrder);
}

void shipToList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();
  else
    _shiptoid = -1;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_number, cust_name, cust_address1 "
                  "FROM cust "
                  "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", param.toInt());
    cust.exec();
    if (cust.first())
    {
      _custNumber->setText(cust.value("cust_number").toString());
      _custName->setText(cust.value("cust_name").toString());
      _custAddress->setText(cust.value("cust_address1").toString());
    }

    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id, shipto_num, shipto_name, shipto_address1,"
                    " (shipto_city || ', ' || shipto_state || '  ' || shipto_zipcode) "
                    "FROM shipto "
                    "WHERE (shipto_cust_id=:cust_id) "
                    "ORDER BY shipto_num;" );
    shipto.bindValue(":cust_id", param.toInt());
    shipto.exec();
    _shipto->populate(shipto, _shiptoid );
  }
}

void shipToList::sClose()
{
  done(_shiptoid);
}

void shipToList::sSelect()
{
  done(_shipto->id());
}

void shipToList::sSearch(const QString &pTarget)
{
  _shipto->clearSelection();
  int i;
  for (i = 0; i < _shipto->topLevelItemCount(); i++)
  {
    if (_shipto->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _shipto->topLevelItemCount())
  {
    _shipto->setCurrentItem(_shipto->topLevelItem(i));
    _shipto->scrollToItem(_shipto->topLevelItem(i));
  }
}
