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

#include "booItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "booitemImage.h"

static char *costReportTypes[] = { "D", "O", "N" };

booItem::booItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_runTime, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
  connect(_runTimePer, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
  connect(_stdopn, SIGNAL(newID(int)), this, SLOT(sHandleStdopn(int)));
  connect(_fixedFont, SIGNAL(toggled(bool)), this, SLOT(sHandleFont(bool)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_invProdUOMRatio, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
  connect(_newImage, SIGNAL(clicked()), this, SLOT(sNewImage()));
  connect(_editImage, SIGNAL(clicked()), this, SLOT(sEditImage()));
  connect(_deleteImage, SIGNAL(clicked()), this, SLOT(sDeleteImage()));

  _booitemid = -1;
  _item->setReadOnly(TRUE);

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _prodUOM->setType(XComboBox::UOMs);

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
		     "FROM wrkcnt "
		     "ORDER BY wrkcnt_code" );

  _stdopn->populate( "SELECT -1, TEXT('None') AS stdopn_number "
		     "UNION "
		     "SELECT stdopn_id, stdopn_number "
		     "FROM stdopn "
		     "ORDER BY stdopn_number" );

  _setupReport->insertItem(tr("Direct Labor"));
  _setupReport->insertItem(tr("Overhead"));
  _setupReport->insertItem(tr("None"));
  _setupReport->setCurrentItem(-1);

  _runReport->insertItem(tr("Direct Labor"));
  _runReport->insertItem(tr("Overhead"));
  _runReport->insertItem(tr("None"));
  _runReport->setCurrentItem(-1);

  _booimage->addColumn(tr("Image Name"),  _itemColumn, Qt::AlignLeft );
  _booimage->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  _booimage->addColumn(tr("Purpose"),     _itemColumn, Qt::AlignLeft );

  
  _fixedFont->setChecked(_preferences->boolean("UsedFixedWidthFonts"));

  // hide the Allow Pull Through option as it doesn't perform
  // any function at this time.
  _pullThrough->hide();
}

booItem::~booItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void booItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse booItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("booitem_id", &valid);
  if (valid)
  {
    _booitemid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
	if (_item->itemType() == "J")
	{
      _receiveStock->setEnabled(FALSE);
	  _receiveStock->setChecked(FALSE);
	}
  }

  param = pParams.value("revision_id", &valid);
  if (valid)
    _revisionid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _stdopn->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      disconnect(_booimage, SIGNAL(valid(bool)), _editImage, SLOT(setEnabled(bool)));
      disconnect(_booimage, SIGNAL(valid(bool)), _deleteImage, SLOT(setEnabled(bool)));

      _dates->setEnabled(FALSE);
      _executionDay->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _setupTime->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _runTime->setEnabled(FALSE);
      _runTimePer->setEnabled(FALSE);
      _reportSetup->setEnabled(FALSE);
      _reportRun->setEnabled(FALSE);
      _issueComp->setEnabled(FALSE);
      _receiveStock->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
      _wipLocation->setEnabled(FALSE);
      _toolingReference->setEnabled(FALSE);
      _setupReport->setEnabled(FALSE);
      _runReport->setEnabled(FALSE);
      _stdopn->setEnabled(FALSE);
      _overlap->setEnabled(FALSE);
      _pullThrough->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
      _newImage->setEnabled(FALSE);
      _save->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  return NoError;
}

