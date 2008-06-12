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

#include "productCategory.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a productCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
productCategory::productCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_category, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
productCategory::~productCategory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void productCategory::languageChange()
{
    retranslateUi(this);
}


void productCategory::init()
{
}

enum SetResponse productCategory::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("prodcat_id", &valid);
  if (valid)
  {
    _prodcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void productCategory::sCheck()
{
  _category->setText(_category->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_category->text().length()) )
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (UPPER(prodcat_code)=UPPER(:prodcat_code));" );
    q.bindValue(":prodcat_code", _category->text());
    q.exec();
    if (q.first())
    {
      _prodcatid = q.value("prodcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
  }
}

void productCategory::sSave()
{
  if (_category->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Category"),
			  tr("You must name this Category before saving it."));
    _category->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE ( (prodcat_id<>:prodcat_id)"
               " AND (prodcat_code=:prodcat_code) );");
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists."
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    q.prepare( "UPDATE prodcat "
               "SET prodcat_code=:prodcat_code, prodcat_descrip=:prodcat_descrip "
               "WHERE (prodcat_id=:prodcat_id);" );
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text().upper());
    q.bindValue(":prodcat_descrip", _description->text());
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT prodcat_id "
               "FROM prodcat "
               "WHERE (prodcat_code=:prodcat_code);");
    q.bindValue(":prodcat_code", _category->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Product Category"),
                             tr( "A Product Category with the entered code already exists.\n"
                                 "You may not create a Product Category with this code." ) );
      _category->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('prodcat_prodcat_id_seq') AS prodcat_id;");
    if (q.first())
      _prodcatid = q.value("prodcat_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO prodcat "
               "( prodcat_id, prodcat_code, prodcat_descrip ) "
               "VALUES "
               "( :prodcat_id, :prodcat_code, :prodcat_descrip );" );
    q.bindValue(":prodcat_id", _prodcatid);
    q.bindValue(":prodcat_code", _category->text().upper());
    q.bindValue(":prodcat_descrip", _description->text());
    q.exec();
  }

  done(_prodcatid);
}

void productCategory::populate()
{
  q.prepare( "SELECT prodcat_code, prodcat_descrip "
             "FROM prodcat "
             "WHERE (prodcat_id=:prodcat_id);" );
  q.bindValue(":prodcat_id", _prodcatid);
  q.exec();
  if (q.first())
  {
    _category->setText(q.value("prodcat_code").toString());
    _description->setText(q.value("prodcat_descrip").toString());
  }
}

