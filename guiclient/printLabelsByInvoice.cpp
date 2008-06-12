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

#include "printLabelsByInvoice.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

/*
 *  Constructs a printLabelsByInvoice as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printLabelsByInvoice::printLabelsByInvoice(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_invoiceNumber, SIGNAL(lostFocus()), this, SLOT(sParseInvoiceNumber()));
    connect(_from, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printLabelsByInvoice::~printLabelsByInvoice()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printLabelsByInvoice::languageChange()
{
    retranslateUi(this);
}


void printLabelsByInvoice::init()
{
  _invcheadid = -1;

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
}

void printLabelsByInvoice::sPrint()
{
  XSqlQuery query;
  query.prepare( "SELECT report_name "
                 "FROM labelform, report "
                 "WHERE ( (labelform_id=:labelform_id) "
                 " AND (report_id=labelform_report_id) );" );
  query.bindValue(":labelform_id", _report->id());
  query.exec();
  if (query.first())
  {
    ParameterList params;
    params.append("invchead_id", _invcheadid);
    params.append("labelFrom", _from->value());
    params.append("labelTo", _to->value());

    orReport report(query.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _invoiceNumber->clear();
    _invoiceNumber->setFocus();
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("Could not locate the report definition the form \"%1\"")
                         .arg(_report->currentText()) );
}

void printLabelsByInvoice::sParseInvoiceNumber()
{
  XSqlQuery query;
  query.prepare( "SELECT invchead_id "
                 "FROM invchead "
                 "WHERE (invchead_invcnumber=:invoiceNumber);" );
  query.bindValue(":invoiceNumber", _invoiceNumber->text().toInt());
  query.exec();
  if (query.first())
  {
    _print->setEnabled(TRUE);

    _invcheadid = query.value("invchead_id").toInt();
  }
  else
  {
    _print->setEnabled(FALSE);
    _invoiceNumber->clear();
    _invoiceNumber->setFocus();

    _invcheadid = -1;
  }
}

void printLabelsByInvoice::sSetToMin(int pValue)
{
  _to->setMinValue(pValue);
}
