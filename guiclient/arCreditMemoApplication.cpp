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

#include "arCreditMemoApplication.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

arCreditMemoApplication::arCreditMemoApplication(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _cust->setReadOnly(TRUE);
}

arCreditMemoApplication::~arCreditMemoApplication()
{
  // no need to delete child widgets, Qt does it all for us
}

void arCreditMemoApplication::languageChange()
{
  retranslateUi(this);
}

enum SetResponse arCreditMemoApplication::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sourceAropenid", &valid);
  if (valid)
    _sourceAropenid = param.toInt();

  param = pParams.value("targetAropenid", &valid);
  if (valid)
  {
    _targetAropenid = param.toInt();
    populate();
  }

  return NoError;
}

void arCreditMemoApplication::sSave()
{
  double amountToApply = _amountToApply->localValue();

  // check to make sure the amount being applied does not exceed
  // the balance due on the target item.
  q.prepare( "SELECT ROUND(currToCurr(aropen_curr_id, :curr_id,"
	     "     aropen_amount - aropen_paid, aropen_docdate), 2) AS balance "
             "  FROM aropen "
             " WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _targetAropenid);
  q.bindValue(":curr_id",   _amountToApply->id());
  q.exec();
  double targetBalance = 0.0;
  if(q.first())
    targetBalance = q.value("balance").toDouble();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if(amountToApply > targetBalance)
  {
    QMessageBox::warning(this, tr("Invalid Application"),
      tr("You may not apply more than the balance due to this Document.") );
    return;
  }

  // check to make sure the mount being applied does not exceed
  // the remaining balance of the source item.
  q.prepare( "SELECT round((aropen_amount - aropen_paid) - "
	     "	COALESCE(SUM(currToCurr(arcreditapply_curr_id, "
	     "				aropen_curr_id, "
	     "				arcreditapply_amount, "
	     "				aropen_docdate)), 0), 2) - COALESCE(prepared,0.0) AS available "
             "FROM aropen LEFT OUTER JOIN arcreditapply "
             "  ON ((arcreditapply_source_aropen_id=aropen_id) "
              " AND (arcreditapply_target_aropen_id<>:targetAropenid)) "
  	         "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, aropen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "WHERE (aropen_id=:sourceAropenid) "
             "GROUP BY aropen_amount, aropen_paid, prepared;" );
  q.bindValue(":sourceAropenid", _sourceAropenid);
  q.bindValue(":targetAropenid", _targetAropenid);
  q.exec();
  double sourceBalance = 0.0;
  if(q.first())
    sourceBalance = q.value("available").toDouble();
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if(amountToApply > sourceBalance)
  {
    QMessageBox::warning(this, tr("Invalid Application"),
      tr("You may not apply more than the amount available to apply for this Credit Memo.") );
    return;
  }

  q.prepare( "SELECT arcreditapply_id "
             "FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:sourceAropenid)"
             " AND (arcreditapply_target_aropen_id=:targetAropenid) );" );
  q.bindValue(":sourceAropenid", _sourceAropenid);
  q.bindValue(":targetAropenid", _targetAropenid);
  q.exec();
  if (q.first())
  {
    int arcreditapplyid = q.value("arcreditapply_id").toInt();

    q.prepare( "UPDATE arcreditapply "
               "SET arcreditapply_amount=:arcreditapply_amount, "
	       "    arcreditapply_curr_id = :arcreditapply_curr_id "
               "WHERE (arcreditapply_id=:arcreditapply_id);" );
    q.bindValue(":arcreditapply_id", arcreditapplyid);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
  {
    q.prepare( "INSERT INTO arcreditapply "
               "( arcreditapply_source_aropen_id, arcreditapply_target_aropen_id, "
	       " arcreditapply_amount, arcreditapply_curr_id ) "
               "VALUES "
               "( :sourceAropenid, :targetAropenid, "
	       "  :arcreditapply_amount, :arcreditapply_curr_id );" );
    q.bindValue(":sourceAropenid", _sourceAropenid);
    q.bindValue(":targetAropenid", _targetAropenid);
  }

  q.bindValue(":arcreditapply_amount", amountToApply);
  q.bindValue(":arcreditapply_curr_id", _amountToApply->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void arCreditMemoApplication::populate()
{
  q.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_doctype,"
             "       aropen_docdate, aropen_duedate, "
             "       aropen_amount, "
			 "       aropen_paid, "
             "       (aropen_amount - aropen_paid) AS f_balance, "
	         "       aropen_curr_id "
             "FROM aropen "
             "WHERE (aropen_id=:aropen_id);" );
  q.bindValue(":aropen_id", _targetAropenid);
  q.exec();
  if (q.first())
  {
    _cust->setId(q.value("aropen_cust_id").toInt());
    _docNumber->setText(q.value("aropen_docnumber").toString());
    _docType->setText(q.value("aropen_doctype").toString());
    _docDate->setDate(q.value("aropen_docdate").toDate(), true);
    _dueDate->setDate(q.value("aropen_duedate").toDate());
    _targetAmount->set(q.value("aropen_amount").toDouble(),
		       q.value("aropen_curr_id").toInt(),
		       q.value("aropen_docdate").toDate(), false);
    _targetPaid->setLocalValue(q.value("aropen_paid").toDouble());
    _targetBalance->setLocalValue(q.value("f_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT COALESCE(arcreditapply_curr_id,aropen_curr_id) AS curr_id,"
             "       currToCurr(aropen_curr_id,"
	         "                 COALESCE(arcreditapply_curr_id,aropen_curr_id),"
	         "                 aropen_amount - aropen_paid, aropen_docdate) - "
	         "		COALESCE(SUM(arcreditapply_amount), 0) - COALESCE(prepared,0.0) AS available,"
             "       aropen_docdate "
             "FROM aropen LEFT OUTER JOIN arcreditapply ON (arcreditapply_source_aropen_id=aropen_id) "
    	     "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, aropen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1 "
	         "         ON (prepared_aropen_id=aropen_id)"
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_amount, aropen_paid, aropen_docdate,"
	         "         arcreditapply_curr_id, aropen_curr_id, prepared;" );
  q.bindValue(":aropen_id", _sourceAropenid);
  q.exec();
  if (q.first())
  {
    _availableToApply->set(q.value("available").toDouble(),
		           q.value("curr_id").toInt(),
		           q.value("aropen_docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT currToCurr(arcreditapply_curr_id, :curr_id, "
	     "                  arcreditapply_amount, :effective) AS arcreditapply_amount "
             "FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:source_aropen_id)"
             " AND (arcreditapply_target_aropen_id=:target_aropen_id) );" );
  q.bindValue(":source_aropen_id", _sourceAropenid);
  q.bindValue(":target_aropen_id", _targetAropenid);
  q.bindValue(":curr_id", _amountToApply->id());
  q.bindValue(":effective", _amountToApply->effective());
  q.exec();
  if (q.first())
    _amountToApply->setLocalValue(q.value("arcreditapply_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
