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

#include "itemSourcePrice.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

/*
 *  Constructs a itemSourcePrice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemSourcePrice::itemSourcePrice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _qtyBreak->setValidator(omfgThis->qtyVal());
  _itemsrcpid = -1;
  _itemsrcid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSourcePrice::~itemSourcePrice()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSourcePrice::languageChange()
{
    retranslateUi(this);
}

enum SetResponse itemSourcePrice::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsrc_id", &valid);
  if (valid)
    _itemsrcid = param.toInt();

  param = pParams.value("curr_id", &valid);
  if (valid)
    _price->setId(param.toInt());

  param = pParams.value("curr_effective", &valid);
  if (valid)
    _price->setEffective(param.toDate());

  param = pParams.value("itemsrcp_id", &valid);
  if (valid)
  {
    _itemsrcpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _qtyBreak->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _qtyBreak->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void itemSourcePrice::sSave()
{
  q.prepare("SELECT itemsrcp_id"
            "  FROM itemsrcp" 
            " WHERE ((itemsrcp_id != :itemsrcp_id)"
            "   AND  (itemsrcp_itemsrc_id=:itemsrcp_itemsrc_id)"
            "   AND  (itemsrcp_qtybreak=:qtybreak));");
  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  q.bindValue(":qtybreak", _qtyBreak->toDouble());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning(this, tr("Duplicate Qty. Break"),
      tr("A Qty. Break with the specified Qty. already exists for this Item Source.") );
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('itemsrcp_itemsrcp_id_seq') AS itemsrcp_id;");
    if (q.first())
      _itemsrcpid = q.value("itemsrcp_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO itemsrcp "
               "(itemsrcp_id, itemsrcp_itemsrc_id, itemsrcp_qtybreak, itemsrcp_price, itemsrcp_updated, itemsrcp_curr_id) "
               "VALUES "
               "(:itemsrcp_id, :itemsrcp_itemsrc_id, :itemsrcp_qtybreak, :itemsrcp_price, CURRENT_DATE, :itemsrcp_curr_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE itemsrcp "
               "SET itemsrcp_qtybreak=:itemsrcp_qtybreak, "
	       "    itemsrcp_price=:itemsrcp_price, "
	       "    itemsrcp_updated=CURRENT_DATE, "
	       "    itemsrcp_curr_id=:itemsrcp_curr_id "
               "WHERE (itemsrcp_id=:itemsrcp_id);" );

  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.bindValue(":itemsrcp_itemsrc_id", _itemsrcid);
  q.bindValue(":itemsrcp_qtybreak", _qtyBreak->toDouble());
  q.bindValue(":itemsrcp_price", _price->localValue());
  q.bindValue(":itemsrcp_curr_id", _price->id());
  q.exec();

  done(_itemsrcpid);
}

void itemSourcePrice::populate()
{
  q.prepare( "SELECT formatQty(itemsrcp_qtybreak) AS qtybreak,"
             "       itemsrcp_price, itemsrcp_curr_id,"
             "       itemsrcp_itemsrc_id "
             "FROM itemsrcp "
             "WHERE (itemsrcp_id=:itemsrcp_id);" );
  q.bindValue(":itemsrcp_id", _itemsrcpid);
  q.exec();
  if (q.first())
  {
    _itemsrcid = q.value("itemsrcp_itemsrc_id").toInt();
    _qtyBreak->setText(q.value("qtybreak").toString());
    _price->setLocalValue(q.value("itemsrcp_price").toDouble());
    _price->setId(q.value("itemsrcp_curr_id").toInt());
  }
}