void booItem::sSave()
{

  if (_wrkcnt->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save BOO Item"),
                           tr("You must select a Work Center for this BOO Item before you may save it.") );
    _wrkcnt->setFocus();
    return;
  }
  
  if (_setupReport->currentItem() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save BOO Item"),
                           tr("You must select a Setup Cost reporting method for this BOO item before you may save it.") );
    _setupReport->setFocus();
    return;
  }

  if (_runReport->currentItem() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save BOO Item"),
                           tr("You must select a Run Cost reporting method for this BOO item before you may save it.") );
    _runReport->setFocus();
    return;
  }  
  
  if (_receiveStock->isChecked())
  {
    q.prepare( "UPDATE booitem "
               "SET booitem_rcvinv=FALSE "
               "WHERE (booitem_item_id=:item_id);" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('booitem_booitem_id_seq') AS _booitem_id;");
    if (q.first())
      _booitemid = q.value("_booitem_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

      q.prepare( "INSERT INTO booitem "
                 "( booitem_effective, booitem_expires, booitem_execday,"
                 "  booitem_id, booitem_item_id,"
                 "  booitem_seqnumber,"
                 "  booitem_wrkcnt_id, booitem_stdopn_id,"
                 "  booitem_descrip1, booitem_descrip2,"
                 "  booitem_toolref,"
                 "  booitem_sutime, booitem_sucosttype, booitem_surpt,"
                 "  booitem_rntime, booitem_rncosttype, booitem_rnrpt,"
                 "  booitem_produom, booitem_invproduomratio,"
                 "  booitem_rnqtyper,"
                 "  booitem_issuecomp, booitem_rcvinv,"
                 "  booitem_pullthrough, booitem_overlap,"
                 "  booitem_configtype, booitem_configid, booitem_configflag,"
                 "  booitem_instruc, booitem_wip_location_id, booitem_rev_id ) "
                 "VALUES "
                 "( :effective, :expires, :booitem_execday,"
                 "  :booitem_id, :booitem_item_id,"
                 "  ((SELECT COALESCE(MAX(booitem_seqnumber), 0) FROM booitem WHERE (booitem_item_id=:booitem_item_id)) + 10),"
                 "  :booitem_wrkcnt_id, :booitem_stdopn_id,"
                 "  :booitem_descrip1, :booitem_descrip2,"
                 "  :booitem_toolref,"
                 "  :booitem_sutime, :booitem_sucosttype, :booitem_surpt,"
                 "  :booitem_rntime, :booitem_rncosttype, :booitem_rnrpt,"
                 "  :booitem_produom, :booitem_invproduomratio,"
                 "  :booitem_rnqtyper,"
                 "  :booitem_issuecomp, :booitem_rcvinv,"
                 "  :booitem_pullthrough, :booitem_overlap,"
                 "  :booitem_configtype, :booitem_configid, :booitem_configflag,"
		 "  :booitem_instruc, :booitem_wip_location_id, :booitem_rev_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE booitem "
               "SET booitem_effective=:effective, booitem_expires=:expires, booitem_execday=:booitem_execday,"
               "    booitem_wrkcnt_id=:booitem_wrkcnt_id, booitem_stdopn_id=:booitem_stdopn_id,"
               "    booitem_descrip1=:booitem_descrip1, booitem_descrip2=:booitem_descrip2,"
               "    booitem_toolref=:booitem_toolref,"
               "    booitem_sutime=:booitem_sutime, booitem_sucosttype=:booitem_sucosttype, booitem_surpt=:booitem_surpt,"
               "    booitem_rntime=:booitem_rntime, booitem_rncosttype=:booitem_rncosttype, booitem_rnrpt=:booitem_rnrpt,"
               "    booitem_produom=:booitem_produom, booitem_invproduomratio=:booitem_invproduomratio, booitem_rnqtyper=:booitem_rnqtyper,"
               "    booitem_issuecomp=:booitem_issuecomp, booitem_rcvinv=:booitem_rcvinv,"
               "    booitem_pullthrough=:booitem_pullthrough, booitem_overlap=:booitem_overlap,"
               "    booitem_configtype=:booitem_configtype, booitem_configid=:booitem_configid, booitem_configflag=:booitem_configflag,"
               "    booitem_instruc=:booitem_instruc, booitem_wip_location_id=:booitem_wip_location_id "
               "WHERE (booitem_id=:booitem_id);" );

  q.bindValue(":booitem_id", _booitemid);
  q.bindValue(":booitem_item_id", _item->id());
  q.bindValue(":effective", _dates->startDate());
  q.bindValue(":expires", _dates->endDate());
  q.bindValue(":booitem_execday", _executionDay->value());
  q.bindValue(":booitem_descrip1", _description1->text());
  q.bindValue(":booitem_descrip2", _description2->text());
  q.bindValue(":booitem_produom", _prodUOM->currentText());
  q.bindValue(":booitem_toolref", _toolingReference->text());
  q.bindValue(":booitem_instruc", _instructions->text());
  q.bindValue(":booitem_invproduomratio", _invProdUOMRatio->toDouble());
  q.bindValue(":booitem_sutime", _setupTime->toDouble());
  q.bindValue(":booitem_rntime", _runTime->toDouble());
  q.bindValue(":booitem_sucosttype", costReportTypes[_setupReport->currentItem()]);
  q.bindValue(":booitem_rncosttype", costReportTypes[_runReport->currentItem()]);
  q.bindValue(":booitem_rnqtyper", _runTimePer->toDouble());
  q.bindValue(":booitem_rnrpt", QVariant(_reportRun->isChecked(), 0));
  q.bindValue(":booitem_surpt", QVariant(_reportSetup->isChecked(), 0));
  q.bindValue(":booitem_issuecomp", QVariant(_issueComp->isChecked(), 0));
  q.bindValue(":booitem_rcvinv", QVariant(_receiveStock->isChecked(), 0));
  q.bindValue(":booitem_pullthrough", QVariant(_pullThrough->isChecked(), 0));
  q.bindValue(":booitem_overlap", QVariant(_overlap->isChecked(), 0));
  q.bindValue(":booitem_wrkcnt_id", _wrkcnt->id());
  q.bindValue(":booitem_wip_location_id", _wipLocation->id());
  q.bindValue(":booitem_stdopn_id", _stdopn->id());
  q.bindValue(":booitem_configtype", "N");
  q.bindValue(":booitem_configid", -1);
  q.bindValue(":booitem_configflag", QVariant(FALSE, 0));
  q.bindValue(":booitem_rev_id", _revisionid);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sBOOsUpdated(_booitemid, TRUE);
  done(_booitemid);
}

void booItem::sHandleStdopn(int pStdopnid)
{
  if (_stdopn->id() != -1)
  {
    q.prepare( "SELECT stdopn_descrip1, stdopn_descrip2, stdopn_instructions, stdopn_toolref,"
               "       stdopn_wrkcnt_id, stdopn_stdtimes,"
               "       stdopn_sucosttype, stdopn_rncosttype, stdopn_produom,"
               "       formatTime(stdopn_sutime) AS f_sutime,"
               "       formatTime(stdopn_rntime) AS f_rntime,"
               "       formatQty(stdopn_rnqtyper) as rnqtyper,"
               "       formatRatio(stdopn_invproduomratio) AS invproduomratio "
               "FROM stdopn "
               "WHERE (stdopn_id=:stdopn_id);" );
    q.bindValue(":stdopn_id", pStdopnid);
    q.exec();
    if (q.first())
    {
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);

      _description1->setText(q.value("stdopn_descrip1"));
      _description2->setText(q.value("stdopn_descrip2"));
      _instructions->setText(q.value("stdopn_instructions").toString());
      _toolingReference->setText(q.value("stdopn_toolref"));
      _wrkcnt->setId(q.value("stdopn_wrkcnt_id").toInt());

      if (q.value("stdopn_stdtimes").toBool())
      {
        _setupTime->setText(q.value("f_sutime"));
        _runTime->setText(q.value("f_rntime"));
        _runTimePer->setText(q.value("rnqtyper"));
        _prodUOM->setText(q.value("stdopn_produom"));
        _invProdUOMRatio->setText(q.value("invproduomratio"));

        if (q.value("stdopn_sucosttype").toString() == "D")
          _setupReport->setCurrentItem(0);
        else if (q.value("stdopn_sucosttype").toString() == "O")
          _setupReport->setCurrentItem(1);
        else if (q.value("stdopn_sucosttype").toString() == "N")
          _setupReport->setCurrentItem(2);

        if (q.value("stdopn_rncosttype").toString() == "D")
          _runReport->setCurrentItem(0);
        else if (q.value("stdopn_rncosttype").toString() == "O")
          _runReport->setCurrentItem(1);
        else if (q.value("stdopn_rncosttype").toString() == "N")
          _runReport->setCurrentItem(2);
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    _description1->setEnabled(TRUE);
    _description1->clear();
    _description2->setEnabled(TRUE);
    _description2->clear();
    _instructions->setEnabled(TRUE);
    _instructions->clear();
  }
}

void booItem::sCalculateInvRunTime()
{
  if ((_runTimePer->toDouble() != 0.0) && (_invProdUOMRatio->toDouble() != 0.0))
  {
    _invRunTime->setText(formatCost(_runTime->toDouble() / _runTimePer->toDouble() / _invProdUOMRatio->toDouble()));

    _invPerMinute->setText(formatCost(_runTimePer->toDouble() / _runTime->toDouble() * _invProdUOMRatio->toDouble()));

  }
  else
  {
    _invRunTime->setText(formatCost(0.0));
    _invPerMinute->setText(formatCost(0.0));
  }
}

void booItem::sHandleFont(bool pFixed)
{
  if (pFixed)
    _instructions->setFont(omfgThis->fixedFont());
  else
    _instructions->setFont(omfgThis->systemFont());
}

void booItem::populate()
{
  XSqlQuery booitem;
  booitem.prepare( "SELECT item_config,"
                   "       booitem_effective, booitem_expires,"
                   "       booitem_execday, booitem_item_id, booitem_seqnumber,"
                   "       booitem_wrkcnt_id, booitem_stdopn_id,"
                   "       booitem_descrip1, booitem_descrip2, booitem_toolref,"
                   "       formatTime(booitem_sutime) AS f_sutime, booitem_sucosttype, booitem_surpt,"
                   "       formatTime(booitem_rntime) AS f_rntime, booitem_rncosttype, booitem_rnrpt,"
                   "       booitem_produom, uom_name,"
                   "       formatRatio(booitem_invproduomratio) AS invproduomratio,"
                   "       formatRatio(booitem_rnqtyper) AS rnqtyper,"
                   "       booitem_issuecomp, booitem_rcvinv,"
                   "       booitem_pullthrough, booitem_overlap,"
                   "       booitem_configtype, booitem_configid, booitem_configflag,"
                   "       booitem_instruc, booitem_wip_location_id "
                   "FROM booitem, item, uom "
                   "WHERE ( (booitem_item_id=item_id)"
                   " AND (item_inv_uom_id=uom_id)"
                   " AND (booitem_id=:booitem_id) );" );
  booitem.bindValue(":booitem_id", _booitemid);
  booitem.exec();
  if (booitem.first())
  {
    _stdopn->setId(booitem.value("booitem_stdopn_id").toInt());
    if (booitem.value("booitem_stdopn_id").toInt() != -1)
    {
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
    }

    _dates->setStartDate(booitem.value("booitem_effective").toDate());
    _dates->setEndDate(booitem.value("booitem_expires").toDate());
    _executionDay->setValue(booitem.value("booitem_execday").toInt());
    _operSeqNum->setText(booitem.value("booitem_seqnumber").toString());
    _description1->setText(booitem.value("booitem_descrip1"));
    _description2->setText(booitem.value("booitem_descrip2"));
    _toolingReference->setText(booitem.value("booitem_toolref"));
    _setupTime->setText(booitem.value("f_sutime"));
    _prodUOM->setText(booitem.value("booitem_produom"));
    _invUOM1->setText(booitem.value("uom_name").toString());
    _invUOM2->setText(booitem.value("uom_name").toString());
    _invProdUOMRatio->setText(booitem.value("invproduomratio"));
    _runTime->setText(booitem.value("f_rntime"));
    _runTimePer->setText(booitem.value("rnqtyper"));

    _reportSetup->setChecked(booitem.value("booitem_surpt").toBool());
    _reportRun->setChecked(booitem.value("booitem_rnrpt").toBool());
    _issueComp->setChecked(booitem.value("booitem_issuecomp").toBool());
    _receiveStock->setChecked(booitem.value("booitem_rcvinv").toBool());
    _overlap->setChecked(booitem.value("booitem_overlap").toBool());
    _pullThrough->setChecked(booitem.value("booitem_pullthrough").toBool());
    _instructions->setText(booitem.value("booitem_instruc").toString());
    _wrkcnt->setId(booitem.value("booitem_wrkcnt_id").toInt());
    _wipLocation->setId(booitem.value("booitem_wip_location_id").toInt());

    if (booitem.value("booitem_sucosttype").toString() == "D")
      _setupReport->setCurrentItem(0);
    else if (booitem.value("booitem_sucosttype").toString() == "O")
      _setupReport->setCurrentItem(1);
    else if (booitem.value("booitem_sucosttype").toString() == "N")
      _setupReport->setCurrentItem(2);

    if (booitem.value("booitem_rncosttype").toString() == "D")
      _runReport->setCurrentItem(0);
    else if (booitem.value("booitem_rncosttype").toString() == "O")
      _runReport->setCurrentItem(1);
    else if (booitem.value("booitem_rncosttype").toString() == "N")
      _runReport->setCurrentItem(2);

    _item->setId(booitem.value("booitem_item_id").toInt());
	if (_item->itemType() == "J")
	{
      _receiveStock->setEnabled(FALSE);
	  _receiveStock->setChecked(FALSE);
	}

    sCalculateInvRunTime();
    sFillImageList();
  }
  else if (booitem.lastError().type() != QSqlError::None)
  {
    systemError(this, booitem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void booItem::sPopulateLocations()
{
  int locid = _wipLocation->id();

  XSqlQuery loc;
  loc.prepare("SELECT location_id, formatLocationName(location_id) AS locationname"
              "  FROM location, wrkcnt"
              " WHERE ( (location_warehous_id=wrkcnt_warehous_id)"
              "   AND   (NOT location_restrict)"
              "   AND   (wrkcnt_id=:wrkcnt_id) ) "
              "UNION "
              "SELECT location_id, formatLocationName(location_id) AS locationname"
              "  FROM locitem, location, wrkcnt"
              " WHERE ( (location_warehous_id=wrkcnt_warehous_id)"
              "   AND   (location_restrict)"
              "   AND   (locitem_location_id=location_id)"
              "   AND   (locitem_item_id=:item_id)"
              "   AND   (wrkcnt_id=:wrkcnt_id) )"
              " ORDER BY locationname; ");
  loc.bindValue(":wrkcnt_id", _wrkcnt->id());
  loc.bindValue(":item_id", _item->id());
  loc.exec();

  _wipLocation->populate(loc, locid);
  if (loc.lastError().type() != QSqlError::None)
  {
    systemError(this, loc.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void booItem::sNewImage()
{
  if(cNew == _mode)
  {
    QMessageBox::information( this, tr("Must Save BOO Item"),
      tr("You must save the BOO Item before you can add images to it.") );
    return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("booitem_id", _booitemid);

  booitemImage newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillImageList();
}

void booItem::sEditImage()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("booimage_id", _booimage->id());

  booitemImage newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillImageList();
}

void booItem::sDeleteImage()
{
  q.prepare( "DELETE FROM booimage "
             "WHERE (booimage_id=:booimage_id);" );
  q.bindValue(":booimage_id", _booimage->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillImageList();
}

void booItem::sFillImageList()
{
  q.prepare( "SELECT booimage_id, image_name, firstLine(image_descrip),"
             "       CASE WHEN (booimage_purpose='I') THEN :inventory"
             "            WHEN (booimage_purpose='P') THEN :product"
             "            WHEN (booimage_purpose='E') THEN :engineering"
             "            WHEN (booimage_purpose='M') THEN :misc"
             "            ELSE :other"
             "       END "
             "FROM booimage, image "
             "WHERE ( (booimage_image_id=image_id)"
             " AND (booimage_booitem_id=:booitem_id) ) "
             "ORDER BY image_name;" );
  q.bindValue(":inventory", tr("Inventory Description"));
  q.bindValue(":product", tr("Product Description"));
  q.bindValue(":engineering", tr("Engineering Reference"));
  q.bindValue(":misc", tr("Miscellaneous"));
  q.bindValue(":other", tr("Other"));
  q.bindValue(":booitem_id", _booitemid);
  q.exec();
  _booimage->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
