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

//  woCluster.cpp
//  Created 03/08/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QLabel>
#include <QPushButton>
#include <QLayout>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>
#include <QMessageBox>

#include "xcombobox.h"
#include "xlineedit.h"

#include "woList.h"
#include "wocluster.h"

#include "../common/format.h"

WoLineEdit::WoLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _woType = 0;
  _warehouseid = -1;
  _parsed = TRUE;
  _useQuery = FALSE;

  _qtyOrdered = 0.0;
  _qtyReceived = 0.0;

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

WoLineEdit::WoLineEdit(int pWoType, QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _woType = pWoType;
  _warehouseid = -1;
  _parsed = TRUE;

  _qtyOrdered = 0.0;
  _qtyReceived = 0.0;

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void WoLineEdit::setId(int pId)
{
  bool found = FALSE;
  if (pId != -1)
  {
    XSqlQuery wo;
        if (_useQuery)
    {
      wo.prepare(_sql);
      wo.exec();
      found = (wo.findFirst("wo_id", pId) != -1);
    }
        else
        {
      wo.prepare( "SELECT formatWONumber(wo_id) AS wonumber,"
                  "       warehous_code, item_id, item_number, uom_name,"
                  "       item_descrip1, item_descrip2,"
                  "       wo_qtyord, wo_qtyrcv, wo_status,"
                  "       wo_duedate,"
                  "       wo_startdate,"
                  "       formatQtyPer(wo_qtyord) AS ordered,"
                  "       formatQtyPer(wo_qtyrcv) AS received, "
                  "       formatQtyPer(noNeg(wo_qtyord - wo_qtyrcv)) AS balance "
                  "FROM wo, itemsite, item, warehous, uom "
                  "WHERE ((wo_itemsite_id=itemsite_id)"
                  " AND (itemsite_item_id=item_id)"
                  " AND (item_inv_uom_id=uom_id)"
                  " AND (itemsite_warehous_id=warehous_id)"
                  " AND (wo_id=:wo_id));" );
      wo.bindValue(":wo_id", pId);
      wo.exec();
      found = (wo.first());
        }
        if (found)
    {
      _id    = pId;
      _valid = TRUE;

      setText(wo.value("wonumber").toString());

      emit newId(_id);
      emit newItemid(wo.value("item_id").toInt());
      emit warehouseChanged(wo.value("warehous_code").toString());
      emit itemNumberChanged(wo.value("item_number").toString());
      emit uomChanged(wo.value("uom_name").toString());
      emit itemDescrip1Changed(wo.value("item_descrip1").toString());
      emit itemDescrip2Changed(wo.value("item_descrip2").toString());
      emit startDateChanged(wo.value("wo_startdate").toDate());
      emit dueDateChanged(wo.value("wo_duedate").toDate());
      emit qtyOrderedChanged(wo.value("ordered").toString());
      emit qtyReceivedChanged(wo.value("received").toString());
      emit qtyBalanceChanged(wo.value("balance").toString());
      emit statusChanged(wo.value("wo_status").toString());
      emit valid(TRUE);

      _qtyOrdered  = wo.value("wo_qtyord").toDouble();
      _qtyReceived = wo.value("wo_qtyrcv").toDouble();

      _parsed = TRUE;

      return;
    }
  }

  _id    = -1;
  _valid = FALSE;

  setText("");

  emit newId(-1);
  emit newItemid(-1);
  emit warehouseChanged("");
  emit itemNumberChanged("");
  emit uomChanged("");
  emit itemDescrip1Changed("");
  emit itemDescrip2Changed("");
  emit startDateChanged(QDate());
  emit dueDateChanged(QDate());
  emit qtyOrderedChanged("");
  emit qtyReceivedChanged("");
  emit qtyBalanceChanged("");
  emit statusChanged("");
  emit valid(FALSE);
    
  _qtyOrdered  = 0;
  _qtyReceived = 0;

  _parsed = TRUE;
}

void WoLineEdit::sParse()
{
  if (!_parsed)
  {
    if (text().stripWhiteSpace().length() == 0)
      setId(-1);

    else if (_useQuery)
    {
      XSqlQuery wo;
      wo.prepare(_sql);
      wo.exec();
      if (wo.findFirst("wonumber", text().stripWhiteSpace().upper()) != -1)
      {
        setId(wo.value("wo_id").toInt());
        return;
      }
    }

    else if (text().contains('-'))
    {
      int soNumber = text().left(text().find('-')).toInt();
      int subNumber = text().right(text().length() - text().find('-') - 1).toInt();
 //     bool statusCheck = FALSE;
      QString sql = QString( "SELECT wo_id "
                             "FROM wo "
                             "WHERE ((wo_number=%1)"
                             " AND (wo_subnumber=%2)" )
                    .arg(soNumber)
                    .arg(subNumber);

  //  Add in the Status checks
      QStringList statuses;
      if (_woType & cWoOpen)
        statuses << "(wo_status='O')";

      if (_woType & cWoExploded)
        statuses << "(wo_status='E')";

      if (_woType & cWoReleased)
        statuses << "(wo_status='R')";

      if (_woType & cWoIssued)
        statuses << "(wo_status='I')";

      if (_woType & cWoClosed)
        statuses << "(wo_status='C')";

      if(!statuses.isEmpty())
        sql += " AND (" + statuses.join(" OR ") + ")";

      sql += ")";

      XSqlQuery wo(sql);

      if (wo.first())
        setId(wo.value("wo_id").toInt());
      else
        setId(-1);
    }

    else
    {
      bool statusCheck = FALSE;
      QString sql = QString( "SELECT wo_id, wo_number "
                             "FROM wo "
                             "WHERE ((wo_number=%1)")
                    .arg(text().toInt());

//  Add in the Status checks
      if (_woType)
      {
        sql += " AND (";

        if (_woType & cWoOpen)
        {
          sql += "(wo_status='O')";
          statusCheck = TRUE;
        }

        if (_woType & cWoExploded)
        {
          if (statusCheck)
            sql += " OR ";
          sql += "(wo_status='E')";
          statusCheck = TRUE;
        }

        if (_woType & cWoReleased)
        {
          if (statusCheck)
            sql += " OR ";
          sql += "(wo_status='R')";
          statusCheck = TRUE;
        }

        if (_woType & cWoIssued)
        {
          if (statusCheck)
            sql += " OR ";
          sql += "(wo_status='I')";
          statusCheck = TRUE;
        }

        if (_woType & cWoClosed)
        {
          if (statusCheck)
            sql += " OR ";
          sql += "(wo_status='C')";
        }

        sql += ")";
      }
      sql += ");";

      XSqlQuery wo(sql);

      if (wo.first())
      {
        if (wo.size() == 1)
          setId(wo.value("wo_id").toInt());
        else
        {
          setId(-1);
          setText(wo.value("wo_number").toString() + "-");
          focusNextPrevChild(FALSE);
          home(FALSE);
          end(FALSE);
        }
      }
      else
        setId(-1);
    }
  }
}


WoCluster::WoCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();
}

