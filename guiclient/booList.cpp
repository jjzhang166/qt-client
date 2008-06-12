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

#include "booList.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include <parameter.h>
#include <openreports.h>
#include "boo.h"
#include "copyBOO.h"

/*
 *  Constructs a booList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
booList::booList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_boo, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_boo, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_boo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
booList::~booList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void booList::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void booList::init()
{
  statusBar()->hide();
  
  _boo->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft );
  _boo->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), SLOT(sFillList(int, bool)));

  if (_privileges->check("MaintainBOOs"))
  {
    connect(_boo, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_boo, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_boo, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

    connect(_boo, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_boo, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  sFillList();

  _searchFor->setFocus();
}

void booList::sCopy()
{
  ParameterList params;
  params.append("item_id", _boo->id());

  copyBOO newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void booList::sDelete()
{

  q.prepare("SELECT booitem_id "
	        "FROM booitem "
			"WHERE ((booitem_item_id=:item_id) "
			"AND (booitem_rev_id > -1));");
  q.bindValue(":item_id",_boo->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(  this, tr("Delete Bill of Operations"),
                                tr("The selected Bill of Operations has revision control records and may not be deleted."));
	return;
  }

  if (  QMessageBox::critical(  this, tr("Delete Bill of Operations"),
                                tr("Are you sure that you want to delete the selected BOO?"),
                                tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0  )

  {
    q.prepare( "DELETE FROM boohead "
               "WHERE (boohead_item_id=:item_id);"
               "DELETE FROM booitem "
               "WHERE (booitem_item_id=:item_id);" );
    q.bindValue(":item_id", _boo->id());
    q.exec();

    omfgThis->sBOOsUpdated(_boo->id(), TRUE);
  }
}

void booList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _boo->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _boo->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sFillList( int pItemid, bool pLocal )
{
  QString sql(  "SELECT DISTINCT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) "
                "FROM item "
                "  LEFT OUTER JOIN booitem ON (item_id=booitem_item_id) "
                "  LEFT OUTER JOIN boohead ON (item_id=boohead_item_id) "
                "WHERE (((booitem_id IS NOT NULL) "
                "OR (boohead_id IS NOT NULL)) ");

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += ") "
         "ORDER BY item_number;";

  if ((pItemid != -1) && (pLocal))
    _boo->populate(sql, pItemid);
  else
    _boo->populate(sql);
}

void booList::sFillList()
{
  sFillList(-1, TRUE);
}

void booList::sPrint()
{
  ParameterList params;
  params.append("item_id", _boo->id());

  orReport report("BillOfOperations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void booList::sSearch( const QString & pTarget )
{
  _boo->clearSelection();
  int i;
  for (i = 0; i < _boo->topLevelItemCount(); i++)
  {
    if (_boo->topLevelItem(i)->text(0).contains(pTarget, Qt::CaseInsensitive))
      break;
  }
    
  if (i < _boo->topLevelItemCount())
  {
    _boo->setCurrentItem(_boo->topLevelItem(i));
    _boo->scrollToItem(_boo->topLevelItem(i));
  }
}

void booList::sPopulateMenu( QMenu *, QTreeWidgetItem * )
{
}
