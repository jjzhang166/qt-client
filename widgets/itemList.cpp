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


#include "itemList.h"

#include <qvariant.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <parameter.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <q3header.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include "itemcluster.h"
#include "xtreewidget.h"

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int);
QString buildItemLineEditTitle(const unsigned int, const QString);

itemList::itemList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
    if ( !name )
      setName( "itemList" );

  _itemid = -1;
  _itemType = ItemLineEdit::cUndefined;
  _useQuery = FALSE;

    setCaption(tr("Item List"));

    Q3VBoxLayout *itemListLayout = new Q3VBoxLayout( this, 5, 5, "itemListLayout"); 
    Q3HBoxLayout *Layout69 = new Q3HBoxLayout( 0, 0, 0, "Layout69"); 
    Q3HBoxLayout *Layout72 = new Q3HBoxLayout( 0, 0, 7, "Layout72"); 
    Q3VBoxLayout *Layout71 = new Q3VBoxLayout( 0, 0, 0, "Layout71"); 
    Q3VBoxLayout *Layout70 = new Q3VBoxLayout( 0, 0, 5, "Layout70"); 
    Q3HBoxLayout *Layout5 = new Q3HBoxLayout( 0, 0, 5, "Layout5"); 
    Q3VBoxLayout *Layout18 = new Q3VBoxLayout( 0, 0, 5, "Layout18"); 
    Q3VBoxLayout *Layout20 = new Q3VBoxLayout( 0, 0, 0, "Layout20"); 

    QLabel *_searchForLit = new QLabel(tr("S&earch for:"), this, "_searchForLit");
    _searchForLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout5->addWidget( _searchForLit );

    _searchFor = new QLineEdit( this, "_searchFor" );
    _searchForLit->setBuddy(_searchFor);
    Layout5->addWidget( _searchFor );
    Layout70->addLayout( Layout5 );

    _showInactive = new QCheckBox(tr("Show &Inactive Items"), this, "_showInactive");
    Layout69->addWidget( _showInactive );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout69->addItem( spacer );
    Layout70->addLayout( Layout69 );
    Layout71->addLayout( Layout70 );

    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    Layout71->addItem( spacer_2 );
    Layout72->addLayout( Layout71 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout18->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setEnabled( FALSE );
    _select->setAutoDefault( TRUE );
    _select->setDefault( TRUE );
    Layout18->addWidget( _select );
    Layout72->addLayout( Layout18 );
    itemListLayout->addLayout( Layout72 );

    QLabel *_itemsLit = new QLabel(tr("&Items:"), this, "_itemsLit");
    Layout20->addWidget( _itemsLit );

    _item = new XTreeWidget(this);
    _item->setName("_item");
    _itemsLit->setBuddy(_item );
    Layout20->addWidget( _item );
    itemListLayout->addLayout( Layout20 );

    resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    connect( _item, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
    connect( _showInactive, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
    connect( _item, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );


  _item->addColumn(tr("Item Number"), 100,  Qt::AlignLeft );
  _item->addColumn(tr("Description"), -1,   Qt::AlignLeft );
}

void itemList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  else
    _itemid = -1;

  param = pParams.value("sql", &valid);
  if (valid)
  {
    _sql = param.toString();
    _useQuery = TRUE;
  }
  else
    _useQuery = FALSE;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setCaption(buildItemLineEditTitle(_itemType, tr("Items")));
  }
  else
    _itemType = ItemLineEdit::cUndefined;

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(FALSE);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));

  param = pParams.value("caption", &valid);
  if (valid)
    setCaption(param.toString());

  sFillList();
}

void itemList::sClose()
{
  done(_itemid);
}

void itemList::sSelect()
{
  done(_item->id());
}

void itemList::sSearch(const QString &pTarget)
{
  if (_item->currentItem())
    _item->setCurrentItem(0);

  _item->clearSelection();

  int i;
  for (i = 0; i < _item->topLevelItemCount(); i++)
    if (_item->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;

  if (i < _item->topLevelItemCount())
  {
    _item->setCurrentItem(_item->topLevelItem(i));
    _item->scrollToItem(_item->topLevelItem(i));
  }
}

void itemList::sFillList()
{
  if (_useQuery)
  {
    _item->populate(_sql, _itemid);
  }
  else
  { 
      QString pre;
      QString post;
      if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
      {
        pre =  "SELECT DISTINCT ON (toNumeric(item_number, 999999999999999), item_number) item_id, item_number, (item_descrip1 || ' ' || item_descrip2)";
        post = "ORDER BY toNumeric(item_number, 999999999999999), item_number";
      }
      else
      {
        pre =  "SELECT DISTINCT item_id, item_number, (item_descrip1 || ' ' || item_descrip2)";
        post = "ORDER BY item_number";
      }

      QStringList clauses;
      clauses = _extraClauses;
      if(!(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
        clauses << "(item_active)";

      _item->populate(buildItemLineEditQuery(pre, clauses, post, _itemType), _itemid);
  }
}

void itemList::reject()
{
  done(_itemid);
}
