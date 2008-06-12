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

#include "standardJournal.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "standardJournalItem.h"

/*
 *  Constructs a standardJournal as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
standardJournal::standardJournal(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_stdjrnlitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardJournal::~standardJournal()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardJournal::languageChange()
{
    retranslateUi(this);
}


void standardJournal::init()
{
  _stdjrnlitem->addColumn(tr("Account"), 200,          Qt::AlignLeft  );
  _stdjrnlitem->addColumn(tr("Notes"),   -1,           Qt::AlignLeft  );
  _stdjrnlitem->addColumn(tr("Debit"),   _priceColumn, Qt::AlignRight );
  _stdjrnlitem->addColumn(tr("Credit"),  _priceColumn, Qt::AlignRight );
}

void standardJournal::destroy()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM stdjrnlitem "
               "WHERE (stdjrnlitem_stdjrnlid=:stdjrnl_id);" );
    q.bindValue(":stdjrnl_id", _stdjrnlid);
    q.exec();
  }
}

enum SetResponse standardJournal::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
  {
    _stdjrnlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('stdjrnl_stdjrnl_id_seq') AS _stdjrnl_id");
      if (q.first())
        _stdjrnlid = q.value("_stdjrnl_id").toInt();

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _stdjrnlitem->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _new->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardJournal::sSave()
{
  if (_mode == cNew)
    q.prepare( "INSERT INTO stdjrnl "
               "(stdjrnl_id, stdjrnl_name, stdjrnl_descrip, stdjrnl_notes) "
               "VALUES "
               "(:stdjrnl_id, :stdjrnl_name, :stdjrnl_descrip, :stdjrnl_notes);" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdjrnl "
               " SET stdjrnl_name=:stdjrnl_name, stdjrnl_descrip=:stdjrnl_descrip, stdjrnl_notes=:stdjrnl_notes "
               " WHERE (stdjrnl_id=:stdjrnl_id);" );

  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.bindValue(":stdjrnl_name", _name->text());
  q.bindValue(":stdjrnl_descrip", _descrip->text());
  q.bindValue(":stdjrnl_notes", _notes->text());
  q.exec();

  done(_stdjrnlid);
}

void standardJournal::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT stdjrnl_id "
               "FROM stdjrnl "
               "WHERE (UPPER(stdjrnl_name)=UPPER(:stdjrnl_name));" );
    q.bindValue(":stdjrnl_name", _name->text());
    q.exec();
    if (q.first())
    {
      _stdjrnlid = q.value("stdjrnl_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void standardJournal::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("stdjrnl_id", _stdjrnlid);

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournal::sDelete()
{
  q.prepare( "DELETE FROM stdjrnlitem "
             "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
  q.bindValue(":stdjrnlitem_id", _stdjrnlitem->id());
  q.exec();
  sFillList();
}

void standardJournal::sFillList()
{
  q.prepare( "SELECT stdjrnlitem_id,"
             "       CASE WHEN(accnt_id IS NOT NULL) THEN (formatGLAccount(accnt_id) || '-' || accnt_descrip)"
             "            ELSE 'ERROR - NO ACCOUNT SPECIFIED'"
             "       END,"
             "       firstLine(stdjrnlitem_notes),"
             "       CASE WHEN (stdjrnlitem_amount < 0) THEN formatMoney(stdjrnlitem_amount * -1)"
             "            ELSE ''"
             "       END AS debit,"
             "       CASE WHEN (stdjrnlitem_amount > 0) THEN formatMoney(stdjrnlitem_amount)"
             "            ELSE ''"
             "       END AS credit "
             "  FROM stdjrnlitem LEFT OUTER JOIN accnt ON (stdjrnlitem_accnt_id=accnt_id)"
             " WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id) "
             " ORDER BY accnt_number, accnt_profit, accnt_sub;" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  _stdjrnlitem->populate(q);

  q.prepare( "SELECT formatMoney( SUM( CASE WHEN (stdjrnlitem_amount < 0) THEN (stdjrnlitem_amount * -1)"
             "                              ELSE 0"
             "                         END ) ) AS f_debit,"
             "       formatMoney( SUM( CASE WHEN (stdjrnlitem_amount > 0) THEN stdjrnlitem_amount"
             "                              ELSE 0"
             "                         END ) ) AS f_credit,"
             "       (SUM(stdjrnlitem_amount) <> 0) AS oob "
             "FROM stdjrnlitem "
             "WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id);" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  if (q.first())
  {
    _debits->setText(q.value("f_debit").toString());
    _credits->setText(q.value("f_credit").toString());

    if (q.value("oob").toBool())
    {
      _debits->setPaletteForegroundColor(QColor("red"));
      _credits->setPaletteForegroundColor(QColor("red"));
    }
    else
    {
      _debits->setPaletteForegroundColor(QColor("black"));
      _credits->setPaletteForegroundColor(QColor("black"));
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void standardJournal::populate()
{
  q.prepare( "SELECT stdjrnl_name, stdjrnl_descrip, stdjrnl_notes "
             "FROM stdjrnl "
             "WHERE (stdjrnl_id=:stdjrnl_id);" );
  q.bindValue(":stdjrnl_id", _stdjrnlid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("stdjrnl_name").toString());
    _descrip->setText(q.value("stdjrnl_descrip").toString());
    _notes->setText(q.value("stdjrnl_notes").toString());

    sFillList();
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

