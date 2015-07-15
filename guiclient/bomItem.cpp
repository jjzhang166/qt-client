/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bomItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "itemSubstitute.h"
#include "itemCost.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

const char *_issueMethods[] = { "S", "L", "M" };

bomItem::bomItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _substituteGroupInt = new QButtonGroup(this);
  _substituteGroupInt->addButton(_noSubstitutes);
  _substituteGroupInt->addButton(_itemDefinedSubstitutes);
  _substituteGroupInt->addButton(_bomDefinedSubstitutes);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSaveClick()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_item, SIGNAL(typeChanged(const QString&)), this, SLOT(sItemTypeChanged(const QString&)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_newSubstitution, SIGNAL(clicked()), this, SLOT(sNewSubstitute()));
  connect(_editSubstitution, SIGNAL(clicked()), this, SLOT(sEditSubstitute()));
  connect(_deleteSubstitution, SIGNAL(clicked()), this, SLOT(sDeleteSubstitute()));
  connect(_itemcost, SIGNAL(itemSelectionChanged()), this, SLOT(sCostSelectionChanged()));
  connect(_newCost, SIGNAL(clicked()), this, SLOT(sNewCost()));
  connect(_editCost, SIGNAL(clicked()), this, SLOT(sEditCost()));
  connect(_deleteCost, SIGNAL(clicked()), this, SLOT(sDeleteCost()));
  connect(_char, SIGNAL(activated(int)), this, SLOT(sCharIdChanged()));

  _item->setType(ItemLineEdit::cGeneralComponents);

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), true);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), true);
  _dates->setEndCaption(tr("Expires"));

  _qtyFxd->setValidator(omfgThis->qtyVal());
  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _scrap->setValidator(omfgThis->scrapVal());

  _bomitemsub->addColumn(tr("Rank"),        _whsColumn,  Qt::AlignCenter, true, "bomitemsub_rank");
  _bomitemsub->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _bomitemsub->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, 
		  "item_descrip1");
  _bomitemsub->addColumn(tr("Ratio"),       _qtyColumn,  Qt::AlignRight, true, "bomitemsub_uomratio");

  _itemcost->addColumn(tr("Element"),     -1,           Qt::AlignLeft,   true, "costelem_type");
  _itemcost->addColumn(tr("Lower"),       _costColumn,  Qt::AlignCenter, true, "itemcost_lowlevel" );
  _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_stdcost"  );
  _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "baseCurr" );
  _itemcost->addColumn(tr("Posted"),      _dateColumn,  Qt::AlignCenter, true, "itemcost_posted" );
  _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_actcost"  );
  _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "costCurr" );
  _itemcost->addColumn(tr("Updated"),     _dateColumn,  Qt::AlignCenter, true, "itemcost_updated" );

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn(3);
      _itemcost->hideColumn(6);
  }

  _parentitemid=0;
  _bomheadid=0;
  _saved=false;
  adjustSize();
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
  XSqlQuery bomet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _bomitemid = param.toInt();
    populate();
  }

  param = pParams.value("bomhead_id", &valid);
  if (valid)
    _bomheadid = param.toInt();
  
  param = pParams.value("item_id", &valid);
  if (valid)
    _parentitemid = param.toInt();

  param = pParams.value("revision_id", &valid);
  if (valid)
    _revisionid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if (!_metrics->boolean("AllowInactiveBomItems"))
        _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

      QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
      for (int counter = 0; counter < _issueMethod->count(); counter++)
      {
        if (issueMethod == _issueMethods[counter])
        {
          _issueMethod->setCurrentIndex(counter);
        }
      }

      bomet.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (bomet.first())
        _bomitemid = bomet.value("bomitem_id").toInt();
      else if (bomet.lastError().type() != QSqlError::NoError)
      {
	systemError(this, bomet.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
  
      //Set up configuration tab if parent item is configured or kit
      bomet.prepare("SELECT item_config, item_type "
                "FROM item "
                "WHERE (item_id=:item_id); ");
      bomet.bindValue(":item_id", _parentitemid);
      bomet.exec();
      if (bomet.first())
      {
        if (bomet.value("item_config").toBool())
        {
          MetaSQLQuery mql = mqlLoad("charass", "populate");

          ParameterList params;
          params.append("name", true);
          params.append("type", "I");
          params.append("id", _parentitemid);

          XSqlQuery qry = mql.toQuery(params);
          _char->populate(qry);
        }
        else
          _tab->removeTab(_tab->indexOf(_configurationTab));
          
        if (bomet.value("item_type").toString() == "K")
        {
          if (_metrics->boolean("AllowInactiveBomItems"))
            _item->setType(ItemLineEdit::cKitComponents);
          else
            _item->setType(ItemLineEdit::cKitComponents | ItemLineEdit::cActive);
        }
      }

      connect(_bomDefinedCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleBomitemCost()));
    }
    else if (param.toString() == "replace")
    {
      _mode = cReplace;

      _item->setId(-1);
      _dates->setStartDate(omfgThis->dbDate());

      _sourceBomitemid = _bomitemid;
      bomet.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (bomet.first())
        _bomitemid = bomet.value("bomitem_id").toInt();
      else if (bomet.lastError().type() != QSqlError::NoError)
      {
	systemError(this, bomet.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(true);
      connect(_bomDefinedCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleBomitemCost()));
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _sourceBomitemid = _bomitemid;
      bomet.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (bomet.first())
        _bomitemid = bomet.value("bomitem_id").toInt();
      else if (bomet.lastError().type() != QSqlError::NoError)
      {
	systemError(this, bomet.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _dates->setStartDate(omfgThis->dbDate());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _qtyFxd->setEnabled(false);
      _qtyPer->setEnabled(false);
      _scrap->setEnabled(false);
      _dates->setEnabled(false);
      _createWo->setEnabled(false);
      _issueWo->setEnabled(false);
      _issueMethod->setEnabled(false);
      _uom->setEnabled(false);
      _comments->setReadOnly(true);
      _ecn->setEnabled(false);
      _substituteGroup->setEnabled(false);
      _notes->setEnabled(false);
      _ref->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);

      _newCost->setEnabled(false);
      _deleteCost->setEnabled(false);
      _editCost->setText("&View");

      connect(_bomDefinedCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleBomitemCost()));
    }
  }

  // Check the parent item type and if it is a Kit then change some widgets
  bomet.prepare("SELECT item_type "
            "FROM item "
            "WHERE (item_id=:item_id); ");
  bomet.bindValue(":item_id", _parentitemid);
  bomet.exec();
  if (bomet.first() && (bomet.value("item_type").toString() == "K"))
  {
    _createWo->setChecked(false);
    _createWo->setEnabled(false);
    _issueMethod->setCurrentIndex(0);
    _issueMethod->setEnabled(false);
    _tab->setTabEnabled(_tab->indexOf(_substitutionsTab), false);
  }
  
  return NoError;
}

