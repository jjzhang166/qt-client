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

#include "userCostingElement.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a userCostingElement as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
userCostingElement::userCostingElement(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_name, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_acceptPO, SIGNAL(toggled(bool)), _useCostItem, SLOT(setDisabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
userCostingElement::~userCostingElement()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void userCostingElement::languageChange()
{
    retranslateUi(this);
}


void userCostingElement::init()
{
  _item->setType(ItemLineEdit::cCosting);
}

enum SetResponse userCostingElement::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("costelem_id", &valid);
  if (valid)
  {
    _costelemid = param.toInt();
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
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _acceptPO->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
      _close->setFocus();
    }
  }

  return NoError;
}

void userCostingElement::sSave()
{
  if (_mode == cNew)
  {
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE (costelem_type=:costelem_type);" );
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "A Costing Elements with the entered code already exists.\n"
                                 "You may not create a Costing Element with this code." ) );
      _name->setFocus();
      return;
    }

    q.exec("SELECT NEXTVAL('costelem_costelem_id_seq') AS _costelem_id");
    if (q.first())
      _costelemid = q.value("_costelem_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO costelem "
               "( costelem_id, costelem_type, costelem_active,"
               "  costelem_sys, costelem_po, costelem_cost_item_id ) "
               "VALUES "
               "( :costelem_id, :costelem_type, :costelem_active,"
               "  FALSE, :costelem_po, :costelem_cost_item_id );" );
  }
  else if (_mode == cEdit)
  {
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE ( (costelem_id <> :costelem_id)"
               "AND (costelem_type=:costelem_type) );" );
    q.bindValue(":costelem_id", _costelemid);
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      QMessageBox::critical( this, tr("Cannot Save Costing Element"),
                             tr( "A Costing Elements with the entered code already exists.\n"
                                 "You may not create a Costing Element with this code." ) );
      _name->setFocus();
      return;
    }

    q.prepare( "UPDATE costelem "
               "SET costelem_type=:costelem_type,"
               "    costelem_active=:costelem_active, costelem_po=:costelem_po,"
               "    costelem_cost_item_id=:costelem_cost_item_id "
               "WHERE (costelem_id=:costelem_id);" );
  }

  q.bindValue(":costelem_id", _costelemid);
  q.bindValue(":costelem_type", _name->text());
  q.bindValue(":costelem_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":costelem_po", QVariant(_acceptPO->isChecked(), 0));

  if (_useCostItem->isChecked())
    q.bindValue(":costelem_cost_item_id", _item->id());
  else
    q.bindValue(":costelem_cost_item_id", -1);

  q.exec();

  done(_costelemid);
}

void userCostingElement::populate()
{
  q.prepare( "SELECT costelem_type, costelem_active,"
             "       costelem_po, costelem_cost_item_id "
             "FROM costelem "
             "WHERE (costelem_id=:costelem_id);" );
  q.bindValue(":costelem_id", _costelemid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("costelem_type").toString());
    _active->setChecked(q.value("costelem_active").toBool());

    if (q.value("costelem_po").toBool())
      _acceptPO->setChecked(TRUE);
    else
    {
      if (q.value("costelem_cost_item_id").toInt() != -1)
      {
        _useCostItem->setChecked(TRUE);
        _item->setId(q.value("costelem_cost_item_id").toInt());
      }
    }
  }
}

void userCostingElement::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ((_mode == cNew) && (_name->text().length()))
  {
    q.prepare( "SELECT costelem_id "
               "FROM costelem "
               "WHERE (UPPER(costelem_type)= UPPER(:costelem_type));" );
    q.bindValue(":costelem_type", _name->text());
    q.exec();
    if (q.first())
    {
      _costelemid = q.value("costelem_id").toInt();
      _mode = cEdit;
      populate();
    }
  }
}

