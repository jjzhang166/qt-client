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

#include "bomItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "booItemList.h"
#include "itemSubstitute.h"
#include "storedProcErrorLookup.h"

bomItem::bomItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _substituteGroupInt = new QButtonGroup(this);
  _substituteGroupInt->addButton(_noSubstitutes);
  _substituteGroupInt->addButton(_itemDefinedSubstitutes);
  _substituteGroupInt->addButton(_bomDefinedSubstitutes);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSaveClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_item, SIGNAL(typeChanged(const QString&)), this, SLOT(sItemTypeChanged(const QString&)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_booitemList, SIGNAL(clicked()), this, SLOT(sBooitemList()));
  connect(_issueMethod, SIGNAL(activated(int)), this, SLOT(sHandleIssueMethod(int)));
  connect(_newSubstitution, SIGNAL(clicked()), this, SLOT(sNewSubstitute()));
  connect(_editSubstitution, SIGNAL(clicked()), this, SLOT(sEditSubstitute()));
  connect(_deleteSubstitution, SIGNAL(clicked()), this, SLOT(sDeleteSubstitute()));
  connect(_char, SIGNAL(activated(int)), this, SLOT(sCharIdChanged()));

#ifndef Q_WS_MAC
  _booitemList->setMaximumWidth(25);
#endif

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _scrap->setValidator(omfgThis->scrapVal());

  _bomitemsub->addColumn(tr("Rank"),        _whsColumn,  Qt::AlignCenter );
  _bomitemsub->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   );
  _bomitemsub->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _bomitemsub->addColumn(tr("Ratio"),       _qtyColumn,  Qt::AlignRight  );

  _item->setFocus();
  
  if (!_metrics->boolean("Routings"))
  {
    _usedAtLit->hide();
    _usedAt->hide();
    _booitemList->hide();
    _scheduleAtWooper->hide();
  }
  
  _saved=FALSE;
}

bomItem::~bomItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void bomItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse bomItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _bomitemid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();

  param = pParams.value("revision_id", &valid);
  if (valid)
    _revisionid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _booitemseqid = -1;

      QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
      if (issueMethod == "S")
        _issueMethod->setCurrentItem(0);
      else if (issueMethod == "L")
        _issueMethod->setCurrentItem(1);
      else if (issueMethod == "M")
        _issueMethod->setCurrentItem(2);

      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
  
      //Set up configuration tab if parent item is configured
      q.prepare("SELECT item_config "
                "FROM item "
                "WHERE (item_id=:item_id); ");
      q.bindValue(":item_id", _itemid);
      q.exec();
      if (q.first())
      {
        if (q.value("item_config").toBool())
          _char->populate(QString( "SELECT -1 AS charass_char_id, '' AS char_name "
                                   "UNION "
                                   "SELECT DISTINCT charass_char_id, char_name "
                                   "FROM charass, char "
                                   "WHERE ((charass_char_id=char_id) "
                                   "AND (charass_target_type='I') "
                                   "AND (charass_target_id= %1)) "
                                   "ORDER BY char_name; ").arg(_itemid));
        else
          _tab->removeTab(_tab->indexOf(_configurationTab));
      }

      _item->setFocus();
    }
    else if (param.toString() == "replace")
    {
      _mode = cReplace;

      _item->setId(-1);
      _dates->setStartDate(omfgThis->dbDate());
      _item->setFocus();

      _sourceBomitemid = _bomitemid;
      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _sourceBomitemid = _bomitemid;
      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _dates->setStartDate(omfgThis->dbDate());
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _qtyPer->setEnabled(FALSE);
      _scrap->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _createWo->setEnabled(FALSE);
      _issueMethod->setEnabled(FALSE);
      _uom->setEnabled(FALSE);
      _booitemList->setEnabled(FALSE);
      _scheduleAtWooper->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _ecn->setEnabled(FALSE);
      _substituteGroup->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void bomItem::sSave()
{
  if (_qtyPer->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity Per"),
                           tr("You must enter a Quantity Per value before saving this BOM Item.") );
    _qtyPer->setFocus();
    return;
  }

  if (_scrap->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Scrap Value"),
                           tr("You must enter a Scrap value before saving this BOM Item.") );
    _scrap->setFocus();
    return;
  }

  if (_mode == cNew && !_saved)
    q.prepare( "SELECT createBOMItem( :bomitem_id, :parent_item_id, :component_item_id, :issueMethod,"
               "                      :bomitem_uom_id, :qtyPer, :scrap,"
               "                      :effective, :expires,"
               "                      :createWo, :booitem_seq_id, :scheduledWithBooItem,"
               "                      :ecn, :subtype, :revision_id,"
               "                      :char_id, :value ) AS result;" );
  else if ( (_mode == cCopy) || (_mode == cReplace) )
    q.prepare( "SELECT createBOMItem( :bomitem_id, :parent_item_id, :component_item_id,"
               "                      bomitem_seqnumber, :issueMethod,"
               "                      :bomitem_uom_id, :qtyPer, :scrap,"
               "                      :effective, :expires,"
               "                      :createWo, :booitem_seq_id, :scheduledWithBooItem,"
               "                      :ecn, :subtype, :revision_id,"
               "                      :char_id, value ) AS result "
               "FROM bomitem "
               "WHERE (bomitem_id=:sourceBomitem_id);" );
  else if (_mode == cEdit  || _saved)
    q.prepare( "UPDATE bomitem "
               "SET bomitem_booitem_seq_id=:booitem_seq_id, bomitem_schedatwooper=:scheduledWithBooItem,"
               "    bomitem_qtyper=:qtyPer, bomitem_scrap=:scrap,"
               "    bomitem_effective=:effective, bomitem_expires=:expires,"
               "    bomitem_createwo=:createWo, bomitem_issuemethod=:issueMethod,"
               "    bomitem_uom_id=:bomitem_uom_id,"
               "    bomitem_ecn=:ecn, bomitem_moddate=CURRENT_DATE, bomitem_subtype=:subtype, "
               "    bomitem_char_id=:char_id, bomitem_value=:value "
               "WHERE (bomitem_id=:bomitem_id);" );
  else
