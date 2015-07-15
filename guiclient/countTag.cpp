/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "countTag.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "inputManager.h"
#include "countTagList.h"

countTag::countTag(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_countTagList, SIGNAL(clicked()), this, SLOT(sCountTagList()));
  connect(_enter, SIGNAL(clicked()), this, SLOT(sEnter()));
  connect(_countTagNumber, SIGNAL(editingFinished()), this, SLOT(sParseCountTagNumber()));

  _item->setReadOnly(true);

#ifndef Q_OS_MAC
  _countTagList->setMaximumWidth(25);
#endif

  _qty->setValidator(omfgThis->qtyVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

countTag::~countTag()
{
  // no need to delete child widgets, Qt does it all for us
}

void countTag::languageChange()
{
    retranslateUi(this);
}

enum SetResponse countTag::set(const ParameterList &pParams)
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
    populate();
    _countTagNumber->setEnabled(false);
    _countTagList->hide();
  }

  param = pParams.value("invhist_id", &valid);
  if (valid)
  {
    _captive = true;

    countet.prepare( "SELECT invcnt_id "
               "FROM invcnt "
               "WHERE (invcnt_invhist_id=:invhist_id);" );
    countet.bindValue(":invhist_id", param.toInt());
    countet.exec();
    if (countet.first())
    {
      _cnttagid = countet.value("invcnt_id").toInt();
      populate();
    }
//  ToDo
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit" || param.toString() == "new")
    {
      _mode = cEdit;

      omfgThis->inputManager()->notify(cBCCountTag, this, this, SLOT(sCatchCounttagid(int)));

      _thaw->hide();

      if(param.toString() == "edit")
        _countTagNumber->setEnabled(false);
    }
    else if (param.toString() == "post")
    {
      _mode = cPost;
      _captive = true;

      setWindowTitle("Post Count Tag");
      _countTagNumber->setEnabled(false);
      _qty->setEnabled(false);
      _enter->setText(tr("&Post Count"));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      setWindowTitle("Count Tag");
      _qty->setEnabled(false);
      _thaw->hide();
      _newComments->setEnabled(false);
      _enter->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  return NoError;
}

void countTag::sCatchCounttagid(int pCnttagid)
{
  _cnttagid = pCnttagid;
  populate();
  _qty->setFocus();
}

void countTag::sEnter()
{
  XSqlQuery countEnter;
  if (_mode == cEdit)
  {
    countEnter.prepare("SELECT enterCount(:cnttagid, :qty, :comments);");
    countEnter.bindValue(":cnttagid", _cnttagid);
    countEnter.bindValue(":qty", _qty->toDouble());
    countEnter.bindValue(":comments", _newComments->toPlainText());
    countEnter.exec();
    if (countEnter.lastError().type() != QSqlError::NoError)
    {
      systemError(this, countEnter.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  else if (_mode == cPost)
  {

//  Make sure that there aren't any unposted Slips for this Tag
    countEnter.prepare( "SELECT cntslip_id "
               "FROM cntslip "
               "WHERE ( (NOT cntslip_posted)"
               " AND (cntslip_cnttag_id=:cnttag_id) ) "
               "LIMIT 1;" );
    countEnter.bindValue(":cnttag_id", _cnttagid);
    countEnter.exec();
    if (countEnter.first())
    {
      QMessageBox::critical( this, tr("Cannot Post Count Tag"),
                             tr( "There are unposted Count Slips for this Count Tag.\n"
                                 "You must either post or delete unposted Count Slips for this Count Tag before you may post this Tag." ) );

      return;
    }
    else if (countEnter.lastError().type() != QSqlError::NoError)
    {
      systemError(this, countEnter.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    countEnter.prepare("SELECT postCountTag(:cnttag_id, :thaw) AS result;");
    countEnter.bindValue(":cnttag_id", _cnttagid);
    countEnter.bindValue(":thaw", QVariant(_thaw->isChecked()));
    countEnter.exec();
    if (countEnter.first())
    {
      switch (countEnter.value("result").toInt())
      {
        case -1:
          QMessageBox::critical(this, tr("Cannot Post Count Tag"),
                                tr("The total quantity indicated by posted "
                                   "Count Slips for this Count Tag is greater "
                                   "than the quantity entered for this Count "
                                   "Tag. Please verify the Count Tag quantity "
                                   "and attempt to post this Count Tag again.") );
          return;

        case -2:
          QMessageBox::critical( this, tr("Cannot Post Count Tag"),
                                  tr( "This Item Site is Lot/Serial # controlled, which means the\n"
                                      "Count Tag Qty. must match the Qty. of Count Slips posted to it.\n"
                                      "\n"
                                      "Please verify the Count Tag Qty. and attempt to post again." ) );
          return;
          
        case -3:
          QMessageBox::critical( this, tr("Cannot Post Count Tag"),
                                  tr( "Either total quantity indicated by posted Count Slips for this Count Tag\n"
                                      "is less than the quantity entered for this Count Tag or there are no Count\n"
                                      "Slips posted to this Count Tag.  The Item Site in question does not have a\n"
                                      "default Location into which the remaining quantity may be distributed.\n"
                                      "Please verify the Count Tag quantity and attempt to post this Count Tag again." ) );
          return;
          
        case -4:
          QMessageBox::critical( this, tr("Cannot Post Count Tag"),
                                  tr( "Either total quantity indicated by posted Count Slips for this Count Tag\n"
                                      "is less than the quantity entered for this Count Tag or there are no Count\n"
                                      "Slips posted to this Count Tag.  This database has been configured to disallow\n"
                                      "default Count Tag posting into the default Location for an Item Site.\n"
                                      "Please verify the Count Tag quantity and attempt to post this Count Tag again." ) );
          return;
          
        case 0:
          break;
        
        default:
          QMessageBox::critical( this, tr("Cannot Post Count Tag"),
                                 tr( "An unknown error occurred while posting this Count Tag.\n"
                                     "Please contact your Systems Administrator and report this issue." ) );
          break;
      }
    }
    else if (countEnter.lastError().type() != QSqlError::NoError)
    {
      systemError(this, countEnter.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_captive)
    accept();
  else
    clear();
}

void countTag::sCountTagList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);
  params.append("tagType", cUnpostedCounts);

  countTagList newdlg(this, "", true);
  newdlg.set(params);
  _cnttagid = newdlg.exec();

  populate();

  _qty->setFocus();
}

void countTag::sParseCountTagNumber()
{
  XSqlQuery countParseCountTagNumber;
  QString sql( "SELECT invcnt_id "
             "FROM invcnt "
             "JOIN itemsite ON (invcnt_itemsite_id=itemsite_id) "
             "JOIN site() ON (itemsite_warehous_id=warehous_id) "
             "WHERE ( (NOT invcnt_posted)"
             " AND (UPPER(invcnt_tagnumber)=UPPER(<? value(\"cnttag_tagnumber\") ?>)) );" );
             
  ParameterList ctp;
  ctp.append("cnttag_tagnumber", _countTagNumber->text().trimmed());
      
  MetaSQLQuery ctq(sql);
  countParseCountTagNumber = ctq.toQuery(ctp);
  if (countParseCountTagNumber.first())
  {
    _cnttagid = countParseCountTagNumber.value("invcnt_id").toInt();
    populate();
  }
  else if (countParseCountTagNumber.lastError().type() != QSqlError::NoError)
  {
    systemError(this, countParseCountTagNumber.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    clear();
}

void countTag::populate()
{
  XSqlQuery countpopulate;
  countpopulate.prepare( "SELECT invcnt_tagnumber, invcnt_itemsite_id,"
             "       invcnt_comments, itemsite_freeze,"
             "       warehous_useslips,"
             "       invcnt_qoh_after,"
             "       COALESCE( (SELECT CASE WHEN (LENGTH(location_descrip) > 0) THEN (formatLocationName(location_id) || '-' || location_descrip)"
             "                              ELSE formatLocationName(location_id)"
             "                              END "
             "                    FROM location"
             "                   WHERE (location_id=invcnt_location_id)), :na) AS f_location "
             "FROM invcnt, itemsite, whsinfo "
             "WHERE ( (invcnt_itemsite_id=itemsite_id)"
             " AND (itemsite_warehous_id=warehous_id)"
             " AND (invcnt_id=:cnttag_id) );" );
  countpopulate.bindValue(":cnttag_id", _cnttagid);
  countpopulate.bindValue(":na", tr("N/A"));
  countpopulate.exec();
  if (countpopulate.first())
  {
    if ((_mode == cView) && (countpopulate.value("invcnt_tagnumber").toString() == "") )
      _countTagNumber->setText("Misc.");
    else
      _countTagNumber->setText(countpopulate.value("invcnt_tagnumber").toString());

    _item->setItemsiteid(countpopulate.value("invcnt_itemsite_id").toInt());
    _qty->setDouble(countpopulate.value("invcnt_qoh_after").toDouble());
    _qty->setEnabled(!countpopulate.value("warehous_useslips").toBool());
    _thaw->setChecked(countpopulate.value("itemsite_freeze").toBool());
    _currentComments->setText(countpopulate.value("invcnt_comments").toString());
    _location->setText(countpopulate.value("f_location").toString());
  }
  else if (countpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, countpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    clear();
}

void countTag::clear()
{
  _cnttagid = -1;
  _countTagNumber->clear();
  _item->setItemsiteid(-1);
  _qty->clear();
  _thaw->setChecked(false);
  _currentComments->clear();
  _newComments->clear();
  _countTagNumber->setFocus();
  _location->setText(tr("N/A"));
}