WoCluster::WoCluster(int pWoType, QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();

  _woNumber->setType(pWoType);
}

void WoCluster::constructor()
{
//  Create the component Widgets
  Q3VBoxLayout *_mainLayout      = new Q3VBoxLayout(this, 0, 2, "_layoutMain"); 
  Q3HBoxLayout *_woLayout        = new Q3HBoxLayout(0, 0, 5, "_layoutLit"); 
  Q3HBoxLayout *_warehouseLayout = new Q3HBoxLayout(0, 0, 5, "_layoutLit"); 
  Q3HBoxLayout *_line1Layout     = new Q3HBoxLayout(0, 0, 7, "_layoutLit"); 
  Q3HBoxLayout *_itemLayout      = new Q3HBoxLayout(0, 0, 5, "_layoutLit"); 
  Q3HBoxLayout *_uomLayout       = new Q3HBoxLayout(0, 0, 5, "_layoutLit"); 
  Q3HBoxLayout *_line2Layout     = new Q3HBoxLayout(0, 0, 7, "_layoutLit"); 
  Q3HBoxLayout *_statusLayout    = new Q3HBoxLayout(0, 0, 5, "_layoutLit"); 

  QLabel *woNumberLit = new QLabel(tr("Work Order #:"), this, "woNumberLit");
  woNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  woNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _woLayout->addWidget(woNumberLit);

  _woNumber = new WoLineEdit(this);
  _woNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _woLayout->addWidget(_woNumber);

  _woList = new QPushButton(tr("..."), this, "_woList");
  _woList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _woList->setMaximumWidth(25);
#endif
  _woList->setFocusPolicy(Qt::NoFocus);
  _woLayout->addWidget(_woList);
  _line1Layout->addLayout(_woLayout);

  QLabel *warehouseLit = new QLabel(tr("Whs.:"), this, "warehouseLit");
  _woNumber->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  warehouseLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _warehouseLayout->addWidget(warehouseLit);

  _warehouse = new QLabel(this, "_warehouse");
  _warehouse->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _warehouse->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouse);
  _line1Layout->addLayout(_warehouseLayout);
  _mainLayout->addLayout(_line1Layout);

  QLabel *itemNumberLit = new QLabel(tr("Item Number:"), this, "itemNumberLit");
  itemNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _itemLayout->addWidget(itemNumberLit);

  _itemNumber = new QLabel(this, "_itemNumber");
  _itemNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _itemNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _itemLayout->addWidget(_itemNumber);
  _line2Layout->addLayout(_itemLayout);

  QLabel *uomLit = new QLabel(tr("UOM:"), this, "uomLit");
  uomLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _uomLayout->addWidget(uomLit);

  _uom = new QLabel(this, "_uom");
  _uom->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _uomLayout->addWidget(_uom);
  _line2Layout->addLayout(_uomLayout);
  _mainLayout->addLayout(_line2Layout);

  _descrip1 = new QLabel(this, "_descrip1");
  _descrip1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip1);

  _descrip2 = new QLabel(this, "_descrip2");
  _descrip2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip2);

  QLabel *statusLit = new QLabel(tr("Status:"), this, "statusLit");
  statusLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  statusLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _statusLayout->addWidget(statusLit);

  _status = new QLabel(this, "_status");
  _status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _statusLayout->addWidget(_status);
  _mainLayout->addLayout(_statusLayout);

