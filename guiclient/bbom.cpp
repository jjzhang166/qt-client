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

#include "bbom.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "bbomItem.h"

bbom::bbom(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bbomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));


  _item->setType(ItemLineEdit::cBreeder);

  _bbomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft, true, "item_number");
  _bbomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "descrip");
  _bbomitem->addColumn(tr("Type"),        _itemColumn,  Qt::AlignLeft, true, "type");
  _bbomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter,true, "uom_name");
  _bbomitem->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight, true, "bbomitem_qtyper");
  _bbomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter,true, "bbomitem_effective");
  _bbomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter,true, "bbomitem_expires");
  _bbomitem->addColumn(tr("Cost %"),      _prcntColumn, Qt::AlignRight, true, "bbomitem_costabsorb");
  
  connect(omfgThis, SIGNAL(bbomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
bbom::~bbom()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bbom::languageChange()
{
    retranslateUi(this);
}


enum SetResponse bbom::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_bbomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_bbomitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_bbomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
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

      _bbomitem->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _close->setText(tr("&Close"));

      connect(_bbomitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void bbom::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  ParameterList params;
  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");

  if(_showFuture->isChecked())
    params.append("showFuture");

  orReport report("BreederBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bbom::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bbomitem_id", _bbomitem->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bbomitem_id", _bbomitem->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sDelete()
{
  if (QMessageBox::warning( this, tr("Delete Breeder BOM Item"),
                            tr("Are you sure you want to delete this Breeder BOM Item?"),
                            tr("&Delete"), tr("&Cancel"), QString::null, 0, 1) == 0)
  {
    q.prepare( "DELETE FROM bbomitem "
               "WHERE (bbomitem_id=:bbomitem_id);" );
    q.bindValue(":bbomitem_id", _bbomitem->id());
    q. exec();
    omfgThis->sBBOMsUpdated(_item->id(), TRUE);
  }
}

void bbom::sMoveUp()
{
  q.prepare("SELECT moveBbomitemUp(:bbomitem_id);");
  q.bindValue(":bbomitem_id", _bbomitem->id());
  q.exec();
}

void bbom::sMoveDown()
{
  q.prepare("SELECT moveBbomitemDown(:bbomitem_id);");
  q.bindValue(":bbomitem_id", _bbomitem->id());
  q.exec();
}

void bbom::sFillList()
{
  sFillList(_item->id(), FALSE);
}

void bbom::sFillList(int pItemid, bool)
{
  if (_item->isValid() && (pItemid == _item->id()))
  {
    QString sql( "SELECT bbomitem.*, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
                 "       CASE WHEN (item_type='C') THEN :coProduct"
                 "            WHEN (item_type='Y') THEN :byProduct"
                 "            ELSE :error"
                 "       END AS type,"
                 "       uom_name,"
		 "       'qty' AS bbomitem_qtyper_xtnumericrole,"
		 "       CASE WHEN (COALESCE(bbomitem_effective, startoftime()) = startoftime()) THEN :always"
		 "       END AS bbomitem_effective_qtdisplayrole,"
		 "       CASE WHEN (COALESCE(bbomitem_expires, endoftime()) = endoftime()) THEN :never"
		 "       END AS bbomitem_expires_qtdisplayrole,"
                 "       CASE WHEN (item_type='Y') THEN :na"
                 "       END AS bbomitem_costabsorb_qtdisplayrole,"
		 "       'scrap' AS bbomitem_costabsorb_xtnumericrole "
                 "FROM bbomitem, item, uom "
                 "WHERE ( (bbomitem_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (bbomitem_parent_item_id=:item_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (bbomitem_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bbomitem_effective <= CURRENT_DATE)";

    sql += ") "
           "ORDER BY bbomitem_seqnumber, bbomitem_effective";

    q.prepare(sql);
    q.bindValue(":coProduct", tr("Co-Product"));
    q.bindValue(":byProduct", tr("By-Product"));
    q.bindValue(":always",    tr("Always"));
    q.bindValue(":never",     tr("Never"));
    q.bindValue(":error",     tr("Error"));
    q.bindValue(":na",        tr("N/A"));
    q.bindValue(":item_id",   _item->id());
    q.exec();
    _bbomitem->populate(q);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT SUM(bbomitem_costabsorb) AS absorb "
               "FROM bbomitem "
               "WHERE (bbomitem_parent_item_id=:item_id);" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      _costsAbsorbed->setDouble(q.value("absorb").toDouble());

      if (q.value("absorb").toDouble() == 1.0)
        _costsAbsorbed->setPaletteForegroundColor(QColor("black"));
      else
        _costsAbsorbed->setPaletteForegroundColor(namedColor("error"));
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (!_item->isValid())
    _bbomitem->clear();
}

void bbom::sPopulateMenu(QMenu *)
{
}
