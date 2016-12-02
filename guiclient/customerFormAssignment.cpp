/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerFormAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include "errorReporter.h"

customerFormAssignment::customerFormAssignment(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
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
  XDialog::set(pParams);
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
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _customerTypeGroup->setEnabled(false);
      _invoiceForm->setEnabled(false);
      _creditMemoForm->setEnabled(false);
      _statementForm->setEnabled(false);
      _quoteForm->setEnabled(false);
      _packingListForm->setEnabled(false);
      _soPickListForm->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void customerFormAssignment::sSave()
{
  XSqlQuery customerSave;
  int custtypeid   = -1;
  QString custtype = ""; 

  if (_selectedCustomerType->isChecked())
    custtypeid = _customerTypes->id();
  else if (_customerTypePattern->isChecked())
    custtype = _customerType->text().trimmed();

  customerSave.prepare("SELECT custform_id"
            "  FROM custform"
            " WHERE((custform_id != :custform_id)"
            "   AND (custform_custtype = :custform_custtype)"
            "   AND (custform_custtype_id=:custform_custtype_id))");
  customerSave.bindValue(":custform_id", _custformid);
  customerSave.bindValue(":custform_custtype", custtype);
  customerSave.bindValue(":custform_custtype_id", custtypeid);
  customerSave.exec();
  if(customerSave.first())
  {
    QMessageBox::critical(this, tr("Duplicate Entry"),
      tr("The Customer Type specified is already in the database.") );
    return;
  }

  if (_mode == cNew)
  {
    customerSave.exec("SELECT NEXTVAL('custform_custform_id_seq') AS _custformid");
    if (customerSave.first())
      _custformid = customerSave.value("_custformid").toInt();
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Form Assignment"),
                                  customerSave, __FILE__, __LINE__))
    {
      return;
    }

    customerSave.prepare( "INSERT INTO custform "
               "( custform_id, custform_custtype, custform_custtype_id,"
               "  custform_invoice_report_name, custform_creditmemo_report_name,"
               "  custform_statement_report_name, custform_quote_report_name,"
               "   custform_packinglist_report_name, custform_sopicklist_report_name ) "
               "VALUES "
               "( :custform_id, :custform_custtype, :custform_custtype_id,"
               "  :custform_invoice_report_name, :custform_creditmemo_report_name,"
               "  :custform_statement_report_name, :custform_quote_report_name,"
               "  :custform_packinglist_report_name, :custform_sopicklist_report_name );" );
  }
  else if (_mode == cEdit)
    customerSave.prepare( "UPDATE custform "
               "SET custform_custtype=:custform_custtype, custform_custtype_id=:custform_custtype_id,"
               "    custform_invoice_report_name=:custform_invoice_report_name,"
               "    custform_creditmemo_report_name=:custform_creditmemo_report_name,"
               "    custform_statement_report_name=:custform_statement_report_name,"
               "    custform_quote_report_name=:custform_quote_report_name,"
               "    custform_packinglist_report_name=:custform_packinglist_report_name,"
	       "    custform_sopicklist_report_name=:custform_sopicklist_report_name "
               "WHERE (custform_id=:custform_id);" );

  customerSave.bindValue(":custform_id", _custformid);
  customerSave.bindValue(":custform_custtype", custtype);
  customerSave.bindValue(":custform_custtype_id", custtypeid);
  customerSave.bindValue(":custform_invoice_report_name", _invoiceForm->code());
  customerSave.bindValue(":custform_creditmemo_report_name", _creditMemoForm->code());
  customerSave.bindValue(":custform_statement_report_name", _statementForm->code());
  customerSave.bindValue(":custform_quote_report_name", _quoteForm->code());
  customerSave.bindValue(":custform_packinglist_report_name", _packingListForm->code());
  customerSave.bindValue(":custform_sopicklist_report_name", _soPickListForm->code());
  customerSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Form Assignment"),
                                customerSave, __FILE__, __LINE__))
  {
    return;
  }

  done(_custformid);
}

void customerFormAssignment::populate()
{
  XSqlQuery customerpopulate;
  customerpopulate.prepare( "SELECT custform_custtype_id, custform_custtype,"
             "       custform_invoice_report_name, custform_creditmemo_report_name,"
             "       custform_statement_report_name, custform_quote_report_name,"
             "       custform_packinglist_report_name, custform_sopicklist_report_name "
             "FROM custform "
             "WHERE (custform_id=:custform_id);" );
  customerpopulate.bindValue(":custform_id", _custformid);
  customerpopulate.exec();
  if (customerpopulate.first())
  {
    if (customerpopulate.value("custform_custtype_id").toInt() == -1)
    {
      _customerTypePattern->setChecked(true);
      _customerType->setText(customerpopulate.value("custform_custtype").toString());
    }
    else
    {
      _selectedCustomerType->setChecked(true);
      _customerTypes->setId(customerpopulate.value("custform_custtype_id").toInt());
    }

    if (customerpopulate.value("custform_invoice_report_name").toString() != NULL)
      _invoiceForm->setCode(customerpopulate.value("custform_invoice_report_name").toString());
    if (customerpopulate.value("custform_creditmemo_report_name").toString() != NULL)
      _creditMemoForm->setCode(customerpopulate.value("custform_creditmemo_report_name").toString());
    if (customerpopulate.value("custform_statement_report_name").toString() != NULL)
      _statementForm->setCode(customerpopulate.value("custform_statement_report_name").toString());
    if (customerpopulate.value("custform_quote_report_name").toString() != NULL)
      _quoteForm->setCode(customerpopulate.value("custform_quote_report_name").toString());
    if (customerpopulate.value("custform_packinglist_report_name").toString() != NULL)
      _packingListForm->setCode(customerpopulate.value("custform_packinglist_report_name").toString());
    if (customerpopulate.value("custform_sopicklist_report_name").toString() != NULL)
      _soPickListForm->setCode(customerpopulate.value("custform_sopicklist_report_name").toString());
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Form Assignment Information"),
                                customerpopulate, __FILE__, __LINE__))
  {
    return;
  }
}
