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

#include "dspItemSourcesByVendor.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByVendor.h"
#include "guiclient.h"

/*
 *  Constructs a dspItemSourcesByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemSourcesByVendor::dspItemSourcesByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsrc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_vendor, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _itemsrc->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft   );
  _itemsrc->addColumn(tr("Description"),        -1,          Qt::AlignLeft   );
  _itemsrc->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter );
  _itemsrc->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft   );
  _itemsrc->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignLeft   );
  _itemsrc->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemSourcesByVendor::~dspItemSourcesByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemSourcesByVendor::languageChange()
{
  retranslateUi(this);
}

void dspItemSourcesByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor"),
                        tr("Please enter a valid Vendor.") );
    _vendor->setFocus();
    return;
  }

  ParameterList params;
  params.append("vend_id", _vendor->id());

  orReport report("ItemSourcesByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSourcesByVendor::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem("Edit...",           this, SLOT(sEdit()),     0 );
  menuThis->insertItem("View Buy Card...",  this, SLOT(sBuyCard()),  0 );
}

void dspItemSourcesByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcesByVendor::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrc->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSourcesByVendor::sFillList()
{
  if (_vendor->isValid())
  {
    q.prepare( "SELECT itemsrc_id, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2),"
               "       uom_name,"
               "       itemsrc_vend_item_number, itemsrc_vend_uom,"
               "       formatQty(itemsrc_invvendoruomratio) "
               "FROM itemsrc, item, uom "
               "WHERE ( (itemsrc_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (itemsrc_vend_id=:vend_id) ) "
               "ORDER By item_number;" );
    q.bindValue(":vend_id", _vendor->id());
    q.exec();
    _itemsrc->populate(q);
  }
  else
    _itemsrc->clear();
}
