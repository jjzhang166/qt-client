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

#include "copyBOM.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a copyBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyBOM::copyBOM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_source, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_target, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = FALSE;

  _source->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cPlanning | ItemLineEdit::cJob);
  _target->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cPlanning | ItemLineEdit::cJob);
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyBOM::~copyBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyBOM::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(FALSE);
    _target->setFocus();
  }

  return NoError;
}

void copyBOM::sCopy()
{
  q.prepare( "SELECT bomitem_id "
             "FROM bomitem "
             "WHERE (bomitem_parent_item_id=:item_id) "
             "LIMIT 1;" );
  q.bindValue(":item_id", _source->id());
  q.exec();
  if (!q.first())
    QMessageBox::information( this, tr("Non-Existent Bill of Materials"),
                              tr("The selected target Item does not have any Bill of Material Component Items associated with it.") );

  else
  {
    q.prepare( "SELECT bomitem_id "
               "FROM bomitem "
               "WHERE ( (bomitem_expires > CURRENT_DATE)"
               " AND (bomitem_parent_item_id=:item_id) ) "
               "LIMIT 1;" );
    q.bindValue(":item_id", _target->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Existing Bill of Materials"),
                                tr( "The selected target Item already has a Bill of Materials associated with it.\n"
                                    "You must first delete the Bill of Materials for the selected target item before\n"
                                    "attempting to copy an existing Bill of Materials." ) );
    else
    {
	
      q.prepare("SELECT bomitem_id "
		        "FROM bomitem(:item_id) "
                "WHERE ( (bomitem_booitem_seq_id != -1) "
                " AND (bomitem_booitem_seq_id IS NOT NULL) ) "
                "LIMIT 1;" );
      q.bindValue(":item_id", _source->id());
      q.exec();
      if (q.first())
        QMessageBox::information( this, tr("Dependent BOO Data"),
          tr("One or more of the components for this Bill of Materials make reference to a\n"
             "Bill of Operations. These references cannot be copied and must be added manually.") );
      
      q.prepare("SELECT copyBOM(:sourceid, :targetid) AS result;");
      q.bindValue(":sourceid", _source->id());
      q.bindValue(":targetid", _target->id());
      q.exec();
      if(q.first() && q.value("result").toInt() < 0)
      {
        QMessageBox::information( this, tr("Recursive BOM"),
          tr("The Item you are trying to copy this Bill of Material to is a\n"
             "component item which would cause a recursive Bill of Material.\n"
             "Cannot copy Bill of Material.") );
        return;
      }
      omfgThis->sBOMsUpdated(_target->id(), TRUE);
    }
  }

  if (_captive)
    close();
  else
  {
    _source->setId(-1);
    _target->setId(-1);
    _source->setFocus();
  }
}

void copyBOM::sHandleButtons()
{
  _copy->setEnabled(_source->isValid() && _target->isValid());
}
