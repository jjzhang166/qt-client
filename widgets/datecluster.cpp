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

#include <QApplication>
#include <QCalendarWidget>
#include <QDateTime>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QPoint>
#include <QRegExp>
#include <QSize>
#include <QVBoxLayout>
#include <QValidator>

#include <xsqlquery.h>
#include <parameter.h>

#include "datecluster.h"
#include "dcalendarpopup.h"
#include "format.h"

#define DEBUG false

DCalendarPopup::DCalendarPopup(const QDate &date, QWidget *parent)
  : QWidget(parent, Qt::Popup)
{
  _cal = new QCalendarWidget(this);
  _cal->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
  _cal->setSelectedDate(date);

  QVBoxLayout *vbox = new QVBoxLayout(this);
  vbox->setMargin(0);
  vbox->setSpacing(0);
  vbox->addWidget(_cal);

  connect(_cal, SIGNAL(activated(QDate)), this, SLOT(dateSelected(QDate)));
  connect(_cal, SIGNAL(clicked(QDate)),   this, SLOT(dateSelected(QDate)));
  connect(_cal, SIGNAL(activated(QDate)), this, SLOT(dateSelected(QDate)));

  // position the center of the popup near the center of its parent
  if (parent)
  {
    QSize parentsize = parent->sizeHint();
    QPoint parentcenter = parent->pos() + QPoint(parentsize.width() / 2,
						 parentsize.height() / 2);
    parentcenter = parent->mapToGlobal(parentcenter);
    QRect screen = QApplication::desktop()->availableGeometry(parentcenter);

    QSize mysize = sizeHint();
    QPoint mycenter = parentcenter;

    if (mycenter.x() + (mysize.width() / 2) > screen.right())
      mycenter.setX(screen.right() - (mysize.width() / 2));
    else if (mycenter.x() - (mysize.width() / 2) < screen.left())
      mycenter.setX(screen.left() + (mysize.width() / 2));

    if (mycenter.y() + (mysize.height() / 2) > screen.bottom())
      mycenter.setY(screen.bottom() - (mysize.height() / 2));
    else if (mycenter.y() - (mysize.height() / 2) < screen.top())
      mycenter.setY(screen.top() + (mysize.height() / 2));

    QPoint myorigin(mycenter.x() - mysize.width() / 2, mycenter.y() - mysize.height() / 2);

    move(myorigin);
  }

  _cal->setFocus();
}

void DCalendarPopup::dateSelected(const QDate &pDate)
{
  if (DEBUG)
    qDebug("DCalendarPopup::dateSelected(%s)", qPrintable(pDate.toString()));
  if (parent())
    ((XDateEdit*)parent())->setDate(pDate);

  emit newDate(pDate);
  close();
}

///////////////////////////////////////////////////////////////////////////////

void XDateEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("date"));
}

XDateEdit::XDateEdit(QWidget *parent, const char *name) :
  XLineEdit(parent, name)
{
  connect(this, SIGNAL(editingFinished()), this, SLOT(parseDate()));
  connect(this, SIGNAL(requestList()),     this, SLOT(showCalendar()));
  connect(this, SIGNAL(requestSearch()),   this, SLOT(showCalendar()));

  if (parent && ! parent->objectName().isEmpty())
    setObjectName(parent->objectName());
  else
    setObjectName(metaObject()->className());

  _allowNull   = FALSE;
  _parsed      = FALSE;
  _nullString  = QString::null;
  _valid       = FALSE;
}

XDateEdit::~XDateEdit()
{
}

