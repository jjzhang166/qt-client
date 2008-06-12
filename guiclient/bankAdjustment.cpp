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

#include "bankAdjustment.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qsqlerror.h>
#include <qstatusbar.h>
#include <qvalidator.h>

/*
 *  Constructs a bankAdjustment as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
bankAdjustment::bankAdjustment(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankAccount(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
bankAdjustment::~bankAdjustment()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bankAdjustment::languageChange()
{
    retranslateUi(this);
}


void bankAdjustment::init()
{
  statusBar()->hide();
  
  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip),"
		       "       bankaccnt_name "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");
  _bankadjtype->populate("SELECT bankadjtype_id,"
                         "       (bankadjtype_name || '-' || bankadjtype_descrip),"
			 "       bankadjtype_name "
                         "FROM bankadjtype "
                         "ORDER BY bankadjtype_name;");
  
  _bankadjid = -1;
}

SetResponse bankAdjustment::set( ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("bankaccnt_id", &valid);
  if (valid)
  {
    _bankaccnt->setId(param.toInt());
    //_bankaccnt->setEnabled(FALSE);
  }

  param = pParams.value("bankadj_id", &valid);
  if(valid)
  {
    _bankadjid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      
      _amount->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _bankaccnt->setEnabled(FALSE);
      _bankadjtype->setEnabled(FALSE);
      _date->setEnabled(FALSE);
      _docNumber->setEnabled(FALSE);
      _amount->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _save->hide();
    }
  }

  return NoError;
}

void bankAdjustment::sSave()
{
  if (!_date->isValid())
  {
    QMessageBox::information( this, tr("Cannot Post Bank Adjustment"),
                          tr("You must enter a date before posting this Bank Adjustment.") );
    _date->setFocus();
    return;
  }

  if (_amount->isZero())
  {
    QMessageBox::information( this, tr("Cannot Post Bank Adjustment"),
                          tr("You must enter an amount before posting this Bank Adjustment.") );
    _amount->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO bankadj "
                   "(bankadj_bankaccnt_id, bankadj_bankadjtype_id,"
                   " bankadj_date, bankadj_docnumber, bankadj_amount, "
		   " bankadj_notes, bankadj_curr_id ) "
                   "VALUES "
                   "(:bankaccnt_id, :bankadjtype_id,"
                   " :date, :docnumber, :amount, :notes, :curr_id);" );
  else if (_mode == cEdit)
  {
    q.prepare ( "UPDATE bankadj "
                    "SET bankadj_bankaccnt_id=:bankaccnt_id,"
                    " bankadj_bankadjtype_id=:bankadjtype_id,"
                    " bankadj_date=:date,"
                    " bankadj_docnumber=:docnumber,"
                    " bankadj_amount=:amount,"
                    " bankadj_notes=:notes, "
                    " bankadj_curr_id=:curr_id "
                    "WHERE ((bankadj_id=:bankadj_id)"
                    " AND (NOT bankadj_posted) ); ");
    q.bindValue(":bankadj_id", _bankadjid);
  }
    
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.bindValue(":bankadjtype_id", _bankadjtype->id());
  q.bindValue(":date", _date->date());
  q.bindValue(":docnumber", _docNumber->text());
  q.bindValue(":amount", _amount->localValue());
  q.bindValue(":notes", _notes->text());
  q.bindValue(":curr_id", _amount->id());
  
  if(!q.exec())
  {
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
    return;
  }

  omfgThis->sBankAdjustmentsUpdated(_bankadjid, TRUE);

  close();
}

void bankAdjustment::populate()
{
  q.prepare( "SELECT bankadj_bankaccnt_id, bankadj_bankadjtype_id,"
             "       bankadj_date, bankadj_docnumber, bankadj_amount,"
             "       bankadj_notes, bankadj_curr_id "
             "FROM bankadj "
             "WHERE (bankadj_id=:bankadj_id);" );
  q.bindValue(":bankadj_id", _bankadjid);
  q.exec();
  if(q.first())
  {
    _bankaccnt->setId(q.value("bankadj_bankaccnt_id").toInt());
    _bankadjtype->setId(q.value("bankadj_bankadjtype_id").toInt());
    _date->setDate(q.value("bankadj_date").toDate());
    _docNumber->setText(q.value("bankadj_docnumber").toString());
    _amount->set(q.value("bankadj_amount").toDouble(),
		 q.value("bankadj_curr_id").toInt(),
		 q.value("bankadj_date").toDate(), false);
    _notes->setText(q.value("bankadj_notes").toString());
  }
}

void bankAdjustment::sBankAccount(int accountId)
{
    XSqlQuery bankQ;
    bankQ.prepare("SELECT bankaccnt_curr_id "
		  "FROM bankaccnt WHERE bankaccnt_id = :accntId;");
    bankQ.bindValue(":accntId", accountId);
    bankQ.exec();
    if (bankQ.first())
	_amount->setId(bankQ.value("bankaccnt_curr_id").toInt());
    else
	QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      q.lastError().databaseText());
}
