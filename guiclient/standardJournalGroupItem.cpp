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

#include "standardJournalGroupItem.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a standardJournalGroupItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
standardJournalGroupItem::standardJournalGroupItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_limited, SIGNAL(toggled(bool)), _toApply, SLOT(setEnabled(bool)));
    connect(_stdjrnl, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardJournalGroupItem::~standardJournalGroupItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardJournalGroupItem::languageChange()
{
    retranslateUi(this);
}


void standardJournalGroupItem::init()
{
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _stdjrnl->populate( "SELECT stdjrnl_id, stdjrnl_name "
                      "FROM stdjrnl "
                      "ORDER BY stdjrnl_name;" );
}

enum SetResponse standardJournalGroupItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
    _stdjrnlgrpid = param.toInt();

  param = pParams.value("stdjrnlgrpitem_id", &valid);
  if (valid)
  {
    _stdjrnlgrpitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _stdjrnl->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _stdjrnl->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _applyGroup->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournalGroupItem::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('stdjrnlgrpitem_stdjrnlgrpitem_id_seq') AS stdjrnlgrpitem_id;");
    if (q.first())
      _stdjrnlgrpitemid = q.value("stdjrnlgrpitem_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO stdjrnlgrpitem "
               "( stdjrnlgrpitem_id, stdjrnlgrpitem_stdjrnlgrp_id, stdjrnlgrpitem_stdjrnl_id,"
               "  stdjrnlgrpitem_toapply, stdjrnlgrpitem_applied,"
               "  stdjrnlgrpitem_effective, stdjrnlgrpitem_expires )"
               "VALUES "
               "( :stdjrnlgrpitem_id, :stdjrnlgrpitem_stdjrnlgrp_id, :stdjrnlgrpitem_stdjrnl_id,"
               "  :stdjrnlgrpitem_toapply, 0,"
               "  :stdjrnlgrpitem_effective, :stdjrnlgrpitem_expires );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdjrnlgrpitem "
               "SET stdjrnlgrpitem_toapply=:stdjrnlgrpitem_toapply,"
               "    stdjrnlgrpitem_effective=:stdjrnlgrpitem_effective, stdjrnlgrpitem_expires=:stdjrnlgrpitem_expires "
               "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );

  q.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitemid);
  q.bindValue(":stdjrnlgrpitem_stdjrnlgrp_id", _stdjrnlgrpid);
  q.bindValue(":stdjrnlgrpitem_stdjrnl_id", _stdjrnl->id());
  q.bindValue(":stdjrnlgrpitem_toapply", ((_limited->isChecked()) ? _toApply->value() : -1));
  q.bindValue(":stdjrnlgrpitem_effective", _dates->startDate());
  q.bindValue(":stdjrnlgrpitem_expires", _dates->endDate());
  q.exec();

  done(_stdjrnlgrpitemid);
}

void standardJournalGroupItem::sCheck()
{
}

void standardJournalGroupItem::populate()
{
  q.prepare( "SELECT stdjrnlgrpitem_stdjrnl_id, stdjrnlgrpitem_toapply,"
             "       stdjrnlgrpitem_effective, stdjrnlgrpitem_expires "
             "FROM stdjrnlgrpitem "
             "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );
  q.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitemid);
  q.exec();
  if (q.first())
  {
    _stdjrnl->setId(q.value("stdjrnlgrpitem_stdjrnl_id").toInt());
    _dates->setStartDate(q.value("stdjrnlgrpitem_effective").toDate());
    _dates->setEndDate(q.value("stdjrnlgrpitem_expires").toDate());

    if (q.value("stdjrnlgrpitem_toapply").toInt() == -1)
      _unlimited->setChecked(TRUE);
    else
    {
      _limited->setChecked(TRUE);
      _toApply->setValue(q.value("stdjrnlgrpitem_toapply").toInt());
    }
  }
}