//  ToDo
    return;

  q.bindValue(":bomitem_id", _bomitemid);
  q.bindValue(":sourceBomitem_id", _sourceBomitemid);
  q.bindValue(":booitem_seq_id", _booitemseqid);
  q.bindValue(":parent_item_id", _itemid);
  q.bindValue(":revision_id", _revisionid);
  q.bindValue(":component_item_id", _item->id());
  q.bindValue(":bomitem_uom_id", _uom->id());
  q.bindValue(":qtyPer", _qtyPer->toDouble());
  q.bindValue(":scrap", (_scrap->toDouble() / 100));
  q.bindValue(":effective", _dates->startDate());
  q.bindValue(":expires", _dates->endDate());
  q.bindValue(":ecn", _ecn->text());

  q.bindValue(":createWo", QVariant(_createWo->isChecked(), 0));
  q.bindValue(":scheduledWithBooItem", QVariant(_scheduleAtWooper->isChecked(), 0));

  if (_issueMethod->currentItem() == 0)
    q.bindValue(":issueMethod", "S");
  if (_issueMethod->currentItem() == 1)
    q.bindValue(":issueMethod", "L");
  if (_issueMethod->currentItem() == 2)
    q.bindValue(":issueMethod", "M");

  if (_noSubstitutes->isChecked())
    q.bindValue(":subtype", "N");
  else if (_itemDefinedSubstitutes->isChecked())
    q.bindValue(":subtype", "I");
  else if (_bomDefinedSubstitutes->isChecked())
    q.bindValue(":subtype", "B");
  q.bindValue(":revision_id", _revisionid);
  
  if (_char->id() != -1)
  {
    q.bindValue(":char_id", _char->id());
    q.bindValue(":value", _value->currentText());
  }

  q.bindValue(":configType", "N");
  q.bindValue(":configId", -1);
  q.bindValue(":configFlag", QVariant(FALSE, 0));

  q.exec();

  if ((_mode == cNew) || (_mode == cReplace) || (_mode == cCopy))
  {
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("createBOMItem", result),
		    __FILE__, __LINE__);
        _item->setFocus();
	return;
      }

      if (_mode == cReplace)
      {
        q.prepare( "UPDATE bomitem "
                   "SET bomitem_expires=:bomitem_expires "
                   "WHERE (bomitem_id=:bomitem_id)" );
        q.bindValue(":bomitem_expires", _dates->startDate());
        q.bindValue(":bomitem_id", _sourceBomitemid);
        q.exec();
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sBOMsUpdated(_itemid, TRUE);
  
  _saved=TRUE;
}

