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

//  custCluster.cpp
//  Created 02/27/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <Q3DragObject>
#include <QHBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSqlError>
#include <QVBoxLayout>
#include <QValidator>

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "crmacctcluster.h"

#include "custcluster.h"

//  Routines for CLineEdit - a customer and prospect validating QLineEdit
CLineEdit::CLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setMaximumWidth(100);

  setAcceptDrops(TRUE);

  _id       = -1;
  _valid    = FALSE;
  _parsed   = TRUE;
  _dragging = FALSE;
  _type     = AllCustomers;
  _autoFocus = true;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
  connect(this, SIGNAL(requestSearch()), this, SLOT(sSearch()));
  connect(this, SIGNAL(requestList()), this, SLOT(sList()));
  
  _mapper = new XDataWidgetMapper(this);
}

void CLineEdit::setAutoFocus(bool yes)
{
  _autoFocus = yes;
}

void CLineEdit::sEllipses()
{
  if(_x_preferences)
  {
    if(_x_preferences->value("DefaultEllipsesAction") == "search")
    {
      sSearch();
      return;
    }
  }

  sList();
}

void CLineEdit::sSearch()
{
  CRMAcctSearch * newdlg = new CRMAcctSearch(this);
  newdlg->setId(_id);

  int id = newdlg->exec();
  if (id != QDialog::Rejected)
  {
    setId(id);

    if (_autoFocus && id != -1)
      focusNextPrevChild(TRUE);
  }
}

void CLineEdit::sList()
{
  CRMAcctList * newdlg = new CRMAcctList(this);
  newdlg->setId(_id);

  int id = newdlg->exec();
  setId(id);

  if (_autoFocus && id != -1)
  {
    this->setFocus();
    focusNextPrevChild(TRUE);
  }
}

void CLineEdit::setSilentId(int pId)
{
//  Make sure there's a change
  if (pId == _id)
    return;

  if (pId != -1)
  {
    QString sql( "<? if exists(\"customer\") ?>"
                 "  SELECT cust_number,"
                 "         cust_name,"
                 "         addr_line1, addr_line2, addr_line3,"
                 "         addr_city,  addr_state, addr_postalcode,"
                 "         addr_country, cust_creditstatus, "
                 "         cntct_addr_id, cust_cntct_id "
                 "  FROM custinfo LEFT OUTER JOIN "
                 "       cntct ON (cust_cntct_id=cntct_id) LEFT OUTER JOIN "
                 "       addr  ON (cntct_addr_id=addr_id) "
                 "  WHERE ( (cust_id=<? value(\"id\") ?>) "
                 "  <? if exists(\"active\") ?>"
                 "   AND (cust_active)"
                 "  <? endif ?>"
                 "  )"
                 "<? endif ?>"
                 "<? if exists(\"prospect\") ?>"
                 "  <? if exists(\"customer\") ?>"
                 "    UNION"
                 "  <? endif ?>"
                 "  SELECT prospect_number AS cust_number,"
                 "         prospect_name AS cust_name,"
                 "         addr_line1, addr_line2, addr_line3,"
                 "         addr_city,  addr_state, addr_postalcode,"
                 "         addr_country, '' AS cust_creditstatus, "
                 "         cntct_addr_id, prospect_cntct_id AS cust_cntct_id "
                 "  FROM prospect LEFT OUTER JOIN "
                 "       cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN "
                 "       addr  ON (cntct_addr_id=addr_id) "
                 "  WHERE ( (prospect_id=<? value(\"id\") ?>) "
                 "  <? if exists(\"active\") ?>"
                 "   AND (prospect_active)"
                 "  <? endif ?>"
                 "  )"
                 "<? endif ?>"
                 ";");

    ParameterList params;
    params.append("id", pId);
    switch (_type)
    {
      case ActiveCustomers:
        params.append("active");
        // fall-through
      case AllCustomers:
        params.append("customer");
        break;

      case ActiveProspects:
        params.append("active");
        // fall-through
      case AllProspects:
        params.append("prospect");
        break;

      case ActiveCustomersAndProspects:
        params.append("active");
        // fall-through
      case AllCustomersAndProspects:
        params.append("customer");
        params.append("prospect");
        break;
    }

    MetaSQLQuery mql(sql);
    XSqlQuery cust = mql.toQuery(params);

    if (cust.first())
    {
      _id = pId;

      setText(cust.value("cust_number").toString());
      
      if (_mapper->model() &&
          _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
        _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());

      emit custNumberChanged(cust.value("cust_number").toString());
      emit custNameChanged(cust.value("cust_name").toString());
      emit custAddr1Changed(cust.value("addr_line1").toString());
      emit custAddr2Changed(cust.value("addr_line2").toString());
      emit custAddr3Changed(cust.value("addr_line3").toString());
      emit custCityChanged(cust.value("addr_city").toString());
      emit custStateChanged(cust.value("addr_state").toString());
      emit custZipCodeChanged(cust.value("addr_postalcode").toString());
      emit custCountryChanged(cust.value("addr_country").toString());
      emit creditStatusChanged(cust.value("cust_creditstatus").toString());
      emit custAddressChanged(cust.value("cntct_addr_id").toInt());
      emit custContactChanged(cust.value("cust_cntct_id").toInt());

      _valid = TRUE;

      return;
    }
    else if (cust.lastError().type() != QSqlError::None)
      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                            .arg(__FILE__)
                            .arg(__LINE__),
                            cust.lastError().databaseText());
//qDebug("the query: %s", cust.lastQuery().toAscii().data());
  }

