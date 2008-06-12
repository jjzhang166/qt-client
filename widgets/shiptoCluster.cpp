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

//  shiptoCluster.cpp
//  Created 03/07/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "shipToList.h"
#include "shiptocluster.h"

//   ShiptoLineEdit - A Ship To # validating XLineEdit
ShiptoEdit::ShiptoEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _custid = -1;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void ShiptoEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    if (text().length() == 0)
      setId(-1);

    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id "
                    "  FROM shipto "
                    " WHERE ( (shipto_cust_id=:cust_id)"
                    "   AND   (UPPER(shipto_num)=UPPER(:shipto_num)) );" );
    shipto.bindValue(":cust_id", _custid);
    shipto.bindValue(":shipto_num", text());
    shipto.exec();
    if (shipto.first())
      setId(shipto.value("shipto_id").toInt());
    else
      clear();
  }
}

void ShiptoEdit::setId(int pId)
{
  if (pId != -1)
  {
    XSqlQuery shipto( QString( "SELECT shipto_num, shipto_name, shipto_address1, shipto_cust_id "
                               "FROM shipto "
                               "WHERE (shipto_id=%1);" )
                      .arg(pId) );
    if (shipto.first())
    {
      _id = pId;
      _valid = TRUE;

      setText(shipto.value("shipto_num").toString());

      if (shipto.value("shipto_cust_id").toInt() != _custid)
      {
        _custid = shipto.value("shipto_cust_id").toInt();
        emit newCustid(_id);
      }

      emit newId(_id);
      emit valid(_valid);
      emit nameChanged(shipto.value("shipto_name").toString());
      emit address1Changed(shipto.value("shipto_address1").toString());
    }
  }
  else
  {
    clear();

    _custid = -1;
    emit newCustid(_custid);
  }
}

void ShiptoEdit::setCustid(int pCustid)
{
  if (pCustid != _custid)
  {
    clear();

    _custid = pCustid;
    emit newCustid(pCustid);

    if (_custid == -1)
    {
      setEnabled(FALSE);
      emit disableList(TRUE);
    }
    else
    {
      setEnabled(TRUE);
      emit disableList(FALSE);
    }
  }
}

void ShiptoEdit::clear()
{
  _id = -1;
  _valid = FALSE;

  setText("");

  emit nameChanged("");
  emit address1Changed("");
  emit newId(_id);
  emit valid(_valid);
}


ShiptoCluster::ShiptoCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
//  Create the component Widgets
  Q3HBoxLayout *_layoutMain = new Q3HBoxLayout( this, 0, 5, "_layoutMain"); 
  Q3VBoxLayout *_layoutLit = new Q3VBoxLayout( 0, 0, 0, "_layoutLit"); 
  Q3VBoxLayout *_layoutData = new Q3VBoxLayout( 0, 0, 0, "_layoutData"); 
  Q3HBoxLayout *_layoutShipto = new Q3HBoxLayout( 0, 0, 0, "_layoutShipto"); 
  Q3HBoxLayout *_layoutShiptoNumber = new Q3HBoxLayout( 0, 0, 7, "_layoutShiptoNumber"); 

  QLabel *_shiptoNumberLit = new QLabel(tr("Ship-To #:"), this, "_shiptoNumberLit");
  _shiptoNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _shiptoNumberLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
  QSize size = _shiptoNumberLit->minimumSize();
  size.setHeight(28);
  _shiptoNumberLit->setMinimumSize(size);
  _layoutLit->addWidget( _shiptoNumberLit );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  _layoutLit->addItem( spacer );
  _layoutMain->addLayout( _layoutLit );

  _shiptoNumber = new ShiptoEdit( this, "_shiptoNumber" );
  _shiptoNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _layoutShiptoNumber->addWidget( _shiptoNumber );

  _list = new QPushButton(tr( "..." ), this, "_list");
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  _layoutShiptoNumber->addWidget( _list );
  _layoutShipto->addLayout( _layoutShiptoNumber );

  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  _layoutShipto->addItem( spacer_2 );
  _layoutData->addLayout( _layoutShipto );

  _shiptoName = new QLabel(this, "_shiptoName");
  _shiptoName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _shiptoName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _layoutData->addWidget( _shiptoName );

  _shiptoAddress1 = new QLabel( this, "_shiptoAddress1" );
  _shiptoAddress1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _shiptoAddress1->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _layoutData->addWidget( _shiptoAddress1 );
  _layoutMain->addLayout( _layoutData );

//  Make some internal connections
  connect(_list,  SIGNAL(clicked()),  this, SLOT(sList()));
  connect(_shiptoNumber, SIGNAL(requestList()), this, SLOT(sList()));
  connect(_shiptoNumber, SIGNAL(nameChanged(const QString &)), _shiptoName, SLOT(setText(const QString &)));
  connect(_shiptoNumber, SIGNAL(address1Changed(const QString &)), _shiptoAddress1, SLOT(setText(const QString &)));
  connect(_shiptoNumber, SIGNAL(disableList(bool)), _list, SLOT(setDisabled(bool)));

  connect(_shiptoNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_shiptoNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));
  connect(_shiptoNumber, SIGNAL(newCustid(int)), this, SIGNAL(newCustid(int)));

  setFocusProxy(_shiptoNumber);

  setCustid(-1);
}

void ShiptoCluster::setId(int pId)
{
  _shiptoNumber->setId(pId);
}

void ShiptoCluster::setCustid(int pId)
{
  _shiptoNumber->setCustid(pId);
}

void ShiptoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _shiptoNumber->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _shiptoNumber->setEnabled(TRUE);
    _list->show();
  }
}

void ShiptoCluster::sList()
{
  ParameterList params;
  params.append("cust_id", _shiptoNumber->_custid);
  params.append("shipto_id", _shiptoNumber->_id);

  shipToList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  _shiptoNumber->setId(id);

  if (id != -1)
  {
    _shiptoNumber->setFocus();
    focusNextPrevChild(TRUE);
  }
}

