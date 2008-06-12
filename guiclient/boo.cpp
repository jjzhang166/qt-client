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

#include "boo.h"

#include <QVariant>
#include <QMessageBox>
#include <QWorkspace>
#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "booItem.h"

/*
 *  Constructs a boo as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
boo::boo(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_booitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_booitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire()));

  statusBar()->hide();

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cJob);

  _booitem->addColumn(tr("#"),           _seqColumn,  Qt::AlignCenter );
  _booitem->addColumn(tr("Std Oper."),   _itemColumn, Qt::AlignLeft   );
  _booitem->addColumn(tr("Work Cntr."),  _itemColumn, Qt::AlignLeft   );
  _booitem->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _booitem->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter );
  _booitem->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter );
  _booitem->addColumn(tr("Exec. Day"),   _qtyColumn,  Qt::AlignCenter );
  
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _activate->hide();
  _revision->setMode(RevisionLineEdit::Maintain);
  _revision->setType("BOO");
}

/*
 *  Destroys the object and frees any allocated resources
 */
boo::~boo()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void boo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse boo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());
   {
     param = pParams.value("revision_id", &valid);
     if (valid)
       _revision->setId(param.toInt());
   }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_booitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }

    if (param.toString() == "new")
    {
      _mode = cNew;

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _revision->setEnabled(FALSE);
      _revisionDate->setEnabled(FALSE);
      _finalLocation->setEnabled(FALSE);
      _closeWO->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _expire->setEnabled(FALSE);
      _moveUp->setEnabled(FALSE);
      _moveDown->setEnabled(FALSE);
      _save->setEnabled(FALSE);

      connect(_booitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void boo::sSave()
{
  if (q.first())
  {
	q.prepare( "UPDATE boohead "
               "SET boohead_docnum=:boohead_docnum, boohead_leadtime=:boohead_leadtime,"
               "    boohead_revision=:boohead_revision, boohead_revisiondate=:boohead_revisiondate,"
               "    boohead_final_location_id=:boohead_final_location_id,"
               "    boohead_closewo=:boohead_closewo "
               "WHERE ((boohead_item_id=:boohead_item_id) "
			   "AND (boohead_rev_id=:boohead_rev_id));" );
    q.bindValue(":boohead_rev_id", _revision->id());
  }
  else
  {
    q.prepare( "INSERT INTO boohead "
               "(boohead_item_id, boohead_docnum, boohead_leadtime, boohead_closewo,"
               " boohead_revision, boohead_revisiondate, boohead_final_location_id) "
               "VALUES "
               "( :boohead_item_id, :boohead_docnum, :boohead_leadtime, :boohead_closewo,"
               "  :boohead_revision, :boohead_revisiondate, :boohead_final_location_id );" );
  }

  q.bindValue(":boohead_item_id", _item->id());
  q.bindValue(":boohead_docnum", _documentNum->text());
  q.bindValue(":boohead_leadtime", _productionLeadTime->text().toInt());
  q.bindValue(":boohead_revision", _revision->number());
  q.bindValue(":boohead_revisiondate", _revisionDate->date());
  q.bindValue(":boohead_final_location_id", _finalLocation->id());
  q.bindValue(":boohead_closewo", QVariant(_closeWO->isChecked(), 0));
  q.exec();

  close();
}

void boo::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  orReport report("BillOfOperations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void boo::sPopulateMenu(QMenu *pMenu)
{
  pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);

  if ( ((_mode == cNew) || (_mode == cEdit)) &&
       (_privileges->check("MaintainBOOs")) )
  {
    pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->insertItem(tr("Expire"), this, SLOT(sExpire()), 0);

    pMenu->insertSeparator();

    pMenu->insertItem(tr("Move Up"),   this, SLOT(sMoveUp()), 0);
    pMenu->insertItem(tr("Move Down"), this, SLOT(sMoveDown()), 0);
  }
}

void boo::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sExpire()
{
  q.prepare( "UPDATE booitem "
             "SET booitem_expires=CURRENT_DATE "
             "WHERE (booitem_id=:booitem_id);" );
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_item->id(), TRUE);
}

void boo::sMoveUp()
{
  q.prepare("SELECT moveBooitemUp(:booitem_id) AS result;");
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_booitem->id(), TRUE);
}

void boo::sMoveDown()
{
  q.prepare("SELECT moveBooitemDown(:booitem_id) AS result;");
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_booitem->id(), TRUE);
}

void boo::sFillList()
{
  sFillList(-1, TRUE);
}