void bomItem::sSaveClick()
{
  sSave();
  if (_saved)
    done(_bomitemid);
}

void bomItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM bomitemsub "
               "WHERE (bomitemsub_bomitem_id=:bomitem_id);" );
    q.bindValue("bomitem_id", _bomitemid);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void bomItem::sItemTypeChanged(const QString &type)
{
  if (type == "M")
    _createWo->setEnabled(TRUE);
  else
  {
    _createWo->setEnabled(FALSE);
    _createWo->setChecked(FALSE);
  }
}

void bomItem::sBooitemList()
{
  ParameterList params;
  params.append("item_id", _itemid);
  params.append("booitem_seq_id", _booitemseqid);

  booItemList newdlg(this, "", TRUE);
  newdlg.set(params);
  _booitemseqid = newdlg.exec();

  if (_booitemseqid != -1)
  {
    q.prepare( "SELECT (TEXT(booitem_seqnumber) || '-' || booitem_descrip1 || ' ' || booitem_descrip2) AS description "
		       "FROM booitem(:item_id) "
               "WHERE (booitem_seq_id=:booitem_seq_id);" );
	q.bindValue(":item_id", _itemid);
    q.bindValue(":booitem_seq_id", _booitemseqid);
    q.exec();
    if (q.first())
    {
      _usedAt->setText(q.value("description").toString());
      _scheduleAtWooper->setEnabled(TRUE);
    }
    else
    {
      _scheduleAtWooper->setEnabled(FALSE);
      _scheduleAtWooper->setChecked(FALSE);
    }
  }
  else
  {
    _usedAt->clear();
    _scheduleAtWooper->setEnabled(FALSE);
    _scheduleAtWooper->setChecked(FALSE);
  }
}

