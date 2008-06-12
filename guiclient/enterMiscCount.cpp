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

#include "enterMiscCount.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a enterMiscCount as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
enterMiscCount::enterMiscCount(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_item, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    init();
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
enterMiscCount::~enterMiscCount()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void enterMiscCount::languageChange()
{
    retranslateUi(this);
}


void enterMiscCount::init()
{
  _captive = FALSE;
}

enum SetResponse enterMiscCount::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _item->setItemsiteid(param.toInt());
    _item->setEnabled(FALSE);
    _warehouse->setEnabled(FALSE);
    _qty->setFocus();
  }

  return NoError;
}

void enterMiscCount::sPost()
{
  q.prepare( "SELECT ( (itemsite_controlmethod IN ('L', 'S')) OR (itemsite_loccntrl) ) AS detailed,"
             "       COALESCE(invcnt_id, -1) AS cnttagid "
             "FROM itemsite LEFT OUTER JOIN invcnt ON ( (invcnt_itemsite_id=itemsite_id) AND (NOT invcnt_posted) ) "
             "WHERE ( (itemsite_warehous_id=:warehous_id)"
             " AND (itemsite_item_id=:item_id) );" );
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    if (q.value("detailed").toBool())
    {
      QMessageBox::warning( this, tr("Cannot Enter Misc. Count"),
                            tr( "The selected Item Site is controlled via a combination of Lot/Serial and/or Location control.\n"
                                "Misc. Counts can only be entered for Item Sites that are not Lot/Serial or Location controlled." ) );
      return;
    }

    if (q.value("cnttagid") != -1)
    {
      QMessageBox::warning( this, tr("Count Tag Previously Created"),
                            tr( "An unposted Count Tag already exists for this Item and Warehouse.\n"
                                "You may not a Misc. Count until the current Count Tag has been posted." ) );
      return;
    }
  }
//  ToDo

  q.prepare( "SELECT postMiscCount(itemsite_id, :qty, :comments) AS cnttag_id "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":qty", _qty->toDouble());
  q.bindValue(":comments", _comments->text());
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();

  if (_captive)
    done(q.value("cnttag_id").toInt());
  else
  {
    _item->setId(-1);
    _qty->clear();
    _comments->clear();
    _item->setFocus();
  }
}

