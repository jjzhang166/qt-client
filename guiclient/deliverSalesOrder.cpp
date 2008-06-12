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

#include "deliverSalesOrder.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a deliverSalesOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
deliverSalesOrder::deliverSalesOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sHandleSoheadid()));

  _captive = FALSE;

  _so->setType(cSoOpen | cSoClosed);
  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SO') "
                     "ORDER BY form_name;" );

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
//  ToDo
}

/*
 *  Destroys the object and frees any allocated resources
 */
deliverSalesOrder::~deliverSalesOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void deliverSalesOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse deliverSalesOrder::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _submit->setFocus();
  }

  return NoError;
}

void deliverSalesOrder::sSubmit()
{
  if (_email->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Queue Sales Order for Delivery"),
                           tr("You must enter a email address to which this Sales Order is to be delivered.") );
    _email->setFocus();
    return;
  }

  q.prepare( "SELECT report_name "
             "  FROM report, form "
             " WHERE ( (form_id=:form_id) "
             "   AND (report_id=form_report_id) );");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (!q.first())
  {
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
    return;
  }
  QString reportName = q.value("report_name").toString();

  q.prepare( "SELECT cohead_number"
             "  FROM cohead"
             " WHERE (cohead_id=:sohead_id);" );
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    QString soNumber = q.value("cohead_number").toString();

    q.prepare( "SELECT submitReportToBatch( :reportname, :fromEmail, :emailAddress, :ccAddress, :subject,"
               "                            :emailBody, :fileName, CURRENT_TIMESTAMP) AS batch_id;" );
    q.bindValue(":reportname", reportName);
    q.bindValue(":fromEmail", _fromEmail->text());
    q.bindValue(":emailAddress", _email->text());
    q.bindValue(":ccAddress", _cc->text());
    q.bindValue(":subject", _subject->text().replace("</docnumber>", soNumber).replace("</doctype>", "SO"));
    q.bindValue(":fileName", _fileName->text().replace("</docnumber>", soNumber).replace("</doctype>", "SO"));
    q.bindValue(":emailBody", _emailBody->text().replace("</docnumber>", soNumber).replace("</doctype>", "SO"));
    q.exec();
    if (q.first())
    {
      int batch_id = q.value("batch_id").toInt();

      q.prepare( "INSERT INTO batchparam "
                 "( batchparam_batch_id, batchparam_order,"
                 "  batchparam_name, batchparam_value ) "
                 "VALUES "
                 "( :batchparam_batch_id, :batchparam_order,"
                 "  :batchparam_name, :batchparam_value );" );

      q.bindValue(":batchparam_batch_id", batch_id);
      q.bindValue(":batchparam_order", 1);
      q.bindValue(":batchparam_name", "sohead_id");
      q.bindValue(":batchparam_value", _so->id());
      q.exec();

      q.bindValue(":batchparam_batch_id", batch_id);
      q.bindValue(":batchparam_order", 2);
      q.bindValue(":batchparam_name", "title");
      q.bindValue(":batchparam_value", "Emailed Customer Copy");
      q.exec();
    }
  }
//  ToDo

  if (_captive)
    accept();
  else
  {
    _so->setId(-1);
    _so->setFocus();
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
  }
}

void deliverSalesOrder::sHandleSoheadid()
{
  q.prepare( "SELECT cust_soemaildelivery, cust_soediemail, cust_soediemailbody, "
             "       cust_soedisubject, cust_soedifilename, cust_soedicc "
             "FROM cohead, custinfo "
             "WHERE ( (cohead_cust_id=cust_id)"
             " AND (cohead_id=:sohead_id) );" );
  q.bindValue(":sohead_id", _so->id());
  q.exec();
  if (q.first())
  {
    if (q.value("cust_soemaildelivery").toBool())
    {
      _email->setText(q.value("cust_soediemail").toString());
      _cc->setText(q.value("cust_soedicc").toString());
      _emailBody->setText(q.value("cust_soediemailbody").toString());
      _subject->setText(q.value("cust_soedisubject").toString());
      _fileName->setText(q.value("cust_soedifilename").toString());
    }
    else
    {
      _email->clear();
      _cc->clear();
      _emailBody->clear();
      _subject->clear();
      _fileName->clear();
    }
  }
}