//  A valid cust could not be found, clear the cust information
  _id = -1;
  setText("");

  emit custNumberChanged("");
  emit custNameChanged("");
  emit custAddr1Changed("");
  emit custAddr2Changed("");
  emit custAddr3Changed("");
  emit custCityChanged("");
  emit custStateChanged("");
  emit custZipCodeChanged("");
  emit custCountryChanged("");
  emit creditStatusChanged("");
  emit custAddressChanged(-1);
  emit custContactChanged(-1);

  _valid = FALSE;
}


void CLineEdit::setId(int pId)
{
  setSilentId(pId);

//  Emit the item information signals
  emit newId(_id);
  emit valid(_valid);
}

void CLineEdit::setNumber(const QString& pNumber)
{
    _parsed = false;
    setText(pNumber);
    sParse();
}


void CLineEdit::setType(CLineEditTypes pType)
{
  _type = pType;
}

void CLineEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    if (text().length() == 0)
      setId(-1);

    QString sql("<? if exists(\"customer\") ?>"
                "  SELECT cust_number, cust_id"
                "  FROM custinfo "
                "  WHERE ((UPPER(cust_number)=UPPER(<? value(\"number\") ?>))"
                "  <? if exists(\"active\") ?>"
                "   AND (cust_active)"
                "  <? endif ?>"
                "  )"
                "<? endif ?>"
                "<? if exists(\"prospect\") ?>"
                "  <? if exists(\"customer\") ?>"
                "    UNION"
                "  <? endif ?>"
                "  SELECT prospect_number AS cust_number,"
                "         prospect_id AS cust_id "
                "  FROM prospect "
                "  WHERE ((UPPER(prospect_number)=UPPER(<? value(\"number\") ?>))"
                "  <? if exists(\"active\") ?>"
                "   AND (prospect_active)"
                "  <? endif ?>"
                "  )"
                "<? endif ?>"
                ";");

    ParameterList params;
    params.append("number", text().stripWhiteSpace());
    switch (_type)
    {
      case ActiveCustomers:
        params.append("active");
        // fall-through
      case AllCustomers:
        params.append("customer");
        break;

      case ActiveProspects:
        params.append("active");
        // fall-through
      case AllProspects:
        params.append("prospect");
        break;

      case ActiveCustomersAndProspects:
        params.append("active");
        // fall-through
      case AllCustomersAndProspects:
        params.append("customer");
        params.append("prospect");
        break;
    }

    MetaSQLQuery mql(sql);
    XSqlQuery cust = mql.toQuery(params);

    if (cust.first())
    {
      setText(cust.value("cust_number").toString());
      setId(cust.value("cust_id").toInt());
    }
    else
    {
      if (cust.lastError().type() != QSqlError::None)
        QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                              cust.lastError().databaseText());
      setText("");
      setId(-1);
    }
