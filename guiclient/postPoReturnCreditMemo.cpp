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

#include "postPoReturnCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "rwInterface.h"
/*
 *  Constructs a postPoReturnCreditMemo as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postPoReturnCreditMemo::postPoReturnCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _porejectid = -1;

  _post->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postPoReturnCreditMemo::~postPoReturnCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postPoReturnCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postPoReturnCreditMemo::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("poreject_id", &valid);
  if(valid)
  {
    _porejectid = param.toInt();
    q.prepare("SELECT pohead_curr_id,"
              "       COALESCE(item_number, poitem_vend_item_number) AS itemnumber,"
              "       formatQty(poreject_qty) AS qty,"
              "       (poitem_unitprice * poreject_qty) AS itemAmount"
              "  FROM pohead, poreject, poitem"
              "       LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
              "       LEFT OUTER JOIN item ON (itemsite_item_id=item_id)"
              " WHERE((poreject_poitem_id=poitem_id)"
              "   AND (pohead_id=poitem_pohead_id)"
              "   AND (poreject_id=:poreject_id));");
    q.bindValue(":poreject_id", _porejectid);
    q.exec();
    if(q.first())
    {
      _item->setText(q.value("itemNumber").toString());
      _qty->setText(q.value("qty").toString());
      _amount->set(q.value("itemAmount").toDouble(), q.value("pohead_curr_id").toInt(), QDate::currentDate());
    }
  }

  return NoError;
}

void postPoReturnCreditMemo::sPost()
{
  q.prepare("SELECT postPoReturnCreditMemo(:poreject_id, :amount) AS result;");
  q.bindValue(":poreject_id", _porejectid);
  q.bindValue(":amount", _amount->localValue());
  if(!q.exec())
  {
    systemError( this, tr("A System Error occurred at postPoReturnCreditMemo::%1.")
                       .arg(__LINE__) );
    return;
  }

  accept();
}
