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

#include "distributeBreederProduction.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "distributeInventory.h"
#include "changeQtyToDistributeFromBreeder.h"

/*
 *  Constructs a distributeBreederProduction as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
distributeBreederProduction::distributeBreederProduction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_distrib, SIGNAL(itemSelected(int)), _changeQty, SLOT(animateClick()));
    connect(_distrib, SIGNAL(valid(bool)), _changeQty, SLOT(setEnabled(bool)));
    connect(_changeQty, SIGNAL(clicked()), this, SLOT(sChangeQty()));
    connect(_distribute, SIGNAL(clicked()), this, SLOT(sDistribute()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
distributeBreederProduction::~distributeBreederProduction()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void distributeBreederProduction::languageChange()
{
    retranslateUi(this);
}


void distributeBreederProduction::init()
{
  _distrib->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft  );
  _distrib->addColumn(tr("Description"), -1,          Qt::AlignLeft  );
  _distrib->addColumn(tr("Qty. Per."),   _qtyColumn,  Qt::AlignRight );
  _distrib->addColumn(tr("Qty."),        _qtyColumn,  Qt::AlignRight );
}

enum SetResponse distributeBreederProduction::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT wo_itemsite_id "
               "FROM wo "
               "WHERE (wo_id=:wo_id);" );
    q.bindValue(":wo_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setItemsiteid(q.value("wo_itemsite_id").toInt());
      _woid = param.toInt();
      _item->setReadOnly(TRUE);
  
      sFillList();
    }
//  ToDo
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _distrib->setFocus();
  }

  return NoError;
}

void distributeBreederProduction::sDistribute()
{
//  Issue the Breeder Item and Receive the Co-Products/By-Products
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT distributeBreederProduction(:wo_id) AS result;");
  q.bindValue(":wo_id", _woid);
  q.exec();
  if (q.first())
  {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Distribute Breeder Production"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at distributeBreederProduction::%1.")
                       .arg(__LINE__) );
  }

  accept();
}

void distributeBreederProduction::sChangeQty()
{
  ParameterList params;
  params.append("brddist_id", _distrib->id());

  changeQtyToDistributeFromBreeder newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void distributeBreederProduction::sFillList()
{
  q.prepare( "SELECT brddist_id, item_number, (item_descrip1 || ' ' || item_descrip2),"
             "       formatQtyPer(brddist_qty / brddist_wo_qty), formatQty(brddist_qty) "
             "FROM brddist, itemsite, item "
             "WHERE ( (NOT brddist_posted)"
             " AND (brddist_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (brddist_wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _woid);
  q.exec();
  _distrib->populate(q);
}