//  Make some internal connections
  connect(_woNumber, SIGNAL(itemNumberChanged(const QString &)), _itemNumber, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(itemDescrip1Changed(const QString &)), _descrip1, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(itemDescrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(warehouseChanged(const QString &)), _warehouse, SLOT(setText(const QString &)));
  connect(_woNumber, SIGNAL(statusChanged(const QString &)), _status, SLOT(setText(const QString &)));

  connect(_woNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_woNumber, SIGNAL(newItemid(int)), this, SIGNAL(newItemid(int)));
  connect(_woNumber, SIGNAL(startDateChanged(const QDate &)), this, SIGNAL(startDateChanged(const QDate &)));
  connect(_woNumber, SIGNAL(dueDateChanged(const QDate &)), this, SIGNAL(dueDateChanged(const QDate &)));
  connect(_woNumber, SIGNAL(qtyOrderedChanged(const QString &)), this, SIGNAL(qtyOrderedChanged(const QString &)));
  connect(_woNumber, SIGNAL(qtyReceivedChanged(const QString &)), this, SIGNAL(qtyReceivedChanged(const QString &)));
  connect(_woNumber, SIGNAL(qtyBalanceChanged(const QString &)), this, SIGNAL(qtyBalanceChanged(const QString &)));
  connect(_woNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_woList, SIGNAL(clicked()), SLOT(sWoList()));
  connect(_woNumber, SIGNAL(requestList()), SLOT(sWoList()));

  setFocusProxy(_woNumber);
}

void WoCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _woNumber->setEnabled(FALSE);
    _woList->hide();
  }
  else
  {
    _woNumber->setEnabled(TRUE);
    _woList->show();
  }
}

void WoCluster::setId(int pId)
{
  _woNumber->setId(pId);
}

void WoCluster::sWoList()
{
  ParameterList params;
  params.append("wo_id", _woNumber->_id);

  if (_woNumber->_useQuery)
  {
    params.append("sql", _woNumber->_sql);
  }
  else 
    params.append("woType", _woNumber->_woType);

  if (_woNumber->_warehouseid != -1)
    params.append("warehous_id", _woNumber->_warehouseid);

  woList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  
  int id = newdlg.exec();
  setId(id);

  if (id != -1)
  {
    _woNumber->setFocus();
    focusNextPrevChild(TRUE);
  }
}


