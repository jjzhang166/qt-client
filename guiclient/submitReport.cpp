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

#include "submitReport.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <xvariant.h>

/*
 *  Constructs a submitReport as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
submitReport::submitReport(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_scheduled, SIGNAL(toggled(bool)), _time, SLOT(setEnabled(bool)));
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_scheduled, SIGNAL(toggled(bool)), _date, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
submitReport::~submitReport()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void submitReport::languageChange()
{
    retranslateUi(this);
}


void submitReport::init()
{
  _error = 0;

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
}

enum SetResponse submitReport::set(ParameterList &pParams)
{
  _params = pParams;

  QVariant param;
  bool     valid;

  param = pParams.value("report_name", &valid);
  if (valid)
  {
    q.prepare( "SELECT report_name "
               "FROM report "
               "WHERE (report_name=:report_name);" );
    q.bindValue(":report_name", param.toString());
    q.exec();
    if (q.first())
    {
      _documentName->setText(q.value("report_name").toString());
      _cachedReportName = q.value("report_name").toString();
    }
    else
      _error = cNoReportDefinition;
  }

  param = pParams.value("report_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT report_name "
               "FROM report "
               "WHERE (report_id=:report_id);" );
    q.bindValue(":report_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _documentName->setText(q.value("report_name").toString());
      _cachedReportName = q.value("report_name").toString();
    }
    else
      _error = cNoReportDefinition;
  }

  param = pParams.value("responseEmail", &valid);
  if (valid)
    _email->setText(param.toString());
  else
  {
    q.prepare( "SELECT usr_email "
               "FROM report, usr "
               "WHERE (usr_username=CURRENT_USER);" );
    q.exec();
    if (q.first())
      _email->setText(q.value("usr_email").toString());
  }
  
  param = pParams.value("responseBody", &valid);
  if (valid)
    _responseBody->setText(param.toString());

  return NoError;
}

int submitReport::check()
{
  return _error;
}

void submitReport::sSubmit()
{
  if (_email->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Submit Report"),
                           tr("You must indicate an Email address to which the completed Report will be sent.") );
    _email->setFocus();
    return;
  }

  if (_asap->isChecked())
    q.prepare("SELECT submitReportToBatch(:reportName, :fromEmail, :emailAddress, '', :responseBody, '', CURRENT_TIMESTAMP) AS batch_id;");
  else
  {
    q.prepare("SELECT submitReportToBatch(:reportName, :fromEmail, :emailAddress, '', :responseBody, '', :scheduled) AS batch_id;");

    QDateTime scheduled;
    scheduled.setDate(_date->date());
    scheduled.setTime(_time->time());
    q.bindValue(":scheduled", scheduled);
  }

  q.bindValue(":reportName", _cachedReportName);
  q.bindValue(":fromEmail", _fromEmail->text());
  q.bindValue(":emailAddress", _email->text());
  q.bindValue(":responseBody", _responseBody->text());
  q.exec();
  if (q.first())
  {
    int batchid = q.value("batch_id").toInt();

    q.prepare( "INSERT INTO batchparam "
               "( batchparam_batch_id, batchparam_order,"
               "  batchparam_name, batchparam_type, batchparam_value ) "
               "VALUES "
               "( :batchparam_batch_id, :batchparam_order,"
               "  :batchparam_name, :batchparam_type, :batchparam_value );" );
    for (int counter = 0; counter < _params.count(); counter++)
    {
      q.bindValue(":batchparam_batch_id", batchid);
      q.bindValue(":batchparam_order", (counter + 1));
      q.bindValue(":batchparam_name", _params.name(counter));
      QVariant v = _params.value(counter);
      q.bindValue(":batchparam_type", QVariant::typeToName(v.type()));
      q.bindValue(":batchparam_value", XVariant::encode(v));
      q.exec();
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );

  accept();
}