//qDebug("the query: %s", cust.lastQuery().toAscii().data());
  }
}

void CLineEdit::mousePressEvent(QMouseEvent *pEvent)
{
  QLineEdit::mousePressEvent(pEvent);

  if (_valid)
    _dragging = TRUE;
}

void CLineEdit::mouseMoveEvent(QMouseEvent *)
{
  if (_dragging)
  {
    Q3DragObject *drag = new Q3TextDrag(QString("custid=%1").arg(_id), this);
    drag->dragCopy();

    _dragging = FALSE;
  }
}

void CLineEdit::dragEnterEvent(QDragEnterEvent *pEvent)
{
  QString dragData;

  if (Q3TextDrag::decode(pEvent, dragData))
  {
    if (dragData.contains("custid="))
      pEvent->accept(TRUE);
  }

  pEvent->accept(FALSE);
}

void CLineEdit::dropEvent(QDropEvent *pEvent)
{
  QString dropData;

  if (Q3TextDrag::decode(pEvent, dropData))
  {
    if (dropData.contains("custid="))
    {
      QString target = dropData.mid((dropData.find("custid=") + 7), (dropData.length() - 7));

      if (target.contains(","))
        target = target.left(target.find(","));

      setId(target.toInt());
    }
  }
}

void CLineEdit::keyPressEvent(QKeyEvent * pEvent)
{
  if(pEvent->key() == Qt::Key_Tab)
    sParse();
  XLineEdit::keyPressEvent(pEvent);
}

CustInfoAction* CustInfo::_custInfoAction = 0;
CustInfo::CustInfo(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
//  Create and place the component Widgets
  QHBoxLayout *layoutMain = new QHBoxLayout(this, 0, 2, "layoutMain"); 
  QHBoxLayout *layoutNumber = new QHBoxLayout(0, 0, 6, "layoutNumber"); 
  QHBoxLayout *layoutButtons = new QHBoxLayout(0, 0, 6, "layoutButtons"); 

  QLabel *_customerNumberLit = new QLabel(tr("Customer #:"), this, "_customerNumberLit");
  _customerNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutNumber->addWidget(_customerNumberLit);

  _customerNumber = new CLineEdit(this, "_customerNumber");
  _customerNumber->setMinimumWidth(100);
  layoutNumber->addWidget(_customerNumber);
  layoutMain->addLayout(layoutNumber);

  _list = new QPushButton(tr("..."), this, "_list");
#ifndef Q_WS_MAC
        _list->setMaximumWidth(25);
#else
    _list->setMinimumWidth(60);
    _list->setMinimumHeight(32);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layoutButtons->addWidget(_list);

  _info = new QPushButton(tr("?"), this, "_info");
#ifndef Q_WS_MAC
  _info->setMaximumWidth(25);
#else
    _info->setMinimumWidth(60);
    _info->setMinimumHeight(32);
#endif
  _info->setFocusPolicy(Qt::NoFocus);
  _info->setMinimumWidth(60);

  layoutButtons->addWidget(_info);
  
  layoutMain->addLayout(layoutButtons);

//  Make some internal connections
  connect(_list, SIGNAL(clicked()), _customerNumber, SLOT(sEllipses()));
  connect(_info, SIGNAL(clicked()), this, SLOT(sInfo()));

  connect(_customerNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_customerNumber, SIGNAL(custNameChanged(const QString &)), this, SIGNAL(nameChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr1Changed(const QString &)), this, SIGNAL(address1Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr2Changed(const QString &)), this, SIGNAL(address2Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custAddr3Changed(const QString &)), this, SIGNAL(address3Changed(const QString &)));
  connect(_customerNumber, SIGNAL(custCityChanged(const QString &)), this, SIGNAL(cityChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custStateChanged(const QString &)), this, SIGNAL(stateChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custZipCodeChanged(const QString &)), this, SIGNAL(zipCodeChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custCountryChanged(const QString &)), this, SIGNAL(countryChanged(const QString &)));
  connect(_customerNumber, SIGNAL(custAddressChanged(const int)), this, SIGNAL(addressChanged(const int)));
  connect(_customerNumber, SIGNAL(custContactChanged(const int)), this, SIGNAL(contactChanged(const int)));
  connect(_customerNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_customerNumber, SIGNAL(creditStatusChanged(const QString &)), this, SLOT(sHandleCreditStatus(const QString &)));

  if(_x_privileges && (!_x_privileges->check("MaintainCustomerMasters") && !_x_privileges->check("ViewCustomerMasters")))
    _info->setEnabled(false);
  else
  {
    connect(_customerNumber, SIGNAL(valid(bool)), _info, SLOT(setEnabled(bool)));
    connect(_customerNumber, SIGNAL(requestInfo()), this, SLOT(sInfo()));
  }

  setFocusProxy(_customerNumber);
}