QString WoCluster::woNumber() const
{
  return _woNumber->text();
}


WomatlCluster::WomatlCluster(QWidget *parent, const char *name) : QWidget(parent, name)
{
  constructor();
}

WomatlCluster::WomatlCluster(WoCluster *wocParent, QWidget *parent, const char *name) : QWidget(parent, name)
{
  constructor();

  setGeometry(0, 0, parent->width(), parent->height());

  connect(wocParent, SIGNAL(newId(int)), SLOT(setWoid(int)));
  connect(this, SIGNAL(newId(int)), wocParent, SLOT(sPopulateInfo(int)));
}

void WomatlCluster::constructor()
{
  _valid  = FALSE;
  _id     = -1;
  _woid   = -1;
  _type   = (Push | Pull | Mixed);

  _source = WoMaterial;
  _sourceId = -1;

  _required = 0.0;
  _issued = 0.0;

//  Create the component Widgets
  QLabel *itemNumberLit = new QLabel(tr("Component Item Number:"), this, "itemNumberLit");
  itemNumberLit->setGeometry(0, 0, 140, 25);
  itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _itemNumber = new XComboBox(this, "_itemNumber");
  _itemNumber->setGeometry(145, 0, 120, 25);
  _itemNumber->setAllowNull(TRUE);

  QLabel *uomLit = new QLabel(tr("UOM:"), this, "uomLit");
  uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _uom = new QLabel(this, "_uom");
  _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  _descrip1 = new QLabel(this, "_descrip1");
  _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  _descrip2 = new QLabel(this, "_descrip2");
  _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  QLabel *qtyPerLit = new QLabel(tr("Qty. Per:"), this, "qtyPerLit");
  qtyPerLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _qtyPer = new QLabel(this, "_qtyPer");
  _qtyPer->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  QLabel *scrapLit = new QLabel(tr("Scrap %:"), this, "scrapLit");
  scrapLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _scrap = new QLabel(this, "_scrap");
  _scrap->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  QLabel *qtyRequiredLit = new QLabel(tr("Qty. Required:"), this, "qtyRequiredLit");
  qtyRequiredLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _qtyRequired = new QLabel(this, "_qtyRequired");
  _qtyRequired->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  QLabel *qtyIssuedLit = new QLabel(tr("Qty. Issued:"), this, "qtyIssuedLit");
  qtyIssuedLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  _qtyIssued = new QLabel(this, "_qtyIssued");
  _qtyIssued->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  // lay out the component widgets
  Q3VBoxLayout *clusterLyt    = new Q3VBoxLayout(this, 5, -1, "clusterLyt");
  Q3HBoxLayout *itemMainLyt   = new Q3HBoxLayout(clusterLyt, -1, "itemMainLyt");
  Q3HBoxLayout *itemDescLyt   = new Q3HBoxLayout(clusterLyt, -1, "itemDescLyt");
  Q3HBoxLayout *qtyHBoxLyt    = new Q3HBoxLayout(clusterLyt, -1, "qtyHBoxLyt");
  Q3VBoxLayout *qtyLitVBoxLyt = new Q3VBoxLayout(qtyHBoxLyt, -1, "qtyLitVBoxLyt");
  Q3VBoxLayout *qtyVBoxLyt    = new Q3VBoxLayout(qtyHBoxLyt, -1, "qtyVBoxLyt");

  itemMainLyt->addWidget(itemNumberLit);
  itemMainLyt->addWidget(_itemNumber);
  itemMainLyt->addWidget(uomLit);
  itemMainLyt->addWidget(_uom);
  itemDescLyt->addWidget(_descrip1);
  itemDescLyt->addWidget(_descrip2);

  qtyLitVBoxLyt->addWidget(qtyPerLit);
  qtyLitVBoxLyt->addWidget(scrapLit);
  qtyLitVBoxLyt->addWidget(qtyRequiredLit);
  qtyLitVBoxLyt->addWidget(qtyIssuedLit);
  qtyVBoxLyt->addWidget(_qtyPer);
  qtyVBoxLyt->addWidget(_scrap);
  qtyVBoxLyt->addWidget(_qtyRequired);
  qtyVBoxLyt->addWidget(_qtyIssued);

  connect(_itemNumber, SIGNAL(newID(int)), SLOT(sPopulateInfo(int)));

  setFocusProxy(_itemNumber);
}