void bomItem::populate()
{
  q.prepare( "SELECT bomitem_item_id, bomitem_parent_item_id, item_config,"
             "       bomitem_booitem_seq_id, bomitem_createwo, bomitem_issuemethod,"
             "       bomitem_schedatwooper, bomitem_ecn, item_type,"
             "       formatQtyper(bomitem_qtyper) AS qtyper,"
             "       formatScrap(bomitem_scrap) AS scrap,"
             "       bomitem_effective, bomitem_expires, bomitem_subtype,"
             "       bomitem_uom_id, "
             "       bomitem_char_id, "
             "       bomitem_value "
             "FROM bomitem, item "
             "WHERE ( (bomitem_parent_item_id=item_id)"
             " AND (bomitem_id=:bomitem_id) );" );
  q.bindValue(":bomitem_id", _bomitemid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("bomitem_parent_item_id").toInt();
    _item->setId(q.value("bomitem_item_id").toInt());
    _uom->setId(q.value("bomitem_uom_id").toInt());

    if (q.value("bomitem_issuemethod").toString() == "S")
      _issueMethod->setCurrentItem(0);
    else if (q.value("bomitem_issuemethod").toString() == "L")
      _issueMethod->setCurrentItem(1);
    else if (q.value("bomitem_issuemethod").toString() == "M")
      _issueMethod->setCurrentItem(2);
    sHandleIssueMethod(_issueMethod->currentItem());

    if (q.value("item_type").toString() == "M" || q.value("item_type").toString() == "F")
      _createWo->setChecked(q.value("bomitem_createwo").toBool());

    _dates->setStartDate(q.value("bomitem_effective").toDate());
    _dates->setEndDate(q.value("bomitem_expires").toDate());
    _qtyPer->setText(q.value("qtyper").toString());
    _scrap->setText(q.value("scrap").toString());

    if (_mode != cCopy)
      _ecn->setText(q.value("bomitem_ecn").toString());

    bool scheduledAtWooper = q.value("bomitem_schedatwooper").toBool();
    _booitemseqid = q.value("bomitem_booitem_seq_id").toInt();

    _comments->setId(_bomitemid);

    if (q.value("item_type").toString() == "M" || 
        q.value("item_type").toString() == "F" || 
        q.value("item_type").toString() == "J")
      _createWo->setChecked(q.value("bomitem_createwo").toBool());
      
    if (q.value("item_config").toBool())
    {
      _char->populate(QString( "SELECT -1 AS charass_char_id, '' AS char_name "
                               "UNION "
                               "SELECT DISTINCT charass_char_id, char_name "
                               "FROM charass, char "
                               "WHERE ((charass_char_id=char_id) "
                               "AND (charass_target_type='I') "
                               "AND (charass_target_id= %1)) "
                               "ORDER BY char_name; ").arg(_itemid));
      _char->setId(q.value("bomitem_char_id").toInt());
      sCharIdChanged();
      _value->setText(q.value("bomitem_value").toString());
    }
    else
      _tab->removeTab(_tab->indexOf(_configurationTab));

    if (q.value("bomitem_subtype").toString() == "I")
      _itemDefinedSubstitutes->setChecked(true);
    else if (q.value("bomitem_subtype").toString() == "B")
      _bomDefinedSubstitutes->setChecked(true);
    else
      _noSubstitutes->setChecked(true);
    sFillSubstituteList();

    if (_booitemseqid != -1)
    {
      q.prepare( "SELECT (TEXT(booitem_seqnumber) || '-' || booitem_descrip1 || ' ' || booitem_descrip2) AS description "
		         "FROM booitem(:item_id) "
                 "WHERE (booitem_seq_id=:booitem_seq_id);" );
	  q.bindValue(":item_id", _itemid);
      q.bindValue(":booitem_seq_id", _booitemseqid);
      q.exec();
      if (q.first())
      {
        _usedAt->setText(q.value("description").toString());
        _scheduleAtWooper->setEnabled(TRUE);
        _scheduleAtWooper->setChecked(scheduledAtWooper);
      }
      else
      {
        _booitemseqid = -1;
        _scheduleAtWooper->setEnabled(FALSE);
      }
    }
    else
      _scheduleAtWooper->setEnabled(FALSE);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bomItem::sHandleIssueMethod(int pItem)
{
  switch (pItem)
  {
    case 1:
    case 2:
      _booitemList->setEnabled(TRUE);
      break;

    default:
      _booitemList->setEnabled(FALSE);
      break;
  }
}

void bomItem::sNewSubstitute()
{
  sSave();
  ParameterList params;
  params.append("mode", "new");
  params.append("bomitem_id", _bomitemid);

  if (_mode == cNew)
    params.append("bomitem_item_id", _item->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sEditSubstitute()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitemsub_id", _bomitemsub->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sDeleteSubstitute()
{
  q.prepare( "DELETE FROM bomitemsub "
             "WHERE (bomitemsub_id=:bomitemsub_id);" );
  q.bindValue(":bomitemsub_id", _bomitemsub->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillSubstituteList();
}

void bomItem::sFillSubstituteList()
{
  q.prepare( "SELECT bomitemsub_id, bomitemsub_rank, item_number, item_descrip1,"
             "       formatRatio(bomitemsub_uomratio) "
             "FROM bomitemsub, item "
             "WHERE ( (bomitemsub_item_id=item_id)"
             " AND (bomitemsub_bomitem_id=:bomitem_id) ) "
             "ORDER BY bomitemsub_rank, item_number" );
  q.bindValue(":bomitem_id", _bomitemid);
  q.exec();
  _bomitemsub->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bomItem::sItemIdChanged()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id),"
              "  itemuom, uomtype "
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id) "
              "   AND (itemuom_itemuomconv_id=itemuomconv_id) "
              "   AND (uomtype_id=itemuom_uomtype_id) "
              "   AND (uomtype_name='MaterialIssue'))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id),"
              "  itemuom, uomtype "
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id) "
              "   AND (itemuom_itemuomconv_id=itemuomconv_id) "
              "   AND (uomtype_id=itemuom_uomtype_id) "
              "   AND (uomtype_name='MaterialIssue'))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _uom->populate(uom);
  uom.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if(uom.first())
    _uom->setId(uom.value("item_inv_uom_id").toInt());
}

void bomItem::sCharIdChanged()
{
  XSqlQuery charass;
  charass.prepare("SELECT charass_id, charass_value "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (COALESCE(charass_value,'')!='')); ");
  charass.bindValue(":item_id", _itemid);
  charass.bindValue(":char_id", _char->id());
  charass.exec();
  _value->populate(charass);
}