void CustInfo::setAutoFocus(bool yes)
{
  _customerNumber->setAutoFocus(yes);
}

void CustInfo::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _customerNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _customerNumber->setEnabled(TRUE);
    _list->show();
  }
}

void CustInfo::setId(int pId)
{
  _customerNumber->setId(pId);
}

void CustInfo::setSilentId(int pId)
{
  _customerNumber->setSilentId(pId);
}

void CustInfo::setType(CLineEdit::CLineEditTypes pType)
{
  _customerNumber->setType(pType);
}

void CustInfo::sInfo()
{
  if(_custInfoAction)
    _custInfoAction->customerInformation(parentWidget(), _customerNumber->_id);
}

void CustInfo::sHandleCreditStatus(const QString &pStatus)
{
  if ((pStatus == "G") || (pStatus == ""))
  {
    _info->setText(tr("?"));
    _info->setPaletteForegroundColor(QColor("black"));
  }
  else
  {
    _info->setText(tr("$"));

    if (pStatus == "W")
      _info->setPaletteForegroundColor(QColor("orange"));
    else if (pStatus == "H")
      _info->setPaletteForegroundColor(QColor("red"));
  }
}

void CustInfo::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_customerNumber, _fieldName, QByteArray("number"));
  _customerNumber->_mapper=m;
}


CustCluster::CustCluster(QWidget *parent, const char *name) :
  QWidget(parent, name)
{
//  Create the component Widgets
  QVBoxLayout *_main = new QVBoxLayout(this);
  _main->setMargin(0);
  _main->setSpacing(0);

  _custInfo = new CustInfo(this, "_custInfo");
  _custInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _main->addWidget(_custInfo);

  _name = new QLabel(this, "_name");
  _name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _main->addWidget(_name);

  _address1 = new QLabel(this, "_address1");
  _address1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _address1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _main->addWidget(_address1);

  setLayout(_main);

//  Make some internal connections
  connect(_custInfo, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_custInfo, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));
  connect(_custInfo, SIGNAL(nameChanged(const QString &)), _name, SLOT(setText(const QString &)));
  connect(_custInfo, SIGNAL(address1Changed(const QString &)), _address1, SLOT(setText(const QString &)));

  setFocusProxy(_custInfo);
}

void CustCluster::setAutoFocus(bool yes)
{
  _custInfo->setAutoFocus(yes);
}

void CustCluster::setReadOnly(bool pReadOnly)
{
  _custInfo->setReadOnly(pReadOnly);
}

void CustCluster::setId(int pId)
{
  _custInfo->setId(pId);
}

void CustCluster::setSilentId(int pId)
{
  _custInfo->setSilentId(pId);
}

void CustCluster::setType(CLineEdit::CLineEditTypes pType)
{
  _custInfo->setType(pType);
}