void XDateEdit::parseDate()
{
  QString dateString = text().stripWhiteSpace();
  bool    isNumeric;

  if (DEBUG)
    qDebug("%s::parseDate() with dateString %s, _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(dateString),
           qPrintable(_currentDate.toString()), _allowNull);

#ifdef OpenMFGGUIClient_h
  QDate today = ofmgThis->dbDate();
#else
  QDate today = QDate::currentDate();
#endif

  if (_parsed)
  {
    if (DEBUG)
      qDebug("%s::parseDate() looks like we've already parsed this string",
             qPrintable(parent() ? parent()->objectName() : objectName()));
    return;
  }

  _valid = false;

  if (dateString == _nullString || dateString.isEmpty())
    setNull();

  else if (dateString == "0")                           // today
    setDate(today, TRUE);

  else if (dateString.contains(QRegExp("^[+-][0-9]+"))) // offset from today
  {
    int offset = dateString.toInt(&isNumeric);
    if (isNumeric)
      setDate(today.addDays(offset), true);
  }

  else if (dateString[0] == '#')                        // julian day
  {
    int offset = dateString.right(dateString.length() - 1).toInt(&isNumeric);
    if (isNumeric)
      setDate(QDate(today.year(), 1, 1).addDays(offset - 1), TRUE);
  }

  else if (dateString.contains(QRegExp("^[0-9][0-9]?$"))) // date in month
  {
    int offset = dateString.toInt(&isNumeric, 10);
    if (isNumeric)
    {
      if (offset > today.daysInMonth())
        offset = today.daysInMonth();
 
      setDate(QDate(today.year(), today.month(), 1).addDays(offset - 1), TRUE);
    }
  }

  else                                                  // interpret with locale
  {
    QString dateFormatStr = QLocale().dateFormat(QLocale::ShortFormat);
    if (DEBUG)
      qDebug("%s::parseDate() trying to parse with %s",
             qPrintable(parent() ? parent()->objectName() : objectName()),
             qPrintable(dateFormatStr));

    QDate tmp = QDate::fromString(dateString, dateFormatStr);
    if (tmp.isValid() && dateFormatStr.indexOf(QRegExp("y{2}")) >= 0)
    {
      qDebug("%s::parseDate() found valid 2-digit year %d",
             qPrintable(parent() ? parent()->objectName() : objectName()),
             tmp.year());
      if (tmp.year() < 1950)     // Qt docs say 2-digit years are 1900-based so
      {
        tmp = tmp.addYears(100); // add backwards-compat with pre-3.0 DLineEdit
        qDebug("%s::parseDate() altered year to %d",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               tmp.year());
      }
    }
    else
    {
      // try 4 digits, ignoring the possibility of '-literals in the format str
      dateFormatStr.replace(QRegExp("y{2}"), "yyyy");
      if (DEBUG)
        qDebug("%s::parseDate() rewriting format string to %s",
               qPrintable(parent() ? parent()->objectName() : objectName()),
               qPrintable(dateFormatStr));
      tmp = QDate::fromString(dateString, dateFormatStr);
    }

    setDate(QDate(tmp.year(), tmp.month(), tmp.day()), true );
  }

  if (!_valid)
    setText("");

  _parsed = true;
}

void XDateEdit::setNull()
{
  if (DEBUG)
    qDebug("%s::setNull() with _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(_currentDate.toString()),
           _allowNull);
  if (_allowNull)
  {
    _valid  = TRUE;
    _parsed = TRUE;
    setText(_nullString);
    _currentDate = _nullDate;
    emit newDate(_currentDate);
  }
  else
  {
    _valid = FALSE;
    _parsed = TRUE;
    setText("");
    _currentDate = QDate();
  }
}

void XDateEdit::setDate(const QDate &pDate, bool pAnnounce)
{
  if (DEBUG)
    qDebug("%s::setDate(%s, %d) with _currentDate %s, _allowNull %d",
           qPrintable(parent() ? parent()->objectName() : objectName()),
           qPrintable(pDate.toString()), pAnnounce,
           qPrintable(_currentDate.toString()), _allowNull);

  if (pDate.isNull())
    setNull();
  else
  {
    _currentDate = pDate;
    _valid = _currentDate.isValid();
    _parsed = _valid;

    if (DEBUG)
      qDebug("%s::setDate() setting text",
             qPrintable(parent() ? parent()->objectName() : objectName()));
    if ((_allowNull) && (_currentDate == _nullDate))
      setText(_nullString);
    else
      setText(formatDate(pDate));
    if (DEBUG)
      qDebug("%s::setDate() done setting text",
             qPrintable(parent() ? parent()->objectName() : objectName()));
  }

  if (pAnnounce)
  {
    if (DEBUG)
      qDebug("%s::setDate() emitting newDate(%s)",
             qPrintable(parent() ? parent()->objectName() : objectName()),
             qPrintable(_currentDate.toString()));
    emit newDate(_currentDate);
  }
}

void XDateEdit::clear()
{
  if (DEBUG)
    qDebug("%s::clear()",
          qPrintable(parent() ? parent()->objectName() : objectName()));
  setDate(_nullDate, true);
}

QDate XDateEdit::date()
{
  if (DEBUG)
    qDebug("%s::date()",
            qPrintable(parent() ? parent()->objectName() : objectName()));

  if (!_parsed)
    parseDate();

  if (!_valid)
    return _nullDate;

  return _currentDate;
}

bool XDateEdit::isNull()
{
  if (DEBUG)
    qDebug("%s::isNull()",
            qPrintable(parent() ? parent()->objectName() : objectName()));

  if (!_parsed)
    parseDate();

  return date().isNull();
}

bool XDateEdit::isValid()
{
  if (DEBUG)
    qDebug("%s::isValid()",
            qPrintable(parent() ? parent()->objectName() : objectName()));
  if (!_parsed)
    parseDate();

  return _valid;
}

