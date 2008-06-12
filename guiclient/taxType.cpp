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

#include "taxType.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a taxType as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
taxType::taxType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxType::~taxType()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxType::languageChange()
{
    retranslateUi(this);
}

enum SetResponse taxType::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("taxtype_id", &valid);
  if (valid)
  {
    _taxtypeid = param.toInt();
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
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void taxType::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));" );
    q.bindValue(":taxtype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _taxtypeid = q.value("taxtype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void taxType::sSave()
{
  if (_name->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical(this, tr("Missing Name"),
			  tr("<p>You must name this Tax Type before saving it."));
    _name->setFocus();
    return;
  }

  if (_mode == cEdit)
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE ( (taxtype_id<>:taxtype_id)"
               " AND (UPPER(taxtype_name)=UPPER(:taxtype_name)) );");
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists."
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    q.prepare( "UPDATE taxtype "
               "SET taxtype_name=:taxtype_name,"
               "    taxtype_descrip=:taxtype_descrip "
               "WHERE (taxtype_id=:taxtype_id);" );
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().stripWhiteSpace());
    q.bindValue(":taxtype_descrip", _description->text());
    q.exec();
  }
  else if (_mode == cNew)
  {
    q.prepare( "SELECT taxtype_id "
               "FROM taxtype "
               "WHERE (UPPER(taxtype_name)=UPPER(:taxtype_name));");
    q.bindValue(":taxtype_name", _name->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create Tax Type"),
                             tr( "A Tax Type with the entered name already exists.\n"
                                 "You may not create a Tax Type with this name." ) );
      _name->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('taxtype_taxtype_id_seq') AS taxtype_id;");
    if (q.first())
      _taxtypeid = q.value("taxtype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO taxtype "
               "( taxtype_id, taxtype_name, taxtype_descrip ) "
               "VALUES "
               "( :taxtype_id, :taxtype_name, :taxtype_descrip );" );
    q.bindValue(":taxtype_id", _taxtypeid);
    q.bindValue(":taxtype_name", _name->text().stripWhiteSpace());
    q.bindValue(":taxtype_descrip", _description->text());
    q.exec();
  }

  done(_taxtypeid);
}

void taxType::populate()
{
  q.prepare( "SELECT taxtype_name, taxtype_descrip, taxtype_sys "
             "FROM taxtype "
             "WHERE (taxtype_id=:taxtype_id);" );
  q.bindValue(":taxtype_id", _taxtypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("taxtype_name").toString());
    if(q.value("taxtype_sys").toBool())
      _name->setEnabled(false);
    _description->setText(q.value("taxtype_descrip").toString());
  }
}

