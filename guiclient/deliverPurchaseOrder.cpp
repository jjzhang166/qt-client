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

#include "deliverPurchaseOrder.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a deliverPurchaseOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
deliverPurchaseOrder::deliverPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_po, SIGNAL(newId(int)), this, SLOT(sHandlePoheadid(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
deliverPurchaseOrder::~deliverPurchaseOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void deliverPurchaseOrder::languageChange()
{
    retranslateUi(this);
}


void deliverPurchaseOrder::init()
{
  _captive = FALSE;
  
  _po->setType(cPOUnposted | cPOOpen);

  q.exec( "SELECT usr_email "
          "FROM usr "
          "WHERE (usr_username=CURRENT_USER);" );
  if (q.first())
    _fromEmail->setText(q.value("usr_email"));
//  ToDo
}

enum SetResponse deliverPurchaseOrder::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _submit->setFocus();
  }

  return NoError;
}

void deliverPurchaseOrder::sSubmit()
{
  if (_email->text().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Queue P/O for Delivery"),
                           tr("You must enter a email address to which this P/O is to be delivered.") );
    _email->setFocus();
    return;
  }

  q.prepare( "SELECT submitReportToBatch( 'PurchaseOrder', :fromEmail, :emailAddress, :ccAddress, :subject,"
             "                            :emailBody, :fileName, CURRENT_TIMESTAMP) AS batch_id;" );
  q.bindValue(":fromEmail", _fromEmail->text());
  q.bindValue(":emailAddress", _email->text());
  q.bindValue(":ccAddress", _cc->text());
  q.bindValue(":subject", _subject->text().replace("</docnumber>", _po->poNumber()).replace("</doctype>", "PO"));
  q.bindValue(":fileName", _fileName->text().replace("</docnumber>", _po->poNumber()).replace("</doctype>", "PO"));
  q.bindValue(":emailBody", _emailBody->text().replace("</docnumber>", _po->poNumber()).replace("</doctype>", "PO"));
  q.exec();
  if (q.first())
  {
    int batchid = q.value("batch_id").toInt();

    q.prepare( "INSERT INTO batchparam "
               "( batchparam_batch_id, batchparam_order,"
               "  batchparam_name, batchparam_value ) "
               "VALUES "
               "( :batchparam_batch_id, :batchparam_order,"
               "  :batchparam_name, :batchparam_value );" );

    q.bindValue(":batchparam_batch_id", batchid);
    q.bindValue(":batchparam_order", 1);
    q.bindValue(":batchparam_name", "pohead_id");
    q.bindValue(":batchparam_value", _po->id());
    q.exec();

    q.bindValue(":batchparam_batch_id", batchid);
    q.bindValue(":batchparam_order", 2);
    q.bindValue(":batchparam_name", "title");
    q.bindValue(":batchparam_value", "Emailed Vendor Copy");
    q.exec();
  }

  if (_markPrinted->isChecked())
  {
    q.prepare( "UPDATE pohead "
               "SET pohead_printed=TRUE "
               "WHERE (pohead_id=:pohead_id);" );
    q.bindValue(":pohead_id", _po->id());
    q.exec();
  }

  if (_captive)
    accept();
  else
  {
    _submit->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _email->clear();
    _cc->clear();
    _emailBody->clear();
    _subject->clear();
    _fileName->clear();
    _po->setId(-1);
    _po->setFocus();
  }
}

void deliverPurchaseOrder::sHandlePoheadid(int pPoheadid)
{
  q.prepare( "SELECT vend_emailpodelivery, vend_ediemail, vend_ediemailbody, "
             "       vend_edisubject, vend_edifilename, vend_edicc "
             "FROM pohead, vend "
             "WHERE ( (pohead_vend_id=vend_id)"
             " AND (pohead_id=:pohead_id) );" );
  q.bindValue(":pohead_id", pPoheadid);
  q.exec();
  if (q.first())
  {
    if (q.value("vend_emailpodelivery").toBool())
    {
      _submit->setEnabled(TRUE);
      _email->setText(q.value("vend_ediemail"));
      _cc->setText(q.value("vend_edicc").toString());
      _emailBody->setText(q.value("vend_ediemailbody").toString());
      _subject->setText(q.value("vend_edisubject"));
      _fileName->setText(q.value("vend_edifilename"));
    }
    else
    {
      _submit->setEnabled(FALSE);
      _email->clear();
      _cc->clear();
      _emailBody->clear();
      _subject->clear();
      _fileName->clear();
    }
  }
}
