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


#include "usernameList.h"

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
#include "usernamecluster.h"
#include "xtreewidget.h"

usernameList::usernameList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setName( "usernameList" );

  _id = -1;
  _type = UsernameLineEdit::UsersAll;

  setCaption(tr("Username List"));

  Q3VBoxLayout *usernameListLayout = new Q3VBoxLayout( this, 5, 5, "usernameListLayout"); 
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
  usernameListLayout->addLayout( Layout72 );

  QLabel *_usernamesLit = new QLabel(tr("&Usernames:"), this, "_usernamesLit");
  Layout20->addWidget( _usernamesLit );

  _user = new XTreeWidget(this);
  _user->setName("_user");
  _usernamesLit->setBuddy(_user);
  Layout20->addWidget(_user);
  usernameListLayout->addLayout( Layout20 );

  resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  connect( _user, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
  connect( _user, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );

  _user->addColumn(tr("Username"), 100,  Qt::AlignLeft );
  _user->addColumn(tr("Real Name"), -1,   Qt::AlignLeft );
}

void usernameList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("id", &valid);
  if (valid)
    _id = param.toInt();
  else
    _id = -1;

  param = pParams.value("type", &valid);
  if (valid)
  {
    _type = param.toUInt();
    if(UsernameLineEdit::UsersActive)
      setCaption(tr("Active Usernames"));
    else if(UsernameLineEdit::UsersInactive)
      setCaption(tr("Inactive Usernames"));
  }
  else
    _type = UsernameLineEdit::UsersAll;

  param = pParams.value("caption", &valid);
  if (valid)
    setCaption(param.toString());

  sFillList();
}

void usernameList::sClose()
{
  done(_id);
}

void usernameList::sSelect()
{
  done(_user->id());
}

void usernameList::sSearch(const QString &pTarget)
{
  _user->clearSelection();
  int i;
  for (i = 0; i < _user->topLevelItemCount(); i++)
  {
    if (_user->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _user->topLevelItemCount())
  {
    _user->setCurrentItem(_user->topLevelItem(i));
    _user->scrollToItem(_user->topLevelItem(i));
  }
}

void usernameList::sFillList()
{
  QString sql("SELECT usr_id, usr_username, usr_propername"
              "  FROM usr");

  if(UsernameLineEdit::UsersActive)
    sql += " WHERE (usr_active)";
  else if(UsernameLineEdit::UsersInactive)
    sql += " WHERE (NOT usr_active)";

  sql += " ORDER BY usr_username;";

  _user->populate(sql, _id);
}

void usernameList::reject()
{
  done(_id);
}

