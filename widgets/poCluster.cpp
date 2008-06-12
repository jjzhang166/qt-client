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

//  poCluster.cpp
//  Created 02/27/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "purchaseOrderList.h"
#include "pocluster.h"


//  Routines for PoLineEdit - an pohead validating XLineEdit
PoLineEdit::PoLineEdit(QWidget *parent, const char *name) :
  XLineEdit(parent, name)
{
  _id     = -1;
  _number = -1;
  _valid  = FALSE;
  _type   = 0;

  _vendid = -1;

  setValidator(new QIntValidator(0, 9999999, this));

  setMaximumWidth(100);

  connect(this, SIGNAL(lostFocus()), SLOT(sParse()));
  connect(this, SIGNAL(textChanged(QString)), SLOT(sMarkDirty()));
}

// TODO: this really should be in XLineEdit
void PoLineEdit::sMarkDirty()
{
  _parsed = FALSE;
}

void PoLineEdit::setId(int pId)
{
  QString sql( "SELECT pohead_number, pohead_vend_id,"
               "       vend_name, vend_address1, vend_address2, vend_address3,"
               "       (vend_city || '  ' || vend_state || '  ' || vend_zip) AS citystatezip "
               "FROM pohead, vend "
               "WHERE ( (pohead_vend_id=vend_id)"
               " AND (pohead_id=:pohead_id)" );

  if (_type & (cPOUnposted | cPOOpen | cPOClosed))
  {
    bool qualifier = FALSE;

    sql += " AND (pohead_status IN (";

    if (_type & cPOUnposted)
    {
      qualifier = TRUE;
      sql += "'U'";
    }
    
    if (_type & cPOOpen)
    {
      if (qualifier)
        sql += ", ";
      else
        qualifier = TRUE;

      sql += "'O'";
    }

    if (_type & cPOClosed)
    {
      if (qualifier)
        sql += ", ";
      else
        qualifier = TRUE;

      sql += "'C'";
    }

    sql += "))";
  }

  sql += " );";
  XSqlQuery pohead;
  pohead.prepare(sql);
  pohead.bindValue(":pohead_id", pId);
  pohead.exec();
  if (pohead.first())
  {
    _id     = pId;
    _number = pohead.value("pohead_number").toInt();
    _vendid = pohead.value("pohead_vend_id").toInt();
    _valid  = TRUE;

    emit numberChanged(pohead.value("pohead_number").toString());
    emit vendNameChanged(pohead.value("vend_name").toString());
    emit vendAddress1Changed(pohead.value("vend_address1").toString());
    emit vendAddress2Changed(pohead.value("vend_address2").toString());
    emit vendAddress3Changed(pohead.value("vend_address3").toString());
    emit vendCityStateZipChanged(pohead.value("citystatezip").toString());
    setText(pohead.value("pohead_number").toString());
  }
  else
  {
    _id     = -1;
    _number = -1;
    _vendid = -1;
    _valid  = FALSE;

    emit numberChanged("");
    emit vendNameChanged("");
    emit vendAddress1Changed("");
    emit vendAddress2Changed("");
    emit vendAddress3Changed("");
    emit vendCityStateZipChanged("");
    setText("");
  }

  emit newId(_id);
  emit vendidChanged(_vendid);
  emit numberChanged(_number);
  emit valid(_valid);

  _parsed = TRUE;
}

void PoLineEdit::setNumber(int pNumber)
{
  XSqlQuery poheadid;
  poheadid.prepare( "SELECT pohead_id "
                    "FROM pohead "
                    "WHERE (pohead_number=:pohead_number);" );
  poheadid.bindValue(":pohead_number", QString("%1").arg(pNumber));
  poheadid.exec();
  if (poheadid.first())
    setId(poheadid.value("pohead_id").toInt());
  else
    setId(-1);
}

void PoLineEdit::clear()
{
  setId(-1);
}

void PoLineEdit::sParse()
{
  if (!_parsed)
  {
    _parsed = TRUE;

    bool numeric;
    int  purchaseOrderNumber = text().toInt(&numeric);

    if (numeric)
      setNumber(purchaseOrderNumber);
    else
      setId(-1);
  }
}


PoCluster::PoCluster(QWidget *parent, const char *name) :
  QWidget(parent, name)
{
//  Create the component Widgets

  QVBoxLayout *layoutMain = new QVBoxLayout(this);
  layoutMain->setMargin(0);
  layoutMain->setSpacing(0);
  QWidget *firstLine = new QWidget(this);
  QHBoxLayout *layoutFirstLine = new QHBoxLayout(firstLine);
  layoutFirstLine->setMargin(0);
  layoutFirstLine->setSpacing(5);

  QLabel *poNumberLit = new QLabel(tr("P/O #:"), firstLine);
  poNumberLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layoutFirstLine->addWidget(poNumberLit);

  _poNumber = new PoLineEdit(firstLine);
  layoutFirstLine->addWidget(_poNumber);

  _poList = new QPushButton(tr("..."), firstLine);
  _poList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _poList->setMaximumWidth(25);
#endif
  _poList->setFocusPolicy(Qt::NoFocus);
  layoutFirstLine->addWidget(_poList);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layoutFirstLine->addItem( spacer );
  firstLine->setLayout(layoutFirstLine);
  layoutMain->addWidget(firstLine);

  _vendName = new QLabel(this);
  _vendName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _vendName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  layoutMain->addWidget(_vendName);

  setLayout(layoutMain);

  connect(_poList, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_poNumber, SIGNAL(vendNameChanged(const QString &)), _vendName, SLOT(setText(const QString &)));
  connect(_poNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_poNumber, SIGNAL(vendidChanged(int)), this, SIGNAL(newVendid(int)));
  connect(_poNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_poNumber);
}

void PoCluster::sList()
{
  ParameterList params;
  params.append("pohead_id", _poNumber->id());
  params.append("poType", _poNumber->type());

  purchaseOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if ((id  = newdlg.exec()) != QDialog::Rejected)
    _poNumber->setId(id);
}

void PoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _poNumber->setEnabled(FALSE);
    _poList->hide();
  }
  else
  {
    _poNumber->setEnabled(TRUE);
    _poList->show();
  }
}

