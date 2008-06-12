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

#include "characteristicAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

/*
 *  Constructs a characteristicAssignment as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
characteristicAssignment::characteristicAssignment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _char->setAllowNull(TRUE);

  _listpriceLit->hide();
  _listprice->hide();

  resize(minimumSize());
}

/*
 *  Destroys the object and frees any allocated resources
 */
characteristicAssignment::~characteristicAssignment()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void characteristicAssignment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse characteristicAssignment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "I";
    handleTargetType();
  }

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "C";
    handleTargetType();
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CRMACCT";
    handleTargetType();
  }

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "ADDR";
    handleTargetType();
  }

  param = pParams.value("cntct_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CNTCT";
    handleTargetType();
  }

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "CT";
    handleTargetType();
  }

  param = pParams.value("ls_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "LS";
    handleTargetType();
  }

  param = pParams.value("lsreg_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "LSR";
    handleTargetType();
  }

  param = pParams.value("ophead_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "OPP";
    handleTargetType();
  }

  param = pParams.value("emp_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
    _targetType = "EMP";
    handleTargetType();
  }



  param = pParams.value("charass_id", &valid);
  if (valid)
  {
    _charassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _char->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _char->setEnabled(FALSE);
      _value->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("showPrices", &valid);
  if (valid)
  {
    _listpriceLit->show();
    _listprice->show();
  }

  return NoError;
}

void characteristicAssignment::sSave()
{
  if (_char->id() == -1)
  {
    QMessageBox::information( this, tr("No Characteristic Selected"),
                              tr("You must select a Characteristic before saving this Item Characteristic.") );
    _char->setFocus();
    return;
  }
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('charass_charass_id_seq') AS charass_id;");
    if (q.first())
    {
      _charassid = q.value("charass_id").toInt();

      q.prepare( "INSERT INTO charass "
                 "( charass_id, charass_target_id, charass_target_type, charass_char_id, charass_value, charass_price, charass_default ) "
                 "VALUES "
                 "( :charass_id, :charass_target_id, :charass_target_type, :charass_char_id, :charass_value, :charass_price, :charass_default );" );
    }
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE charass "
               "SET charass_char_id=:charass_char_id, charass_value=:charass_value, "
               "charass_price=:charass_price, charass_default=:charass_default "
               "WHERE (charass_id=:charass_id);" );

  q.bindValue(":charass_id", _charassid);
  q.bindValue(":charass_target_id", _targetId);
  q.bindValue(":charass_target_type", _targetType);
  q.bindValue(":charass_char_id", _char->id());
  q.bindValue(":charass_value", _value->text());
  q.bindValue(":charass_price", _listprice->toDouble());
  q.bindValue(":charass_default", QVariant(_default->isChecked(), 0));
  q.exec();

  done(_charassid);
}

void characteristicAssignment::sCheck()
{
  if ((_mode == cNew) || (_char->id() == -1))
  {
    q.prepare( "SELECT charass_id "
               "FROM charass "
               "WHERE ( (charass_target_type=:charass_target_id)"
               " AND (charass_target_id=:charass_target_id)"
               " AND (charass_char_id=:char_id) );" );
    q.bindValue(":charass_target_type", _targetType);
    q.bindValue(":charass_target_id", _targetId);
    q.bindValue(":char_id", _char->id());
    q.exec();
    if (q.first())
    {
      _charassid = q.value("charass_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

void characteristicAssignment::populate()
{
  q.prepare( "SELECT charass_target_id, charass_target_type, charass_char_id, charass_value, charass_default, charass_price "
                   "FROM charass "
                   "WHERE (charass_id=:charass_id);" );
  q.bindValue(":charass_id", _charassid);
  q.exec();
  if (q.first())
  {
    _targetId = q.value("charass_target_id").toInt();
    _targetType = q.value("charass_target_type").toString();
    handleTargetType();

    _char->setId(q.value("charass_char_id").toInt());
    _value->setText(q.value("charass_value").toString());
    _listprice->setText(formatSalesPrice(q.value("charass_price").toDouble()));
    _default->setChecked(q.value("charass_default").toBool());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristicAssignment::handleTargetType()
{
  if((_targetType != "I") && (_targetType != "CT"))
    _default->hide();

  if(_targetType != "I")
    _listprice->hide();

  QString boolColumn;
  if ((_targetType == "C") || (_targetType == "CT"))
  {
    setCaption(tr("Customer Characteristic"));
    boolColumn = "char_customers";
  }
  else if (_targetType == "I")
  {
    setCaption(tr("Item Characteristic"));
    boolColumn = "char_items";
  }
  else if (_targetType == "CNTCT")
  {
    setCaption(tr("Contact Characteristic"));
    boolColumn = "char_contacts";
  }
  else if (_targetType == "ADDR")
  {
    setCaption(tr("Address Characteristic"));
    boolColumn = "char_addresses";
  }
  else if (_targetType == "CRMACCT")
  {
    setCaption(tr("CRM Account Characteristic"));
    boolColumn = "char_crmaccounts";
  }
  else if (_targetType == "LS" || _targetType == "LSR")
  {
    setCaption(tr("Lot Serial Characteristic"));
    boolColumn = "char_lotserial";
  }
  else if (_targetType == "OPP")
  {
    setCaption(tr("Opportunity Characteristic"));
    boolColumn = "char_opportunity";
  }
  else if (_targetType == "EMP")
  {
    setCaption(tr("Employee Characteristic"));
    boolColumn = "char_employees";
  }
  _char->populate(QString("SELECT char_id, char_name, char_name "
			 "FROM char "
			 "WHERE (%1) "
			 "ORDER BY char_name; ").arg(boolColumn));
}
