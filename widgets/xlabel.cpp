/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlabel.h"

#include <QLocale>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QtScript>

#include "format.h"
#include <quuencode.h>
#include "xsqlquery.h"

#define DEBUG false

class XLabelPrivate {
  public:
    XLabelPrivate(XLabel *parent)
      : _precision(0)
    {
      Q_UNUSED(parent);
    }

    QString _image;
    QString _fieldName;
    int     _precision;
};

XLabel::XLabel(QWidget *parent, const char *name) :
  QLabel(parent)
{
  if(name)
    setObjectName(name);

  _data = new XLabelPrivate(this);
}

QString XLabel::fieldName() const { return _data->_fieldName; }
QString XLabel::image()     const { return _data->_image; }
int     XLabel::precision() const { return _data->_precision; }

XLabel::~XLabel()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

void XLabel::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _data->_fieldName, QByteArray("text"), QByteArray("defaultText"));
}

void XLabel::setPrecision(int pPrec)
{
  _data->_precision = pPrec;
}

void XLabel::setDouble(const double pDouble, const int pPrec)
{
  QLabel::setText(formatNumber(pDouble, (pPrec < 0) ? _data->_precision : pPrec));
}

void XLabel::setFieldName(QString p)    { _data->_fieldName = p; }

void XLabel::setImage(QString image)
{
  if (_data->_image == image)
    return;

  _data->_image = image;
  XSqlQuery qry;
  qry.prepare("SELECT image_data "
              "FROM image "
              "WHERE (image_name=:image);");
  qry.bindValue(":image", _data->_image);
  qry.exec();
  if (qry.first())
  {
    QImage img;
    img.loadFromData(QUUDecode(qry.value("image_data").toString()));
    setPixmap(QPixmap::fromImage(img));
    return;
  }
  else if (qry.lastError().type() != QSqlError::NoError)
    QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          qry.lastError().databaseText());
  setPixmap(QPixmap());
}

void XLabel::setPrecision(QValidator *pVal)
{
  if (qobject_cast<QDoubleValidator *>(pVal))
    _data->_precision = (qobject_cast<QDoubleValidator *>(pVal))->decimals();
  else if (qobject_cast<QIntValidator *>(pVal))
    _data->_precision = 0;
  else
    qWarning("XLabel %s::setPrecision called with unexpected validator",
             qPrintable(objectName()));
}

void XLabel::setText(const QVariant &pVariant)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QVariant & = %s)",
           qPrintable(pVariant.toString()));
  if (pVariant.type() == QVariant::Double ||
      pVariant.type() == QVariant::Int)
    QLabel::setText(formatNumber(pVariant.toDouble(), _data->_precision));
  else
    QLabel::setText(pVariant.toString());
}

void XLabel::setText(const char *pText)
{
  if (DEBUG) qDebug("XLabel::setText(const char * = %s)", pText);
  setText(QString(pText));
}

void XLabel::setText(const QString &pText)
{
  if (DEBUG)
    qDebug("XLabel::setText(const QString & = %s)", qPrintable(pText));
  if (_data->_precision == 0)
    QLabel::setText(pText);
  else
  {
    bool ok;
    double val = QLocale().toDouble(pText, &ok);
    if (DEBUG) qDebug("XLabel::setText() %f %d", val, ok);
    if (ok)
      setDouble(val);
    else
      QLabel::setText(pText);
  }
}

double XLabel::toDouble(bool *pIsValid)
{
  return QLocale().toDouble(text(), pIsValid);
}

void XLabel::setTextColor(const QString &pColorName)
{
  QColor c = namedColor(pColorName);
  QPalette p = palette();
  p.setColor(foregroundRole(), c);
  setPalette(p);
}

// script exposure /////////////////////////////////////////////////////////////

QScriptValue constructXLabel(QScriptContext *context, QScriptEngine *engine)
{
#if QT_VERSION > 0x050000
  XLabel *obj = 0;
  if (context->argumentCount() == 1)
    obj = new XLabel(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else if (context->argumentCount() >= 2)
    obj = new XLabel(qobject_cast<QWidget*>(context->argument(0).toQObject()),
                     context->argument(1).toString().toLatin1().data());

  return engine->toScriptValue(obj);
#else
  Q_UNUSED(context); Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupXLabel(QScriptEngine *engine)
{
  if (! engine->globalObject().property("XLabel").isFunction())
  {
    QScriptValue ctor = engine->newFunction(constructXLabel);
    QScriptValue meta = engine->newQMetaObject(&XLabel::staticMetaObject, ctor);

    engine->globalObject().setProperty("XLabel", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
