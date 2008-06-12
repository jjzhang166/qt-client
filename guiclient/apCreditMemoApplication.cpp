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

#include "apCreditMemoApplication.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QValidator>

#include "storedProcErrorLookup.h"

apCreditMemoApplication::apCreditMemoApplication(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _vend->setReadOnly(TRUE);
}

apCreditMemoApplication::~apCreditMemoApplication()
{
    // no need to delete child widgets, Qt does it all for us
}

void apCreditMemoApplication::languageChange()
{
    retranslateUi(this);
}

enum SetResponse apCreditMemoApplication::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sourceApopenid", &valid);
  if (valid)
    _sourceApopenid = param.toInt();

  param = pParams.value("targetApopenid", &valid);
  if (valid)
  {
    _targetApopenid = param.toInt();
    populate();
  }

  return NoError;
}

void apCreditMemoApplication::sSave()
{
  q.prepare("SELECT createAPCreditMemoApplication(:source, :target, "
	    "                                   :amount, :curr_id) AS result;");
  q.bindValue(":source",  _sourceApopenid);
  q.bindValue(":target",  _targetApopenid);
  q.bindValue(":amount",  _amountToApply->localValue());
  q.bindValue(":curr_id", _amountToApply->id());

  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("createApCreditMemoApplication",
					      result), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void apCreditMemoApplication::populate()
{
  q.prepare( "SELECT apopen_vend_id, apopen_docnumber, apopen_doctype,"
             "       apopen_docdate, apopen_duedate,"
             "       apopen_amount,"
             "       apopen_paid,"
             "       (apopen_amount - apopen_paid - COALESCE(selected,0.0) - COALESCE(prepared,0.0)) AS f_balance, "
	     "       apopen_curr_id "
             "  FROM apopen"
             "       LEFT OUTER JOIN (SELECT apopen_id AS selected_apopen_id,"
             "                             SUM(currToCurr(apselect_curr_id, apopen_curr_id, apselect_amount + apselect_discount, apselect_date)) AS selected"
             "                        FROM apselect JOIN apopen ON (apselect_apopen_id=apopen_id)"
             "                       WHERE (apopen_id=:apopen_id)"
             "                       GROUP BY apopen_id) AS sub1"
             "         ON (apopen_id=selected_apopen_id)"
             "       LEFT OUTER JOIN (SELECT apopen_id AS prepared_apopen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN apopen ON (checkitem_apopen_id=apopen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void)"
             "                           AND  (apopen_id=:apopen_id))"
             "                         GROUP BY apopen_id) AS sub2"
             "         ON (prepared_apopen_id=apopen_id)"
             " WHERE (apopen_id=:apopen_id);" );
  q.bindValue(":apopen_id", _targetApopenid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apopen_vend_id").toInt());
    _docNumber->setText(q.value("apopen_docnumber").toString());
    _docType->setText(q.value("apopen_doctype").toString());
    _docDate->setDate(q.value("apopen_docdate").toDate(), true);
    _dueDate->setDate(q.value("apopen_duedate").toDate());
    _targetAmount->set(q.value("apopen_amount").toDouble(), 
		       q.value("apopen_curr_id").toInt(),
		       q.value("apopen_docdate").toDate(), false);
    _targetPaid->setLocalValue(q.value("apopen_paid").toDouble());
    _targetBalance->setLocalValue(q.value("f_balance").toDouble());
  }
  else if (q.lastError().type() != QSqlError::None)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT COALESCE(apcreditapply_curr_id,apopen_curr_id) AS curr_id,"
	     "       currToCurr(apopen_curr_id,"
	     "		        COALESCE(apcreditapply_curr_id,apopen_curr_id),"
	     "		        apopen_amount - apopen_paid, apopen_docdate) -"
	     "		   COALESCE(SUM(apcreditapply_amount), 0) AS available,"
             "       apopen_docdate "
             "FROM apopen LEFT OUTER JOIN apcreditapply ON (apcreditapply_source_apopen_id=apopen_id) "
             "WHERE (apopen_id=:apopen_id) "
             "GROUP BY apopen_amount, apopen_paid, apopen_docdate,"
	     "         apcreditapply_curr_id, apopen_curr_id;" );
  q.bindValue(":apopen_id", _sourceApopenid);
  q.exec();
  if (q.first())
  {
    _availableToApply->set(q.value("available").toDouble(), 
		           q.value("curr_id").toInt(),
		           q.value("apopen_docdate").toDate(), false);
  }
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT currToCurr(apcreditapply_curr_id, :curr_id, "
	     "                  apcreditapply_amount, :effective) AS apcreditapply_amount "
             "FROM apcreditapply "
             "WHERE ( (apcreditapply_source_apopen_id=:source_apopen_id)"
             " AND (apcreditapply_target_apopen_id=:target_apopen_id) );" );
  q.bindValue(":source_apopen_id", _sourceApopenid);
  q.bindValue(":target_apopen_id", _targetApopenid);
  q.bindValue(":curr_id", _amountToApply->id());
  q.bindValue(":effective", _amountToApply->effective());
  q.exec();
  if (q.first())
    _amountToApply->setLocalValue(q.value("apcreditapply_amount").toDouble());
  else if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}
