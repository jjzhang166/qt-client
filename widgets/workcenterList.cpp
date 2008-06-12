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


#include "workcenterList.h"

#include <QVariant>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include <parameter.h>

#include "workcentercluster.h"
#include "xlistview.h"

workcenterList::workcenterList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setObjectName( "workcenterList" );

  _id = -1;

  setCaption(tr("Work Center List"));

  Q3VBoxLayout *workcenterListLayout = new Q3VBoxLayout( this, 5, 5, "workcenterListLayout"); 
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
  workcenterListLayout->addLayout( Layout72 );

  QLabel *_workcentersLit = new QLabel(tr("&Work Centers:"), this, "_workcentersLit");
  Layout20->addWidget( _workcentersLit );

  _workcenter = new XListView( this, "_workcenter" );
  _workcentersLit->setBuddy(_workcenter);
  Layout20->addWidget(_workcenter);
  workcenterListLayout->addLayout( Layout20 );

  resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  connect( _workcenter, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
  connect( _workcenter, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );

  _workcenter->addColumn(tr("Work Center"), 100,  Qt::AlignLeft );
  _workcenter->addColumn(tr("Description"), -1,   Qt::AlignLeft );
}

void workcenterList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("id", &valid);
  if (valid)
    _id = param.toInt();
  else
    _id = -1;

  param = pParams.value("caption", &valid);
  if (valid)
    setCaption(param.toString());

  sFillList();
}

void workcenterList::sClose()
{
  done(_id);
}

void workcenterList::sSelect()
{
  done(_workcenter->id());
}

void workcenterList::sSearch(const QString &pTarget)
{
  Q3ListViewItem *target;

  if (_workcenter->selectedItem())
    _workcenter->setSelected(_workcenter->selectedItem(), FALSE);

  _workcenter->clearSelection();

  target = _workcenter->firstChild();
  while ((target != NULL) && (pTarget.upper() != target->text(0).left(pTarget.length())))
    target = target->nextSibling();

  if (target != NULL)
  {
    _workcenter->setSelected(target, TRUE);
    _workcenter->ensureItemVisible(target);
  }
}

void workcenterList::sFillList()
{
  QString sql("SELECT wrkcnt_id, wrkcnt_code, wrkcnt_descrip"
              "  FROM wrkcnt"
              " ORDER BY wrkcnt_code;");

  _workcenter->populate(sql, _id);
}

void workcenterList::reject()
{
  done(_id);
}

