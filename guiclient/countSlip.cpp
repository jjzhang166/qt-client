/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "countSlip.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

#include "inputManager.h"
#include "countTagList.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

countSlip::countSlip(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
  connect(_countTagList, SIGNAL(clicked()), this, SLOT(sCountTagList()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateItemSiteInfo()));
  connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateItemSiteInfo()));

  _captive = false;

#ifndef Q_OS_MAC
  _countTagList->setMaximumWidth(25);
#endif

  _item->setReadOnly(true);
  _qty->setValidator(omfgThis->qtyVal());
  _expiration->setAllowNullDate(true);
  _expiration->setAllowNullDate(false);
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
countSlip::~countSlip()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void countSlip::languageChange()
{
    retranslateUi(this);
}

enum SetResponse countSlip::set(const ParameterList &pParams)
{
  XSqlQuery countet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
  {
    _captive = true;

    _cnttagid = param.toInt();
    populateTagInfo();
  }

  param = pParams.value("cntslip_id", &valid);
  if (valid)
  {
    _captive = true;

    _cntslipid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      omfgThis->inputManager()->notify(cBCCountTag, this, this, SLOT(sCatchCounttagid(int)));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _captive = true;

      countet.prepare( "SELECT cntslip_posted "
                 "FROM cntslip "
                 "WHERE (cntslip_id=:cntslip_id);" );
      countet.bindValue(":cntslip_id", _cntslipid);
      countet.exec();
      if (countet.first())
      {
        if (countet.value("cntslip_posted").toBool())
        {
//  ToDo
          reject();
        }
      }

      _countTagList->hide();
      _number->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _countTagList->hide();
      _number->setEnabled(false);
      _qty->setEnabled(false);
      _comments->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
    else if (param.toString() == "post")
    {
      _mode = cPost;

      _location->setEnabled(false);
      _lotSerial->setEnabled(false);
      _countTagList->hide();
      _number->setEnabled(false);
      _qty->setEnabled(false);
      _comments->setEnabled(false);
      _save->setText(tr("&Post"));
      _expiration->setEnabled(false);
      _warranty->setEnabled(false);
    }
  }

  return NoError;
}

void countSlip::sCatchCounttagid(int pCnttagid)
{
  _cnttagid = pCnttagid;
  populate();

  _number->setFocus();
}