void bomItem::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_qtyPer->toDouble() == 0.0 && _qtyFxd->toDouble() == 0.0, _qtyPer,
                          tr("You must enter a Quantity Per value before saving this BOM Item."))
         << GuiErrorCheck(_scrap->text().length() == 0, _scrap,
                          tr("You must enter a Scrap value before saving this BOM Item."))
         << GuiErrorCheck(_dates->endDate() < _dates->startDate(), _dates,
                          tr("The expiration date cannot be earlier than the effective date."))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Bill of Material Item"), errors))
    return;

  // Check the component item type and if it is a Reference then issue a warning
//  if (_item->type() == ItemLineEdit::cReference)
  XSqlQuery itemtype;
  itemtype.prepare("SELECT item_type FROM item WHERE (item_id=:item_id); ");
  itemtype.bindValue(":item_id", _item->id());
  itemtype.exec();
  if (itemtype.first() && itemtype.value("item_type").toString() == "R")
  {
    int answer = QMessageBox::question(this, tr("Reference Item"),
                            tr("<p>Adding a Reference Item to a Bill of Material "
                               "may cause W/O variances. <p> "
                               "OK to continue? "),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No);
    if (answer == QMessageBox::No)
      return;
  }

  XSqlQuery bomitem;
  if ( (_mode == cNew && !_saved) || (_mode == cCopy) || (_mode == cReplace) )
    bomitem.prepare( "INSERT INTO bomitem"
                     " ( bomitem_id, bomitem_parent_item_id, bomitem_seqnumber,"
                     "   bomitem_item_id, bomitem_qtyper, bomitem_scrap,"
                     "   bomitem_status, bomitem_effective, bomitem_expires,"
                     "   bomitem_createwo, bomitem_issuemethod, bomitem_schedatwooper,"
                     "   bomitem_ecn, bomitem_moddate, bomitem_subtype,"
                     "   bomitem_uom_id, bomitem_rev_id, bomitem_booitem_seq_id,"
                     "   bomitem_char_id, bomitem_value, bomitem_notes,"
                     "   bomitem_ref, bomitem_qtyfxd, bomitem_issuewo )"
                     " VALUES"
                     " ( :bomitem_id, :bomitem_parent_item_id, :bomitem_seqnumber,"
                     "   :bomitem_item_id, :bomitem_qtyper, :bomitem_scrap,"
                     "   :bomitem_status, :bomitem_effective, :bomitem_expires,"
                     "   :bomitem_createwo, :bomitem_issuemethod, :bomitem_schedatwooper,"
                     "   :bomitem_ecn, :bomitem_moddate, :bomitem_subtype,"
                     "   :bomitem_uom_id, :bomitem_rev_id, :bomitem_booitem_seq_id,"
                     "   :bomitem_char_id, :bomitem_value, :bomitem_notes,"
                     "   :bomitem_ref, :bomitem_qtyfxd, :bomitem_issuewo )"
                     ";" );
  else if (_mode == cEdit  || _saved)
    bomitem.prepare( "UPDATE bomitem "
                     "SET bomitem_qtyfxd=:bomitem_qtyfxd, bomitem_qtyper=:bomitem_qtyper,"
                     "    bomitem_scrap=:bomitem_scrap, bomitem_effective=:bomitem_effective,"
                     "    bomitem_expires=:bomitem_expires, bomitem_createwo=:bomitem_createwo,"
                     "    bomitem_issuewo=:bomitem_issuewo, bomitem_issuemethod=:bomitem_issuemethod,"
                     "    bomitem_uom_id=:bomitem_uom_id, bomitem_ecn=:bomitem_ecn,"
                     "    bomitem_moddate=CURRENT_DATE, bomitem_subtype=:bomitem_subtype,"
                     "    bomitem_char_id=:bomitem_char_id, bomitem_value=:bomitem_value,"
                     "    bomitem_notes=:bomitem_notes, bomitem_ref=:bomitem_ref "
                     "WHERE (bomitem_id=:bomitem_id);" );
  else
