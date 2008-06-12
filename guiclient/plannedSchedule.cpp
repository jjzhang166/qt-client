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

#include "plannedSchedule.h"

#include <QVariant>
#include <QMessageBox>

#include "plannedScheduleItem.h"

plannedSchedule::plannedSchedule(QWidget * parent, const char * name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pschheadid = -1;
  _presaved = false;

  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));  
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));  
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));  
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sNumberChanged()));

  _list->addColumn(tr("#"),           _whsColumn,  Qt::AlignRight  );
  _list->addColumn(tr("Sched. Date"), _dateColumn, Qt::AlignCenter );
  _list->addColumn(tr("Item Number"), -1,          Qt::AlignLeft   );
  _list->addColumn(tr("Qty"),         _qtyColumn,  Qt::AlignRight  );
  _list->addColumn(tr("Status"),      _whsColumn,  Qt::AlignCenter );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

}

plannedSchedule::~plannedSchedule()
{
}

void plannedSchedule::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannedSchedule::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if(valid)
  {
    _pschheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
    {
      _mode = cNew;

      q.exec("SELECT nextval('pschhead_pschhead_id_seq') AS pschhead_id;");
      if(q.first())
        _pschheadid = q.value("pschhead_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }
      _new->setEnabled(false);

      _number->setFocus();
    }
    else if("edit" == mode)
    {
      _mode = cEdit;

      _number->setEnabled(false);
      _warehouse->setEnabled(false);

      _descrip->setFocus();
    }
    else if("view" == mode)
    {
      _mode = cView;

      _number->setEnabled(false);
      _descrip->setEnabled(false);
      _dates->setEnabled(false);
      _warehouse->setEnabled(false);
      _schedtype->setEnabled(false);

      _save->hide();
      _cancel->setFocus();
    }
  }

  return NoError;
}

void plannedSchedule::sSave()
{
  // TODO: Check and rules that need to be enforced here.
  q.prepare("SELECT pschitem_id "
            "  FROM pschitem "
            " WHERE ((pschitem_scheddate NOT BETWEEN :startDate AND :endDate) "
            "   AND  (pschitem_pschhead_id=:pschhead_id)); ");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":startDate", _dates->startDate());
  q.bindValue(":endDate", _dates->endDate());
  q.exec();
  if(q.first())
  {
    QMessageBox::warning( this, tr("Scheduled Items Out of Date Range"),
      tr("One or more of the Scheduled Items is outside the specified date\n"
         "range for this Planned Schedule. Please fix this before continuing.") );
    return;
  }

  if (_schedtype->currentItem() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Save Schedule"),
                           tr( "You must select a schedule type for this Schedule before creating it.\n" ) );
    _schedtype->setFocus();
    return;
  }

  if(cNew == _mode && _presaved != true)
  {
    q.prepare("INSERT INTO pschhead "
              "      (pschhead_id, pschhead_number, pschhead_warehous_id,"
              "       pschhead_descrip,"
              "       pschhead_start_date, pschhead_end_date, pschhead_type) "
              "VALUES(:pschhead_id, :number, :warehous_id,"
              "       :descrip,"
              "       :startDate, :endDate, :schedtype);");
  }
  else
    q.prepare("UPDATE pschhead "
              "   SET pschhead_descrip=:descrip,"
              "       pschhead_start_date=:startDate,"
              "       pschhead_end_date=:endDate,"
              "       pschhead_type=:schedtype "
              " WHERE (pschhead_id=:pschhead_id); ");
  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":number", _number->text().trimmed().toUpper());
  q.bindValue(":descrip", _descrip->text());
  q.bindValue(":warehous_id", _warehouse->id());
  q.bindValue(":startDate", _dates->startDate());
  q.bindValue(":endDate", _dates->endDate());
  if (_schedtype->currentItem() == 0)
    q.bindValue(":schedtype", "F");
  else if (_schedtype->currentItem() == 1)
    q.bindValue(":schedtype", "N");
  else if (_schedtype->currentItem() == 2)
    q.bindValue(":schedtype", "P");

  q.exec();

  accept();
}

void plannedSchedule::reject()
{
  if(cNew == _mode)
  {
    q.prepare("DELETE FROM pschitem WHERE pschitem_pschhead_id=:pschhead_id;"
              "DELETE FROM pschhead WHERE pschhead_id=:pschhead_id;");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
  }

  XDialog::reject();
}

