/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdoccopysetter.h"

#include <QtScript>

#include "editwatermark.h"

static QString yesStr = QT_TRANSLATE_NOOP("XDocCopySetter", "Yes");
static QString noStr  = QT_TRANSLATE_NOOP("XDocCopySetter", "No");

/**
  @class XDocCopySetter

  @brief The XDocCopySetter gives the %user a unified interface for telling
         the system how many copies of a particular document should be printed
         when it gets sent to a printer, what the watermark should be on the
         report, and whether the report should show prices or not.

  @see printInvoice

 */

XDocCopySetter::XDocCopySetter(QWidget *parent, const char *pName) :
  QWidget(parent)
{
  setupUi(this);

  if(pName)
    setObjectName(pName);

  connect(_watermarks,  SIGNAL(itemSelected(int)), this, SLOT(sEditWatermark()));
  connect(_numOfCopies, SIGNAL(valueChanged(int)), this, SLOT(setNumCopies(int)));

  _watermarks->addColumn(tr("Copy #"),      80, Qt::AlignRight);
  _watermarks->addColumn(tr("Watermark"),   -1, Qt::AlignLeft);
  _watermarks->addColumn(tr("Show Prices"), 80, Qt::AlignCenter);
}

XDocCopySetter::~XDocCopySetter()
{
}

void XDocCopySetter::languageChange()
{
}

QString XDocCopySetter::labelText()       const { return _copiesGroup->title();}
int     XDocCopySetter::numCopies()       const { return _numOfCopies->value();}
QString XDocCopySetter::numCopiesMetric() const { return _numCopiesMetric;     }
QString XDocCopySetter::showPriceMetric() const { return _showPriceMetric;     }
QString XDocCopySetter::watermarkMetric() const { return _watermarkMetric;     }

bool XDocCopySetter::showCosts(const int row) const
{
  return (_watermarks->topLevelItem(row)->text(2) == yesStr);
}

QString XDocCopySetter::watermark(const int row) const
{
  return _watermarks->topLevelItem(row)->text(1);
}

bool XDocCopySetter::save()
{
  if (_x_metrics)
  {
    if (! _numCopiesMetric.isEmpty())
      _x_metrics->set(_numCopiesMetric, _numOfCopies->value());

    QString fullmetric("%1%2");
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      XTreeWidgetItem *item = _watermarks->topLevelItem(i);
      if (! _watermarkMetric.isEmpty())
        _x_metrics->set(fullmetric.arg(_watermarkMetric).arg(i), item->text(1));
      if (! _showPriceMetric.isEmpty())
        _x_metrics->set(fullmetric.arg(_showPriceMetric).arg(i), (item->text(2) == yesStr));
    }
  }
  return true;
}

void XDocCopySetter::setLabelText(const QString text)
{
  _copiesGroup->setTitle(text);
}

void XDocCopySetter::setNumCopies(const int numCopies)
{
  _numOfCopies->setValue(numCopies);

  while (_watermarks->topLevelItemCount() > numCopies)
    _watermarks->takeTopLevelItem(_watermarks->topLevelItemCount() - 1);

  for (int i = _watermarks->topLevelItemCount(); i < numCopies; i++)
  {
    QString watermark = _watermarkMetric.isEmpty() ? ""
             : _x_metrics->value(QString("%1%2").arg(_watermarkMetric).arg(i));
    QString showprice;
    if (_showPriceMetric.isEmpty() ||
        _x_metrics->boolean(QString("%1%2").arg(_showPriceMetric).arg(i)) ||
        _x_metrics->value(QString("%1%2").arg(_showPriceMetric).arg(i)).isEmpty())
      showprice = yesStr;
    else
      showprice = noStr;
    new XTreeWidgetItem(_watermarks,
                        _watermarks->topLevelItem(_watermarks->topLevelItemCount() - 1),
                        i, i, tr("Copy #%1").arg(i + 1), watermark, showprice);
  }
}

void XDocCopySetter::setNumCopiesMetric(const QString metric)
{
  _numCopiesMetric = metric;

  // _numCopies->setValue cascades to setNumCopies
  if (! _numCopiesMetric.isEmpty() && _x_metrics)
    _numOfCopies->setValue(_x_metrics->value(metric).toInt());
}

void XDocCopySetter::setShowPriceMetric(const QString metric)
{
  _showPriceMetric = metric;

  if (! _showPriceMetric.isEmpty() && _x_metrics)
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      QString fullmetric = QString("%1%2").arg(metric).arg(i);
      if (_x_metrics->value(fullmetric).isEmpty() ||
          _x_metrics->boolean(fullmetric))
        _watermarks->topLevelItem(i)->setText(2, yesStr);
      else
        _watermarks->topLevelItem(i)->setText(2, noStr);
    }
  }
}

void XDocCopySetter::setWatermarkMetric(const QString metric)
{
  _watermarkMetric = metric;

  if (! _watermarkMetric.isEmpty() && _x_metrics)
  {
    for (int i = 0; i < _watermarks->topLevelItemCount(); i++)
    {
      QString fullmetric = QString("%1%2").arg(metric).arg(i);
      if (! _x_metrics->value(fullmetric).isEmpty())
        _watermarks->topLevelItem(i)->setText(1, _x_metrics->value(fullmetric));
    }
  }
}

void XDocCopySetter::sEditWatermark()
{
  QTreeWidgetItem *cursor = _watermarks->currentItem();
  ParameterList params;
  params.append("watermark",  cursor->text(1));
  params.append("showPrices", (cursor->text(2) == yesStr));

  EditWatermark newdlg(this);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
  {
    cursor->setText(1, newdlg.watermark());
    cursor->setText(2, ((newdlg.showPrices()) ? yesStr : noStr));
  }
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue XDocCopySettertoScriptValue(QScriptEngine *engine, XDocCopySetter* const &item)
{
  return engine->newQObject(item);
}

void XDocCopySetterfromScriptValue(const QScriptValue &obj, XDocCopySetter* &item)
{
  item = qobject_cast<XDocCopySetter*>(obj.toQObject());
}

QScriptValue constructXDocCopySetter(QScriptContext *context,
                                       QScriptEngine  *engine)
{
  QWidget *parent = (qscriptvalue_cast<QWidget*>(context->argument(0)));
  const char *objname = "_xDocCopySetter";
  if (context->argumentCount() > 1)
    objname = context->argument(1).toString().toLatin1().data();
  return engine->toScriptValue(new XDocCopySetter(parent, objname));
}

void setupXDocCopySetter(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags stdflags = QScriptValue::ReadOnly |
                                         QScriptValue::Undeletable;

  qScriptRegisterMetaType(engine, XDocCopySettertoScriptValue,
                          XDocCopySetterfromScriptValue);

  QScriptValue constructor = engine->newFunction(constructXDocCopySetter);
  engine->globalObject().setProperty("XDocCopySetter", constructor, stdflags);

  //constructor.setProperty("ChangeFuture", QScriptValue(engine, XDocCopySetter::ChangeFuture), stdflags);
}
