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


void assortmentItem::init()
{
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));
  _qtyPer->setValidator(omfgThis->qtyPerVal());
}

enum SetResponse assortmentItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("assitem_id", &valid);
  if (valid)
  {
    _assitemid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "replace")
    {
      _mode = cReplace;

      _item->setId(-1);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _qtyPer->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void assortmentItem::sSave()
{
  if (_qtyPer->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity Per"),
                           tr("You must enter a Quantity Per value befor saving this Assortment Item.") );
    _qtyPer->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('assitem_assitem_id_seq') AS assitem_id;");
    if (q.first())
      _assitemid = q.value("assitem_id").toInt();

    q.prepare( "INSERT INTO assitem "
               "( assitem_id, assitem_seqnumber,"
               "  assitem_parent_item_id, assitem_item_id, assitem_qtyper,"
               "  assitem_effective, assitem_expires ) "
               "VALUES "
               "( :assitem_id, :assitem_seqnumber,"
               "  :assitem_parent_item_id, :assitem_item_id, :assitem_qtyper,"
               "  :assitem_effective, :assitem_expires );" );
   }
   else if ( (_mode == cCopy) || (_mode == cReplace) )
    q.prepare( "INSERT into assitem "
               "( assitem_seqnumber,"
               "  assitem_parent_item_id, assitem_item_id, assitem_qtyper,"
               "  assitem_effective, assitem_expires ) "
               "SELECT assitem_seqnumber,"
               "       assitem_parent_item_id, :assitem_item_id, :assitem_qtyper,"
               "       :assitem_effective, :assitem_expires "
               "FROM assitem "
               "WHERE (assitem_id=:assitem_id);" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE assitem "
               "SET assitem_qtyper=:assitem_qtyper, assitem_moddate=CURRENT_DATE,"
               "    assitem_effective=:assitem_effective, assitem_expires=:assitem_expires "
               "WHERE (assitem_id=:assitem_id);" );
  else
//  ToDo
    return;

  q.bindValue(":assitem_id", _assitemid);
  q.bindValue(":assitem_parent_item_id", _itemid);
  q.bindValue(":assitem_item_id", _item->id());
  q.bindValue(":assitem_qtyper", _qtyPer->toDouble());
  q.bindValue(":assitem_effective", _dates->startDate());
  q.bindValue(":assitem_expires", _dates->endDate());
  q.exec();

  omfgThis->sAssortmentsUpdated(_itemid, TRUE);

  done(_assitemid);
}

void assortmentItem::populate()
{
  q.prepare( "SELECT assitem_item_id, assitem_parent_item_id,"
             "       formatQtyper(assitem_qtyper) AS qtyper,"
             "       assitem_effective, assitem_expires, CURRENT_DATE AS today "
             "FROM assitem, item "
             "WHERE ( (assitem_parent_item_id=item_id)"
             " AND (assitem_id=:assitem_id) );" );
  q.bindValue(":assitem_id", _assitemid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("assitem_parent_item_id").toInt();
    _item->setId(q.value("assitem_item_id").toInt());

    if (_mode == cCopy)
      _dates->setStartDate(q.value("today").toDate());
    else
      _dates->setStartDate(q.value("assitem_effective").toDate());

    _qtyPer->setText(q.value("qtyper").toString());
  }
//  ToDo
}

