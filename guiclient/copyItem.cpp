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

#include "copyItem.h"

#include <QMessageBox>
#include <QVariant>

#include "itemSite.h"

copyItem::copyItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_source, SIGNAL(typeChanged(const QString&)), this, SLOT(sHandleItemType(const QString&)));

  _captive = FALSE;
}

copyItem::~copyItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void copyItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyItem::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(FALSE);
    _targetItemNumber->setFocus();
  }

  return NoError;
}

void copyItem::sHandleItemType(const QString &pItemType)
{
  if ((pItemType == "M") || (pItemType == "F") || (pItemType == "J"))
  {
    _copyBOM->setChecked(TRUE);
    _copyBOM->setEnabled(TRUE);
    if (_metrics->boolean("Routings"))
    {
      _copyBOO->setChecked(TRUE);
      _copyUsedAt->setChecked(TRUE);
      _copyBOO->setEnabled(TRUE);
      _copyUsedAt->setEnabled(TRUE);
    }
    else
    {
      _copyBOO->setChecked(FALSE);
      _copyUsedAt->setChecked(FALSE);
      _copyBOO->hide();
      _copyUsedAt->hide();
    }
  }
  else
  {
    _copyBOM->setChecked(FALSE);
    _copyBOM->setEnabled(FALSE);
    _copyBOO->setChecked(FALSE);
    _copyUsedAt->setChecked(FALSE);
    if (_metrics->boolean("Routings"))
    {
      _copyBOO->setEnabled(FALSE);
      _copyUsedAt->setEnabled(FALSE);
    }
    else
    {
      _copyBOO->hide();
      _copyUsedAt->hide();
    }
  }
  if(pItemType == "L")
  {
    _copyBOM->setChecked(TRUE);
    _copyBOM->setEnabled(TRUE);
  }
}

void copyItem::sCopy()
{
  _targetItemNumber->setText(_targetItemNumber->text().stripWhiteSpace().upper());

  if (_targetItemNumber->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr( "You must enter a valid target Item Number before\n"
                              "attempting to copy the selected Item.\n"
                              "Please enter a valid Item Number before continuing.") );
    _targetItemNumber->setFocus();
    return;
  }

  q.prepare( "SELECT item_number "
             "FROM item "
             "WHERE item_number=:item_number;" );
  q.bindValue(":item_number", _targetItemNumber->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(  this, tr("Item Number Exists"),
                            tr( "An Item with the item number '%1' already exists.\n"
                                "You may not copy over an existing item." )
                            .arg(_targetItemNumber->text()) );

    _targetItemNumber->clear();
    _targetItemNumber->setFocus();
    return;
  }

  int itemid = -1;

  q.prepare("SELECT copyItem(:source_item_id, :newItemNumber, :copyBOM, :copyBOO, :copyItemCosts, :copyUsedAt) AS itemid;");
  q.bindValue(":source_item_id", _source->id());
  q.bindValue(":newItemNumber", _targetItemNumber->text());
  q.bindValue(":copyBOM", QVariant(_copyBOM->isChecked(), 0));
  q.bindValue(":copyBOO", QVariant(_copyBOO->isChecked(), 0));
  q.bindValue(":copyItemCosts", QVariant(_copyCosts->isChecked(), 0));
  q.bindValue(":copyUsedAt", QVariant(_copyUsedAt->isChecked(), 0));
  q.exec();
  if (q.first())
  {
    itemid = q.value("itemid").toInt();

    omfgThis->sItemsUpdated(itemid, TRUE);

    if (_copyBOM->isChecked())
      omfgThis->sBOMsUpdated(itemid, TRUE);

    if (_copyBOO->isChecked())
      omfgThis->sBOOsUpdated(itemid, TRUE);

    if (QMessageBox::information( this, tr("Create New Item Sites"),
                                  tr("Would you like to create new Item Sites for the newly created Item?"),
                                  tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
    {
      ParameterList params;
      params.append("mode", "new");
      params.append("item_id", itemid);
      
      itemSite newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
  }
//  ToDo

  if (_captive)
    done(itemid);
  else
  {
    _source->setId(-1);
    _targetItemNumber->clear();
    _source->setFocus();
    _copyBOM->setEnabled(TRUE);
    _copyBOO->setEnabled(TRUE);
    _copyUsedAt->setEnabled(TRUE);
    _close->setText(tr("&Close"));
  }
}

