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


#include "itemAliasList.h"

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

#include <xsqlquery.h>
#include "itemcluster.h"
#include "xtreewidget.h"

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int);
QString buildItemLineEditTitle(const unsigned int, const QString);

itemAliasList::itemAliasList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
 QDialog( parent, name, modal, fl )
{
  setCaption(tr( "Item Aliases"));

  _itemType = ItemLineEdit::cUndefined;

  if ( !name )
   setName( "itemAliasList" );

  Q3VBoxLayout *_mainLayout = new Q3VBoxLayout(this, 5, 5); 
  Q3HBoxLayout *_itemLayout = new Q3HBoxLayout(0, 0, 5); 
  Q3HBoxLayout *_line1Layout = new Q3HBoxLayout(0, 0, 7); 
  Q3VBoxLayout *_enterLayout = new Q3VBoxLayout(0, 0, 5); 
  Q3HBoxLayout *_topLayout = new Q3HBoxLayout( 0, 0, 7); 
  Q3VBoxLayout *_buttonsLayout= new Q3VBoxLayout(0, 0, 5); 
  Q3VBoxLayout *_listLayout = new Q3VBoxLayout(0, 0, 0); 

  QLabel *_aliasLit = new QLabel(tr("&Alias:"), this);
  _aliasLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _aliasLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  _itemLayout->addWidget(_aliasLit);

  _alias = new QLineEdit( this, "_alias" );
  _alias->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _aliasLit->setBuddy(_alias);
  _itemLayout->addWidget(_alias);
  _line1Layout->addLayout(_itemLayout);

  _showInactive = new QCheckBox(tr("Show &Inactive Items"), this);
  _line1Layout->addWidget(_showInactive);
  _enterLayout->addLayout(_line1Layout);

  QSpacerItem* spacer = new QSpacerItem( 20, 32, QSizePolicy::Minimum, QSizePolicy::Fixed );
  _enterLayout->addItem( spacer );
  _topLayout->addLayout(_enterLayout);

  _close = new QPushButton(tr("&Cancel"), this);
  _buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this);
  _select->setEnabled( FALSE );
  _select->setAutoDefault( TRUE );
  _select->setDefault( TRUE );
  _buttonsLayout->addWidget(_select);

  _topLayout->addLayout(_buttonsLayout);
  _mainLayout->addLayout(_topLayout);

  QLabel *_itemsLit = new QLabel(tr("&Items:"), this);
  _listLayout->addWidget(_itemsLit);

  _item = new XTreeWidget(this);
  _item->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _itemsLit->setBuddy(_item);
  _listLayout->addWidget(_item);
  _mainLayout->addLayout(_listLayout);

// signals and slots connections
  connect( _item, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _alias, SIGNAL( lostFocus() ), this, SLOT( sFillList() ) );
  connect( _item, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );

  _useQuery = FALSE;

  _item->addColumn(tr("Alias Number"), 100,  Qt::AlignLeft );
  _item->addColumn(tr("Item Number"),  100,  Qt::AlignLeft );
  _item->addColumn(tr("Description"),  -1,   Qt::AlignLeft );
}

void itemAliasList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setCaption(buildItemLineEditTitle(_itemType, tr("Item Aliases")));
  }
  else
  {
    _itemType = ItemLineEdit::cUndefined;
    setCaption(tr("Item Aliases"));
  }

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(FALSE);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));

  param = pParams.value("sql", &valid);
  if (valid)
    _sql = param.toString();
}

void itemAliasList::sSelect()
{
  done(_item->altId());
}

void itemAliasList::sFillList()
{
  _item->clear();

  if (_alias->text().stripWhiteSpace().length() == 0)
    return;

  QString pre( "SELECT item_id, itemalias_id, itemalias_number, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip "
               "FROM (SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2");
  QString post(") AS data, itemalias "
               "WHERE ( (itemalias_item_id=item_id)"
               " AND (UPPER(itemalias_number)~UPPER(:searchString)) )" );

  if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
    post += " ORDER BY toNumeric(item_number, 999999999999999), item_number";
  else
    post += " ORDER BY item_number";

  QStringList clauses;
  clauses = _extraClauses;
  if ( !(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
    clauses << "(item_active)";

  XSqlQuery alias;
  alias.prepare(buildItemLineEditQuery(pre, clauses, post, _itemType));
  alias.bindValue(":searchString", _alias->text().stripWhiteSpace());
  alias.exec();
  if (alias.first())
  {
    if (_useQuery)
    {
      XSqlQuery item(_sql);
      if (item.first())
      {
          XTreeWidgetItem *last = NULL;
          do
          {
            if (item.findFirst("item_id", alias.value("item_id").toInt()) != -1)
              last = new XTreeWidgetItem( _item, last, alias.value("item_id").toInt(), alias.value("itemalias_id").toInt(),
                                        alias.value("itemalias_number").toString(), alias.value("item_number").toString(), alias.value("item_descrip").toString() );
          }
          while (alias.next());
      }
    }
    else
      _item->populate(alias, TRUE);
  }
}
