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

#include "vendorType.h"

#include <qvariant.h>
#include <qmessagebox.h>
/*
 *  Constructs a vendorType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
vendorType::vendorType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
vendorType::~vendorType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void vendorType::languageChange()
{
    retranslateUi(this);
}


void vendorType::init()
{
}

enum SetResponse vendorType::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vendtype_id", &valid);
  if (valid)
  {
    _vendtypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void vendorType::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT vendtype_id "
               "FROM vendtype "
               "WHERE (UPPER(vendtype_code)=UPPER(:venttype_code));" );
    q.bindValue(":vendtype_code", _code->text());
    q.exec();
    if (q.first())
    {
      _vendtypeid = q.value("vendtype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void vendorType::sSave()
{
  if (_code->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Vendor Type Code"),
                              tr("You must enter a valid Code for this Vendor Type before creating it.")  );
    _code->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('vendtype_vendtype_id_seq') AS _vendtype_id;");
    if (q.first())
      _vendtypeid = q.value("_vendtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO vendtype "
               "(vendtype_id, vendtype_code, vendtype_descrip) "
               "VALUES "
               "(:vendtype_id, :vendtype_code, :vendtype_descrip);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE vendtype "
               "SET vendtype_code=:vendtype_code,"
               "    vendtype_descrip=:vendtype_descrip "
               "WHERE (vendtype_id=:vendtype_id);" );

  q.bindValue(":vendtype_id", _vendtypeid);
  q.bindValue(":vendtype_code", _code->text().stripWhiteSpace());
  q.bindValue(":vendtype_descrip", _description->text().stripWhiteSpace());
  q.exec();

  done(_vendtypeid);
}

void vendorType::populate()
{
  q.prepare( "SELECT vendtype_code, vendtype_descrip "
             "FROM vendtype "
             "WHERE (vendtype_id=:vendtype_id);" );
  q.bindValue(":vendtype_id", _vendtypeid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("vendtype_code"));
    _description->setText(q.value("vendtype_descrip"));
  }
}
