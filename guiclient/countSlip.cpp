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

#include "countSlip.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "inputManager.h"
#include "countTagList.h"

/*
 *  Constructs a countSlip as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
countSlip::countSlip(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
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

  _captive = FALSE;

#ifndef Q_WS_MAC
  _countTagList->setMaximumWidth(25);
#endif

  _item->setReadOnly(TRUE);
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
  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _cnttagid = param.toInt();
    populateTagInfo();
  }

  param = pParams.value("cntslip_id", &valid);
  if (valid)
  {
    _captive = TRUE;

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

      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _captive = TRUE;

      q.prepare( "SELECT cntslip_posted "
                 "FROM cntslip "
                 "WHERE (cntslip_id=:cntslip_id);" );
      q.bindValue(":cntslip_id", _cntslipid);
      q.exec();
      if (q.first())
      {
        if (q.value("cntslip_posted").toBool())
        {
//  ToDo
          reject();
        }
      }

      _countTagList->hide();
      _number->setEnabled(FALSE);

      _qty->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _countTagList->hide();
      _number->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
    else if (param.toString() == "post")
    {
      _mode = cPost;

      _location->setEnabled(FALSE);
      _lotSerial->setEnabled(FALSE);
      _countTagList->hide();
      _number->setEnabled(FALSE);
      _qty->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _save->setText(tr("&Post"));
      _expiration->setEnabled(FALSE);
      _warranty->setEnabled(FALSE);

      _save->setFocus();
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
  QString slipNumber = _number->text().stripWhiteSpace().upper();
  if (slipNumber.length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Count Slip #"),
                           tr("You must enter a Count Slip # for this Count Slip.") );
    _number->setFocus();
    return;
  }

  if (_qty->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Count Slip Quantity"),
                           tr("You must enter a counted quantity for this Count Slip.") );
    _qty->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    QString countSlipAuditing = _metrics->value("CountSlipAuditing");
    if (countSlipAuditing == "W")
    {
      q.prepare( "SELECT cntslip_id "
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
      q.bindValue(":cnttag_id", _cnttagid);
      q.bindValue(":cntslip_number", slipNumber); 
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Duplicate Count Slip #"),
                               tr( "An unposted Count Slip for this Warehouse has already been entered\n"
                                   "with this #.  OpenMFG's I/M Module has been configured to disallow the\n"
                                   "duplication of unposted Count Slip #s within a given Warehouse.\n"
                                   "Please verify the # of the Count Slip you are entering." ) );
        return;
      }
    }
    else if (countSlipAuditing == "A")
    {
      q.prepare( "SELECT cntslip_id "
                 "FROM cntslip "
                 "WHERE ( (NOT cntslip_posted)"
                 " AND (cntslip_number=:cntslip_number));" );
      q.bindValue(":cntslip_number", slipNumber); 
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Duplicate Count Slip #"),
                               tr( "An unposted Count Slip has already been entered with this #.\n"
                                   "OpenMFG's I/M Module has been configured to disallow the\n"
                                   "duplication of unposted Count Slip #s.\n"
                                   "Please verify the # of the Count Slip you are entering." ) );
        return;
      }
    }
    else if (countSlipAuditing == "X")
    {
      q.prepare( "SELECT cntslip_id "
                 "FROM cntslip, "
                 "     invcnt AS newtag, invcnt AS oldtag,"
                 "     itemsite AS newsite, itemsite AS oldsite "
                 "WHERE ( (cntslip_cnttag_id=oldtag.invcnt_id)"
                 " AND (oldtag.invcnt_itemsite_id=oldsite.itemsite_id)"
                 " AND (newtag.invcnt_itemsite_id=newsite.itemsite_id)"
                 " AND (oldsite.itemsite_warehous_id=newsite.itemsite_warehous_id)"
                 " AND (newtag.invcnt_id=:cnttag_id)"
                 " AND (cntslip_number=:cntslip_number));" );
      q.bindValue(":cnttag_id", _cnttagid);
      q.bindValue(":cntslip_number", slipNumber); 
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Duplicate Count Slip #"),
                               tr( "An Count Slip for this Warehouse has already been entered with this #.\n"
                                   "OpenMFG's I/M Module has been configured to disallow the duplication\n"
                                   "of Count Slip #s within a given Warehouse.\n"
                                   "Please verify the # of the Count Slip you are entering." ) );
        return;
      }
    }
    else if (countSlipAuditing == "B")
    {
      q.prepare( "SELECT cntslip_id "
                 "FROM cntslip "
                 "WHERE (cntslip_number=:cntslip_number);" );
      q.bindValue(":cntslip_number", slipNumber); 
      q.exec();
      if (q.first())
      {
        QMessageBox::critical( this, tr("Cannot Duplicate Count Slip #"),
                               tr( "An Count Slip has already been entered with this #.  OpenMFG's\n"
                                   "I/M Module has been configured to disallow the duplication of Count Slip #s.\n"
                                   "Please verify the # of the Count Slip you are entering." ) );
        return;
      }
    }

    q.exec("SELECT NEXTVAL('cntslip_cntslip_id_seq') AS cntslip_id");
    if (q.first())
      _cntslipid = q.value("cntslip_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO cntslip "
               "( cntslip_id, cntslip_cnttag_id,"
               "  cntslip_user_id, cntslip_entered, cntslip_posted,"
               "  cntslip_number, cntslip_qty,"
               "  cntslip_location_id, cntslip_lotserial,"
               "  cntslip_lotserial_expiration,"
               "  cntslip_lotserial_warrpurc,"
               "  cntslip_comments ) "
               "SELECT :cntslip_id, :cnttag_id,"
               "       currentUserId(), CURRENT_TIMESTAMP, FALSE,"
               "       :cntslip_number, :cntslip_qty,"
               "       :cntslip_location_id, :cntslip_lotserial,"
               "       :cntslip_lotserial_expiration,"
               "       :cntslip_lotserial_warrpurc,"
               "       :cntslip_comments;" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE cntslip "
               "SET cntslip_user_id=currentUserId(), cntslip_qty=:cntslip_qty, cntslip_comments=:cntslip_comments,"
               "    cntslip_entered=CURRENT_TIMESTAMP,"
               "    cntslip_location_id=:cntslip_location_id, cntslip_lotserial=:cntslip_lotserial,"
               "    cntslip_lotserial_expiration=:cntslip_lotserial_expiration, "
               "    cntslip_lotserial_warrpurc=:cntslip_lotserial_warrpurc "
               "WHERE (cntslip_id=:cntslip_id);" );
  else if (_mode == cPost)
    q.prepare("SELECT postCountSlip(:cntslip_id) AS result;");

  q.bindValue(":cntslip_id", _cntslipid);
  q.bindValue(":cnttag_id", _cnttagid);
  q.bindValue(":cntslip_number", slipNumber);
  q.bindValue(":cntslip_qty", _qty->toDouble());
  q.bindValue(":cntslip_location_id", _location->id());
  q.bindValue(":cntslip_lotserial", _lotSerial->text());
  q.bindValue(":cntslip_comments", _comments->text());
  if(_expiration->isEnabled())
    q.bindValue(":cntslip_lotserial_expiration", _expiration->date());
  else
    q.bindValue(":cntslip_lotserial_expiration", QVariant());
  if(_warranty->isEnabled())
    q.bindValue(":cntslip_lotserial_warrpurc", _warranty->date());
  q.exec();

  if (_mode == cPost)
  {
    if (q.first())
    {
      if (q.value("result").toInt() == -1)
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
    _location->setEnabled(FALSE);
    _lotSerial->clear();
    _lotSerial->setEnabled(FALSE);
    _close->setText(tr("&Close"));

    _countTagList->setFocus();
  }
}

void countSlip::sCountTagList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);
  params.append("tagType", cUnpostedCounts);

  countTagList newdlg(this, "", TRUE);
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
             "       formatQty(cntslip_qty) AS qty "
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
    _qty->setText(r.value("qty").toString());
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
  q.prepare( "SELECT itemsite_loccntrl, itemsite_controlmethod, itemsite_location_id, "
             "  itemsite_perishable, itemsite_warrpurc "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();
  if (q.first())
  {
    QString controlMethod(q.value("itemsite_controlmethod").toString());
    _expiration->setEnabled(q.value("itemsite_perishable").toBool());
    _warranty->setEnabled(q.value("itemsite_warrpurc").toBool());

    if (q.value("itemsite_loccntrl").toBool())
    {
      XSqlQuery location;
      location.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname "
                        "FROM location, itemsite "
                        "WHERE ( (validLocation(location_id, itemsite_id))"
                        " AND (itemsite_warehous_id=:warehous_id)"
                        " AND (itemsite_item_id=:item_id) ) "
                        "ORDER BY locationname;" );
      location.bindValue(":warehous_id", _warehouse->id());
      location.bindValue(":item_id", _item->id());
      location.exec();

      _location->populate(location);
      _location->setId(q.value("itemsite_location_id").toInt());
      _location->setEnabled(TRUE);
    }
    else
    {
      _location->clear();
      _location->setEnabled(FALSE);
    }

    if (controlMethod == "L")
    {
      _lotSerial->setEnabled(TRUE);
      _qty->setEnabled(TRUE);
    }
    else if (controlMethod == "S")
    {
      _lotSerial->setEnabled(TRUE);
      _qty->setText("1.0");
      _qty->setEnabled(FALSE);
    }
    else
    {
      _lotSerial->setEnabled(FALSE);
      _qty->setEnabled(TRUE);
    }
  }
}

void countSlip::populateTagInfo()
{
  q.prepare( "SELECT invcnt_tagnumber, invcnt_itemsite_id,"
             "       COALESCE(invcnt_location_id, -1) AS location "
             "FROM invcnt "
             "WHERE (invcnt_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttagid);
  q.exec();
  if (q.first())
  {
    int locationid = q.value("location").toInt();
    _countTagNumber->setText(q.value("invcnt_tagnumber").toString());
    _item->setItemsiteid(q.value("invcnt_itemsite_id").toInt());
    if(locationid != -1)
    {
      _location->setId(locationid);
      _location->setEnabled(false);
    }
  }
}