void WomatlCluster::setReadOnly(bool)
{
}

void WomatlCluster::setWooperid(int pWooperid)
{
  _source = Wooper;
  _sourceId = pWooperid;

  bool qual = FALSE;
  QString sql( "SELECT womatl_id AS womatlid, item_number,"
               "       wo_id, uom_name, item_descrip1, item_descrip2,"
               "       womatl_qtyreq AS _qtyreq, womatl_qtyiss AS _qtyiss,"
               "       formatQtyPer(womatl_qtyper) AS qtyper,"
               "       formatScrap(womatl_scrap) AS scrap,"
               "       formatQtyPer(womatl_qtyreq) AS qtyreq,"
               "       formatQtyPer(womatl_qtyiss) AS qtyiss,"
               "       formatQtyPer(womatl_qtywipscrap) AS qtywipscrap "
               "FROM womatl, wo, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (womatl_wooper_id=:wooper_id)"
               " AND (womatl_issuemethod IN (" );

  if (_type & Push)
  {
    sql += "'S'";
    qual = TRUE;
  }

  if (_type & Pull)
  {
    if (qual)
      sql += ",";
    else
      qual = TRUE;

    sql += "'L'";
  }

  if (_type & Mixed)
  {
    if (qual)
      sql += ",";

    sql += "'M'";
  }

  sql += ")) );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":wooper_id", pWooperid);
  query.exec();
  if (query.first())
  {
    _womatl.prepare(sql);
    _womatl.bindValue(":wooper_id", pWooperid);
    _womatl.exec();
    _itemNumber->populate(query);
  }
  else
  {
    _id = -1;
    _woid = -1;
    _valid = FALSE;
    _required = 0.0;
    _issued  = 0.0;
    
    emit newId(-1);
    emit newQtyRequired(formatQtyPer(0.0));
    emit newQtyIssued(formatQtyPer(0.0));
    emit newQtyScrappedFromWIP(formatQtyPer(0.0));

    _itemNumber->clear();
  }
}

void WomatlCluster::setWoid(int pWoid)
{
  _source = WorkOrder;
  _sourceId = pWoid;

  bool qual = FALSE;
  QString sql( "SELECT womatl_id AS womatlid, item_number,"
               "       wo_id, uom_name, item_descrip1, item_descrip2,"
               "       womatl_qtyreq AS _qtyreq, womatl_qtyiss AS _qtyiss,"
               "       formatQtyPer(womatl_qtyper) AS qtyper,"
               "       formatScrap(womatl_scrap) AS scrap,"
               "       formatQtyPer(womatl_qtyreq) AS qtyreq,"
               "       formatQtyPer(womatl_qtyiss) AS qtyiss,"
               "       formatQtyPer(womatl_qtywipscrap) AS qtywipscrap "
               "FROM womatl, wo, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (wo_id=:wo_id)"
               " AND (womatl_issuemethod IN (" );

  if (_type & Push)
  {
    sql += "'S'";
    qual = TRUE;
  }

  if (_type & Pull)
  {
    if (qual)
      sql += ",";
    else
      qual = TRUE;

    sql += "'L'";
  }

  if (_type & Mixed)
  {
    if (qual)
      sql += ",";

    sql += "'M'";
  }

  sql += ")) );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":wo_id", pWoid);
  query.exec();
  if (query.first())
  {
    _womatl.prepare(sql);
    _womatl.bindValue(":wo_id", pWoid);
    _womatl.exec();
    _itemNumber->populate(query);
  }
  else
  {
    _id = -1;
    _woid = -1;
    _valid = FALSE;
    _required = 0.0;
    _issued  = 0.0;
    
    emit newId(-1);
    emit newQtyRequired(formatQtyPer(0.0));
    emit newQtyIssued(formatQtyPer(0.0));
    emit newQtyScrappedFromWIP(formatQtyPer(0.0));

    _itemNumber->clear();
  }
}

