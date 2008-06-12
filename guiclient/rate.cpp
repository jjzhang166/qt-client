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

#include "rate.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>

/*
 *  Constructs a rate as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rate::rate(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
rate::~rate()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rate::languageChange()
{
    retranslateUi(this);
}


void rate::init()
{
  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _price->setValidator(omfgThis->moneyVal());
}

enum SetResponse rate::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("destination_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _destination->setId(param.toInt());
  }

  param = pParams.value("carrier_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _carrier->setId(param.toInt());
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _destination->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _destination->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _destination->setEnabled(FALSE);
      _carrier->setEnabled(FALSE);
      _price->setEnabled(FALSE);
      _stops->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void rate::sSave()
{
  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('rate_rate_id_seq') AS _rate_id");
    if (q.first())
      _rateid = q.value("_rate_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO rate "
               "( rate_id, rate_destination_id, rate_carrier_id,"
               "  rate_rank,"
               "  rate_stops, rate_price,"
               "  rate_effective, rate_expires ) "
               "SELECT :rate_id, :rate_destination_id, :rate_carrier_id,"
               "       ( ( SELECT COALESCE(MAX(rate_rank), 0)"
               "           FROM rate"
               "           WHERE rate_destination_id=:rate_destination_id) + 1),"
               "       :rate_stops, :rate_price,"
               "       :rate_effective, :rate_expires );" );
  }

  else if (_mode == cEdit)
    q.prepare( "UPDATE rate "
               "SET rate_destination_id=:rate_destination_id, rate_carrier_id=:rate_carrier_id,"
               "    rate_stops=:rate_stops, rate_price=:rate_price,"
               "    rate_effective=:rate_effective, rate_expires=:rate_expires "
               "WHERE (rate_id=:rate_id);" );

  q.bindValue(":rate_id", _rateid);
  q.bindValue(":rate_destination_id", _destination->id());
  q.bindValue(":rate_carrier_id", _carrier->id());
  q.bindValue(":rate_stops", _stops->value());
  q.bindValue(":rate_price", _price->toDouble());
  q.bindValue(":rate_effective", _dates->startDate());
  q.bindValue(":rate_expires", _dates->endDate());
  q.exec();

  done(_rateid);
}

void rate::populate()
{
  q.prepare( "SELECT rate_destination_id, rate_carrier_id,"
             "       rate_stops, formatMoney(rate_price) AS price, rate_comments,"
             "       rate_effective, rate_expires "
             "FROM rate "
             "WHERE (rate_id=:rate_id);" );
  q.bindValue(":rate_id", _rateid);
  q.exec();
  if (q.first())
  {
    _destination->setId(q.value("rate_destination_id").toInt());
    _carrier->setId(q.value("rate_carrier_id").toInt());
    _stops->setValue(q.value("rate_stops").toInt());
    _price->setText(q.value("price").toString());
    _dates->setStartDate(q.value("rate_effective").toDate());
    _dates->setEndDate(q.value("rate_expires").toDate());
  }
}