void countSlip::sSave()
{
  XSqlQuery countSave;

  QString slipNumber = _number->text().trimmed().toUpper();

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(slipNumber.length() == 0, _number,
                          tr("You must enter a Count Slip # for this Count Slip."))
         << GuiErrorCheck(_qty->text().trimmed().length() == 0, _qty,
                          tr("You must enter a counted quantity for this Count Slip."))
     ;

  if (_mode == cNew)
  {
    QString countSlipAuditing = _metrics->value("CountSlipAuditing");
    if (countSlipAuditing == "W")
    {
      countSave.prepare( "SELECT cntslip_id "
                 "FROM cntslip, "
                 "     invcnt AS newtag, invcnt AS oldtag,"
                 "     itemsite AS newsite, itemsite AS oldsite "
                 "WHERE ( (cntslip_cnttag_id=oldtag.invcnt_id)"
                 " AND (oldtag.invcnt_itemsite_id=oldsite.itemsite_id)"
                 " AND (newtag.invcnt_itemsite_id=newsite.itemsite_id)"
                 " AND (oldsite.itemsite_warehous_id=newsite.itemsite_warehous_id)"
                 " AND (NOT cntslip_posted)"
                 " AND (newtag.invcnt_id=:cnttag_id)"
                 " AND (cntslip_number=:cntslip_number) );" );
      countSave.bindValue(":cnttag_id", _cnttagid);
      countSave.bindValue(":cntslip_number", slipNumber); 
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _number,
                               tr( "An unposted Count Slip for this Site has already been entered\n"
                                   "with this #.  The I/M Module has been configured to disallow the\n"
                                   "duplication of unposted Count Slip #s within a given Site.\n"
                                   "Please verify the # of the Count Slip you are entering." ));
      }
    }
    else if (countSlipAuditing == "A")
    {
      countSave.prepare( "SELECT cntslip_id "
                 "FROM cntslip "
                 "WHERE ( (NOT cntslip_posted)"
                 " AND (cntslip_number=:cntslip_number));" );
      countSave.bindValue(":cntslip_number", slipNumber); 
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _number,
                               tr( "An unposted Count Slip has already been entered with this #.\n"
                                   "The I/M Module has been configured to disallow the\n"
                                   "duplication of unposted Count Slip #s.\n"
                                   "Please verify the # of the Count Slip you are entering." ));
      }
    }
    else if (countSlipAuditing == "X")
    {
      countSave.prepare( "SELECT cntslip_id "
                 "FROM cntslip, "
                 "     invcnt AS newtag, invcnt AS oldtag,"
                 "     itemsite AS newsite, itemsite AS oldsite "
                 "WHERE ( (cntslip_cnttag_id=oldtag.invcnt_id)"
                 " AND (oldtag.invcnt_itemsite_id=oldsite.itemsite_id)"
                 " AND (newtag.invcnt_itemsite_id=newsite.itemsite_id)"
                 " AND (oldsite.itemsite_warehous_id=newsite.itemsite_warehous_id)"
                 " AND (newtag.invcnt_id=:cnttag_id)"
                 " AND (cntslip_number=:cntslip_number));" );
      countSave.bindValue(":cnttag_id", _cnttagid);
      countSave.bindValue(":cntslip_number", slipNumber); 
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _number,
                               tr( "An Count Slip for this Site has already been entered with this #.\n"
                                   "The I/M Module has been configured to disallow the duplication\n"
                                   "of Count Slip #s within a given Site.\n"
                                   "Please verify the # of the Count Slip you are entering." ));
      }
    }
    else if (countSlipAuditing == "B")
    {
      countSave.prepare( "SELECT cntslip_id "
                 "FROM cntslip "
                 "WHERE (cntslip_number=:cntslip_number);" );
      countSave.bindValue(":cntslip_number", slipNumber); 
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _number,
                               tr( "An Count Slip has already been entered with this #.  The\n"
                                   "I/M Module has been configured to disallow the duplication of Count Slip #s.\n"
                                   "Please verify the # of the Count Slip you are entering." ));
      }
    }

    // Check for duplicate Serial #
    countSave.prepare( "SELECT cntslip_id "
               "FROM invcnt JOIN itemsite ON (itemsite_id=invcnt_itemsite_id)"
               "            JOIN cntslip ON (cntslip_cnttag_id=invcnt_id)"
               "WHERE ( (invcnt_id=:cnttag_id)"
               "  AND   (itemsite_controlmethod='S')"
               "  AND   (cntslip_lotserial=:cntslip_lotserial) );" );
    countSave.bindValue(":cnttag_id", _cnttagid);
    countSave.bindValue(":cntslip_lotserial", _lotSerial->text());
    countSave.exec();
    if (countSave.first())
    {
      errors << GuiErrorCheck(true, _lotSerial,
                             tr( "A Count Slip has already been entered with this Serial #.\n"
                                 "Please verify the Serial # of the Count Slip you are entering." ));
    }

    // Check for duplicate Serial # in different Location from Location being counted
    if (_metrics->boolean("MultiWhs"))
    {
      countSave.prepare( "SELECT itemloc_id "
               "FROM invcnt JOIN itemsite ON (itemsite_id=invcnt_itemsite_id)"
               "            JOIN itemloc ON (itemloc_itemsite_id=itemsite_id)"
               "            JOIN ls ON (ls_id=itemloc_ls_id)"
               "WHERE ( (invcnt_id=:cnttag_id)"
               "  AND   (invcnt_location_id IS NOT NULL)"
               "  AND   (itemsite_controlmethod='S')"
               "  AND   (itemloc_location_id <> :cntslip_location_id)"
               "  AND   (ls_number=:cntslip_lotserial) );" );
      countSave.bindValue(":cnttag_id", _cnttagid);
      countSave.bindValue(":cntslip_location_id", _location->id());
      countSave.bindValue(":cntslip_lotserial", _lotSerial->text());
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _lotSerial,
                               tr( "This Serial # exists in a different Location.\n"
                                   "Please verify the Serial # of the Count Slip you are entering." ));
      }
    }

    // Check for duplicate Serial # in different Site from Site being counted
    if (_metrics->boolean("MultiWhs"))
    {
      countSave.prepare( "SELECT itemloc_id "
                        "FROM itemsite JOIN itemloc ON (itemloc_itemsite_id=itemsite_id)"
                        "              JOIN ls ON (ls_id=itemloc_ls_id)"
                        "WHERE ( (itemsite_item_id=:item_id)"
                        "  AND   (itemsite_warehous_id != :warehous_id)"
                        "  AND   (itemsite_controlmethod='S')"
                        "  AND   (UPPER(ls_number)=UPPER(:cntslip_lotserial)) );" );
      countSave.bindValue(":item_id", _item->id());
      countSave.bindValue(":warehous_id", _warehouse->id());
      countSave.bindValue(":cntslip_lotserial", _lotSerial->text());
      countSave.exec();
      if (countSave.first())
      {
        errors << GuiErrorCheck(true, _lotSerial,
                                tr( "This Serial # exists in a different Site.\n"
                                   "Please verify the Serial # of the Count Slip you are entering." ));
      }
    }
    
    if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Count Slip"), errors))
      return;

    countSave.exec("SELECT NEXTVAL('cntslip_cntslip_id_seq') AS cntslip_id");
    if (countSave.first())
      _cntslipid = countSave.value("cntslip_id").toInt();
