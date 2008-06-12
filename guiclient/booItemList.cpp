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

#include "booItemList.h"

#include <QVariant>

/*
 *  Constructs a booItemList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
booItemList::booItemList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_booitem, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));

  _booitem->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter );
  _booitem->addColumn(tr("Description"), -1,         Qt::AlignLeft   );
}

/*
 *  Destroys the object and frees any allocated resources
 */
booItemList::~booItemList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void booItemList::languageChange()
{
    retranslateUi(this);
}

enum SetResponse booItemList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("booitem_seq_id", &valid);
  if (valid)
    _booitemseqid = param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void booItemList::sClose()
{
  done(_booitemseqid);
}

void booItemList::sClear()
{
  done(-1);
}

void booItemList::sSelect()
{
  done(_booitem->id());
}

void booItemList::sFillList()
{
  q.prepare( "SELECT booitem_seq_id, booitem_seqnumber,"
             "       (booitem_descrip1 || ' ' || booitem_descrip2) "
			 "FROM booitem(:item_id) "
             "WHERE ( (CURRENT_DATE BETWEEN booitem_effective AND booitem_expires) ) "
             "ORDER BY booitem_seqnumber;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _booitem->populate(q, _booitemseqid );
}

