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

#include "apAccountAssignment.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a apAccountAssignment as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
apAccountAssignment::apAccountAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_selectedVendorType, SIGNAL(toggled(bool)), _vendorTypes, SLOT(setEnabled(bool)));
    connect(_vendorTypePattern, SIGNAL(toggled(bool)), _vendorType, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
apAccountAssignment::~apAccountAssignment()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void apAccountAssignment::languageChange()
{
    retranslateUi(this);
}


void apAccountAssignment::init()
{
  _vendorTypes->setType(XComboBox::VendorTypes);
}

enum SetResponse apAccountAssignment::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("apaccnt_id", &valid);
  if (valid)
  {
    _apaccntid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _vendorType->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vendorTypeGroup->setEnabled(FALSE);
      _ap->setReadOnly(TRUE);
      _prepaid->setReadOnly(TRUE);
      _discount->setReadOnly(TRUE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void apAccountAssignment::sSave()
{
  if (!_ap->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select an A/P Account before saving this A/P Account Assignment") );
    _ap->setFocus();
    return;
  }

  if (!_prepaid->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select a Prepaid Account before saving this A/P Account Assignment") );
    _prepaid->setFocus();
    return;
  }

  if (!_discount->isValid())
  {
    QMessageBox::warning( this, tr("Cannot Save A/P Account Assignment"),
                          tr("You must select a Discount Account before saving this A/P Account Assignment") );
    _discount->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT apaccnt_id "
               "FROM apaccnt "
               "WHERE ( (apaccnt_vendtype_id=:apaccnt_vendtype_id)"
               " AND (apaccnt_vendtype=:apaccnt_vendtype) );" );

    if (_allVendorTypes->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", -1);
      q.bindValue(":apaccnt_vendtype", ".*");
    }
    else if (_selectedVendorType->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
      q.bindValue(":apaccnt_vendtype", "");
    }
    else if (_vendorTypePattern->isChecked())
    {
      q.bindValue(":apaccnt_vendtype_id", -1);
      q.bindValue(":apaccnt_vendtype", _vendorType->text().stripWhiteSpace());
    }

    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Create A/P Account Assignment"),
                             tr("You may not save this A/P Account Assignment as it already exists.") );
      return;
    }

    q.exec("SELECT NEXTVAL('apaccnt_apaccnt_id_seq') AS _apaccnt_id;");
    if (q.first())
      _apaccntid = q.value("_apaccnt_id").toInt();
//  ToDo

    q.prepare( "INSERT INTO apaccnt "
               "( apaccnt_id, apaccnt_vendtype_id, apaccnt_vendtype,"
               "  apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
               "  apaccnt_discount_accnt_id ) "
               "VALUES "
               "( :apaccnt_id, :apaccnt_vendtype_id, :apaccnt_vendtype,"
               "  :apaccnt_ap_accnt_id, :apaccnt_prepaid_accnt_id,"
               "  :apaccnt_discount_accnt_id ) " );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE apaccnt "
               "SET apaccnt_vendtype_id=:apaccnt_vendtype_id,"
               "    apaccnt_vendtype=:apaccnt_vendtype,"
               "    apaccnt_ap_accnt_id=:apaccnt_ap_accnt_id,"
               "    apaccnt_prepaid_accnt_id=:apaccnt_prepaid_accnt_id,"
               "    apaccnt_discount_accnt_id=:apaccnt_discount_accnt_id "
               "WHERE (apaccnt_id=:apaccnt_id);" );

  q.bindValue(":apaccnt_id", _apaccntid);
  q.bindValue(":apaccnt_ap_accnt_id", _ap->id());
  q.bindValue(":apaccnt_prepaid_accnt_id", _prepaid->id());
  q.bindValue(":apaccnt_discount_accnt_id", _discount->id());

  if (_allVendorTypes->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", -1);
    q.bindValue(":apaccnt_vendtype", ".*");
  }
  else if (_selectedVendorType->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", _vendorTypes->id());
    q.bindValue(":apaccnt_vendtype", "");
  }
  else if (_vendorTypePattern->isChecked())
  {
    q.bindValue(":apaccnt_vendtype_id", -1);
    q.bindValue(":apaccnt_vendtype", _vendorType->text().stripWhiteSpace());
  }

  q.exec();

  done(_apaccntid);
}

void apAccountAssignment::populate()
{
  q.prepare( "SELECT apaccnt_vendtype_id, apaccnt_vendtype,"
             "       apaccnt_ap_accnt_id, apaccnt_prepaid_accnt_id,"
             "       apaccnt_discount_accnt_id "
             "FROM apaccnt "
             "WHERE (apaccnt_id=:apaccnt_id);" );
  q.bindValue(":apaccnt_id", _apaccntid);
  q.exec();
  if (q.first())
  {
    if (q.value("apaccnt_vendtype_id").toInt() == -1)
    {
      if (q.value("apaccnt_vendtype").toString() == ".*")
        _allVendorTypes->setChecked(TRUE);
      else
      {
        _vendorTypePattern->setChecked(TRUE);
        _vendorType->setText(q.value("apaccnt_vendtype").toString());
      }
    }
    else
    {
      _selectedVendorType->setChecked(TRUE);
      _vendorTypes->setId(q.value("apaccnt_vendtype_id").toInt());
    }

    _ap->setId(q.value("apaccnt_ap_accnt_id").toInt());
    _prepaid->setId(q.value("apaccnt_prepaid_accnt_id").toInt());
    _discount->setId(q.value("apaccnt_discount_accnt_id").toInt());
  }
}


