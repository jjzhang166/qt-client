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

//  PlCluster.cpp
//  Created 04/15/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <xsqlquery.h>
#include <parameter.h>

#include "plannedOrderList.h"
#include "plCluster.h"

PlanOrdLineEdit::PlanOrdLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _qtyOrdered = 0.0;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void PlanOrdLineEdit::setId(int pId)
{
  XSqlQuery planord;
  planord.prepare( "SELECT formatPloNumber(planord_id) AS plnumber,"
                   "       warehous_code, item_number, uom_name,"
                   "       item_descrip1, item_descrip2,"
                   "       formatQty(planord_qty) AS qty,"
                   "       formatDate(planord_duedate) AS duedate "
                   "FROM planord, itemsite, item, warehous, uom "
                   "WHERE ( (planord_itemsite_id=itemsite_id)"
                   " AND (itemsite_item_id=item_id)"
                   " AND (item_inv_uom_id=uom_id)"
                   " AND (itemsite_warehous_id=warehous_id)"
                   " AND (planord_id=:planord_id) );" );
  planord.bindValue(":planord_id", pId);
  planord.exec();
  if (planord.first())
  {
    _id    = pId;
    _valid = TRUE;

    setText(planord.value("plnumber").toString());

    emit newId(_id);
    emit warehouseChanged(planord.value("warehous_code").toString());
    emit itemNumberChanged(planord.value("item_number").toString());
    emit uomChanged(planord.value("uom_name").toString());
    emit itemDescrip1Changed(planord.value("item_descrip1").toString());
    emit itemDescrip2Changed(planord.value("item_descrip2").toString());
    emit dueDateChanged(planord.value("duedate").toString());
    emit qtyChanged(planord.value("qty").toString());
    emit valid(TRUE);
  }
  else
  {
    _id    = -1;
    _valid = FALSE;

    setText("");

    emit newId(-1);
    emit warehouseChanged("");
    emit itemNumberChanged("");
    emit uomChanged("");
    emit itemDescrip1Changed("");
    emit itemDescrip2Changed("");
    emit dueDateChanged("");
    emit qtyChanged("");
    emit valid(FALSE);
  }
}

void PlanOrdLineEdit::sParse()
{
  if (text().contains('-'))
  {
    int number = text().left(text().find('-')).toInt();
    int subNumber = text().right(text().length() - text().find('-') - 1).toInt();

    XSqlQuery planord;
    planord.prepare( "SELECT planord_id "
                     "FROM planord "
                     "WHERE ( (planord_number=:planord_number)"
                     " AND (planord_subnumber=:planord_subnumber) );" );
    planord.bindValue(":planord_number", number);
    planord.bindValue(":planord_subnumber", subNumber);
    planord.exec();
    if (planord.first())
      setId(planord.value("planord_id").toInt());
    else
      setId(-1);
  }

  else if (text().length())
  {
    XSqlQuery planord;
    planord.prepare( "SELECT planord_id, planord_number "
                     "FROM planord "
                     "WHERE (planord_number=:planord_number);" );
    planord.bindValue(":planord_number", text().toInt());
    planord.exec();
    if (planord.first())
    {
      if (planord.size() == 1)
        setId(planord.value("planord_id").toInt());
      else
      {
        setId(-1);
        setText(planord.value("planord_number").toString() + "-");
        setFocus();
      }
    }
    else
      setId(-1);
  }
  else
    setId(-1);
}


PlanOrdCluster::PlanOrdCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();
}

void PlanOrdCluster::constructor()
{
//  Create the component Widgets
  Q3VBoxLayout *_mainLayout       = new Q3VBoxLayout(this, 0, 0);
  Q3HBoxLayout *_firstLineLayout  = new Q3HBoxLayout(0, 0, 7);
  Q3HBoxLayout *_orderLayout      = new Q3HBoxLayout(0, 0, 5);
  Q3HBoxLayout *_warehouseLayout  = new Q3HBoxLayout(0, 0, 5);
  Q3HBoxLayout *_itemLineLayout   = new Q3HBoxLayout(0, 0, 7);
  Q3HBoxLayout *_itemNumberLayout = new Q3HBoxLayout(0, 0, 5);
  Q3HBoxLayout *_uomLayout        = new Q3HBoxLayout(0, 0, 5);
  Q3HBoxLayout *_statusLayout     = new Q3HBoxLayout(0, 0, 7);

  QLabel *_numberLit = new QLabel(tr("Planned Order #:"), this, "_numberLit");

  _numberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _orderLayout->addWidget(_numberLit);

  _number = new PlanOrdLineEdit(this);
  _orderLayout->addWidget(_number);

  _list = new QPushButton(tr("..."), this, "_list");
  _list->setFocusPolicy(Qt::NoFocus);
  _orderLayout->addWidget(_list);
  _firstLineLayout->addLayout(_orderLayout);

  QLabel *_warehouseLit = new QLabel(tr("Whs.:"), this, "_warehouseLit");
  _warehouseLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouseLit);

  _warehouse = new QLabel(this, "_warehouse");
  _warehouse->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouse);
  _firstLineLayout->addLayout(_warehouseLayout);
  _mainLayout->addLayout(_firstLineLayout);

  QLabel *_itemNumberLit = new QLabel(tr("Item Number:"), this, "_itemNumberLit");
  _itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _itemNumberLayout->addWidget(_itemNumberLit);

  _itemNumber = new QLabel(this, "_itemNumber");
  _itemNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _itemNumberLayout->addWidget(_itemNumber);
  _itemLineLayout->addLayout(_itemNumberLayout);

  QLabel *_uomLit = new QLabel(tr("UOM:"), this, "_uomLit");
  _uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _uomLayout->addWidget(_uomLit);

  _uom = new QLabel(this, "_uom");
  _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _uomLayout->addWidget(_uom);
  _itemLineLayout->addLayout(_uomLayout);
  _mainLayout->addLayout(_itemLineLayout);

  _descrip1 = new QLabel(this, "_descrip1");
  _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip1);

  _descrip2 = new QLabel(this, "_descrip2");
  _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip2);

  QLabel *_statusLit = new QLabel(tr("Status:"), this, "_statusLit");
  _statusLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _statusLayout->addWidget(_statusLit);

  _status = new QLabel(this, "_status");
  _status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _statusLayout->addWidget(_status);
  _mainLayout->addLayout(_statusLayout);

//  Make some internal connections
  connect(_number, SIGNAL(itemNumberChanged(const QString &)), _itemNumber, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(itemDescrip1Changed(const QString &)), _descrip1, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(itemDescrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(warehouseChanged(const QString &)), _warehouse, SLOT(setText(const QString &)));

  connect(_number, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_number, SIGNAL(dueDateChanged(const QString &)), this, SIGNAL(dueDateChanged(const QString &)));
  connect(_number, SIGNAL(qtyChanged(const QString &)), this, SIGNAL(qtyChanged(const QString &)));
  connect(_number, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_list, SIGNAL(clicked()), SLOT(sList()));

  setFocusProxy(_number);
}

void PlanOrdCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _number->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _number->setEnabled(TRUE);
    _list->show();
  }
}

void PlanOrdCluster::setId(int pId)
{
  _number->setId(pId);
}

void PlanOrdCluster::sList()
{
  ParameterList params;
  params.append("planord_id", _number->_id);

  plannedOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  setId(id);

  if (id != -1)
  {
    _number->setFocus();
    focusNextPrevChild(TRUE);
  }
}

QString PlanOrdCluster::woNumber() const
{
  return _number->text();
}

