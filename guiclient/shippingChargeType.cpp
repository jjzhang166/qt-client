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

#include "shippingChargeType.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a shippingChargeType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
shippingChargeType::shippingChargeType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shippingChargeType::~shippingChargeType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shippingChargeType::languageChange()
{
    retranslateUi(this);
}


void shippingChargeType::init()
{
}

enum SetResponse shippingChargeType::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("shipchrg_id", &valid);
  if (valid)
  {
    _shipchrgid = param.toInt();
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
      _customerFreight->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void shippingChargeType::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().stripWhiteSpace().length()))
  {
    q.prepare( "SELECT shipchrg_id "
               "FROM shipchrg "
               "WHERE (UPPER(shipchrg_name)=UPPER(:shipchrg_name));" );
    q.bindValue(":shipchrg_name", _name->text());
    q.exec();
    if (q.first())
    {
      _shipchrgid = q.value("shipchrg_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void shippingChargeType::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipchrg_shipchrg_id_seq') AS shipchrg_id;");
    if (q.first())
      _shipchrgid = q.value("shipchrg_id").toInt();

    q.prepare( "INSERT INTO shipchrg "
               "(shipchrg_id, shipchrg_name, shipchrg_descrip, shipchrg_custfreight) "
               "VALUES "
               "(:shipchrg_id, :shipchrg_name, :shipchrg_descrip, :shipchrg_custfreight);" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT shipchrg_id "
               "FROM shipchrg "
               "WHERE ( (shipchrg_id<>:shipchrg_id)"
               " AND (UPPER(shipchrg_name)=UPPER(:shipchrg_name)) );" );
    q.bindValue(":shipchrg_id", _shipchrgid);
    q.bindValue(":shipchrg_name", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Shipping Charge Type"),
                             tr( "The new Shipping Charge Type information cannot be saved as the new Shipping Charge Type that you\n"
                                 "entered conflicts with an existing Shipping Charge Type.  You must uniquely name this Shipping Charge Type\n"
                                 "before you may save it." ) );
      return;
    }

    q.prepare( "UPDATE shipchrg "
               "SET shipchrg_name=:shipchrg_name, shipchrg_descrip=:shipchrg_descrip,"
               "    shipchrg_custfreight=:shipchrg_custfreight "
               "WHERE (shipchrg_id=:shipchrg_id);" );
  }

  q.bindValue(":shipchrg_id", _shipchrgid);
  q.bindValue(":shipchrg_name", _name->text().stripWhiteSpace());
  q.bindValue(":shipchrg_descrip", _description->text().stripWhiteSpace());
  q.bindValue(":shipchrg_custfreight", QVariant(_customerFreight->isChecked(), 0));
  q.exec();

  done(_shipchrgid);
}

void shippingChargeType::populate()
{
  q.prepare( "SELECT shipchrg_name, shipchrg_descrip, shipchrg_custfreight "
             "FROM shipchrg "
             "WHERE (shipchrg_id=:shipchrg_id);" );
  q.bindValue(":shipchrg_id", _shipchrgid);
  q.exec();
  if (q.first()) 
  {
    _name->setText(q.value("shipchrg_name").toString());
    _description->setText(q.value("shipchrg_descrip").toString());
    _customerFreight->setChecked(q.value("shipchrg_custfreight").toBool());
  }
}
