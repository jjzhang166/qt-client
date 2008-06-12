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


#include "projectList.h"

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
#include "projectcluster.h"
#include "xtreewidget.h"

projectList::projectList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setName( "projectList" );

  _id = -1;

  setCaption(tr("Project List"));

  Q3VBoxLayout *projectListLayout = new Q3VBoxLayout( this, 5, 5, "projectListLayout"); 
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
  projectListLayout->addLayout( Layout72 );

  QLabel *_projectsLit = new QLabel(tr("&Projects:"), this, "_projectsLit");
  Layout20->addWidget( _projectsLit );

  _project = new XTreeWidget(this);
  _project->setName("_project" );
  _projectsLit->setBuddy(_project);
  Layout20->addWidget(_project);
  projectListLayout->addLayout( Layout20 );

  resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  connect( _project, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
  connect( _project, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );

  _project->addColumn(tr("Number"), 100,  Qt::AlignLeft );
  _project->addColumn(tr("Name"), -1,   Qt::AlignLeft );
}

void projectList::set(ParameterList &pParams)
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
    _type = param.toInt();

  param = pParams.value("caption", &valid);
  if (valid)
    setCaption(param.toString());

  sFillList();
}

void projectList::sClose()
{
  done(_id);
}

void projectList::sSelect()
{
  done(_project->id());
}

void projectList::sSearch(const QString &pTarget)
{
  _project->clearSelection();
  int i;
  for (i = 0; i < _project->topLevelItemCount(); i++)
  {
    if (_project->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _project->topLevelItemCount())
  {
    _project->setCurrentItem(_project->topLevelItem(i));
    _project->scrollToItem(_project->topLevelItem(i));
  }
}

void projectList::sFillList()
{
  QString sql("SELECT prj_id, prj_number, prj_name"
              "  FROM prj");
  bool checkStatus = false;
  if(_type & ProjectLineEdit::SalesOrder)
  {
    sql += " WHERE ((prj_so)";
    checkStatus = true;
  }

  if(_type & ProjectLineEdit::WorkOrder)
  {
    if(checkStatus)
      sql += "    OR  ";
    else
      sql += " WHERE (";
    sql += "(prj_wo)";
    checkStatus = true;
  }

  if(_type & ProjectLineEdit::PurchaseOrder)
  {
    if(checkStatus)
      sql += "    OR  ";
    else
      sql += " WHERE (";
    sql += "(prj_po)";
    checkStatus = true;
  }

  if(checkStatus)
    sql += ")";

  sql += " ORDER BY prj_number;";

  _project->populate(sql, _id);
}

void projectList::reject()
{
  done(_id);
}

