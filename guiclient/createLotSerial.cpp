/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createLotSerial.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

createLotSerial::createLotSerial(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl),
      _lotsFound(false)
{
  setupUi(this);

  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));
  connect(_lotSerial, SIGNAL(textChanged(QString)), this, SLOT(sHandleLotSerial()));
  connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sHandleCharacteristics()));

  _item->setReadOnly(true);

  _serial = false;
  _itemsiteid = -1;
  _preassigned = false;
  adjustSize();

  _qtyToAssign->setValidator(omfgThis->qtyVal());

  _charWidgets = LotSerialUtils::addLotCharsToGridLayout(this, gridLayout, _lschars);
}

createLotSerial::~createLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void createLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createLotSerial::set(const ParameterList &pParams)
{
  XSqlQuery createet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemloc_series", &valid);
  if (valid)
    _itemlocSeries = param.toInt();

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();

    createet.prepare( "SELECT item_fractional, itemsite_controlmethod, itemsite_item_id,"
               "       itemsite_id, itemsite_perishable, itemsite_warrpurc, "
               "       COALESCE(itemsite_lsseq_id,-1) AS itemsite_lsseq_id, "
               "       invhist_ordtype, invhist_transtype, invhist_ordnumber "
               "FROM itemlocdist, itemsite, item, invhist "
               "WHERE ( (itemlocdist_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemlocdist_invhist_id=invhist_id) "
               " AND (itemlocdist_id=:itemlocdist_id) );" );
    createet.bindValue(":itemlocdist_id", _itemlocdistid);
    createet.exec();
    if (createet.first())
    {
      if (createet.value("itemsite_controlmethod").toString() == "S")
      {
        _serial = true;
        _qtyToAssign->setText("1");
        _qtyToAssign->setEnabled(false);
      }
      else
        _serial = false;

      _item->setItemsiteid(createet.value("itemsite_id").toInt());
      _itemsiteid = createet.value("itemsite_id").toInt();
      _expiration->setEnabled(createet.value("itemsite_perishable").toBool());
      _warranty->setEnabled(createet.value("itemsite_warrpurc").toBool() && createet.value("invhist_ordtype").toString() == "PO");
      _fractional = createet.value("item_fractional").toBool();
      
      //If there is preassigned trace info for an associated order, force user to select from list
      //Pick latest lsdetail record as it can be duplicated when lot/serial returned/re-added to the shipment
      XSqlQuery preassign;
      preassign.prepare("SELECT MAX(lsdetail_id) AS lsdetail_id,ls_number,ls_number "
                        "FROM lsdetail JOIN ls ON (ls_id=lsdetail_ls_id) "
                        "WHERE ( (lsdetail_source_number=:docnumber) "
                        "AND (lsdetail_source_type=:transtype) "
                        "AND (lsdetail_itemsite_id IN (SELECT itemsite_id FROM itemsite "
                        "                              WHERE itemsite_item_id = (SELECT itemsite_item_id "
                        "                               FROM itemsite WHERE itemsite_id = :itemsite))) "
                        "AND (lsdetail_qtytoassign > 0) ) "
                        "GROUP BY 2,3");
      preassign.bindValue(":transtype", createet.value("invhist_transtype").toString());
      preassign.bindValue(":docnumber", createet.value("invhist_ordnumber").toString());
      preassign.bindValue(":itemsite", createet.value("itemsite_id").toInt());
      preassign.exec();
      if (preassign.first())
      {
        _lotSerial->setAllowNull(true);
        _lotSerial->populate(preassign);
        _preassigned = true;
        connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sLotSerialSelected()));
      }
      else if (preassign.lastError().type() != QSqlError::NoError)
      {
        systemError(this, preassign.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      else if (createet.value("itemsite_lsseq_id").toInt() != -1)
      {
        // Auto sequence
        XSqlQuery fetchlsnum;
        fetchlsnum.prepare("SELECT fetchlsnumber(:lsseq_id) AS lotserial;");
        fetchlsnum.bindValue(":lsseq_id", createet.value("itemsite_lsseq_id").toInt());
        fetchlsnum.exec();
        if (fetchlsnum.first())
        {
          _lotSerial->setAllowNull(true);
          _lotSerial->setText(fetchlsnum.value("lotserial").toString());
        }
        else if (fetchlsnum.lastError().type() != QSqlError::NoError)
        {
          systemError(this, fetchlsnum.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
      }
      else if (!_serial) {
        XSqlQuery lots;
        lots.prepare("SELECT itemloc_id, ls_number, ls_number "
                     "FROM ls "
                     " JOIN itemloc ON (itemloc_ls_id=ls_id) "
                     "WHERE (itemloc_itemsite_id=:itemsite_id);");
        lots.bindValue(":itemsite_id", createet.value("itemsite_id").toInt());
        lots.exec();
        if (lots.lastError().type() != QSqlError::NoError)
        {
          systemError(this, lots.lastError().databaseText(), __FILE__, __LINE__);
          return UndefinedError;
        }
        else {
          _lotSerial->setAllowNull(true);
          _lotSerial->populate(lots);
          connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sLotSerialSelected()));
        }
      }
    }
    else if (createet.lastError().type() != QSqlError::NoError)
    {
      systemError(this, createet.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }
  
  param = pParams.value("qtyRemaining", &valid);
  if (valid)
    _qtyRemaining->setText(param.toString());

  return NoError;
}

void createLotSerial::sHandleLotSerial()
{
    _lotSerial->setText(_lotSerial->currentText().toUpper());
}

void createLotSerial::sHandleCharacteristics()
{
    if (_lotSerial->currentText().length() == 0)
    {
        clearCharacteristics();
        return;
    }

    int ls_id = -1;
    XSqlQuery lotcharq;
    lotcharq.prepare(QString("SELECT ls_id FROM ls WHERE ls_item_id=:ls_item_id AND ls_number=:ls_number"));
    lotcharq.bindValue(":ls_number", _lotSerial->currentText());
    lotcharq.bindValue(":ls_item_id", _item->id());
    bool success = lotcharq.exec();
    if (!success)
    {
        qDebug() << __FUNCTION__ << __LINE__ << lotcharq.lastError().text();
        return;
    }
    if( lotcharq.first() )
    {
        ls_id = lotcharq.value("ls_id").toInt();
    }
    _lotsFound = ls_id > -1;
    if (_lotsFound)
    {
        XSqlQuery charQuery;
        charQuery.prepare(QString("SELECT DISTINCT char.char_name, char.char_type, charass.charass_value "
                          " FROM charass INNER JOIN char ON charass.charass_char_id=char.char_id "
                          " WHERE charass.charass_target_type='LS' AND "
                          " charass.charass_target_id=%1 "
                          " ORDER BY char.char_name ASC").arg(ls_id));
        success = charQuery.exec();
        if (!success)
        {
            qDebug() << __FUNCTION__ << __LINE__ << charQuery.lastError().text();
        }
        for (int i = 0; charQuery.next() && i < _charWidgets.length(); i++)
        {
            QString charass_value = charQuery.value("charass_value").toString();
            int char_type = charQuery.value("char_type").toInt();
            if (char_type == 2)
            {
                DLineEdit *l = qobject_cast<DLineEdit *>(_charWidgets.at(i));
                if (l)
                    l->setDate(QDate::fromString(charass_value, "yyyy-MM-dd"));
            }
            else if (char_type == 1)
            {
                XComboBox *x = qobject_cast<XComboBox *>(_charWidgets.at(i));
                if (x) {
                  int index = x->findText(charass_value);
                  if (index > -1) {
                    x->setCurrentIndex(index);
                  }
                }
            }
            else
            {
                QLineEdit *l = qobject_cast<QLineEdit *>(_charWidgets.at(i));
                if (l)
                    l->setText(charass_value);
            }
        }
    }
    else
    {
      clearCharacteristics();
    }
    foreach (QWidget *w, _charWidgets)
    {
        w->setEnabled(!_lotsFound);
    }
}

void createLotSerial::clearCharacteristics()
{
  QList<int> char_types = _lschars.getLotCharTypes();
  for (int i=0; i < _lschars.numLotChars(); i++)
  {
      if (char_types.at(i) == 2)
      {
          DLineEdit *l = qobject_cast<DLineEdit *>(_charWidgets.at(i));
          if (l)
            l->clear();
      }
      else if (char_types.at(i) == 1)
      {
          XComboBox *x = qobject_cast<XComboBox *>(_charWidgets.at(i));
          if (x)
            x->setCurrentIndex(0);
      }
      else
      {
          QLineEdit *l = qobject_cast<QLineEdit *>(_charWidgets.at(i));
          if (l)
            l->clear();
      }
   }
}

void createLotSerial::sAssign()
{
  XSqlQuery createAssign;
  int  lsid=-1;
  
  if (_lotSerial->currentText().isEmpty())
  {
    QMessageBox::critical( this, tr("Enter Lot/Serial Number"),
                           tr("<p>You must enter a Lot/Serial number."));
    _lotSerial->setFocus();
    return;
  }
  else if (_lotSerial->currentText().contains(QRegExp("\\s")) &&
           QMessageBox::question(this, tr("Lot/Serial Number Contains Spaces"),
                                 tr("<p>The Lot/Serial Number contains spaces. Do "
                                    "you want to save it anyway?"),
                                 QMessageBox::Yes,
                                 QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
  {
    _lotSerial->setFocus();
    return;
  }

  if (_qtyToAssign->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity"),
                           tr("<p>You must enter a positive value to assign to "
                              "this Lot/Serial number.") );
    _qtyToAssign->setFocus();
    return;
  }

  if ( (_expiration->isEnabled()) && (!_expiration->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Expiration Date"),
                           tr("<p>You must enter an expiration date to this "
                              "Perishable Lot/Serial number.") );
    _expiration->setFocus();
    return;
  }
  
  if ( (_warranty->isEnabled()) && (!_warranty->isValid()) )
  {
    QMessageBox::critical( this, tr("Enter Warranty Expire Date"),
                           tr("<p>You must enter a warranty expiration date for this "
                              "Lot/Serial number.") );
    _warranty->setFocus();
    return;
  }

  if (!_fractional)
  {
    if (_qtyToAssign->toDouble() != _qtyToAssign->text().toInt())
    {
      QMessageBox::critical( this, tr("Item is Non-Fractional"),
                             tr( "<p>The Item in question is not stored in "
                                 "fractional quantities. You must enter a "
                                 "whole value to assign to this Lot/Serial "
                                 "number." ) );
      _qtyToAssign->setFocus();
      return;
    }
  }
  
  if (_preassigned)
  {
    createAssign.prepare("SELECT SUM(lsd.lsdetail_qtytoassign) AS qtytoassign "
              "FROM lsdetail lsd "
              "JOIN lsdetail lsd2 "
              "  ON (lsd.lsdetail_source_id=lsd2.lsdetail_source_id "
              "    AND lsd.lsdetail_ls_id = lsd2.lsdetail_ls_id "
              "    AND lsd.lsdetail_source_type = lsd2.lsdetail_source_type "
              "    AND lsd.lsdetail_source_number = lsd2.lsdetail_source_number) "
              " WHERE (lsd2.lsdetail_id=:lsdetail_id);");
    createAssign.bindValue(":lsdetail_id", _lotSerial->id());
    createAssign.exec();
    if (createAssign.first())
    {
      if ( _qtyToAssign->toDouble() > createAssign.value("qtytoassign").toDouble() )
      {
        QMessageBox::critical( this, tr("Invalid Qty"),
                               tr( "<p>The quantity being assigned is greater than the "
                                   " quantity preassigned to the order being received." ) );
        return;
      }
    }
    else
    {
      QMessageBox::critical( this, tr("Invalid Number"),
                             tr( "<p>The number entered is not valid.  Please select from the list "
                                 "of valid numbers." ) );
      _lotSerial->removeItem(_lotSerial->currentIndex());
      _lotSerial->setCurrentIndex(0);
      return;
    }
  }

  if (_serial)
  {
    createAssign.prepare("SELECT COUNT(*) AS count FROM "
              "(SELECT itemloc_id AS count "
              "FROM itemloc,itemsite,ls "
              "WHERE ((itemloc_itemsite_id=:itemsite_id)"
              "  AND (itemloc_itemsite_id=itemsite_id)"
              "  AND (itemsite_item_id=ls_item_id)"
              "  AND (itemloc_ls_id=ls_id)"
              "  AND (UPPER(ls_number)=UPPER(:lotserial)))"
              "UNION "
              "SELECT itemlocdist_id "
              "FROM itemlocdist,itemsite,ls "
              "WHERE ((itemlocdist_itemsite_id=:itemsite_id) "
              "  AND (itemlocdist_itemsite_id=itemsite_id)"
              "  AND (itemlocdist_ls_id=ls_id)"
              "  AND (UPPER(ls_number)=UPPER(:lotserial)) "
              "  AND (itemlocdist_source_type='D'))) as data;");
    createAssign.bindValue(":itemsite_id", _itemsiteid);
    createAssign.bindValue(":lotserial", _lotSerial->currentText());
    createAssign.exec();
    if (createAssign.first())
    {
      if (createAssign.value("count").toInt() > 0)
      {
        QMessageBox::critical(this, tr("Duplicate Serial Number"),
                              tr("This Serial Number has already been used "
                                 "and cannot be reused."));

        return;
      }
    }
    else if (createAssign.lastError().type() != QSqlError::NoError)
    {
      systemError(this, createAssign.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    createAssign.prepare("SELECT ls_id, ls_number "
              "FROM ls,itemsite "
              "WHERE ( (UPPER(ls_number)=UPPER(:lotserial)) "
              "AND (ls_item_id=itemsite_item_id) "
              "AND (itemsite_id=:itemsiteid) );");
    createAssign.bindValue(":itemsiteid", _itemsiteid);
    createAssign.bindValue(":lotserial", _lotSerial->currentText());
    createAssign.exec();
    if (createAssign.first())
    {
      if (!_serial)
        if (QMessageBox::question(this, tr("Use Existing?"),
                                  tr("<p>A record with for lot number %1 for this item already exists.  "
                                     "Reference existing lot?").arg(createAssign.value("ls_number").toString().toUpper()),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No  | QMessageBox::Escape) == QMessageBox::No)
          return;
      lsid=createAssign.value("ls_id").toInt();
    }
    else if (createAssign.lastError().type() != QSqlError::NoError)
    {
      systemError(this, createAssign.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  QString sql;
  if (_preassigned)
    sql = "SELECT createlotserial(:itemsite_id,:lotserial,:itemlocseries,lsdetail_source_type,lsdetail_source_id,:itemlocdist_id,:qty,:expiration,:warranty) "
          "FROM lsdetail,itemlocdist "
          "WHERE ((lsdetail_id=:lsdetail_id)"
          "AND (itemlocdist_id=:itemlocdist_id));";

  else
    sql = "SELECT createlotserial(:itemsite_id,:lotserial,:itemlocseries,'I',NULL,itemlocdist_id,:qty,:expiration,:warranty) "
          "FROM itemlocdist "
          "WHERE (itemlocdist_id=:itemlocdist_id);";

  createAssign.prepare(sql);
  createAssign.bindValue(":itemsite_id", _itemsiteid);
  createAssign.bindValue(":lotserial", _lotSerial->currentText().toUpper());
  createAssign.bindValue(":itemlocseries", _itemlocSeries);
  createAssign.bindValue(":lsdetail_id", _lotSerial->id());
  createAssign.bindValue(":qty", _qtyToAssign->toDouble());
  if (_expiration->isEnabled())
    createAssign.bindValue(":expiration", _expiration->date());
  else
    createAssign.bindValue(":expiration", omfgThis->endOfTime());
  if (_warranty->isEnabled())
    createAssign.bindValue(":warranty", _warranty->date());
  createAssign.bindValue(":itemlocdist_id", _itemlocdistid);
  createAssign.exec();
  if (createAssign.lastError().type() != QSqlError::NoError)
  {
    systemError(this, createAssign.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (!_lotsFound)
  {
      /* createlotserial calls "NEXTVAL('ls_ls_id_seq') which will advance the
       * next ls_id.  Fetch the last ls_id */
      int ls_id = -1;
      XSqlQuery ls_id_query;
      ls_id_query.prepare("SELECT ls_id FROM ls WHERE ls_number=:lotserial");
      ls_id_query.bindValue(":lotserial", _lotSerial->currentText().toUpper());
      ls_id_query.exec();
      if (ls_id_query.first()) {
          ls_id = ls_id_query.value("ls_id").toInt();
          _lschars.updateLotCharacteristics(ls_id, _charWidgets);
      }
  }

  accept();
}

void createLotSerial::sLotSerialSelected()
{
    if (!_expiration->isEnabled() &&
        !_warranty->isEnabled())
      return;

    if (_lotSerial->currentText().length() == 0)
    {
        _expiration->clear();
        _warranty->clear();
        return;
    }

    XSqlQuery itemloc;
    itemloc.prepare("SELECT itemloc_expiration, itemloc_warrpurc "
                    "FROM itemloc "
                    "WHERE itemloc_id=:itemloc_id "
                    "UNION "
                    "SELECT itemloc_expiration, itemloc_warrpurc "
                    "FROM lsdetail JOIN itemloc ON (itemloc_itemsite_id=lsdetail_itemsite_id AND itemloc_ls_id=lsdetail_ls_id) "
                    "WHERE lsdetail_id=:itemloc_id "
                    "UNION "
                    "SELECT lsdetail_expiration, lsdetail_warrpurc "
                    "FROM lsdetail "
                    "WHERE lsdetail_id=:itemloc_id; ");
    itemloc.bindValue(":itemloc_id", _lotSerial->id());
    itemloc.exec();
    if (itemloc.first()) {
      if (_expiration->isEnabled() &&
          itemloc.value("itemloc_expiration").toDate() != omfgThis->endOfTime())
        _expiration->setDate(itemloc.value("itemloc_expiration").toDate());
      if (_warranty->isEnabled())
        _warranty->setDate(itemloc.value("itemloc_warrpurc").toDate());
    }
}
