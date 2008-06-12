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

#include "uomConv.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a uomConv as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
uomConv::uomConv(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _uomidFrom = -1;
  _uomconvid = -1;
  _ignoreSignals = false;

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_uomFrom, SIGNAL(currentIndexChanged(int)), this, SLOT(sFromChanged()));
  connect(_uomTo, SIGNAL(currentIndexChanged(int)), this, SLOT(sToChanged()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));

  _uomFrom->setType(XComboBox::UOMs);
  _uomTo->setType(XComboBox::UOMs);
}

/*
 *  Destroys the object and frees any allocated resources
 */
uomConv::~uomConv()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void uomConv::languageChange()
{
  retranslateUi(this);
}

enum SetResponse uomConv::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("from_uom_id", &valid);
  if (valid)
  {
    _uomidFrom = param.toInt();
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }

  param = pParams.value("uomconv_id", &valid);
  if (valid)
  {
    _uomconvid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _fromValue->setEnabled(false);
      _toValue->setEnabled(false);
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      _fractional->setEnabled(false);
      _cancel->setText(tr("&Close"));
      _save->hide();

      _cancel->setFocus();
    }
  }

  return NoError;
}

void uomConv::sSave()
{
  bool valid;
  if (_fromValue->toDouble(&valid) == 0)
  {
    QMessageBox::information( this, tr("No Ratio Entered"),
                              tr("You must enter a valid Ratio before saving this UOM Conversion.") );
    _fromValue->setFocus();
    return;
  }

  if (_toValue->toDouble(&valid) == 0)
  {
    QMessageBox::information( this, tr("No Ratio Entered"),
                              tr("You must enter a valid Ratio before saving this UOM Conversion.") );
    _toValue->setFocus();
    return;
  }

  if (_mode == cEdit)
    q.prepare( "UPDATE uomconv "
               "   SET uomconv_from_value=:uomconv_from_value,"
               "       uomconv_to_value=:uomconv_to_value,"
               "       uomconv_fractional=:uomconv_fractional "
               " WHERE(uomconv_id=:uomconv_id);" );
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('uomconv_uomconv_id_seq') AS uomconv_id");
    if (q.first())
      _uomconvid = q.value("uomconv_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO uomconv "
               "( uomconv_id, uomconv_from_uom_id, uomconv_from_value, uomconv_to_uom_id, uomconv_to_value, uomconv_fractional ) "
               "VALUES "
               "( :uomconv_id, :uomconv_from_uom_id, :uomconv_from_value, :uomconv_to_uom_id, :uomconv_to_value, :uomconv_fractional );" );
  }

  q.bindValue(":uomconv_id", _uomconvid);
  q.bindValue(":uomconv_from_uom_id", _uomFrom->id());
  q.bindValue(":uomconv_to_uom_id", _uomTo->id());
  q.bindValue(":uomconv_from_value", _fromValue->toDouble());
  q.bindValue(":uomconv_to_value", _toValue->toDouble());
  q.bindValue(":uomconv_fractional", QVariant(_fractional->isChecked(), 0));
  q.exec();

  accept();
}

void uomConv::sFromChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomTo->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sToChanged()
{
  if(cNew != _mode || _ignoreSignals)
    return;

  if(_uomFrom->id() != _uomidFrom && _uomTo->id() != _uomidFrom)
  {
    _ignoreSignals = true;
    _uomFrom->setId(_uomidFrom);
    _ignoreSignals = false;
  }
  sCheck();
}

void uomConv::sCheck()
{
  if ( (_mode == cNew) )
  {
    q.prepare( "SELECT uomconv_id"
               "  FROM uomconv"
               " WHERE((uomconv_from_uom_id=:from AND uomconv_to_uom_id=:to)"
               "    OR (uomconv_from_uom_id=:to AND uomconv_to_uom_id=:from));" );
    q.bindValue(":from", _uomFrom->id());
    q.bindValue(":to", _uomTo->id());
    q.exec();
    if (q.first())
    {
      _uomconvid = q.value("uomconv_id").toInt();
      _mode = cEdit;
      _uomTo->setEnabled(false);
      _uomFrom->setEnabled(false);
      populate();
    }
  }
}

void uomConv::populate()
{
  q.prepare( "SELECT uomconv_from_uom_id,"
             "       uomconv_from_value,"
             "       uomconv_to_uom_id,"
             "       uomconv_to_value,"
             "       uomconv_fractional "
             "  FROM uomconv"
             " WHERE(uomconv_id=:uomconv_id);" );
  q.bindValue(":uomconv_id", _uomconvid);
  q.exec();
  if (q.first())
  {
    _uomFrom->setId(q.value("uomconv_from_uom_id").toInt());
    _uomTo->setId(q.value("uomconv_to_uom_id").toInt());
    _fromValue->setText(q.value("uomconv_from_value").toString());
    _toValue->setText(q.value("uomconv_to_value").toString());
    _fractional->setChecked(q.value("uomconv_fractional").toBool());
  }
}

