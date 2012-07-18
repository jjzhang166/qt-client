/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xmoney.h"
#include "format.h"

#include <QDebug>
#include <QRegExp>

#include "errorReporter.h"
#include "metasql.h"
#include "parameter.h"
#include "xsqlquery.h"

#define DEBUG true


/* alternate implementations:
        store as long character string
        store as long long int and number of places
        store as long int for integer portion, long int for fractional portion,
          and number of places
*/

class xmoneyPrivate {
  public:
    xmoneyPrivate(xmoney *parent, float pamt,
                  QString pcurr = QString::null, int pextraprec = 0)
      : _parent(parent),
        _amount(pamt),
        _extraPlaces(pextraprec)
    {
      // for now, assume base currency unless told otherwise
      if (pcurr.isEmpty() || pcurr == "\"\"")
      {
        XSqlQuery currq("SELECT curr_abbr FROM curr_symbol WHERE curr_base;");
        if (currq.first())
          _currency = currq.value("curr_abbr").toString();
        else
          ErrorReporter::error(QtCriticalMsg, 0, 
                               QT_TRANSLATE_NOOP("xmoney",
                                                 "Error Getting Base Currency"),
                                currq, __FILE__, __LINE__);
      }
      else
        _currency    = pcurr;

      _decimalPlaces = getCurrScale(pcurr) + pextraprec;
      _regExp        = QRegExp(moneyRegExpString);
    }

    static int getCurrScale(QString pcurr)
    {
      int returnVal = 0;
      ParameterList params;
      if (! pcurr.isEmpty())
        params.append("currency", pcurr);
      MetaSQLQuery mql("SELECT getNumScale('MONEY') AS curr_scale, 2 AS seq"
                       "<? if exists('currency') ?>"
                       " UNION "
                       "SELECT curr_scale, 1 FROM curr_symbol"
                       " WHERE (<? value('currency') ?>=curr_abbr)"
                       " ORDER BY seq LIMIT 1"
                       "<? endif ?>"
                         ";");
      XSqlQuery currq = mql.toQuery(params);
      currq.exec();
      if (currq.first())
        returnVal = currq.value("curr_scale").toInt();
      else if (ErrorReporter::error(QtCriticalMsg, 0,
                                    QT_TRANSLATE_NOOP("xmoney",
                                                      "Error Getting Currency"),
                                    currq, __FILE__, __LINE__))
        returnVal = 0;
      else
        ErrorReporter::error(QtCriticalMsg, 0,
                             QT_TRANSLATE_NOOP("xmoney", "Currency Not Found"),
                     QString(QT_TRANSLATE_NOOP("xmoney",
                                               "Could not find the currency %1"))
                                               .arg(pcurr),
                             __FILE__, __LINE__);
      return returnVal;
    }

    QString toDebugString() const
    {
      return QString("xmoney[%1, %2 places with %3 extras]")
                      .arg(_parent->toDbString())
                      .arg(_decimalPlaces).arg(_extraPlaces);
    }

    xmoney  *_parent;
    float    _amount;
    QString  _currency;
    int      _decimalPlaces;
    int      _extraPlaces;

    QRegExp  _regExp;

    static QString moneyRegExpString;
};

QString xmoneyPrivate::moneyRegExpString = QString("\\(([^,)\"]*),([^,)]*)\\)");

xmoney::xmoney(float pamt, QString pcurr, int pextraprec)
  : _data(0)
{
  _data = new xmoneyPrivate(this, pamt, pcurr, pextraprec);
}

xmoney::xmoney(const xmoney &p)
  : _data(0)
{
  _data = new xmoneyPrivate(this, p.amount(), p.currency(), p.extraPlaces());
}

xmoney::xmoney(QVariant pvariant)
  : _data(0)
{
  if (pvariant.type() == QVariant::Double || pvariant.type() == QVariant::Int)
    _data = new xmoneyPrivate(this, pvariant.toDouble());
  else
  {
    // this duplicates functionality in openrpt/common/utils.cpp
    QRegExp tmpre(xmoneyPrivate::moneyRegExpString);
    if (-1 < tmpre.indexIn(pvariant.toString()))
    {
      // we could find absolute number of decimal places by examining cap(1)
      int decimalpos = tmpre.cap(1).indexOf('.');
      int numplaces  = (decimalpos >= 0)
                     ? tmpre.cap(1).size() - decimalpos - 1 : 0;
      int extraplaces= numplaces - _data->getCurrScale(tmpre.cap(2));

      _data = new xmoneyPrivate(this,
                                xtround(tmpre.cap(1).toDouble(), numplaces),
                                tmpre.cap(2), extraplaces);
    }
  }
  if (DEBUG)
    qDebug() << (_data ? _data->toDebugString() : QString("[ _data not set ]"));
}

xmoney::~xmoney()
{
  if (_data)
  {
    delete _data;
    _data = 0;
  }
}

float xmoney::amount() const
{
  return _data->_amount;
}

QString xmoney::currency() const
{
  return _data->_currency;
}

int xmoney::decimalPlaces() const
{
  return _data->_decimalPlaces;
}

int xmoney::extraPlaces() const
{
  return _data->_extraPlaces;
}

int xmoney::getCurrId() const
{
  int returnVal = -1;

  XSqlQuery currq;
  currq.prepare("SELECT curr_id FROM curr_symbol WHERE (curr_abbr=:abbr);");
  currq.bindValue(":abbr", _data->_currency);
  currq.exec();
  if (currq.first())
    returnVal = currq.value("curr_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, 0,
                                QT_TRANSLATE_NOOP("xmoney",
                                                  "Error Getting Currency"),
                                currq, __FILE__, __LINE__))
    returnVal = -1;

  return returnVal;
}

QString xmoney::toDbString() const
{
  return QString("(%1,%2)").arg(formatNumber(amount(), decimalPlaces()),
                                currency().isEmpty() ? "\"\"" : currency());
}
