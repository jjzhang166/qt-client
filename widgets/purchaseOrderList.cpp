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


#include "purchaseOrderList.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLayout>
#include <QToolTip>

#include <parameter.h>
#include <metasql.h>

#include "xtreewidget.h"

#include "pocluster.h"

purchaseOrderList::purchaseOrderList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
  QDialog(parent, name, modal, fl)
{
  if (!name)
    setName("purchaseOrderList");

  _poheadid = -1;
  _type = (cPOUnposted | cPOOpen | cPOClosed);

  setCaption(tr("Purchase Orders"));

  QHBoxLayout *purchaseOrderListLayout = new QHBoxLayout(this);
  QVBoxLayout *tableLayout = new QVBoxLayout(purchaseOrderListLayout);
  QVBoxLayout *buttonLayout = new QVBoxLayout(purchaseOrderListLayout);

  QHBoxLayout *vendLayout = new QHBoxLayout(tableLayout);
  _vend = new VendorCluster(this, "_vend");
  vendLayout->addWidget(_vend);
  vendLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding,
				      QSizePolicy::Minimum));

  QLabel *_poheadLit = new QLabel(tr("&Purchase Orders:"), this, "_poheadLit");
  tableLayout->addWidget( _poheadLit );

  _pohead = new XTreeWidget(this);
  _pohead->setName("_pohead");
  _poheadLit->setBuddy(_pohead);
  tableLayout->addWidget(_pohead);

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  buttonLayout->addWidget( _close );

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setEnabled( FALSE );
  _select->setDefault( TRUE );
  buttonLayout->addWidget( _select );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  buttonLayout->addItem( spacer );

  resize( QSize(550, 350).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

    // signals and slots connections
  connect(_close,  SIGNAL(clicked()),		this,	SLOT(sClose()	     ));
  connect(_pohead, SIGNAL(itemSelected(int)), _select,	SLOT(animateClick()  ));
  connect(_pohead, SIGNAL(valid(bool)),       _select,	SLOT(setEnabled(bool)));
  connect(_select, SIGNAL(clicked()),		this,	SLOT(sSelect()	     ));
  connect(_vend,   SIGNAL(newId(int)),		this,	SLOT(sFillList()     ));

  _type = 0;

  _pohead->addColumn(tr("Number"), _orderColumn, Qt::AlignRight  );
  _pohead->addColumn(tr("Vendor"), -1,           Qt::AlignLeft   );
  _pohead->addColumn(tr("Agent"),  _itemColumn,  Qt::AlignCenter );
  _pohead->addColumn(tr("Order Date"),	_dateColumn,	Qt::AlignLeft  );
  _pohead->addColumn(tr("First Item"),	_itemColumn,	Qt::AlignLeft  );

  _pohead->setFocus();
}

void purchaseOrderList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
    _poheadid = param.toInt();
    
  param = pParams.value("poType", &valid);
  if (valid)
    _type = param.toInt();
 
  sFillList();
}

void purchaseOrderList::sSelect()
{
  done(_pohead->id());
}

void purchaseOrderList::sClose()
{
  done(_poheadid);
}

void purchaseOrderList::sFillList()
{
  QString sql( "SELECT pohead_id, pohead_number,"
               "       vend_name, pohead_agent_username,"
	       "       pohead_orderdate,"
	       "       item_number "
               "FROM vend, pohead LEFT OUTER JOIN"
	       "     poitem ON (poitem_pohead_id=pohead_id AND poitem_linenumber=1)"
	       "     LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
	       "     LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
	       "<? if exists(\"typeList\") ?>"
	       "  AND   (pohead_status IN ("
               "<? foreach(\"typeList\") ?>"
               "  <? if not isfirst(\"typeList\") ?>"
               "    ,"
               "  <? endif ?>"
               "  <? value(\"typeList\") ?>"
               "<? endforeach ?>"
               "                          ))"
	       "<? endif ?>"
	       "<? if exists(\"vend_id\") ?>"
	       "  AND   (pohead_vend_id=<? value(\"vend_id\") ?>)"
	       "<? endif ?>"
	       " ) "
	       "ORDER BY pohead_number DESC;" );
  ParameterList params;
  if (_vend->isValid())
    params.append("vend_id", _vend->id());

  QStringList typeList;
  if (_type & cPOUnposted)
    typeList += "U";

  if (_type & cPOOpen)
    typeList += "O";

  if (_type & cPOClosed)
    typeList += "C";

  if (! typeList.isEmpty())
    params.append("typeList", typeList);

  MetaSQLQuery mql(sql);
  XSqlQuery poheadq = mql.toQuery(params);
  _pohead->populate(poheadq, _poheadid);
}
