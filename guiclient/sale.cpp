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

#include "sale.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a sale as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
sale::sale(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
sale::~sale()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void sale::languageChange()
{
    retranslateUi(this);
}


void sale::init()
{
  _ipshead->populate( QString( "SELECT ipshead_id, ipshead_name "
                               "FROM ipshead "
                               "ORDER BY ipshead_name;" ) );
}

enum SetResponse sale::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("sale_id", &valid);
  if (valid)
  {
    _saleid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _startDate->setEnabled(FALSE);
      _endDate->setEnabled(FALSE);
      _ipshead->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void sale::sSave()
{
  _name->setText(_name->text().stripWhiteSpace());

  if (_name->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Sale Name"),
                           tr("You must enter a name for this Sale before saving it.") );
    _name->setFocus();
    return;
  }

  if (!_startDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a start date for this Sale before saving it.") );
    _startDate->setFocus();
    return;
  }

  if (!_endDate->isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a end date for this Sale before saving it.") );
    _endDate->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('sale_sale_id_seq') AS _sale_id;");
    if (q.first())
      _saleid = q.value("_sale_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO sale "
               "( sale_id, sale_name, sale_descrip,"
               "  sale_ipshead_id, sale_startdate, sale_enddate ) "
               "VALUES "
               "( :sale_id, :sale_name, :sale_descrip,"
               "  :sale_ipshead_id, :sale_startdate, :sale_enddate );" );
  }
  else
    q.prepare( "UPDATE sale "
               "SET sale_name=:sale_name, sale_descrip=:sale_descrip,"
               "    sale_ipshead_id=:sale_ipshead_id,"
               "    sale_startdate=:sale_startdate, sale_enddate=:sale_enddate "
               "WHERE (sale_id=:sale_id);" );

  q.bindValue(":sale_id", _saleid);
  q.bindValue(":sale_name", _name->text());
  q.bindValue(":sale_descrip", _description->text());
  q.bindValue(":sale_ipshead_id", _ipshead->id());
  q.bindValue(":sale_startdate", _startDate->date());
  q.bindValue(":sale_enddate", _endDate->date());
  q.exec();

  done(_saleid);
}

void sale::populate()
{
  q.prepare( "SELECT sale_name, sale_descrip, sale_ipshead_id,"
             "       sale_startdate, sale_enddate "
             "FROM sale "
             "WHERE (sale_id=:sale_id);" );
  q.bindValue(":sale_id", _saleid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("sale_name").toString());
    _description->setText(q.value("sale_descrip").toString());
    _startDate->setDate(q.value("sale_startdate").toDate());
    _endDate->setDate(q.value("sale_enddate").toDate());
    _ipshead->setId(q.value("sale_ipshead_id").toInt());
  }
}