void boo::sFillList(int pItemid, bool pLocalUpdate)
{
  _booitem->clear();

  if (_item->itemType() == "J")
  {
    _closeWO->setEnabled(FALSE);
	_closeWO->setChecked(FALSE);
  }

  int locid = _finalLocation->id();
  q.prepare("SELECT location_id, (warehous_code || '-' || formatLocationName(location_id)) AS locationname"
            "  FROM location, warehous"
            " WHERE ( (NOT location_restrict)"
            "   AND   (location_warehous_id=warehous_id) ) "
            "UNION "
            "SELECT location_id, (warehous_code || '-' || formatLocationName(location_id)) AS locationname"
            "  FROM location, warehous, locitem"
            " WHERE ( (location_warehous_id=warehous_id)"
            "   AND   (location_restrict)"
            "   AND   (locitem_location_id=location_id)"
            "   AND   (locitem_item_id=:item_id) ) "
            "ORDER BY locationname;");
  q.bindValue(":item_id", pItemid);
  q.exec();
  _finalLocation->populate(q, locid);

  q.prepare( "SELECT boohead_id, boohead_docnum, boohead_revision,"
             "       boohead_revisiondate, boohead_final_location_id,"
             "       boohead_closewo "
             "FROM boohead "
             "WHERE ((boohead_item_id=:item_id) "
			 "AND (boohead_rev_id=:revision_id));" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":revision_id", _revision->id());
  q.exec();
  if (q.first())
  {
    _documentNum->setText(q.value("boohead_docnum").toString());
    _revision->setNumber(q.value("boohead_revision").toString());
    _revisionDate->setDate(q.value("boohead_revisiondate").toDate());
    _finalLocation->setId(q.value("boohead_final_location_id").toInt());
    _closeWO->setChecked(q.value("boohead_closewo").toBool());
  }

  if (_revision->description() == "Inactive")
  {
	  _save->setEnabled(FALSE);
      _new->setEnabled(FALSE);
	  _documentNum->setEnabled(FALSE);
	  _revisionDate->setEnabled(FALSE);
	  _closeWO->setEnabled(FALSE);
	  _finalLocation->setEnabled(FALSE);
	  _booitem->setEnabled(FALSE);
  }

  if ((_revision->description() == "Pending") || (_revision->description() == "Active"))
  {
	  _save->setEnabled(TRUE);
      _new->setEnabled(TRUE);
	  _documentNum->setEnabled(TRUE);
	  _revisionDate->setEnabled(TRUE);
	  _closeWO->setEnabled(TRUE);
	  _finalLocation->setEnabled(TRUE);
	  _booitem->setEnabled(TRUE);
  }

  q.prepare( "SELECT MAX(booitem_execday) AS leadtime "
             "FROM booitem(:item_id,:revision_id);" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":revision_id",_revision->id());
  q.exec();
  if (q.first())
    _productionLeadTime->setText(q.value("leadtime").toString());

  QString sql( "SELECT booitem_id, booitem_seqnumber,"
               "       COALESCE(stdopn_number, :none) AS f_stdopnnumber,"
               "       wrkcnt_code, (booitem_descrip1 || ' ' || booitem_descrip2) AS description,"
               "       formatDate(booitem_effective, 'Always') AS f_effective,"
               "       formatDate(booitem_expires, 'Never') AS f_expires,"
               "       booitem_execday, (booitem_configtype<>'N') AS config "
               "FROM wrkcnt,"
			   "     booitem(:item_id,:revision_id) LEFT OUTER JOIN stdopn ON (booitem_stdopn_id=stdopn_id) "
               "WHERE ((booitem_wrkcnt_id=wrkcnt_id)" );

  if (!_showExpired->isChecked())
    sql += " AND (booitem_expires > CURRENT_DATE)";

  if (!_showFuture->isChecked())
    sql += " AND (booitem_effective <= CURRENT_DATE)";

  sql += ") "
         "ORDER BY booitem_seqnumber, booitem_effective";

  q.prepare(sql);
  q.bindValue(":none", tr("None"));
  q.bindValue(":item_id", _item->id());
  q.bindValue(":revision_id",_revision->id());
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *selected = 0;
    XTreeWidgetItem *last = 0;
    do
    {
      last = new XTreeWidgetItem( _booitem, last, q.value("booitem_id").toInt(),
                                  q.value("booitem_seqnumber"), q.value("f_stdopnnumber"),
                                  q.value("wrkcnt_code"), q.value("description"),
                                  q.value("f_effective"), q.value("f_expires"),
                                  q.value("booitem_execday") );

      if (q.value("config").toBool())
        last->setTextColor("blue");

      if (q.value("booitem_id").toInt() == pItemid)
        selected = last;
    }
    while (q.next());
  
    if ( (selected) && (pLocalUpdate) )
    {
      _booitem->setCurrentItem(selected);
      _booitem->scrollTo(_booitem->currentIndex());
    }
  }
}

