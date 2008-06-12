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

#include "customerFormAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customerFormAssignment::customerFormAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
}

customerFormAssignment::~customerFormAssignment()
{
    // no need to delete child widgets, Qt does it all for us
}

void customerFormAssignment::languageChange()
{
    retranslateUi(this);
}

enum SetResponse customerFormAssignment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("custform_id", &valid);
  if (valid)
  {
    _custformid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _customerType->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerTypeGroup->setEnabled(FALSE);
      _invoiceForm->setEnabled(FALSE);
      _creditMemoForm->setEnabled(FALSE);
      _statementForm->setEnabled(FALSE);
      _quoteForm->setEnabled(FALSE);
      _packingListForm->setEnabled(FALSE);
      _soPickListForm->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void customerFormAssignment::sSave()
{
  int custtypeid   = -1;
  QString custtype = ""; 

  if (_selectedCustomerType->isChecked())
    custtypeid = _customerTypes->id();
  else if (_customerTypePattern->isChecked())
    custtype = _customerType->text().stripWhiteSpace();

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('custform_custform_id_seq') AS _custformid");
    if (q.first())
      _custformid = q.value("_custformid").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO custform "
               "( custform_id, custform_custtype, custform_custtype_id,"
               "  custform_invoice_report_id, custform_creditmemo_report_id,"
               "  custform_statement_report_id, custform_quote_report_id,"
               "   custform_packinglist_report_id, custform_sopicklist_report_id ) "
               "VALUES "
               "( :custform_id, :custform_custtype, :custform_custtype_id,"
               "  :custform_invoice_report_id, :custform_creditmemo_report_id,"
               "  :custform_statement_report_id, :custform_quote_report_id,"
               "  :custform_packinglist_report_id, :custform_sopicklist_report_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE custform "
               "SET custform_custtype=:custform_custtype, custform_custtype_id=:custform_custtype_id,"
               "    custform_invoice_report_id=:custform_invoice_report_id,"
               "    custform_creditmemo_report_id=:custform_creditmemo_report_id,"
               "    custform_statement_report_id=:custform_statement_report_id,"
               "    custform_quote_report_id=:custform_quote_report_id,"
               "    custform_packinglist_report_id=:custform_packinglist_report_id,"
	       "    custform_sopicklist_report_id=:custform_sopicklist_report_id "
               "WHERE (custform_id=:custform_id);" );

  q.bindValue(":custform_id", _custformid);
  q.bindValue(":custform_custtype", custtype);
  q.bindValue(":custform_custtype_id", custtypeid);
  q.bindValue(":custform_invoice_report_id", _invoiceForm->id());
  q.bindValue(":custform_creditmemo_report_id", _creditMemoForm->id());
  q.bindValue(":custform_statement_report_id", _statementForm->id());
  q.bindValue(":custform_quote_report_id", _quoteForm->id());
  q.bindValue(":custform_packinglist_report_id", _packingListForm->id());
  q.bindValue(":custform_sopicklist_report_id",	_soPickListForm->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_custformid);
}

void customerFormAssignment::populate()
{
  q.prepare( "SELECT custform_custtype_id, custform_custtype,"
             "       custform_invoice_report_id, custform_creditmemo_report_id,"
             "       custform_statement_report_id, custform_quote_report_id,"
             "       custform_packinglist_report_id, custform_sopicklist_report_id "
             "FROM custform "
             "WHERE (custform_id=:custform_id);" );
  q.bindValue(":custform_id", _custformid);
  q.exec();
  if (q.first())
  {
    if (q.value("custform_custtype_id").toInt() == -1)
    {
      _customerTypePattern->setChecked(TRUE);
      _customerType->setText(q.value("custform_custtype").toString());
    }
    else
    {
      _selectedCustomerType->setChecked(TRUE);
      _customerTypes->setId(q.value("custform_custtype_id").toInt());
    }

    _invoiceForm->setId(q.value("custform_invoice_report_id").toInt());
    _creditMemoForm->setId(q.value("custform_creditmemo_report_id").toInt());
    _statementForm->setId(q.value("custform_statement_report_id").toInt());
    _quoteForm->setId(q.value("custform_quote_report_id").toInt());
    _packingListForm->setId(q.value("custform_packinglist_report_id").toInt());
    _soPickListForm->setId(q.value("custform_sopicklist_report_id").toInt());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