void plannedSchedule::sNumberChanged()
{
  if(cNew != _mode)
    return;

  QString str = _number->text().trimmed().toUpper();

  if(str.isEmpty())
    return;

  q.prepare("SELECT pschhead_id "
            "  FROM pschhead "
            " WHERE (pschhead_number=:pschhead_number);");
  q.bindValue(":pschhead_number", str);
  q.exec();
  if(q.first())
  {
    _number->setText("");
    _number->setFocus();
    QMessageBox::warning( this, tr("Invalid Number"),
      tr("The Number you specified for this Planned Schedule already exists.") );
    
    return;
  }

  _new->setEnabled(true);
}

void plannedSchedule::sNew()
{
  if(cNew == _mode && _presaved != true)
  {
    q.prepare("INSERT INTO pschhead "
              "      (pschhead_id, pschhead_number, pschhead_warehous_id,"
              "       pschhead_start_date, pschhead_end_date) "
              "VALUES(:pschhead_id, :number, :warehous_id, startOfTime(), startOfTime());");
    q.bindValue(":pschhead_id", _pschheadid);
    q.bindValue(":number", _number->text().trimmed().toUpper());
    q.bindValue(":warehous_id", _warehouse->id());
    if(!q.exec())
    {
      QMessageBox::warning( this, tr("Error"),
        tr("There was an error updating the database.") );
      return;
    }

    _number->setEnabled(false);
    _warehouse->setEnabled(false);
    _presaved = true;
   
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("pschhead_id", _pschheadid);

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sEdit()
{
  ParameterList params;
  if(cView == _mode)
    params.append("mode", "view");
  else
    params.append("mode", "edit");
  params.append("pschitem_id", _list->id());

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sCopy()
{
  if(cView == _mode)
    return;

  ParameterList params;
  params.append("mode", "copy");
  params.append("pschitem_id", _list->id());

  plannedScheduleItem newdlg(this, "", true);
  newdlg.set(params);
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void plannedSchedule::sDelete()
{
  q.prepare("UPDATE pschitem SET pschitem_status = 'X' WHERE (pschitem_id=:pschitem_id);");
  q.bindValue(":pschitem_id", _list->id());
  q.exec();
  sFillList();
}

void plannedSchedule::populate()
{
  q.prepare("SELECT pschhead_number,"
            "       pschhead_descrip,"
            "       pschhead_warehous_id,"
            "       pschhead_start_date,"
            "       pschhead_end_date,"
            "       pschhead_status,"
            "       pschhead_type "
            "  FROM pschhead "
            " WHERE (pschhead_id=:pschhead_id);");
  q.bindValue(":pschhead_id", _pschheadid);
  q.exec();
  if(q.first())
  {
    _number->setText(q.value("pschhead_number").toString());
    _descrip->setText(q.value("pschhead_descrip").toString());
    _warehouse->setId(q.value("pschhead_warehous_id").toInt());
    _dates->setStartDate(q.value("pschhead_start_date").toDate());
    _dates->setEndDate(q.value("pschhead_end_date").toDate());
    if (q.value("pschhead_status").toString() == "U")
      _status->setText("Unreleased");
    else if (q.value("pschhead_status").toString() == "R")
      _status->setText("Released");
    else
      _status->setText(q.value("pschhead_status").toString());
    if (q.value("pschhead_type").toString() == "F")
      _schedtype->setCurrentItem(0);
    else if (q.value("pschhead_type").toString() == "N")
      _schedtype->setCurrentItem(1);
    else if (q.value("pschhead_type").toString() == "P")
      _schedtype->setCurrentItem(2);
    else
      _schedtype->setCurrentItem(-1);


    _number->setEnabled(false);
    _warehouse->setEnabled(false);
    _new->setEnabled(true);

    sFillList();
  }
}

void plannedSchedule::sFillList()
{
  q.prepare("SELECT pschitem_id,"
            "       pschitem_linenumber,"
            "       formatDate(pschitem_scheddate), "
            "       item_number,"
            "       pschitem_qty,"
            "       pschitem_status "
            "  FROM pschitem JOIN "
            "       (itemsite JOIN item "
            "         ON (itemsite_item_id=item_id)) "
            "       ON (pschitem_itemsite_id=itemsite_id) "
            " WHERE (pschitem_pschhead_id=:pschhead_id) "
            " ORDER BY pschitem_linenumber;");
  q.bindValue(":pschhead_id", _pschheadid);
  q.exec();
  _list->populate(q);
}

