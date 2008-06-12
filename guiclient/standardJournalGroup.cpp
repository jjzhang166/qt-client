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

#include "standardJournalGroup.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <q3dragobject.h>
#include "standardJournalGroupItem.h"

/*
 *  Constructs a standardJournalGroup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
standardJournalGroup::standardJournalGroup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardJournalGroup::~standardJournalGroup()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardJournalGroup::languageChange()
{
    retranslateUi(this);
}


void standardJournalGroup::init()
{
  setAcceptDrops(TRUE);

  _stdjrnlgrpitem->addColumn(tr("Name"),        _itemColumn,  Qt::AlignLeft   );
  _stdjrnlgrpitem->addColumn(tr("Description"), -1,           Qt::AlignLeft   );
  _stdjrnlgrpitem->addColumn(tr("To Apply"),    _dateColumn,  Qt::AlignRight  );
  _stdjrnlgrpitem->addColumn(tr("Applied"),     _dateColumn,  Qt::AlignRight  );
  _stdjrnlgrpitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter );
  _stdjrnlgrpitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter );
}

enum SetResponse standardJournalGroup::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnlgrp_id", &valid);
  if (valid)
  {
    _stdjrnlgrpid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('stdjrnlgrp_stdjrnlgrp_id_seq') AS _stdjrnlgrp_id;");
      if (q.first())
        _stdjrnlgrpid = q.value("_stdjrnlgrp_id").toInt();
//  ToDo

      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _new->setEnabled(FALSE);
      connect(_stdjrnlgrpitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournalGroup::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    q.prepare( "SELECT stdjrnlgrp_id "
               "FROM stdjrnlgrp "
               "WHERE (UPPER(stdjrnlgrp_name)=UPPER(:stdjrnlgrp_name));" );
    q.bindValue(":stdjrnlgrp_name", _name->text());
    q.exec();
    if (q.first())
    {
      _stdjrnlgrpid = q.value("stdjrnlgrp_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void standardJournalGroup::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM stdjrnlgrpitem "
               "WHERE (stdjrnlgrpitem_stdjrnlgrp_id=:stdjrnlgrp_id);" 
               "DELETE FROM stdjrnlgrp "
               "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );
    q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
    q.exec();
  }

  reject();
}

void standardJournalGroup::sSave()
{
  if (_name->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal Group"),
                          tr("You must enter a Name for this Standard Journal Group before you may save it.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO stdjrnlgrp "
               "(stdjrnlgrp_id, stdjrnlgrp_name, stdjrnlgrp_descrip) "
               "VALUES "
               "(:stdjrnlgrp_id, :stdjrnlgrp_name, :stdjrnlgrp_descrip);" );
  else
    q.prepare( "UPDATE stdjrnlgrp "
               "SET stdjrnlgrp_name=:stdjrnlgrp_name, stdjrnlgrp_descrip=:stdjrnlgrp_descrip "
               "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );

  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.bindValue(":stdjrnlgrp_name", _name->text());
  q.bindValue(":stdjrnlgrp_descrip", _descrip->text().stripWhiteSpace());
  q.exec();

  done(_stdjrnlgrpid);
}

void standardJournalGroup::sDelete()
{
  q.prepare( "DELETE FROM stdjrnlgrpitem "
             "WHERE (stdjrnlgrpitem_id=:stdjrnlgrpitem_id);" );
  q.bindValue(":stdjrnlgrpitem_id", _stdjrnlgrpitem->id());
  q.exec();

  sFillList();
}

void standardJournalGroup::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("stdjrnlgrp_id", _stdjrnlgrpid);

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroup::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlgrpitem_id", _stdjrnlgrpitem->id());

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournalGroup::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlgrpitem_id", _stdjrnlgrpitem->id());

  standardJournalGroupItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournalGroup::sFillList()
{
  QString sql( "SELECT stdjrnlgrpitem_id, stdjrnl_name, stdjrnl_descrip,"
               "       CASE WHEN (stdjrnlgrpitem_toapply=-1) THEN :always"
               "            ELSE TEXT(stdjrnlgrpitem_toapply)"
               "       END,"
               "       stdjrnlgrpitem_applied,"
               "       formatDate(stdjrnlgrpitem_effective),"
               "       formatDate(stdjrnlgrpitem_expires) "
               "FROM stdjrnlgrpitem, stdjrnl "
               "WHERE ((stdjrnlgrpitem_stdjrnl_id=stdjrnl_id)" );

  if (!_showExpired->isChecked())
    sql += " AND (stdjrnlgrpitem_expires > CURRENT_DATE)"
           " AND ( (stdjrnlgrpitem_applied < stdjrnlgrpitem_toapply) OR (stdjrnlgrpitem_toapply = -1) )";

  if (!_showFuture->isChecked())
    sql += " AND (stdjrnlgrpitem_effective <= CURRENT_DATE)";


  sql += " AND (stdjrnlgrpitem_stdjrnlgrp_id=:stdjrnlgrp_id) ) "
         "ORDER BY stdjrnl_name;";

  q.prepare(sql);
  q.bindValue(":always", tr("Always"));
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.exec();
  _stdjrnlgrpitem->populate(q);
}

void standardJournalGroup::populate()
{
  q.prepare( "SELECT stdjrnlgrp_name, stdjrnlgrp_descrip "
             "FROM stdjrnlgrp "
             "WHERE (stdjrnlgrp_id=:stdjrnlgrp_id);" );
  q.bindValue(":stdjrnlgrp_id", _stdjrnlgrpid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("stdjrnlgrp_name").toString());
    _descrip->setText(q.value("stdjrnlgrp_descrip").toString());

    sFillList();
  }
}