//  ToDo

    countSave.prepare( "INSERT INTO cntslip "
               "( cntslip_id, cntslip_cnttag_id,"
               "  cntslip_username, cntslip_entered, cntslip_posted,"
               "  cntslip_number, cntslip_qty,"
               "  cntslip_location_id, cntslip_lotserial,"
               "  cntslip_lotserial_expiration,"
               "  cntslip_lotserial_warrpurc,"
               "  cntslip_comments ) "
               "SELECT :cntslip_id, :cnttag_id,"
               "       getEffectiveXtUser(), CURRENT_TIMESTAMP, false,"
               "       :cntslip_number, :cntslip_qty,"
               "       :cntslip_location_id, :cntslip_lotserial,"
               "       :cntslip_lotserial_expiration,"
               "       :cntslip_lotserial_warrpurc,"
               "       :cntslip_comments;" );
  }
  else if (_mode == cEdit)
    countSave.prepare( "UPDATE cntslip "
               "SET cntslip_username=getEffectiveXtUser(), cntslip_qty=:cntslip_qty, cntslip_comments=:cntslip_comments,"
               "    cntslip_entered=CURRENT_TIMESTAMP,"
               "    cntslip_location_id=:cntslip_location_id, cntslip_lotserial=:cntslip_lotserial,"
               "    cntslip_lotserial_expiration=:cntslip_lotserial_expiration, "
               "    cntslip_lotserial_warrpurc=:cntslip_lotserial_warrpurc "
               "WHERE (cntslip_id=:cntslip_id);" );
  else if (_mode == cPost)
    countSave.prepare("SELECT postCountSlip(:cntslip_id) AS result;");

  countSave.bindValue(":cntslip_id", _cntslipid);
  countSave.bindValue(":cnttag_id", _cnttagid);
  countSave.bindValue(":cntslip_number", slipNumber);
  countSave.bindValue(":cntslip_qty", _qty->toDouble());
  countSave.bindValue(":cntslip_location_id", _location->id());
  countSave.bindValue(":cntslip_lotserial", _lotSerial->text());
  countSave.bindValue(":cntslip_comments", _comments->toPlainText());
  if(_expiration->isEnabled())
    countSave.bindValue(":cntslip_lotserial_expiration", _expiration->date());
  else
    countSave.bindValue(":cntslip_lotserial_expiration", QVariant());
  if(_warranty->isEnabled())
    countSave.bindValue(":cntslip_lotserial_warrpurc", _warranty->date());
  countSave.exec();

  if (_mode == cPost)
  {
    if (countSave.first())
    {
      if (countSave.value("result").toInt() == -1)
        _cntslipid = XDialog::Rejected;
    }
//  ToDo
  }

  if (_captive)
    done(_cntslipid);
  else
  {
    _cnttagid = -1;
    _countTagNumber->clear();
    _item->setId(-1);
    _number->clear();
    _qty->clear();
    _comments->clear();
    _location->clear();
    _location->setEnabled(false);
    _lotSerial->clear();
    _lotSerial->setEnabled(false);
    _close->setText(tr("&Close"));

    _countTagList->setFocus();
  }
}

void countSlip::sCountTagList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);
  params.append("tagType", cUnpostedCounts);

  countTagList newdlg(this, "", true);
  newdlg.set(params);
  _cnttagid = newdlg.exec();

  populateTagInfo();
}

