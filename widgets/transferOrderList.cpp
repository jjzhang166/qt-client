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

#include "transferOrderList.h"

#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <metasql.h>
#include <parameter.h>

#include "xtreewidget.h"
#include "warehousegroup.h"
#include "socluster.h"

transferOrderList::transferOrderList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setName( "transferOrderList" );

  _toheadid = -1;
  _type = (cToOpen | cToClosed);

  setCaption(tr("Transfer Orders"));

  QVBoxLayout *mainLayout	= new QVBoxLayout(this, 5, 5, "mainLayout"); 
  QHBoxLayout *warehouseLayout	= new QHBoxLayout(0, 0, 0, "warehouseLayout"); 
  QHBoxLayout *topLayout	= new QHBoxLayout( 0, 0, 7, "topLayout"); 
  QVBoxLayout *buttonsLayout	= new QVBoxLayout(0, 0, 5, "buttonsLayout");
  QVBoxLayout *listLayout	= new QVBoxLayout( 0, 0, 0, "listLayout"); 

  _srcwhs = new WarehouseGroup(this, "_srcwhs");
  _srcwhs->setTitle(tr("From Warehouse:"));
  _dstwhs = new WarehouseGroup(this, "_dstwhs");
  _dstwhs->setTitle(tr("To Warehouse:"));
  _dstwhs->setAll();
  warehouseLayout->addWidget(_srcwhs);
  warehouseLayout->addWidget(_dstwhs);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
  warehouseLayout->addItem(spacer);
  topLayout->addLayout(warehouseLayout);

  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  topLayout->addItem(spacer_2);

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonsLayout->addWidget(_select);
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_transferOrdersLit = new QLabel(tr("Transfer Orders:"), this, "_transferOrdersLit");
  listLayout->addWidget(_transferOrdersLit);

  _to = new XTreeWidget(this);
  _to->setName("_to");
  listLayout->addWidget(_to);
  mainLayout->addLayout(listLayout);

  resize( QSize(490, 390).expandedTo(minimumSizeHint()) );

  connect(_close,	SIGNAL(clicked()), this,    SLOT( sClose() ) );
  connect(_dstwhs,	SIGNAL(updated()), this,    SLOT( sFillList() ) );
  connect(_select,	SIGNAL(clicked()), this,    SLOT( sSelect() ) );
  connect(_srcwhs,	SIGNAL(updated()), this,    SLOT( sFillList() ) );
  connect(_to,	      SIGNAL(valid(bool)), _select, SLOT( setEnabled(bool) ) );
  connect(_to,	SIGNAL(itemSelected(int)), _select, SLOT( animateClick() ) );

  _to->addColumn(tr("Order #"),   _orderColumn, Qt::AlignLeft   );
  _to->addColumn(tr("From Whs."),           -1, Qt::AlignLeft   );
  _to->addColumn(tr("To Whs."),   _orderColumn, Qt::AlignLeft   );
  _to->addColumn(tr("Ordered"),    _dateColumn, Qt::AlignCenter );
  _to->addColumn(tr("Scheduled"),  _dateColumn, Qt::AlignCenter );

  setTabOrder(_srcwhs,	_dstwhs);
  setTabOrder(_dstwhs,	_to);
  setTabOrder(_to,	_select);
  setTabOrder(_select,	_close);
  setTabOrder(_close,	_srcwhs);
  _srcwhs->setFocus();
}

void transferOrderList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("tohead_id", &valid);
  if (valid)
    _toheadid = param.toInt();
    
  param = pParams.value("toType", &valid);
  if (valid)
    _type = param.toInt();
 
  sFillList();
}

void transferOrderList::sClose()
{
  done(_toheadid);
}

void transferOrderList::sSelect()
{
  done(_to->id());
}

void transferOrderList::sFillList()
{
  QString sql;

  sql = "SELECT DISTINCT tohead_id, tohead_number, tohead_srcname, "
	"                tohead_destname,"
	"                formatDate(tohead_orderdate),"
	"                formatDate(MIN(toitem_schedshipdate)) "
	"<? if exists(\"atshipping\") ?>"
	"FROM tohead, toitem, tosmisc, toship "
	"WHERE ((tohead_id=toitem_tohead_id)"
	"  AND  (toitem_status<>'X')"
	"  AND  (toitem_id=toship_toitem_id)"
	"  AND  (toship_tosmisc_id=tosmisc_id)"
	"  AND  (NOT tosmisc_shipped)"
	"<? else ?>"	// not restricted to atshipping
	"FROM tohead, toitem "
	"WHERE ((tohead_id=toitem_tohead_id)"
	"  AND  (toitem_status<>'X')"
	"  <? if exists(\"toitem_statuslist\") ?>"
	"  AND  (toitem_status IN (<? literal(\"toitem_statuslist\") ?>))"
	"  <? endif ?>"
	"<? endif ?>"
	"<? if exists(\"srcwarehous_id\") ?>"
	" AND (tohead_src_warehous_id=<? value(\"srcwarehous_id\") ?>)"
	"<? endif ?>"
	"<? if exists(\"dstwarehous_id\") ?>"
	" AND (tohead_dest_warehous_id=<? value(\"dstwarehous_id\") ?>)"
	"<? endif ?>"
	") "
	"GROUP BY tohead_id, tohead_number, tohead_srcname,"
	"         tohead_destname, tohead_orderdate "
	"ORDER BY tohead_number;";

  ParameterList params;
  if (_type == cToAtShipping)
    params.append("atshipping");
  else
  {
    QString toitem_statuslist;

    bool statusCheck = FALSE;
    if (_type & cToOpen)
    {
      toitem_statuslist += "'O'";
      statusCheck = TRUE;
    }

    if (_type & cToClosed)
    {
      if (statusCheck)
        toitem_statuslist += ", ";
      toitem_statuslist += "'C'";
      statusCheck = TRUE;
    }

    if (statusCheck)
      params.append("toitem_statuslist", toitem_statuslist);
  }

  if (_srcwhs->isSelected())
    params.append("srcwarehous_id", _srcwhs->id());
  if (_dstwhs->isSelected())
    params.append("dstwarehous_id", _dstwhs->id());

  MetaSQLQuery mql(sql);
  XSqlQuery q = mql.toQuery(params);
  _to->populate(q, _toheadid);
}
