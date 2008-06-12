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

#include "rescheduleSoLineItems.h"

#include <QVariant>
#include <QMessageBox>
#include "inputManager.h"
#include "salesOrderList.h"
#include "rescheduleSoLineItems.h"

/*
 *  Constructs a rescheduleSoLineItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rescheduleSoLineItems::rescheduleSoLineItems(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSoList()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));
  connect(_so, SIGNAL(valid(bool)), _reschedule, SLOT(setEnabled(bool)));
  connect(_so, SIGNAL(requestList()), this, SLOT(sSoList()));

  _captive = FALSE;

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
rescheduleSoLineItems::~rescheduleSoLineItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rescheduleSoLineItems::languageChange()
{
    retranslateUi(this);
}

enum SetResponse rescheduleSoLineItems::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _so->setEnabled(FALSE);
    _date->setFocus();
  }

  return NoError;
}

void rescheduleSoLineItems::sReschedule()
{
  if (!_so->isValid())
  {
    QMessageBox::critical( this, tr("Select Sales Order"),
                          tr("You must select a Sales Order whose Line Item you wish to reschedule.") );
    _so->setFocus();
    return;
  }

  if (!_date->isValid())
  {
    QMessageBox::critical( this, tr("Enter New Scheduled Date"),
                           tr("You must enter a new Schedule Date." ) );
    _date->setFocus();
    return;
  }

  XSqlQuery reschedule;
  reschedule.prepare( "UPDATE coitem "
                      "SET coitem_scheddate=:newDate "
                      "WHERE ( (coitem_status NOT IN ('C','X'))"
                      " AND (coitem_cohead_id=:sohead_id) );" );
  reschedule.bindValue(":newDate", _date->date());
  reschedule.bindValue(":sohead_id", _so->id());
  reschedule.exec();

  if (_captive)
    accept();
  else
  {
    _so->setId(-1);
    _close->setText(tr("&Close"));
    _so->setFocus();
  }
}

void rescheduleSoLineItems::sSoList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("soType", cSoOpen);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  _so->setId(newdlg.exec());
}

void rescheduleSoLineItems::sPopulate(int pSoheadid)
{
  if (pSoheadid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT cohead_number,"
                   "       cohead_custponumber,"
                   "       cust_name, cust_phone "
                   "FROM cohead, cust "
                   "WHERE ( (cohead_cust_id=cust_id)"
                   " AND (cohead_id=:sohead_id) );" );
    query.bindValue(":sohead_id", pSoheadid); 
    query.exec();
    if (query.first())
    {
      _poNumber->setText(query.value("cohead_custponumber").toString());
      _custName->setText(query.value("cust_name").toString());
      _custPhone->setText(query.value("cust_phone").toString());
    }
  }
  else
  {
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}