//  ToDo
    return;

  bomitem.bindValue(":bomitem_id", _bomitemid);
  bomitem.bindValue(":bomitem_parent_item_id", _parentitemid);
  bomitem.bindValue(":bomitem_rev_id", _revisionid);
  bomitem.bindValue(":bomitem_item_id", _item->id());
  bomitem.bindValue(":bomitem_uom_id", _uom->id());
  bomitem.bindValue(":bomitem_qtyfxd", _qtyFxd->toDouble());
  bomitem.bindValue(":bomitem_qtyper", _qtyPer->toDouble());
  bomitem.bindValue(":bomitem_scrap", (_scrap->toDouble() / 100));
  bomitem.bindValue(":bomitem_effective", _dates->startDate());
  bomitem.bindValue(":bomitem_expires", _dates->endDate());
  bomitem.bindValue(":bomitem_ecn", _ecn->text());
  bomitem.bindValue(":bomitem_notes",	_notes->toPlainText());
  bomitem.bindValue(":bomitem_ref",   _ref->toPlainText());

  bomitem.bindValue(":bomitem_createwo", QVariant(_createWo->isChecked()));
  bomitem.bindValue(":bomitem_issuewo", QVariant(_createWo->isChecked() && _issueWo->isChecked()));
  bomitem.bindValue(":bomitem_issuemethod", _issueMethods[_issueMethod->currentIndex()]);
  bomitem.bindValue(":bomitem_schedatwooper", false);

  if (_noSubstitutes->isChecked())
    bomitem.bindValue(":bomitem_subtype", "N");
  else if (_itemDefinedSubstitutes->isChecked())
    bomitem.bindValue(":bomitem_subtype", "I");
  else if (_bomDefinedSubstitutes->isChecked())
    bomitem.bindValue(":bomitem_subtype", "B");
  
  if (_char->id() != -1)
  {
    bomitem.bindValue(":bomitem_char_id", _char->id());
    bomitem.bindValue(":bomitem_value", _value->currentText());
  }

