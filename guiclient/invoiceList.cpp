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

#include "invoiceList.h"

#include <qvariant.h>

/*
 *  Constructs a invoiceList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
invoiceList::invoiceList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
    connect(_invoice, SIGNAL(itemSelected(int)), this, SLOT(sSelect()));
    connect(_cust, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
invoiceList::~invoiceList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void invoiceList::languageChange()
{
    retranslateUi(this);
}


void invoiceList::init()
{
  QDate today(omfgThis->dbDate());
  _dates->setEndDate(today);
  _dates->setStartDate(today.addMonths(-1));

  _invoice->addColumn(tr("Invoice #"),    _orderColumn, Qt::AlignRight  );
  _invoice->addColumn(tr("Invoice Date"), _dateColumn,  Qt::AlignCenter );
  _invoice->addColumn(tr("S/O #"),        _orderColumn, Qt::AlignRight  );
  _invoice->addColumn(tr("Ship Date"),    _dateColumn,  Qt::AlignCenter );
  _invoice->addColumn(tr("Cust. P/O #"),  -1,           Qt::AlignLeft   );
}

enum SetResponse invoiceList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("invoiceNumber", &valid);
  if (valid)
    _invoiceNumber = param.toInt();
  else
    _invoiceNumber = -1;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());
  else if (_invoiceNumber != -1)
  {
    q.prepare( "SELECT invchead_cust_id "
               "FROM invchead "
               "WHERE (invchead_invcnumber=:invoiceNumber) ;" );
    q.bindValue(":invoiceNumber", _invoiceNumber);
    q.exec();
    if (q.first())
      _cust->setId(q.value("invchead_cust_id").toInt());
  }

  return NoError;
}

void invoiceList::sClose()
{
  done(_invoice->id());
}

void invoiceList::sSelect()
{
  done(_invoice->id());
}

void invoiceList::sFillList()
{
  q.prepare( "SELECT DISTINCT invchead_invcnumber, invchead_invcnumber, formatDate(invchead_invcdate),"
             "                invchead_ordernumber, formatDate(invchead_shipdate),"
             "                COALESCE(invchead_ponumber, '') "
             "FROM invchead "
             "WHERE ( (invchead_posted)"
             "  AND   (invchead_invcdate BETWEEN :startDate AND :endDate)"
             " AND (invchead_cust_id=:cust_id) ) "
             "ORDER BY invchead_invcnumber DESC;" );
  q.bindValue(":cust_id", _cust->id());
  _dates->bindValue(q);
  q.exec();
  _invoice->populate(q, _invoiceNumber);
}

