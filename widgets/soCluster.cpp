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

//  soCluster.cpp
//  Created 03/04/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QLabel>
#include <QPushButton>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "salesOrderList.h"
#include "socluster.h"

//  Routines for SoLineEdit - an sohead validating XLineEdit
SoLineEdit::SoLineEdit(QWidget *pParent, const char *name) : XLineEdit(pParent, name)
{
  _id     = -1;
  _number = -1;
  _custid = -1;

  setValidator(new QIntValidator(0, 9999999, this));
  _mapper = new XDataWidgetMapper(this);
  
  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void SoLineEdit::setId(int pId)
{
  if (pId == _id)
    return;

  XSqlQuery sohead;
  sohead.prepare( "SELECT cohead_number, cohead_cust_id, cohead_billtoname "
                  "FROM cohead, cust "
                  "WHERE ( (cohead_cust_id=cust_id)"
                  " AND (cohead_id=:sohead_id) );" );
  sohead.bindValue(":sohead_id", pId);
  sohead.exec();
  if (sohead.first())
  {
    _id     = pId;
    _number = sohead.value("cohead_number").toInt();
    _custid = sohead.value("cohead_cust_id").toInt();
    _valid  = TRUE;

    emit numberChanged(sohead.value("cohead_number").toString());
    emit custNameChanged(sohead.value("cohead_billtoname").toString());

    setText(sohead.value("cohead_number").toString());
    
    if (_mapper->model() &&
        _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _custid = -1;
    _valid  = FALSE;

    emit numberChanged("");
    emit custNameChanged("");
    setText("");
  }

  emit newId(_id);
  emit custidChanged(_custid);
  emit numberChanged(_number);
  emit valid(_valid);

  _parsed = TRUE;
}

void SoLineEdit::setCustId(int pId)
{
  if (pId == _custid)
    return;
    
  _custid=pId;
  setNumber(_number);
  
  emit custidChanged(_custid);
}

void SoLineEdit::setNumber(QString pNumber)
{
  XSqlQuery sohead;

  if (_type == cSoOpen)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status NOT IN ('C','X'))"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoClosed)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (coitem_status='C')"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoAtShipping)
    sohead.prepare( "SELECT DISTINCT cohead_id "
                    "FROM cohead, coitem, cosmisc, coship "
                    "WHERE ( (coitem_cohead_id=cohead_id)"
                    " AND (cosmisc_cohead_id=cohead_id)"
                    " AND (coship_coitem_id=coitem_id)"
                    " AND (NOT cosmisc_shipped)"
                    " AND (cohead_number=:sohead_number) );" );
  else if (_type == cSoCustomer)
  {
    sohead.prepare( "SELECT cohead_id "
                    "FROM cohead "
                    "WHERE ( (cohead_number=:sohead_number) "
                    "AND (cohead_cust_id=:cust_id) );" );
    sohead.bindValue(":cust_id", _custid);
  }
  else
    sohead.prepare( "SELECT cohead_id "
                    "FROM cohead "
                    "WHERE (cohead_number=:sohead_number);" );

  sohead.bindValue(":sohead_number", pNumber);
  sohead.exec();
  if (sohead.first())
    setId(sohead.value("cohead_id").toInt());
  else
    setId(-1);
}

void SoLineEdit::clear()
{
  setId(-1);
}

void SoLineEdit::sParse()
{
  if (!_parsed)
    setNumber(text());
}


SoCluster::SoCluster(QWidget *pParent, const char *name) : QWidget(pParent, name)
{
  constructor();

  _soNumber->_type = 0;
}

SoCluster::SoCluster(int pType, QWidget *pParent) : QWidget(pParent)
{
  constructor();

  _soNumber->_type = pType;
}

void SoCluster::constructor()
{
  _readOnly = false;
  QVBoxLayout *_layoutMain = new QVBoxLayout(this);
  _layoutMain->setMargin(0);
  _layoutMain->setSpacing(5);
  QWidget * _layoutOrderNumberWidget = new QWidget(this);
  QHBoxLayout *_layoutOrderNumber = new QHBoxLayout(_layoutOrderNumberWidget);
  _layoutOrderNumber->setMargin(0);
  _layoutOrderNumber->setSpacing(5);

  _soNumberLit = new QLabel(tr("Sales Order #:"), _layoutOrderNumberWidget, "_soNumberLit");
  _soNumberLit->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  _soNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _layoutOrderNumber->addWidget(_soNumberLit);

  _soNumber = new SoLineEdit(_layoutOrderNumberWidget);
  _soNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _layoutOrderNumber->addWidget(_soNumber);

  _list = new QPushButton(tr("..."), _layoutOrderNumberWidget, "_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  _layoutOrderNumber->addWidget(_list);
  _layoutOrderNumberWidget->setLayout(_layoutOrderNumber);
  _layoutMain->addWidget(_layoutOrderNumberWidget);

  _custName = new QLabel(this, "_custName");
  _custName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _custName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _layoutMain->addWidget(_custName);

  setLayout(_layoutMain);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_soNumber, SIGNAL(requestList()), SLOT(sList()));
  connect(_soNumber, SIGNAL(custNameChanged(const QString &)), _custName, SLOT(setText(const QString &)));
  connect(_soNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_soNumber, SIGNAL(custidChanged(int)), this, SIGNAL(newCustid(int)));
  connect(_soNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_soNumber);
}

void SoCluster::setId(int pSoid)
{
  _soNumber->setId(pSoid);
}

void SoCluster::setCustId(int pCustid)
{
  _soNumber->setCustId(pCustid);
}

void SoCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_soNumber, _fieldName, QByteArray("number"));
  _soNumber->_mapper=m;
}

void SoCluster::setLabel(QString p)
{
  _label=p;
  _soNumberLit->setText(p);
}

void SoCluster::setNumber(QString pNumber)
{
  _soNumber->setNumber(pNumber);
}

void SoCluster::sList()
{
  ParameterList params;
  params.append("sohead_id", _soNumber->_id);
  params.append("soType", _soNumber->_type);
  params.append("cust_id", _soNumber->_custid);

  salesOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id = newdlg.exec()) != -1)
    _soNumber->setId(id);
}

void SoCluster::setReadOnly(bool ro)
{
  _soNumber->setEnabled(!ro);
  _list->setVisible(!ro);
}
