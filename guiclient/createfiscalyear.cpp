/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createfiscalyear.h"

#include <QDebug>

#include "errorReporter.h"
#include "storedProcErrorLookup.h"

static QString saveButtonText;

bool createFiscalYear::hasPriv(const int mode, const int pId)
{
  Q_UNUSED(mode); Q_UNUSED(pId);
  return _privileges->check("MaintainAccountingPeriods");
}

createFiscalYear::createFiscalYear(QWidget *parent) :
  XDialog(parent)
{
  setupUi(this);

  saveButtonText = _save->text();

  connect(_cancel,   SIGNAL(clicked(bool)),  SLOT(sCancel()));
  connect(_choose,   SIGNAL(newID(int)),     SLOT(sHandleChoice(int)));
  connect(_fy,       SIGNAL(valid(bool)),    SLOT(sHandleButtons()));
  connect(_firstDay, SIGNAL(newDate(QDate)), SLOT(sHandleButtons()));
  connect(_period,   SIGNAL(updated()),      SLOT(sHandleButtons()));
  connect(_save,     SIGNAL(clicked(bool)),  SLOT(sSave()));
  connect(_style,    SIGNAL(currentIndexChanged(int)), SLOT(sHandleButtons()));

  _choose->append(1, tr("Add a new accounting period"),  "PERIOD");
  _choose->append(2, tr("Add a new Fiscal Year"),        "NEWFY");
  _choose->append(3, tr("Copy an existing Fiscal Year"), "COPYFY");

  _firstDay->setDate(QDate(QDate::currentDate().year(), 1, 1));

  _fy->addColumn(tr("Starting"), -1, Qt::AlignLeft, true, "start");
  _fy->addColumn(tr("Ending"),   -1, Qt::AlignLeft, true, "end");

  XSqlQuery q;

  q.prepare("WITH maxid AS (SELECT * FROM yearperiod"
            "                ORDER BY yearperiod_start DESC LIMIT 1) "
            "SELECT yearperiod_id, 0 AS xtindentrole,"
            "       yearperiod_start AS start, yearperiod_end AS end"
            "  FROM maxid"
            " UNION "
            "SELECT period_yearperiod_id, 1,"
            "       period_start, period_end"
            "  FROM period"
            " WHERE period_yearperiod_id = (SELECT yearperiod_id FROM maxid)"
            " ORDER BY xtindentrole, start;");
  q.exec();
  _fy->populate(q, true);
  if (_fy->topLevelItemCount() < 0)
    _choose->setCode("NEWFY");
  else
    _fy->setCurrentItem(_fy->topLevelItem(0));

  q.prepare("SELECT period_start + interval '1 month' AS start,"
            "       period_end   + interval '1 month' AS end,"
            "       1 AS seq"
            "  FROM period"
            " UNION "
            "SELECT date_trunc('month', current_timestamp),"
            "       date_trunc('month', current_timestamp) + INTERVAL '1 month' - INTERVAL '1 day',"
            "       2 AS seq"
            " ORDER BY seq, start DESC LIMIT 1;");
  q.exec();
  if (q.first())
  {
    _period->setStartDate(q.value("start").toDate().addDays(1));
    _period->setEndDate(q.value("end").toDate());
  }

  _style->append(1, tr("Monthly"),   "M");
  _style->append(2, tr("Quarterly"), "Q");

  sHandleChoice(_choose->id());
}

void createFiscalYear::sCancel()
{
  reject();
}

void createFiscalYear::sHandleButtons()
{
  XTreeWidgetItem *lastFY = _fy->topLevelItem(0);
  bool newAP  = _choose->code() == "PERIOD" && _period->allValid()
                && lastFY
                && lastFY->rawValue("start").toDate() <= QDate::currentDate()
                && lastFY->rawValue("end").toDate()   >= QDate::currentDate();
  bool newFY  = _choose->code() == "NEWFY"
                && _firstDay->isValid() && _style->isValid();
  bool copyFY = _choose->code() == "COPYFY" && _fy->id() != -1;

  _save->setEnabled(newAP || newFY || copyFY);
}

void createFiscalYear::sHandleChoice(int selection)
{
  QWidget *page = NULL;
  XTreeWidgetItem *lastFY = _fy->topLevelItem(0);
  bool fyExists = (lastFY != NULL);
  bool fyIsCurrent = fyExists
                  && lastFY->rawValue("start").toDate() <= QDate::currentDate()
                  && lastFY->rawValue("end").toDate()   >= QDate::currentDate();

  _fy->setEnabled(hasPriv(cEdit));
  _period->setEnabled(hasPriv(cEdit));

  _period->setVisible(fyIsCurrent);
  _noFYLit->setVisible(! fyIsCurrent);

  _fy->setVisible(fyExists);
  _noFYToCopyLit->setVisible(! fyExists);

  _save->setText(saveButtonText);

  if (! hasPriv())
    page = _noPrivPage;
  else
    switch (selection) {
      case 1:
        page = _newPeriodPage;
        break;
      case 2:
        page = _newFYPage;
        break;
      case 3:
        page = _copyFYPage;
        _save->setText(tr("Copy Fiscal Year"));
        break;
      default:
        qWarning() << "Probable bug at" << __FILE__ << __LINE__ << ":" << selection;
    }
  if (page)
  {
    _stack->setCurrentWidget(page);
  }
  sHandleButtons();
}

void createFiscalYear::sSave()
{
  QString funcname;
  XSqlQuery q;
  switch (_choose->id()) {
    case 1:
      funcname = "createAccountingPeriod";
      q.prepare("SELECT createAccountingPeriod(:start, :end, yearperiod_id,"
                "       EXTRACT(QUARTER FROM"
                "               justify_days(timestamp :end - yearperiod_start)"
                "       )::INTEGER) AS result"
                "  FROM yearperiod WHERE yearperiod_id = :fy;");
      q.bindValue(":start", _period->startDate());
      q.bindValue(":end",   _period->endDate());
      q.bindValue(":fy",    _fy->id());
      break;
    case 2:
    {
      funcname = "createFiscalYear";
      q.prepare("SELECT createFiscalYear(:start, :style) AS result;");
      q.bindValue(":start", _firstDay->date());
      q.bindValue(":style", _style->code());
      break;
    }
    case 3:
    {
      funcname = "copyAccountingYearPeriod";
      q.prepare("SELECT copyAccountingYearPeriod(:fy) AS result;");
      q.bindValue(":fy", _fy->id());
      break;
    }
    default:
      qWarning() << "Probable bug at" << __FILE__ << __LINE__
                 << ":" << _choose->id();
  }
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                           storedProcErrorLookup(funcname, result), __FILE__, __LINE__);
      return;
    }
  } else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                                  q, __FILE__, __LINE__))
    return;


  accept();
}