void WomatlCluster::setId(int pWomatlid)
{
  _source = WoMaterial;
  _sourceId = pWomatlid;

  if (pWomatlid == -1)
    sPopulateInfo(-1);

  else
  {
    bool qual = FALSE;
    QString sql( "SELECT list.womatl_id AS womatlid, item_number,"
                 "       wo_id, uom_name, item_descrip1, item_descrip2,"
                 "       list.womatl_qtyreq AS _qtyreq, list.womatl_qtyiss AS _qtyiss,"
                 "       formatQtyPer(list.womatl_qtyper) AS qtyper,"
                 "       formatScrap(list.womatl_scrap) AS scrap,"
                 "       formatQtyPer(list.womatl_qtyreq) AS qtyreq,"
                 "       formatQtyPer(list.womatl_qtyiss) AS qtyiss,"
                 "       formatQtyPer(list.womatl_qtywipscrap) AS qtywipscrap "
                 "FROM womatl AS list, womatl AS target, wo, itemsite, item, uom "
                 "WHERE ( (list.womatl_wo_id=wo_id)"
                 " AND (target.womatl_wo_id=wo_id)"
                 " AND (list.womatl_itemsite_id=itemsite_id)"
                 " AND (itemsite_item_id=item_id)"
                 " AND (list.womatl_uom_id=uom_id)"
                 " AND (target.womatl_id=:womatl_id)"
                 " AND (list.womatl_issuemethod IN (" );

    if (_type & Push)
    {
      sql += "'S'";
      qual = TRUE;
    }

    if (_type & Pull)
    {
      if (qual)
        sql += ",";
      else
        qual = TRUE;

      sql += "'L'";
    }

    if (_type & Mixed)
    {
      if (qual)
        sql += ",";

      sql += "'M'";
    }

    sql += ")) );";

    XSqlQuery query;
    query.prepare(sql);
    query.bindValue(":womatl_id", pWomatlid);
    query.exec();
    if (query.first())
    {
      _womatl.prepare(sql);
      _womatl.bindValue(":womatl_id", pWomatlid);
      _womatl.exec();

      emit newId(pWomatlid);

      _valid = TRUE;
      _id = pWomatlid;

      _itemNumber->populate(query);
      _itemNumber->setId(pWomatlid);
    }
    else
    {
      _valid = FALSE;
      _woid = -1;
      _id = -1;

      emit newId(-1);
      emit valid(FALSE);

      _itemNumber->clear();
    }
  }
}

void WomatlCluster::setType(int pType)
{
  if (pType)
    _type = pType;
}

void WomatlCluster::sRefresh()
{
  if (_source == WorkOrder)
    setWoid(_sourceId);
  else if (_source == Wooper)
    setWooperid(_sourceId);
  else if (_source == WoMaterial)
    setId(_sourceId);
}

void WomatlCluster::sPopulateInfo(int pWomatlid)
{
  if (pWomatlid == -1)
  {
    _itemNumber->setCurrentItem(0);
    _uom->setText("");
    _descrip1->setText("");
    _descrip2->setText("");
    _qtyPer->setText("");
    _scrap->setText("");
    _qtyRequired->setText("");
    _qtyIssued->setText("");

    _id = -1;
    _valid = FALSE;
    _required = 0;
    _issued = 0;

    emit newId(-1);
    emit newQtyScrappedFromWIP("");
    emit valid(FALSE);
  }
  else if (_womatl.findFirst("womatlid", pWomatlid) != -1)
  {
    _uom->setText(_womatl.value("uom_name").toString());
    _descrip1->setText(_womatl.value("item_descrip1").toString());
    _descrip2->setText(_womatl.value("item_descrip2").toString());
    _qtyPer->setText(_womatl.value("qtyper").toString());
    _scrap->setText(_womatl.value("scrap").toString());
    _qtyRequired->setText(_womatl.value("qtyreq").toString());
    _qtyIssued->setText(_womatl.value("qtyiss").toString());

    _id = pWomatlid;
    _valid = TRUE;
    _required = _womatl.value("_qtyreq").toDouble();
    _issued = _womatl.value("_qtyiss").toDouble();

    emit newId(_id);
    emit newQtyScrappedFromWIP(_womatl.value("qtywipscrap").toString());
    emit valid(TRUE);
  }
}