//  Not used?
//  bomitem.bindValue(":configType", "N");
//  bomitem.bindValue(":configId", -1);
//  bomitem.bindValue(":configFlag", QVariant(false));

  bomitem.exec();
  if (bomitem.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bomitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cReplace || _mode == cCopy)
  {
    // update the sequence number and revision of the new bomitemitem
    XSqlQuery replace;
    replace.prepare( "UPDATE bomitem "
                     "SET bomitem_seqnumber=(SELECT bomitem_seqnumber"
                     "                       FROM bomitem"
                     "                       WHERE bomitem_id=:sourcebomitemid), "
                     "    bomitem_rev_id=(SELECT bomitem_rev_id"
                     "                       FROM bomitem"
                     "                       WHERE bomitem_id=:sourcebomitemid) "
                     "WHERE (bomitem_id=:bomitem_id);" );
    replace.bindValue(":sourcebomitemid", _sourceBomitemid);
    replace.bindValue(":bomitem_id", _bomitemid);
    replace.exec();
    if (replace.lastError().type() != QSqlError::NoError)
    {
      systemError(this, replace.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cReplace)
  {
    // update the expiration date of the source item
    XSqlQuery replace;
    replace.prepare( "UPDATE bomitem "
                     "SET bomitem_expires=:bomitem_expires "
                     "WHERE (bomitem_id=:sourcebomitemid);" );
    replace.bindValue(":bomitem_expires", _dates->startDate());
    replace.bindValue(":sourcebomitemid", _sourceBomitemid);
    replace.exec();
    if (replace.lastError().type() != QSqlError::NoError)
    {
      systemError(this, replace.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sBOMsUpdated(_parentitemid, true);
  
  emit saved(_bomitemid);

  _saved=true;
}

void bomItem::sSaveClick()
{
  sSave();
  if (_saved)
    done(_bomitemid);
}

void bomItem::sClose()
{
  XSqlQuery bomClose;
  if (_mode == cNew)
  {
    bomClose.prepare( "DELETE FROM bomitemsub "
               "WHERE (bomitemsub_bomitem_id=:bomitem_id);"
               "DELETE FROM bomitemcost "
               "WHERE (bomitemcost_bomitem_id=:bomitem_id);" );
    bomClose.bindValue("bomitem_id", _bomitemid);
    bomClose.exec();
    if (bomClose.lastError().type() != QSqlError::NoError)
    {
      systemError(this, bomClose.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void bomItem::sItemTypeChanged(const QString &type)
{
  if (type == "M")
    _createWo->setEnabled(true);
  else
  {
    _createWo->setEnabled(false);
    _createWo->setChecked(false);
  }
}

void bomItem::populate()
{
  XSqlQuery qbomitem;
  qbomitem.prepare("SELECT bomitem.*, item_type, item_config"
                   "  FROM bomitem JOIN item ON (item_id=bomitem_parent_item_id) "
                   " WHERE (bomitem_id=:bomitem_id);" );
  qbomitem.bindValue(":bomitem_id", _bomitemid);
  qbomitem.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting BOM Item"),
                           qbomitem, __FILE__, __LINE__))
    return;

  if (qbomitem.first())
  {
    _parentitemid = qbomitem.value("bomitem_parent_item_id").toInt();
    _parentitemtype = qbomitem.value("item_type").toString();
    _item->setId(qbomitem.value("bomitem_item_id").toInt());
    _uom->setId(qbomitem.value("bomitem_uom_id").toInt());
    _notes->setText(qbomitem.value("bomitem_notes").toString());
    _ref->setText(qbomitem.value("bomitem_ref").toString());

    for (int counter = 0; counter < _issueMethod->count(); counter++)
    {
      if (QString(qbomitem.value("bomitem_issuemethod").toString()[0]) == _issueMethods[counter])
      {
        _issueMethod->setCurrentIndex(counter);
      }
    }

    if (_parentitemtype == "M" || _parentitemtype == "F")
    {
      _createWo->setChecked(qbomitem.value("bomitem_createwo").toBool());
      _issueWo->setChecked(qbomitem.value("bomitem_issuewo").toBool());
    }

    _dates->setStartDate(qbomitem.value("bomitem_effective").toDate());
    _dates->setEndDate(qbomitem.value("bomitem_expires").toDate());
    _qtyFxd->setDouble(qbomitem.value("bomitem_qtyfxd").toDouble());
    _qtyPer->setDouble(qbomitem.value("bomitem_qtyper").toDouble());
    _scrap->setDouble(qbomitem.value("bomitem_scrap").toDouble() * 100);

    if (_mode != cCopy)
      _ecn->setText(qbomitem.value("bomitem_ecn").toString());

    _comments->setId(_bomitemid);

    if (qbomitem.value("item_config").toBool())
    {
      MetaSQLQuery mql = mqlLoad("charass", "populate");

      ParameterList params;
      params.append("name", true);
      params.append("type", "I");
      params.append("id", _parentitemid);

      XSqlQuery qry = mql.toQuery(params);
      _char->populate(qry);
      _char->setId(qbomitem.value("bomitem_char_id").toInt());
      sCharIdChanged();
      _value->setText(qbomitem.value("bomitem_value").toString());
    }
    else
      _tab->removeTab(_tab->indexOf(_configurationTab));

    if (qbomitem.value("bomitem_subtype").toString() == "I")
      _itemDefinedSubstitutes->setChecked(true);
    else if (qbomitem.value("bomitem_subtype").toString() == "B")
      _bomDefinedSubstitutes->setChecked(true);
    else
      _noSubstitutes->setChecked(true);

    sFillSubstituteList();
    sFillCostList();
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

  itemSubstitute newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sEditSubstitute()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitemsub_id", _bomitemsub->id());

  itemSubstitute newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sDeleteSubstitute()
{
  XSqlQuery bomDeleteSubstitute;
  bomDeleteSubstitute.prepare( "DELETE FROM bomitemsub "
             "WHERE (bomitemsub_id=:bomitemsub_id);" );
  bomDeleteSubstitute.bindValue(":bomitemsub_id", _bomitemsub->id());
  bomDeleteSubstitute.exec();
  if (bomDeleteSubstitute.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bomDeleteSubstitute.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillSubstituteList();
}

void bomItem::sFillSubstituteList()
{
  XSqlQuery bomFillSubstituteList;
  bomFillSubstituteList.prepare( "SELECT bomitemsub.*, item_number, item_descrip1,"
             "       'ratio' AS bomitemsub_uomratio_xtnumericrole "
             "FROM bomitemsub, item "
             "WHERE ( (bomitemsub_item_id=item_id)"
             " AND (bomitemsub_bomitem_id=:bomitem_id) ) "
             "ORDER BY bomitemsub_rank, item_number" );
  bomFillSubstituteList.bindValue(":bomitem_id", _bomitemid);
  bomFillSubstituteList.exec();
  _bomitemsub->populate(bomFillSubstituteList);
  if (bomFillSubstituteList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, bomFillSubstituteList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bomItem::sItemIdChanged()
{
// Prevent job costed items from being added to the BOM
  XSqlQuery qry;
  qry.prepare("SELECT EXISTS(SELECT 1 FROM itemsite WHERE itemsite_item_id = :item_id AND itemsite_costmethod = 'J')");
  qry.bindValue(":item_id", _item->id());
  qry.exec();
  if (qry.first() && qry.value("exists").toBool())
  {
    QMessageBox::question(this, tr("Job Costed Item"),
                            tr("You cannot add a Job Costed item to a Bill of Material"));
    _item->setId(-1);
    return;
  }
  else if (qry.lastError().type() != QSqlError::NoError)
      ErrorReporter::error(QtCriticalMsg, this, tr("Item Job cost check"),
                           qry, __FILE__, __LINE__);

  MetaSQLQuery muom = mqlLoad("uoms", "item");

  ParameterList params;
  params.append("uomtype", "MaterialIssue");
  params.append("item_id", _item->id());

  XSqlQuery quom = muom.toQuery(params);
  _uom->populate(quom);

  XSqlQuery qitem;
  qitem.prepare("SELECT item_inv_uom_id, item_type "
                          "  FROM item"
                          " WHERE(item_id=:item_id);");
  qitem.bindValue(":item_id", _item->id());
  qitem.exec();
  if(qitem.first())
  {
    _uom->setId(qitem.value("item_inv_uom_id").toInt());
    if (qitem.value("item_type").toString() != "T" && qitem.value("item_type").toString() != "R")
    {
      if (_qtyPer->text().length() == 0)
      {
        _qtyFxd->setDouble(0.0);
        _qtyPer->setDouble(1.0);
      }
    }
    else if (_qtyPer->text().length() == 0)
    {
      _qtyFxd->setDouble(1.0);
      _qtyPer->setDouble(0.0);
    }
    if (_scrap->text().length() == 0)
      _scrap->setDouble(0.0);
  }
}

void bomItem::sCharIdChanged()
{
  MetaSQLQuery mql = mqlLoad("charass", "populate");

  ParameterList params;
  params.append("value", true);
  params.append("char", _char->id());
  params.append("type", "I");
  params.append("id", _parentitemid);

  XSqlQuery qry = mql.toQuery(params);
  _value->populate(qry);
}

void bomItem::sFillCostList()
{
  if (_item->isValid() && _bomitemid > 0)
  {
    _itemcost->clear();
    double standardCost = 0.0;
    double actualCostBase = 0.0;
    double actualCostLocal = 0.0;

    MetaSQLQuery mql = mqlLoad("itemCost", "list");

    ParameterList params;
    params.append("error", tr("!ERROR!"));
    params.append("never", tr("Never"));
    params.append("bomitem_id", _bomitemid);

    XSqlQuery qry = mql.toQuery(params);
    if (qry.first())
    {
      _bomDefinedCosts->setChecked(true);
      if (_privileges->check("CreateCosts") && _mode != cView)
        _newCost->setEnabled(true);
    }
    else
    {
      params.append("item_id", _item->id());
      qry = mql.toQuery(params);
      _bomDefinedCosts->setChecked(false);
      _newCost->setEnabled(false);
    }

    _itemcost->populate(qry, true);
    if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    bool multipleCurrencies = false;
    int firstCurrency = 0;
    bool baseKnown = true;
    if (qry.first())
    {
      firstCurrency = qry.value("itemcost_curr_id").toInt();
      do
      {
        standardCost += qry.value("itemcost_stdcost").toDouble();
        if (qry.value("itemcost_actcost").isNull())
            baseKnown = false;
        else
            actualCostBase += qry.value("actcostBase").toDouble();
        actualCostLocal += qry.value("itemcost_actcost").toDouble();
        if (! multipleCurrencies &&
            qry.value("itemcost_curr_id").toInt() != firstCurrency)
            multipleCurrencies = true;
      }
      while (qry.next());
    }

    XSqlQuery convert;
    double actualCost = 0;
    if (multipleCurrencies)
    {
        actualCost = actualCostBase;
        convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
                        "       currConcat(baseCurrId()) AS currConcat;");
    }
    else
    {
        actualCost = actualCostLocal;
        baseKnown = true; // not necessarily but we can trust actualCost
        convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
                        "	currConcat(:firstCurrency) AS currConcat;" );
        convert.bindValue(":firstCurrency", firstCurrency);
    }
    convert.exec();
    if (convert.first())
        new XTreeWidgetItem(_itemcost,
                            _itemcost->topLevelItem(_itemcost->topLevelItemCount() - 1), -1,
                            QVariant(tr("Totals")),
                            "",
                            formatCost(standardCost),
                            convert.value("baseConcat"),
                            "",
                            baseKnown ? formatCost(actualCost) : tr("?????"),
                            convert.value("currConcat"));
    else if (convert.lastError().type() != QSqlError::NoError)
        systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);

  }
  else
    _itemcost->clear();
}

void bomItem::sHandleBomitemCost()
{
  XSqlQuery qry;
  qry.prepare("SELECT toggleBomitemCost(:bomitem_id, :enabled);");
  qry.bindValue(":bomitem_id", _bomitemid);
  qry.bindValue(":enabled", _bomDefinedCosts->isChecked());
  qry.exec();
  if (qry.lastError().type() != QSqlError::NoError)
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
  sFillCostList();
}

void bomItem::sCostSelectionChanged()
{
  bool yes = (_itemcost->id() != -1);

  if (_privileges->check("EnterActualCosts"))
    _editCost->setEnabled(yes);

  if (_privileges->check("DeleteCosts") && _mode != cView)
    _deleteCost->setEnabled(yes);
}

void bomItem::sNewCost()
{
  ParameterList params;
  params.append("bomitem_id", _bomitemid);
  params.append("mode", "new");

  if (_mode == cNew)
    params.append("bomitem_item_id", _item->id());

  itemCost newdlg(this, "", true);
  if (newdlg.set(params) == NoError && newdlg.exec())
    sFillCostList();
}

void bomItem::sEditCost()
{
  ParameterList params;
  params.append("bomitemcost_id", _itemcost->id());
  if (_mode == cEdit)
    params.append("mode", "edit");
  if (_mode == cView)
    params.append("mode", "view");

  itemCost newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec())
    sFillCostList();
}

void bomItem::sDeleteCost()
{
  XSqlQuery bomDeleteCost;
  bomDeleteCost.prepare( "DELETE FROM bomitemcost WHERE (bomitemcost_id=:bomitemcost_id);" );
  bomDeleteCost.bindValue(":bomitemcost_id", _itemcost->id());
  bomDeleteCost.exec();
  if (bomDeleteCost.lastError().type() != QSqlError::NoError)
    systemError(this, bomDeleteCost.lastError().databaseText(), __FILE__, __LINE__);

  sFillCostList();
}