void countSlip::populate()
{
  XSqlQuery r;
  r.prepare( "SELECT cntslip_cnttag_id, invcnt_tagnumber, invcnt_itemsite_id,"
             "       cntslip_number, cntslip_comments,"
             "       cntslip_location_id, cntslip_lotserial,"
             "       cntslip_lotserial_expiration,"
             "       cntslip_lotserial_warrpurc,"
             "       cntslip_qty "
             "FROM cntslip, invcnt "
             "WHERE ( (cntslip_cnttag_id=invcnt_id)"
             " AND (cntslip_id=:cntslip_id) );" );
  r.bindValue(":cntslip_id", _cntslipid);
  r.exec();
  if (r.first())
  {
    _cnttagid = r.value("cntslip_cnttag_id").toInt();

    _countTagNumber->setText(r.value("invcnt_tagnumber").toString());
    _number->setText(r.value("cntslip_number").toString());
    _qty->setDouble(r.value("cntslip_qty").toDouble());
    _comments->setText(r.value("cntslip_comments").toString());
    _lotSerial->setText(r.value("cntslip_lotserial").toString());
    if(r.value("cntslip_lotserial_expiration").toString().isEmpty())
      _expiration->clear();
    else
      _expiration->setDate(r.value("cntslip_lotserial_expiration").toDate());
    if(r.value("cntslip_lotserial_warrpurc").toString().isEmpty())
      _warranty->clear();
    else
      _warranty->setDate(r.value("cntslip_lotserial_warrpurc").toDate());
    _item->setItemsiteid(r.value("invcnt_itemsite_id").toInt());
    _location->setId(r.value("cntslip_location_id").toInt());
  }
}

void countSlip::sPopulateItemSiteInfo()
{
  XSqlQuery countPopulateItemSiteInfo;
  countPopulateItemSiteInfo.prepare( "SELECT itemsite_loccntrl, itemsite_controlmethod, itemsite_location_id, "
             "  itemsite_perishable, itemsite_warrpurc "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  countPopulateItemSiteInfo.bindValue(":item_id", _item->id());
  countPopulateItemSiteInfo.bindValue(":warehous_id", _warehouse->id());
  countPopulateItemSiteInfo.exec();
  if (countPopulateItemSiteInfo.first())
  {
    QString controlMethod(countPopulateItemSiteInfo.value("itemsite_controlmethod").toString());
    _expiration->setEnabled(countPopulateItemSiteInfo.value("itemsite_perishable").toBool());
    _warranty->setEnabled(countPopulateItemSiteInfo.value("itemsite_warrpurc").toBool());

    if (countPopulateItemSiteInfo.value("itemsite_loccntrl").toBool())
    {
      XSqlQuery location;
      location.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname "
                        "FROM location, itemsite "
                        "WHERE ( (validLocation(location_id, itemsite_id))"
                        " AND (itemsite_warehous_id=:warehous_id)"
                        " AND (itemsite_item_id=:item_id) "
                        " AND (location_active) ) "
                        "ORDER BY locationname;" );
      location.bindValue(":warehous_id", _warehouse->id());
      location.bindValue(":item_id", _item->id());
      location.exec();

      _location->populate(location);
      _location->setId(countPopulateItemSiteInfo.value("itemsite_location_id").toInt());
      _location->setEnabled(true);
    }
    else
    {
      _location->clear();
      _location->setEnabled(false);
    }

    if (controlMethod == "L")
    {
      _lotSerial->setEnabled(true);
      _qty->setEnabled(true);
    }
    else if (controlMethod == "S")
    {
      _lotSerial->setEnabled(true);
      _qty->setText("1.0");
      _qty->setEnabled(false);
    }
    else
    {
      _lotSerial->setEnabled(false);
      _qty->setEnabled(true);
    }
  }
}

void countSlip::populateTagInfo()
{
  XSqlQuery countpopulateTagInfo;
  countpopulateTagInfo.prepare( "SELECT invcnt_tagnumber, invcnt_itemsite_id,"
             "       COALESCE(invcnt_location_id, -1) AS location "
             "FROM invcnt "
             "WHERE (invcnt_id=:cnttag_id);" );
  countpopulateTagInfo.bindValue(":cnttag_id", _cnttagid);
  countpopulateTagInfo.exec();
  if (countpopulateTagInfo.first())
  {
    int locationid = countpopulateTagInfo.value("location").toInt();
    _countTagNumber->setText(countpopulateTagInfo.value("invcnt_tagnumber").toString());
    _item->setItemsiteid(countpopulateTagInfo.value("invcnt_itemsite_id").toInt());
    if(locationid != -1)
    {
      _location->setId(locationid);
      _location->setEnabled(false);
    }
  }
}
