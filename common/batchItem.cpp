/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
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
  newdlg._recur->setEnabled(false);

  return newdlg.exec();
}

void batchItem::sSave()
{
  if (_mode == cReschedule)
  {
    RecurrenceWidget::RecurrenceChangePolicy cp = _recur->getChangePolicy();
    if (cp == RecurrenceWidget::NoPolicy)
      return;

    XSqlQuery beginq("BEGIN;");
    XSqlQuery rollbackq;
    rollbackq.prepare("ROLLBACK;");

    XSqlQuery schedq(_db);
    schedq.prepare("UPDATE xtbatch.batch"
                   "   SET batch_scheduled=CAST(:date AS DATE) + CAST(:time AS TIME),"
                   "       batch_started=NULL, batch_completed=NULL,"
                   "       batch_recurring_batch_id=:parentid"
                   " WHERE (batch_id=:batchid);");
    schedq.bindValue(":date",     _scheduledDate->date());
    schedq.bindValue(":time",     _scheduledTime->time());
    if (_recur->isRecurring())
      schedq.bindValue(":parentid", _recur->parentId());
    schedq.bindValue(":batchid",  _batchid);
    schedq.exec();
    if (schedq.lastError().type() != QSqlError::NoError)
    {
      rollbackq.exec();
      QMessageBox::critical(this,
                            tr("Error rescheduling job at %1::%2")
                              .arg(__FILE__, __LINE__),
                              schedq.lastError().databaseText());
      return;
    }

    QString errmsg;
    if (! _recur->save(true, cp, &errmsg))
    {
      rollbackq.exec();
      QMessageBox::critical(this, "Error Saving Recurrence", errmsg);
      return;
    }

    XSqlQuery commitq("COMMIT;");
  }
  done(_batchid);
}

void batchItem::populate()
{
  XSqlQuery batch(_db);
  batch.prepare( "SELECT batch_action, batch_parameter,"
                 "       batch_user, batch_email,"
                 "       date(batch_submitted) AS submitted_date,"
                 "       CAST(batch_submitted AS TIME) AS submitted_time,"
                 "       date(batch_scheduled) AS scheduled_date,"
                 "       CAST(batch_scheduled AS TIME) AS scheduled_time,"
                 "       date(batch_started) AS started_date,"
                 "       CAST(batch_started AS TIME) AS started_time,"
                 "       date(batch_completed) AS completed_date,"
                 "       CAST(batch_completed AS TIME) AS completed_time,"
                 "       batch_recurring_batch_id "
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

    _submittedDate->setDate(batch.value("submitted_date").toDate());
    _submittedTime->setTime(batch.value("submitted_time").toTime());
    _scheduledDate->setDate(batch.value("scheduled_date").toDate());
    _scheduledTime->setTime(batch.value("scheduled_time").toTime());
    _startedDate->setDate(batch.value("started_date").toDate());
    _startedTime->setTime(batch.value("started_time").toTime());
    _completedDate->setDate(batch.value("completed_date").toDate());
    _completedTime->setTime(batch.value("completed_time").toTime());

    _recur->setParent(batch.value("batch_recurring_batch_id").isNull() ?
                      _batchid : batch.value("batch_recurring_batch_id").toInt(),
                      batch.value("batch_action").toString());
  }
}
