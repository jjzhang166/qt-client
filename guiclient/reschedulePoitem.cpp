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

#include "reschedulePoitem.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a reschedulePoitem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reschedulePoitem::reschedulePoitem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));
  connect(_po, SIGNAL(valid(bool)), _poitem, SLOT(setEnabled(bool)));
  connect(_po, SIGNAL(newId(int)), this, SLOT(sPopulatePoitem(int)));
  connect(_poitem, SIGNAL(newID(int)), this, SLOT(sPopulate(int)));
  connect(_poitem, SIGNAL(valid(bool)), _reschedule, SLOT(setEnabled(bool)));

  _po->setType(cPOOpen);
}

/*
 *  Destroys the object and frees any allocated resources
 */
reschedulePoitem::~reschedulePoitem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reschedulePoitem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse reschedulePoitem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _po->setReadOnly(TRUE);
    _new->setFocus();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT pohead_id "
               "FROM pohead, poitem "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "  AND (pohead_status='O')"
               "  AND (poitem_id=:poitem_id));" );
    q.bindValue(":poitem_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _po->setId(q.value("pohead_id").toInt());
      _po->setReadOnly(TRUE);
      _poitem->setId(param.toInt());
    }
    else
    {
      QMessageBox::critical(this, tr("P/O Not Open"),
        tr("The P/O line item you are trying to reschedule does\n"
           "not belong to an Open P/O.") );
      return UndefinedError;
    }

    _new->setFocus();
  }

  return NoError;
}

void reschedulePoitem::sPopulatePoitem(int pPoheadid)
{
  q.prepare( "SELECT poitem_id,"
             "       ( poitem_linenumber || '-' ||"
             "         COALESCE(item_number, poitem_vend_item_number) || ' (' ||"
             "         COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) || ')' ) "
             "FROM poitem LEFT OUTER JOIN "
             "     ( itemsite JOIN item "
             "       ON (itemsite_item_id=item_id)"
             "     ) ON (poitem_itemsite_id=itemsite_id) "
             "WHERE ( (poitem_status <> 'C')"
             " AND (poitem_pohead_id=:pohead_id) ) "
             "ORDER BY poitem_linenumber;" );
  q.bindValue(":pohead_id", pPoheadid);
  q.exec();
  _poitem->populate(q);
}

void reschedulePoitem::sPopulate(int pPoitemid)
{
  if (pPoitemid == -1)
  {
    _original->clear();
    _current->clear();
    _new->clear();
  }
  else
  {
    q.prepare( "SELECT poitem_duedate "
               "FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    q.bindValue(":poitem_id", pPoitemid);
    q.exec();
    if (q.first())
    {
      _original->setDate(q.value("poitem_duedate").toDate());
      _current->setDate(q.value("poitem_duedate").toDate());
    }
  }
}

void reschedulePoitem::sReschedule()
{
  q.prepare("SELECT changePoitemDueDate(:poitem_id, :dueDate);");
  q.bindValue(":poitem_id", _poitem->id());
  q.bindValue(":dueDate", _new->date());
  q.exec();

  omfgThis->sPurchaseOrdersUpdated(_po->id(), TRUE);

  accept();
}

