/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "batchItem.h"

#include <QMessageBox>
#include <QSqlError>

#include <xsqlquery.h>

#include "storedProcErrorLookup.h"

#define cUndefined   0x00
// #define cView        0x03       // already defined in widgets.h
#define cReschedule  0x10

batchItem::batchItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, fl)
{
  setupUi(this);
  if (name)
    setObjectName(name);
  if (modal)
    setModal(modal);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _mode = cUndefined;
  _batchid = -1;
  _db = QSqlDatabase();
}

batchItem::~batchItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void batchItem::languageChange()
{
  retranslateUi(this);
}

int batchItem::Reschedule( int pBatchid, QWidget * pParent )
{
  return Reschedule(pBatchid, pParent, QSqlDatabase());
}

int batchItem::Reschedule(int pBatchid, QWidget *pParent, QSqlDatabase pDb)
{
  batchItem newdlg(pParent);

  newdlg._mode = cReschedule;
  newdlg._batchid = pBatchid;
  newdlg._db = pDb;
  newdlg.populate();

  newdlg._email->setEnabled(FALSE);

  return newdlg.exec();
}

int batchItem::view(int pBatchid, QWidget *pParent, QSqlDatabase pDb)
{
  batchItem newdlg(pParent);

  newdlg._mode    = cView;
  newdlg._batchid = pBatchid;
  newdlg._db      = pDb;
  newdlg.populate();

  newdlg._save->hide();
  newdlg._close->setText(tr("&Close"));
  newdlg._scheduledDate->setEnabled(false);
  newdlg._scheduledTime->setEnabled(false);
  newdlg._email->setEnabled(false);
  newdlg._reschedule->setEnabled(false);
  newdlg._interval->setEnabled(false);

  return newdlg.exec();
}

void batchItem::sSave()
{
  QString interval;

  if (_reschedule->isChecked())
  {
    switch (_interval->currentIndex())
    {
      case 0:
        interval = "D";
        break;

      case 1:
        interval = "W";
        break;

      case 2:
        interval = "M";
        break;
    }
  }

  if (interval.length() == 0)
    interval = "N";

  if (_mode == cReschedule)
  {
    XSqlQuery schedq(_db);
    schedq.prepare("SELECT xtbatch.rescheduleBatchItem(:batchid,"
                   "                           CAST(:date AS DATE) +"
                   "                           CAST(:time AS TIME),"
                   "                           :interval) AS result;");
    schedq.bindValue(":batchid",  _batchid);
    schedq.bindValue(":date",     _scheduledDate->date());
    schedq.bindValue(":time",     _scheduledTime->time());
    schedq.bindValue(":interval", interval); 
    schedq.exec();
    if (schedq.first())
    {
      int result = schedq.value("result").toInt();
      if (result < 0)
      {
        QMessageBox::critical(this,
                              tr("Error Rescheduling Batch Item at %1::%2")
                                .arg(__FILE__, __LINE__),
                              storedProcErrorLookup("rescheduleBatchItem",
                                                    result));
        return;
      }
    }
    else if (schedq.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this,
                            tr("Error Rescheduling Batch Item at %1::%2")
                              .arg(__FILE__, __LINE__),
                              schedq.lastError().databaseText());
      return;
    }
  }
  done(_batchid);
}

void batchItem::populate()
{
  XSqlQuery batch(_db);
  batch.prepare( "SELECT batch_action, batch_parameter,"
                 "       batch_user, batch_email, batch_reschedinterval,"
                 "       date(batch_submitted) AS submitted_date,"
                 "       CAST(batch_submitted AS TIME) AS submitted_time,"
                 "       date(batch_scheduled) AS scheduled_date,"
                 "       CAST(batch_scheduled AS TIME) AS scheduled_time,"
                 "       date(batch_started) AS started_date,"
                 "       CAST(batch_started AS TIME) AS started_time,"
                 "       date(batch_completed) AS completed_date,"
                 "       CAST(batch_completed AS TIME) AS completed_time "
                 "FROM xtbatch.batch "
                 "WHERE (batch_id=:batch_id);" );
  batch.bindValue(":batch_id", _batchid);
  batch.exec();
  if (batch.first())
  {
    _submittedBy->setText(batch.value("batch_user").toString());
    _email->setText(batch.value("batch_email").toString());
    _action->setText(batch.value("batch_action").toString());
    _parameter->setText(batch.value("batch_parameter").toString());

    QString interval = batch.value("batch_reschedinterval").toString();
    if (interval != "N")
    {
      _reschedule->setChecked(TRUE);

      if (interval == "D")
        _interval->setCurrentIndex(0);
      else if (interval == "W")
        _interval->setCurrentIndex(1);
      else if (interval == "M")
        _interval->setCurrentIndex(2);
    }

    _submittedDate->setDate(batch.value("submitted_date").toDate());
    _submittedTime->setTime(batch.value("submitted_time").toTime());
    _scheduledDate->setDate(batch.value("scheduled_date").toDate());
    _scheduledTime->setTime(batch.value("scheduled_time").toTime());
    _startedDate->setDate(batch.value("started_date").toDate());
    _startedTime->setTime(batch.value("started_time").toTime());
    _completedDate->setDate(batch.value("completed_date").toDate());
    _completedTime->setTime(batch.value("completed_time").toTime());
  }
}