void XDateEdit::showCalendar()
{
  if (DEBUG)
    qDebug("%s::showCalendar()",
            qPrintable(parent() ? parent()->objectName() : objectName()));
  DCalendarPopup *cal = new DCalendarPopup(date(), this);
  connect(cal, SIGNAL(newDate(const QDate &)), this, SIGNAL(newDate(const QDate &)));
  cal->show();
}

///////////////////////////////////////////////////////////////////////////////

DLineEdit::DLineEdit(QWidget *parent, const char *name) :
  QWidget(parent)
{
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setFocusPolicy(Qt::StrongFocus);
  setMaximumWidth(200);
  setName(name);

  QHBoxLayout *hbox = new QHBoxLayout(this);
  hbox->addWidget(&_lineedit);
  hbox->addWidget(&_calbutton);
  hbox->setSpacing(1);
  hbox->setMargin(0);

  QPixmap pixmap(":/widgets/images/calendar_24.png"); 
  _calbutton.setIconSize(QSize(24,24));
  _calbutton.setIcon(QIcon(pixmap));
  _calbutton.setFlat(true);
  _calbutton.setMaximumSize(pixmap.size());
  _calbutton.setFocusPolicy(Qt::NoFocus);

  connect(&_calbutton,            SIGNAL(clicked()), &_lineedit, SLOT(showCalendar()));
  connect(&_lineedit,     SIGNAL(editingFinished()), &_lineedit, SLOT(parseDate()));
  connect(&_lineedit,SIGNAL(newDate(const QDate &)), this,       SIGNAL(newDate(const QDate &)));

  setFocusProxy(&_lineedit);
}

void DLineEdit::setEnabled(const bool p)
{
  QWidget::setEnabled(p);
  _lineedit.setEnabled(p);
  _calbutton.setEnabled(p);
}

DateCluster::DateCluster(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  if(pName)
    setObjectName(pName);

  QSizePolicy tsizePolicy(static_cast<QSizePolicy::Policy>(0), static_cast<QSizePolicy::Policy>(0));
  tsizePolicy.setHorizontalStretch(0);
  tsizePolicy.setVerticalStretch(0);
  tsizePolicy.setHeightForWidth(sizePolicy().hasHeightForWidth());
  setSizePolicy(tsizePolicy);

  QHBoxLayout *mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(5);
  mainLayout->setMargin(0);
  mainLayout->setObjectName(QString::fromUtf8("mainLayout"));

  QVBoxLayout *literalLayout = new QVBoxLayout();
  literalLayout->setSpacing(5);
  literalLayout->setMargin(0);
  literalLayout->setObjectName(QString::fromUtf8("literalLayout"));

  _startDateLit = new QLabel(tr("Start Date:"), this, "_startDateLit");
  _startDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_startDateLit);

  _endDateLit = new QLabel(tr("End Date:"), this, "_endDateLit");
  _endDateLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  literalLayout->addWidget(_endDateLit);

  mainLayout->addLayout(literalLayout);

  QVBoxLayout *dataLayout = new QVBoxLayout();
  dataLayout->setSpacing(5);
  dataLayout->setMargin(0);
  dataLayout->setObjectName(QString::fromUtf8("dataLayout"));

  _startDate = new DLineEdit(this, "_startDate");
  dataLayout->addWidget(_startDate);

  _endDate = new DLineEdit(this, "_endDate");
  dataLayout->addWidget(_endDate);

  mainLayout->addLayout(dataLayout);

  _startDateLit->setBuddy(_startDate);
  _endDateLit->setBuddy(_endDate);

  connect(_startDate, SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));
  connect(_endDate,   SIGNAL(newDate(const QDate &)), this, SIGNAL(updated()));

  //setTabOrder(_startDate, _endDate);
  //setTabOrder(_endDate, _startDate);
  setFocusProxy(_startDate);
}

void DateCluster::setStartNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _startDate->setAllowNullDate(TRUE);
  _startDate->setNullString(pString);
  _startDate->setNullDate(pDate);

  if (pSetNull)
    _startDate->setNull();
}

void DateCluster::setEndNull(const QString &pString, const QDate &pDate, bool pSetNull)
{
  _endDate->setAllowNullDate(TRUE);
  _endDate->setNullString(pString);
  _endDate->setNullDate(pDate);

  if (pSetNull)
    _endDate->setNull();
}

void DateCluster::setStartCaption(const QString &pString)
{
  _startDateLit->setText(pString);
}

void DateCluster::setEndCaption(const QString &pString)
{
  _endDateLit->setText(pString);
}

void DateCluster::appendValue(ParameterList &pParams)
{
  pParams.append("startDate", _startDate->date());
  pParams.append("endDate", _endDate->date());
}

void DateCluster::bindValue(XSqlQuery &pQuery)
{
  pQuery.bindValue(":startDate", _startDate->date());
  pQuery.bindValue(":endDate", _endDate->date());
}

