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

#include "workCenter.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include <qstatusbar.h>
/*
 *  Constructs a workCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
workCenter::workCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    QButtonGroup* _runRateGroupInt = new QButtonGroup(this);
    _runRateGroupInt->addButton(_runSpecifyRate);
    _runRateGroupInt->addButton(_runUseSelectedRate);

    QButtonGroup* _setupRateGroupInt = new QButtonGroup(this);
    _setupRateGroupInt->addButton(_setupSpecifyRate);
    _setupRateGroupInt->addButton(_setupUseSelectedRate);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_code, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_overheadPerLaborHour, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
    connect(_overheadPerMachHour, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
    connect(_overheadPrcntOfLabor, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
    connect(_runRate, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
    connect(_runSpecifyRate, SIGNAL(toggled(bool)), _runRate, SLOT(setEnabled(bool)));
    connect(_runUseSelectedRate, SIGNAL(toggled(bool)), _stdRunRate, SLOT(setEnabled(bool)));
    connect(_runUseSelectedRate, SIGNAL(clicked()), this, SLOT(sPopulateRunRate()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_setupRate, SIGNAL(textChanged(const QString&)), this, SLOT(sPopulateOverheadRate()));
    connect(_setupSpecifyRate, SIGNAL(toggled(bool)), _setupRate, SLOT(setEnabled(bool)));
    connect(_setupUseSelectedRate, SIGNAL(toggled(bool)), _stdSetupRate, SLOT(setEnabled(bool)));
    connect(_setupUseSelectedRate, SIGNAL(clicked()), this, SLOT(sPopulateSetupRate()));
    connect(_stdRunRate, SIGNAL(newID(int)), this, SLOT(sPopulateRunRate()));
    connect(_stdSetupRate, SIGNAL(newID(int)), this, SLOT(sPopulateSetupRate()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations()));
    init();

    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
workCenter::~workCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void workCenter::languageChange()
{
    retranslateUi(this);
}


void workCenter::init()
{
  statusBar()->hide();

  _warehouse->setNull();
  
  _lbrrate.exec( "SELECT lbrrate_id, lbrrate_code, lbrrate_rate "
                 "FROM lbrrate "
                 "ORDER BY lbrrate_code;" );

  _stdSetupRate->populate(_lbrrate);
  if (!_stdSetupRate->count())
    _setupUseSelectedRate->setEnabled(FALSE);

  _stdRunRate->populate(_lbrrate);
  if (!_stdRunRate->count())
    _runUseSelectedRate->setEnabled(FALSE);

  _setupRate->setValidator(omfgThis->moneyVal());
  _runRate->setValidator(omfgThis->moneyVal());
  _overheadPrcntOfLabor->setValidator(omfgThis->percentVal());
  _overheadPerLaborHour->setValidator(omfgThis->moneyVal());
  _overheadPerMachHour->setValidator(omfgThis->moneyVal());
  _overheadPerUnit->setValidator(omfgThis->moneyVal());

  _setupType->setCurrentItem(-1);
}

enum SetResponse workCenter::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wrkcnt_id", &valid);
  if (valid)
  {
    _wrkcntid = param.toInt();
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
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cNew;
      _code->clear();
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _department->setEnabled(FALSE);
      _warehouse->setEnabled(FALSE);
      _wipLocation->setEnabled(FALSE);
      _numOfMachines->setEnabled(FALSE);
      _numOfPeople->setEnabled(FALSE);
      _setupSpecifyRate->setEnabled(FALSE);
      _setupUseSelectedRate->setEnabled(FALSE);
      _setupRate->setEnabled(FALSE);
      _stdSetupRate->setEnabled(FALSE);
      _setupType->setEnabled(FALSE);
      _runSpecifyRate->setEnabled(FALSE);
      _runUseSelectedRate->setEnabled(FALSE);
      _runRate->setEnabled(FALSE);
      _stdRunRate->setEnabled(FALSE);
      _overheadPrcntOfLabor->setEnabled(FALSE);
      _overheadPerLaborHour->setEnabled(FALSE);
      _overheadPerMachHour->setEnabled(FALSE);
      _overheadPerUnit->setEnabled(FALSE);
      _avgQueueDays->setEnabled(FALSE);
      _avgSetup->setEnabled(FALSE);
      _dailyCapacity->setEnabled(FALSE);
      _efficiencyFactor->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void workCenter::sCheck()
{
  _code->setText(_code->text().stripWhiteSpace());
  if ((_mode == cNew) && (_code->text().length()))
  {
    q.prepare( "SELECT wrkcnt_id "
               "FROM wrkcnt "
               "WHERE (UPPER(wrkcnt_code)=UPPER(:wrkcnt_code));" );
    q.bindValue(":wrkcnt_code", _code->text());
    q.exec();
    if (q.first())
    {
      _wrkcntid = q.value("wrkcnt_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void workCenter::sSave()
{
  if (_warehouse->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Work Center"),
                           tr("You must select a Warehouse for this Work Center before you may save it.") );
    _warehouse->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('wrkcnt_wrkcnt_id_seq') AS _wrkcnt_id;");
    if (q.first())
      _wrkcntid = q.value("_wrkcnt_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO wrkcnt "
               "( wrkcnt_id, wrkcnt_code, wrkcnt_descrip,"
               "  wrkcnt_dept_id, wrkcnt_warehous_id,"
               "  wrkcnt_nummachs, wrkcnt_numpeople,"
               "  wrkcnt_setup_lbrrate_id, wrkcnt_setuprate,"
               "  wrkcnt_run_lbrrate_id, wrkcnt_runrate,"
               "  wrkcnt_brd_prcntlbr, wrkcnt_brd_rateperlbrhr,"
               "  wrkcnt_brd_ratepermachhr, wrkcnt_brd_rateperunitprod,"
               "  wrkcnt_avgqueuedays, wrkcnt_avgsutime,"
               "  wrkcnt_dailycap, wrkcnt_caploaduom, wrkcnt_efficfactor,"
               "  wrkcnt_comments, wrkcnt_wip_location_id) "
               "VALUES "
               "( :wrkcnt_id, :wrkcnt_code, :wrkcnt_descrip,"
               "  :wrkcnt_dept_id, :wrkcnt_warehous_id,"
               "  :wrkcnt_nummachs, :wrkcnt_numpeople,"
               "  :wrkcnt_setup_lbrrate_id, :wrkcnt_setuprate,"
               "  :wrkcnt_run_lbrrate_id, :wrkcnt_runrate,"
               "  :wrkcnt_brd_prcntlbr, :wrkcnt_brd_rateperlbrhr,"
               "  :wrkcnt_brd_ratepermachhr, :wrkcnt_brd_rateperunitprod,"
               "  :wrkcnt_avgqueuedays, :wrkcnt_avgsutime,"
               "  :wrkcnt_dailycap, :wrkcnt_caploaduom, :wrkcnt_efficfactor,"
               "  :wrkcnt_comments, :wrkcnt_wip_location_id );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE wrkcnt "
               "SET wrkcnt_code=:wrkcnt_code, wrkcnt_descrip=:wrkcnt_descrip,"
               "    wrkcnt_dept_id=:wrkcnt_dept_id, wrkcnt_warehous_id=:wrkcnt_warehous_id,"
               "    wrkcnt_nummachs=:wrkcnt_nummachs, wrkcnt_numpeople=:wrkcnt_numpeople,"
               "    wrkcnt_setup_lbrrate_id=:wrkcnt_setup_lbrrate_id, wrkcnt_setuprate=:wrkcnt_setuprate,"
               "    wrkcnt_run_lbrrate_id=:wrkcnt_run_lbrrate_id, wrkcnt_runrate=:wrkcnt_runrate,"
               "    wrkcnt_brd_prcntlbr=:wrkcnt_brd_prcntlbr, wrkcnt_brd_rateperlbrhr=:wrkcnt_brd_rateperlbrhr,"
               "    wrkcnt_brd_ratepermachhr=:wrkcnt_brd_ratepermachhr, wrkcnt_brd_rateperunitprod=:wrkcnt_brd_rateperunitprod,"
               "    wrkcnt_avgqueuedays=:wrkcnt_avgqueuedays, wrkcnt_avgsutime=:wrkcnt_avgsutime,"
               "    wrkcnt_dailycap=:wrkcnt_dailycap, wrkcnt_caploaduom=:wrkcnt_caploaduom, wrkcnt_efficfactor=:wrkcnt_efficfactor,"
               "    wrkcnt_comments=:wrkcnt_comments, wrkcnt_wip_location_id=:wrkcnt_wip_location_id "
               "WHERE (wrkcnt_id=:wrkcnt_id);" );

  q.bindValue(":wrkcnt_id", _wrkcntid);
  q.bindValue(":wrkcnt_code", _code->text().stripWhiteSpace());
  q.bindValue(":wrkcnt_descrip", _description->text().stripWhiteSpace());
  if(_department->id() != -1)
    q.bindValue(":wrkcnt_dept_id", _department->id());
  q.bindValue(":wrkcnt_warehous_id", _warehouse->id());
  q.bindValue(":wrkcnt_wip_location_id", _wipLocation->id());
  q.bindValue(":wrkcnt_nummachs", _numOfMachines->value());
  q.bindValue(":wrkcnt_numpeople", _numOfPeople->value());
  q.bindValue(":wrkcnt_setup_lbrrate_id", ((_setupUseSelectedRate->isChecked()) ? _stdSetupRate->id() : -1));
  q.bindValue(":wrkcnt_setuprate", _setupRate->toDouble());
  q.bindValue(":wrkcnt_run_lbrrate_id", ((_runUseSelectedRate->isChecked()) ? _stdRunRate->id() : -1));
  q.bindValue(":wrkcnt_runrate", _runRate->toDouble());
  q.bindValue(":wrkcnt_brd_prcntlbr", (_overheadPrcntOfLabor->toDouble() / 100.0));
  q.bindValue(":wrkcnt_brd_rateperlbrhr", _overheadPerLaborHour->toDouble());
  q.bindValue(":wrkcnt_brd_ratepermachhr", _overheadPerMachHour->toDouble());
  q.bindValue(":wrkcnt_brd_rateperunitprod", _overheadPerUnit->toDouble());
  q.bindValue(":wrkcnt_avgqueuedays", _avgQueueDays->value());
  q.bindValue(":wrkcnt_avgsutime", _avgSetup->toDouble());
  q.bindValue(":wrkcnt_dailycap", _dailyCapacity->toDouble());
  q.bindValue(":wrkcnt_efficfactor", (_efficiencyFactor->toDouble() / 100.0));
  q.bindValue(":wrkcnt_comments", _comments->text());

  if (_setupType->currentItem() == 0)
    q.bindValue(":wrkcnt_caploaduom", "M");
  else
    q.bindValue(":wrkcnt_caploaduom", "L");

  q.exec();

  omfgThis->sWorkCentersUpdated();
  close();
}

void workCenter::sPopulateOverheadRate()
{
  double runRate;

  if (_runUseSelectedRate->isChecked())
  {
    _lbrrate.findFirst("lbrrate_id", _stdRunRate->id());
    runRate = _lbrrate.value("lbrrate_rate").toDouble();
  }
  else
    runRate = _runRate->toDouble();

  _overheadRate->setText(formatCost( ((_numOfPeople->value() * runRate) * _overheadPrcntOfLabor->toDouble() / 100) +
                                     (_numOfPeople->value() * _overheadPerLaborHour->toDouble()) +
                                     (_numOfMachines->value() * _overheadPerMachHour->toDouble()) ) );
}

void workCenter::sPopulateSetupRate()
{
  XSqlQuery rate;
  rate.prepare( "SELECT formatMoney(lbrrate_rate) AS rate "
                "FROM lbrrate "
                "WHERE (lbrrate_id=:lbrrate_id);" );
  rate.bindValue(":lbrrate_id", _stdSetupRate->id());
  rate.exec();
  if (rate.first())
    _setupRate->setText(rate.value("rate"));
}

void workCenter::sPopulateRunRate()
{
  XSqlQuery rate;
  rate.prepare( "SELECT formatMoney(lbrrate_rate) AS rate "
                "FROM lbrrate "
                "WHERE (lbrrate_id=:lbrrate_id);" );
  rate.bindValue(":lbrrate_id", _stdRunRate->id());
  rate.exec();
  if (rate.first())
    _runRate->setText(rate.value("rate"));
}

void workCenter::populate()
{
  q.prepare( "SELECT wrkcnt_code, wrkcnt_descrip, COALESCE(wrkcnt_dept_id, -1) AS wrkcnt_dept_id,"
             "       wrkcnt_warehous_id, wrkcnt_caploaduom, wrkcnt_comments,"
             "       wrkcnt_nummachs, wrkcnt_numpeople,"
             "       formatScrap(wrkcnt_brd_prcntlbr) AS brd_prcntlbr,"
             "       formatMoney(wrkcnt_brd_rateperlbrhr) AS brd_rateperlbrhr,"
             "       formatMoney(wrkcnt_brd_ratepermachhr) AS brd_ratepermachhr,"
             "       formatCost(wrkcnt_brd_rateperunitprod) AS brd_rateperunit,"
             "       wrkcnt_avgqueuedays,"
             "       formatTime(wrkcnt_avgsutime) AS avgsutime,"
             "       formatTime(wrkcnt_dailycap) AS dailycap,"
             "       formatPrcnt(wrkcnt_efficfactor) AS efficiencyfactor,"
             "       formatMoney(wrkcnt_setuprate) AS setuprate,"
             "       formatMoney(wrkcnt_runrate) AS runrate,"
             "       wrkcnt_setup_lbrrate_id, wrkcnt_run_lbrrate_id,"
             "       wrkcnt_wip_location_id "
             "FROM wrkcnt "
             "WHERE (wrkcnt_id=:wrkcnt_id);" );
  q.bindValue(":wrkcnt_id", _wrkcntid);
  q.exec();
  if (q.first())
  {
    _code->setText(q.value("wrkcnt_code"));
    _description->setText(q.value("wrkcnt_descrip"));
    _department->setId(q.value("wrkcnt_dept_id").toInt());
    _numOfMachines->setValue(q.value("wrkcnt_nummachs").toInt());
    _numOfPeople->setValue(q.value("wrkcnt_numpeople").toInt());
    _overheadPrcntOfLabor->setText(q.value("brd_prcntlbr"));
    _overheadPerLaborHour->setText(q.value("brd_rateperlbrhr"));
    _overheadPerMachHour->setText(q.value("brd_ratepermachhr"));
    _overheadPerUnit->setText(q.value("brd_rateperunit"));
    _avgQueueDays->setValue(q.value("wrkcnt_avgqueuedays").toInt());
    _avgSetup->setText(q.value("avgsutime"));
    _dailyCapacity->setText(q.value("dailycap"));
    _efficiencyFactor->setText(q.value("efficiencyfactor"));
    _warehouse->setId(q.value("wrkcnt_warehous_id").toInt());
    _wipLocation->setId(q.value("wrkcnt_wip_location_id").toInt());

    if (q.value("wrkcnt_setup_lbrrate_id").toInt() != -1)
    {
      _setupUseSelectedRate->setChecked(TRUE);
      _stdSetupRate->setId(q.value("wrkcnt_setup_lbrrate_id").toInt());
    }
    else
      _setupSpecifyRate->setChecked(TRUE);

    _setupRate->setText(q.value("setuprate"));
    sPopulateOverheadRate();

    if (q.value("wrkcnt_run_lbrrate_id").toInt() != -1)
    {
      _runUseSelectedRate->setChecked(TRUE);
      _stdRunRate->setId(q.value("wrkcnt_run_lbrrate_id").toInt());
    }
    else
      _runSpecifyRate->setChecked(TRUE);

    _runRate->setText(q.value("runrate"));
    sPopulateOverheadRate();

    if (q.value("wrkcnt_caploaduom").toString() == "M")
      _setupType->setCurrentItem(0);
    else if (q.value("wrkcnt_caploaduom").toString() == "L")
      _setupType->setCurrentItem(1);
    else _setupType->setCurrentItem(-1);

    _comments->setText(q.value("wrkcnt_comments").toString());
  }
}

void workCenter::sPopulateLocations()
{
  XSqlQuery loc;
  loc.prepare("SELECT location_id, formatLocationName(location_id) AS locationname"
              "  FROM location"
              " WHERE ( (NOT location_restrict)"
              "   AND   (location_warehous_id=:warehous_id) ) "
              "ORDER BY locationname;");
  loc.bindValue(":warehous_id", _warehouse->id());
  loc.exec();
  _wipLocation->populate(loc);
}

