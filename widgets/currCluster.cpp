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

#include <QDateTime>
#include <QLabel>
#include <QValidator>
#include <QMessageBox>
#include <QRect>
#include <QGridLayout>
#include <QSqlError>

#include "xsqlquery.h"
#include "xcombobox.h"
#include "format.h"

#include <math.h>

#include "currcluster.h"

// d should be the number of places used by the id() currency
#define EPSILON(d) (pow(10.0, -1 * (1 + d + decimals())) * 5)
#define ABS(f)	((f) > 0 ? (f) : 0 - (f))

#define MINWIDTH       100
#define MAXWIDTH       200
#define MAXHEIGHT    32767
#define MAXCURRWIDTH    80	/* max width of _currency combobox */

#define UNKNOWNSTR "?????"

#define DEBUG false

/*
   If there's no conversion rate available for the given currency on the
   given date, then produce an error message.  However, since there are often
   multiple CurrDisplay and/or CurrCluster widgets in the same window, only
   complain once.  When the window is closed and the CurrDisplay/CurrCluster
   widgets are destroyed or when the user switches to another currency or
   to an existing conversion rate, then reset so subsequent errors or repeats
   of the same error can be reported.
*/
#include <QPair>
#include <QMutex>
typedef QMap<QPair<int, QDate>, int> DateToCurrMap;
static DateToCurrMap	errorMap;
static QDate nullDate = QDate();

static QMutex    errorListLock;

/* Report if there is no conversion rate for the currency passed in
   (curr_id in curr_symbol and curr_rate tables).
   curr_id < 0 indicates that we really want to report that no base currency
   has been defined (sNoConversionRate is overloaded to avoid duplicating its
   logic in sNoBase(QWidget*) below).
 */
static void sNoConversionRate(QWidget* parent, const int curr_id, const QDate& effective)
{
    bool displayMsg = false;
    QPair<int, QDate> currDatePair =
			qMakePair(curr_id, curr_id < 0 ? nullDate : effective);

    errorListLock.lock();
    DateToCurrMap::Iterator currMap = errorMap.find(currDatePair);	// int*
    if (currMap == errorMap.end())
    {
	errorMap[currDatePair] = 0;
	currMap = errorMap.find(currDatePair);
    }
    if (currMap.data() <= 0)
    {
	displayMsg = true;
	errorMap[currDatePair] = currMap.data() + 1;
    }
    errorListLock.unlock();

    if (displayMsg)
    {
	if (curr_id >= 0)
	    QMessageBox::critical(parent,
			  QObject::tr("No Currency Exchange Rate"),
			  QObject::tr("This document uses a currency which has "
			     "no valid Exchange Rate.\nPlease review your "
			     "Exchange Rates to correct the problem.\n(%1 on %2)")
			  .arg(curr_id)
			  .arg(effective.toString()));
	else
	    QMessageBox::critical(parent,
			      QObject::tr("No Base Currency"),
			      QObject::tr("No base currency has been defined.\n"
					  "Call your system administrator."));
    }
}

static void sZeroErrorCount(const int curr_id, const QDate& effective)
{
    // see comment on negative curr_id before sNoConversionRate()
    QPair<int, QDate> currDatePair =
			qMakePair(curr_id, curr_id < 0 ? nullDate : effective);

    errorListLock.lock();
    DateToCurrMap::Iterator currMap = errorMap.find(currDatePair);	// int*
    if (currMap != errorMap.end() && *currMap > 0)
	errorMap[currDatePair] = 0;
    errorListLock.unlock();
}

static void sNoBase(QWidget* parent)
{
    sNoConversionRate(parent, -1, nullDate);
}

/* create the following widget for currency entry:
+----------+------------+
|xcombobox |   textfield|
|    ABR-S | valueInBase|
+----------+------------+
   
where xcombobox is a drop-down list of currencies defaulting to base
      textfield is a monetary data entry field checked by a QDoubleValidator
                showing the value in the currency displayed in the xcombobox
      ABR-S     is the currConcat() for the base currency
      valueInBase is a label showing textfield converted to the base currency
ABR-S and valueInBase are hidden if the selected currency IS the base currency

There's a property called localControl that determines whether changing the
xcombobox or the effective date should convert the current base currency to
the local currency or the local currency to the base.

Conversions are made using the current date unless effective is changed.

Beware - there are subtle differences between how _valueLocalWidget and
_valueBaseWidget are handled.  This is because _valueLocalWidget is an XLineEdit
while _valueBaseWidget is a QLabel; they emit different signals.
*/

CurrCluster::CurrCluster(QWidget * parent, const char* name)
    : CurrDisplay(parent, name)
{
    setName("CurrCluster");
    setCaption("CurrCluster");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    _grid->remove(_valueLocalWidget);
    _grid->expand(2, 2);

    _currency = new XComboBox(this);
    //_currency->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _currency->setMaximumSize(MAXCURRWIDTH, MAXHEIGHT);
    _grid->addWidget(_currency, 0, 0);

    _grid->addWidget(_valueLocalWidget, 0, 1);

    _valueBaseLit = new QLabel("", this);
    _valueBaseLit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _valueBaseLit->setAlignment(Qt::AlignRight);
    _grid->addWidget(_valueBaseLit, 1, 0);

    _valueBaseWidget = new QLabel("", this);
    _valueBaseWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _grid->addWidget(_valueBaseWidget, 1, 1);

    setFocusProxy(_valueLocalWidget);
    setFocusPolicy(Qt::StrongFocus);

    _currency->setType(XComboBox::Currencies);

    _validator = new QDoubleValidator(0, +999999999, _localScale, this);
    _valueLocalWidget->setValidator(_validator);
    
    _valueBaseWidget->setAlignment(Qt::AlignRight|Qt::AlignTop);

    setTabOrder(_currency, _valueLocalWidget);
    setTabOrder(_valueLocalWidget, 0);

    connect(_currency, SIGNAL(newID(int)), this, SLOT(sId(int)));
    connect(_valueLocalWidget, SIGNAL(lostFocus()), this, SLOT(sLostFocus()));
    connect(_valueLocalWidget, SIGNAL(textChanged(const QString&)), this, SLOT(sValueLocalChanged()));

    _valueBaseLit->setText(_baseAbbr);

    _valueBaseLit->setHidden(TRUE);
    _valueBaseWidget->setHidden(TRUE);

    clear();

    setEnabled(TRUE);
    setBaseVisible(TRUE);
    setCurrencyEnabled(TRUE);
}

void CurrCluster::clear()
{
    _state = New;
    _valueBaseWidget->clear();
    CurrDisplay::clear();
}

void CurrCluster::setCurrencyVisible(bool showCurrency)
{
    _currency->setHidden(! showCurrency);
}

void CurrCluster::setBaseVisible(bool showBase)
{
    _baseVisible = showBase;
    if (showBase)
    {
	_valueBaseLit->setHidden(_baseId == _currency->id());
	_valueBaseWidget->setHidden(_baseId == _currency->id());
    }
    else
    {
	_valueBaseLit->setHidden(TRUE);
	_valueBaseWidget->setHidden(TRUE);
    }
}

void CurrCluster::setId(int curr_id)
{
    if (_currency->id() != curr_id)
    {
	_currency->setId(curr_id);
	emit idChanged(curr_id);
    }
}

void CurrCluster::sId(int id)
{
    if (_baseVisible)
    {
	_valueBaseLit->setHidden(id == _baseId);
	_valueBaseWidget->setHidden(id == _baseId);
    }

    if (_localControl)
	sValueLocalChanged(_valueLocal);
    else
	sValueBaseChanged(_valueBase);
    sReformat();
    emit idChanged(id);
}

void CurrCluster::sLostFocus()
{
  _valueLocal = _valueLocalWidget->toDouble();
  sReformat();
  emit(lostFocus());
}

void CurrCluster::sReformat() const
{
    if (DEBUG)
      qDebug("CC %s::sReformat() entered with _state %d, _format %d",
             qPrintable(objectName()), _state, _format);

    QString na = tr("N/A");
    QString unknown = tr(UNKNOWNSTR);
    QString base = "";
    QString local = "";

    switch (_state)
    {
      case New:
	base = local = "";
	break;
      case NANew:
	base = local = na;
	break;
      case Initialized:
	if (isZero())
          switch (_format)
          {
            case SalesPrice:
              base = local = formatSalesPrice(0.0, id()); break;
            case PurchPrice:
              base = local = formatPurchPrice(0.0, id()); break;
            case ExtPrice:
              base = local = formatExtPrice(0.0, id());   break;
            case Cost:
              base = local = formatCost(0.0, id());       break;
            case Money:
            default:
              base = local = formatMoney(0.0, id());      break;
          }
	else
	{
          if (_baseKnown)
            switch (_format)
            {
              case SalesPrice:
                base = formatSalesPrice(_valueBase, id()); break;
              case PurchPrice:
                base = formatPurchPrice(_valueBase, id()); break;
              case ExtPrice:
                base = formatExtPrice(_valueBase, id());   break;
              case Cost:
                base = formatCost(_valueBase, id());       break;
              case Money:
              default:
                base = formatMoney(_valueBase, id());      break;
            }
          else
            base = unknown;

          if (_localKnown)
            switch (_format)
            {
              case SalesPrice:
                local = formatSalesPrice(_valueLocal, id()); break;
              case PurchPrice:
                local = formatPurchPrice(_valueLocal, id()); break;
              case ExtPrice:
                local = formatExtPrice(_valueLocal, id());   break;
              case Cost:
                local = formatCost(_valueLocal, id());       break;
              case Money:
              default:
                local = formatMoney(_valueLocal, id());      break;
            }
          else
            local = unknown;
	}
	break;
      case NAInit:
	if (isZero())
	  base = local = na;
	else
	{
          switch (_format)
          {
            case SalesPrice:
              base  = formatSalesPrice(_valueBase,  id()); break;
              local = formatSalesPrice(_valueLocal, id()); break;
            case PurchPrice:
              base  = formatPurchPrice(_valueBase,  id()); break;
              local = formatPurchPrice(_valueLocal, id()); break;
            case ExtPrice:
              base  = formatExtPrice(_valueBase,  id());   break;
              local = formatExtPrice(_valueLocal, id());   break;
            case Cost:
              base  = formatCost(_valueBase,  id());       break;
              local = formatCost(_valueLocal, id());       break;
            case Money:
            default:
              base  = formatMoney(_valueBase,  id());      break;
              local = formatMoney(_valueLocal, id());      break;
          }
	}
	break;
      default:
	break;
    }
    _valueLocalWidget->setText(local);
    _valueBaseWidget->setText(base);
}

void CurrCluster::set(const double newValue, const int newId, const QDate& newDate, const bool isBase)
{
    bool emitId = false;
    bool emitDate = false;

    if (newId != id())
    {
	emitId = true;
	_currency->setId(newId);
    }

    if (newDate != _effective)
    {
	emitDate = true;
	_effective = newDate;
    }

    if (isBase)
	setBaseValue(newValue);
    else
	setLocalValue(newValue);

    if (emitId)
	emit idChanged(newId);

    if (emitDate)
	emit effectiveChanged(newDate);
}

void CurrCluster::setAllowNegative(bool value)
{
    if (value)
	_validator->setBottom(0 - _validator->top());
    else
	_validator->setBottom(0);
}

void CurrCluster::setFormat(CurrDisplayFormats pFormat)
{

  CurrDisplay::setFormat(pFormat);
  _validator->setDecimals(_decimals + _localScale);
  if (DEBUG)
    qDebug("%s::setFormat() returning with _validator set to %d",
           qPrintable(objectName()), _validator->decimals());
}

void CurrCluster::setEnabled(bool newValue)
{
    CurrDisplay::setEnabled(newValue);
    _currency->setEnabled(newValue && currencyEnabled());
}

void CurrCluster::setPaletteForegroundColor(const QColor & newColor)
{
    CurrDisplay::setPaletteForegroundColor(newColor);
    _currency->setPaletteForegroundColor(newColor);
    _valueBaseLit->setPaletteForegroundColor(newColor);
    _valueBaseWidget->setPaletteForegroundColor(newColor);
}

// deprecated
void CurrCluster::setCurrencyEditable(bool newValue)
{
  setCurrencyEnabled(newValue);
}

void CurrCluster::setCurrencyEnabled(bool newValue)
{
    _currencyEnabled = newValue;
    _currency->setEnabled(newValue && _valueLocalWidget->isEnabled());
}

void CurrCluster::setCurrencyDisabled(bool newValue)
{
    setCurrencyEnabled(! newValue);
}

void CurrCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(_currency, _fieldNameCurr);
  m->addMapping(this, _fieldNameValue, QByteArray("localValue"));
  _mapper=m;
}

///////////////////////////////

int	CurrDisplay::_baseId	= -1;
QString	CurrDisplay::_baseAbbr	= QString();
int     CurrDisplay::_baseScale	= 2;

int CurrDisplay::baseId()
{
  if (_baseId <= 0)
  {
    XSqlQuery baseQuery;
    baseQuery.prepare("SELECT curr_id, curr_symbol " //currConcat(curr_id) AS curr_symbol "
		      "FROM curr_symbol "
		      "WHERE curr_base = TRUE;");
    baseQuery.exec();
    if (baseQuery.first())
    {
	_baseId = baseQuery.value("curr_id").toInt();
	_baseAbbr = baseQuery.value("curr_symbol").toString();
    }
    else if (baseQuery.lastError().number() == 2000 /* NOT FOUND */)
    {
	sNoBase(0);
    }
    else if (baseQuery.lastError().type() != QSqlError::None)
    {
	QMessageBox::critical(0, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      baseQuery.lastError().databaseText());
    }
  // TODO: add # decimal places to the currency descriptions
    _baseScale = 2;
  }

  return _baseId;
}

QString CurrDisplay::baseCurrAbbr()
{
  (void)baseId();
  return _baseAbbr;
}

CurrDisplay::CurrDisplay(QWidget * parent, const char* name)
    : QWidget(parent, name)
{
    setName("CurrDisplay");
    setCaption("CurrDisplay");

    _grid = new QGridLayout(this);
    _grid->setMargin(2);
    _grid->setSpacing(2);

    _valueLocalWidget = new XLineEdit(this);
    _valueLocalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _valueLocalWidget->setMinimumWidth(QFontMetrics(QFont()).width("99999.9999"));
    _grid->addWidget(_valueLocalWidget, 0, 0);

    _valueLocalWidget->setAlignment(Qt::AlignRight|Qt::AlignTop);

    setLayout(_grid);
    connect(this, SIGNAL(valueLocalChanged(const double)),
						this, SIGNAL(valueChanged()));
    connect(this, SIGNAL(valueBaseChanged(const double)),
						this, SIGNAL(valueChanged()));
    _localId = baseId();
    _localScale = _baseScale;

    clear();
    _effective = QDate().currentDate();
    _decimals = 0;
    _format = Money;

    setEnabled(FALSE);
    setLocalControl(TRUE);
    setMinimumSize(MINWIDTH, 0);
    setMaximumSize(MAXWIDTH, MAXHEIGHT);
    
    _mapper = new XDataWidgetMapper(this);
}

CurrDisplay::~CurrDisplay()
{
    sZeroErrorCount(id(), effective());
    sZeroErrorCount(-1, effective());
}


void CurrDisplay::clear()
{
    _state = New;
    _valueBase = 0;
    _valueLocal = 0;
    _baseKnown = false;
    _localKnown = false;
    _valueLocalWidget->clear();
    emit valueChanged();
}

void CurrDisplay::reset()
{
    clear();
    setId(_baseId);
    setEffective(QDate().currentDate());
}

void CurrDisplay::setFormat(CurrDisplayFormats pFormat)
{
  if (DEBUG)
    qDebug("%s::setFormat(%d)", qPrintable(objectName()), pFormat);

  int precision = _decimals + _localScale;
  _format = pFormat;
  switch (pFormat)
  {
    case SalesPrice:
      precision = decimalPlaces("salesprice"); break;
    case PurchPrice:
      precision = decimalPlaces("purchprice"); break;
    case ExtPrice:
      precision = decimalPlaces("extprice"); break;
    case Cost:
      precision = decimalPlaces("cost"); break;
    case Money:
    default:
      precision = decimalPlaces("curr"); break;
  }

  _decimals = precision - _localScale;

  if (DEBUG)
    qDebug("%s::setFormat(%d) returning with _format %d, _decimals %d",
           qPrintable(objectName()), pFormat, _format, _decimals);
}

void CurrDisplay::setLocalControl(bool newValue)
{
    _localControl = newValue;
}

void CurrDisplay::sReformat() const
{
    if (DEBUG)
      qDebug("CD %s::sReformat() entered with _state %d, _format %d",
             qPrintable(objectName()), _state, _format);

    QString na = tr("N/A");
    QString unknown = tr(UNKNOWNSTR);
    QString local = "";

    switch (_state)
    {
      case New:
	local = "";
	break;
      case NANew:
	local = na;
	break;
      case Initialized:
	if (isZero())
          switch (_format)
          {
            case SalesPrice:
              local = formatSalesPrice(0.0, id()); break;
            case PurchPrice:
              local = formatPurchPrice(0.0, id()); break;
            case ExtPrice:
              local = formatExtPrice(0.0, id());   break;
            case Cost:
              local = formatCost(0.0, id());       break;
            case Money:
            default:
              local = formatMoney(0.0, id());      break;
          }
	else if (_localKnown)
          switch (_format)
          {
            case SalesPrice:
              local = formatSalesPrice(_valueLocal, id()); break;
            case PurchPrice:
              local = formatPurchPrice(_valueLocal, id()); break;
            case ExtPrice:
              local = formatExtPrice(_valueLocal, id());   break;
            case Cost:
              local = formatCost(_valueLocal, id());       break;
            case Money:
            default:
              local = formatMoney(_valueLocal, id());      break;
          }
        else
          local = unknown;
	break;
      case NAInit:
	if (isZero())
	  local = na;
	else
          switch (_format)
          {
            case SalesPrice:
              local = formatSalesPrice(_valueLocal, id()); break;
            case PurchPrice:
              local = formatPurchPrice(_valueLocal, id()); break;
            case ExtPrice:
              local = formatExtPrice(_valueLocal, id());   break;
            case Cost:
              local = formatCost(_valueLocal, id());       break;
            case Money:
            default:
              local = formatMoney(_valueLocal, id());      break;
          }
	break;
      default:
	break;
    }
    _valueLocalWidget->setText(local);
}

void CurrDisplay::setId(int curr_id)
{
    // ToDo - set _localScale when the currency id changes
    if (_localId != curr_id)
    {
	_localId = curr_id;
	if (_localControl)
	    sValueLocalChanged(_valueLocal);
	else
	    sValueBaseChanged(_valueBase);
	sReformat();
	emit idChanged(curr_id);
    }
}

void CurrDisplay::setBaseValue(double newValue)
{
    _state = (_state == NANew || _state == NAInit) ? NAInit : Initialized;

    // either the value has significantly changed or we're explicitly setting 0
    if (ABS(_valueBase - newValue) > EPSILON(_baseScale) ||
        newValue < EPSILON(_baseScale))
    {
	_valueBase = newValue;
	_baseKnown = true;
	emit valueBaseChanged(_valueBase);
	sValueBaseChanged(_valueBase);
	sReformat();
    }
}

void CurrDisplay::setLocalValue(double newValue)
{
    _state = (_state == NANew || _state == NAInit) ? NAInit : Initialized;

    // either the value has significantly changed or we're explicitly setting 0
    if (ABS(_valueLocal - newValue) > EPSILON(_localScale) ||
	newValue < EPSILON(_localScale))
    {
	_valueLocal = newValue;
	_localKnown = true;
	emit valueLocalChanged(_valueLocal);
	sValueLocalChanged(_valueLocal);
	sReformat();
    }
    
    if (_mapper->model() &&
        _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toDouble() != _valueLocal)
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), _valueLocal);
}

void CurrDisplay::sValueBaseChanged()
{
    sValueBaseChanged(_valueBase);
}

void CurrDisplay::sValueBaseChanged(double newValue)
{
    double oldLocal = _valueLocal;
    if (ABS(newValue) < EPSILON(_baseScale) || _effective.isNull())
    {
	if (ABS(_valueLocal) >= EPSILON(_localScale))
	{
	    _valueLocal = 0;
	    _valueLocalWidget->clear();
	}
    }
    else
    {
	XSqlQuery convertVal;
	convertVal.prepare("SELECT currToLocal(:curr_id, :value, :date) "
			     " AS localValue;");
	convertVal.bindValue(":curr_id", id());
	convertVal.bindValue(":value", newValue);
	convertVal.bindValue(":date", _effective);
	convertVal.exec();
	if (convertVal.first())
	{
	    _valueLocal = convertVal.value("localValue").toDouble();
	    sZeroErrorCount(id(), effective());
	    _localKnown = true;
	}
	else if (convertVal.lastError().type() != QSqlError::None)
	{
	    if (convertVal.lastError().databaseText().contains("No exchange rate"))
	    {
		emit noConversionRate();
		sNoConversionRate(this, id(), effective());
	    }
	    else
	      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
				    convertVal.lastError().databaseText());
	    _localKnown = false;
	}
    }
    if (ABS(oldLocal - _valueLocal) > EPSILON(_localScale))
	emit valueLocalChanged(_valueLocal);
}

void CurrDisplay::sValueLocalChanged()
{
    if (_valueLocalWidget->text().stripWhiteSpace().isEmpty())
      _state = New;
    else
      _state = (_state == NANew || _state == NAInit) ? NAInit : Initialized;

    if (_valueLocalWidget->text() != tr(UNKNOWNSTR))
    {
	_valueLocal = _valueLocalWidget->toDouble();
	_localKnown = true;
	sValueLocalChanged(_valueLocal);
    }
}

void CurrDisplay::sValueLocalChanged(double newValue)
{
    double oldBase = _valueBase;
    if (ABS(newValue) < EPSILON(_localScale) || _effective.isNull())
    {
	if (ABS(_valueBase) >= EPSILON(_baseScale))
	{
	    _valueBase = 0;
	    _baseKnown = true;
	}
    }
    else
    {
	XSqlQuery convertVal;
	convertVal.prepare("SELECT currToBase(:curr_id, :value, :date) "
			   " AS baseValue;");
	convertVal.bindValue(":curr_id", id());
	convertVal.bindValue(":value", newValue);
	convertVal.bindValue(":date", _effective);
	convertVal.exec();
	if (convertVal.first())
	{
	    _valueBase = convertVal.value("baseValue").toDouble();
	      sZeroErrorCount(id(), effective());
	      _baseKnown = true;
	}
	else if (convertVal.lastError().type() != QSqlError::None)
	{
	    if (convertVal.lastError().databaseText().contains("No exchange rate"))
	    {
		emit noConversionRate();
		sNoConversionRate(this, id(), effective());
	    }
	    else
	      QMessageBox::critical(this, tr("A System Error occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
				    convertVal.lastError().databaseText());
	    _baseKnown = false;
	}
    }
    if (ABS(oldBase - _valueBase) > EPSILON(_baseScale))
	emit valueBaseChanged(_valueBase);
}

void CurrDisplay::set(const double newValue, const int newId, const QDate& newDate, const bool isBase)
{
    bool emitId = false;
    bool emitDate = false;

    if (newId != id())
    {
	emitId = true;
	_localId = newId;
    }

    if (newDate != _effective)
    {
	emitDate = true;
	_effective = newDate;
    }

    if (isBase)
	setBaseValue(newValue);
    else
	setLocalValue(newValue);

    if (emitId)
	emit idChanged(newId);

    if (emitDate)
	emit effectiveChanged(newDate);
}

void CurrDisplay::setEffective(const QDate& newValue)
{
//  qWarning(QString("CurrDisplay::setEffective(%1)").arg(newValue.toString()));
    if (! newValue.isNull())
    {
	_effective = newValue;
	if (_localControl)
	    sValueLocalChanged(_valueLocal);
	else
	    sValueBaseChanged(_valueBase);
	sReformat();
	emit effectiveChanged(newValue);
    }
}

bool CurrDisplay::isZero() const
{
    if (_localControl)
	return ABS(_valueLocal) < EPSILON(_localScale);
    else
	return ABS(_valueBase) < EPSILON(_baseScale);
}

// cache it?
QString	CurrDisplay::currAbbr() const
{
    QString returnValue("");

    XSqlQuery getAbbr;
    getAbbr.prepare("SELECT currConcat(:curr_id) AS currConcat;");
    getAbbr.bindValue(":curr_id", id());
    getAbbr.exec();
    if (getAbbr.first())
	returnValue = getAbbr.value("currConcat").toString();
    else if (getAbbr.lastError().type() != QSqlError::None)
	QMessageBox::critical(0, tr("A System Error occurred at %1::%2.")
			      .arg(__FILE__)
			      .arg(__LINE__),
			      getAbbr.lastError().databaseText());
    return returnValue;
}

QString CurrDisplay::currSymbol(const int pid)
{
  XSqlQuery symq;
  symq.prepare("SELECT curr_symbol FROM curr_symbol WHERE (curr_id=:id);");
  symq.bindValue(":id", pid);
  if (symq.first())
      return symq.value("curr_symbol").toString();
  else if (symq.lastError().type() != QSqlError::None)
      QMessageBox::critical(0, tr("A System Error occurred at %1::%2.")
						  .arg(__FILE__).arg(__LINE__),
			    symq.lastError().databaseText());
  return "";
}

void CurrDisplay::setPaletteForegroundColor(const QColor & newColor)
{
    _valueLocalWidget->setPaletteForegroundColor(newColor);
}

void CurrDisplay::setNA(bool isNA)
{
    switch (_state)
    {
      case New:
      case NANew:
	_state = (isNA) ? NANew : New;
	break;

      case NAInit:
      case Initialized:
	_state = (isNA) ? NAInit : Initialized;
	break;
    }
    if (isNA)
      setLocalValue(0);
}

double CurrDisplay::convert(const int from, const int to, const double amount, const QDate& date)
{
  if (from == to)
    return amount;

  XSqlQuery convq;
  convq.prepare("SELECT currToCurr(:from, :to, :amount, :date) AS result;");
  convq.bindValue(":from",   from);
  convq.bindValue(":to",     to);
  convq.bindValue(":amount", amount);
  convq.bindValue(":date",   date);
  convq.exec();
  if (convq.first())
    return convq.value("result").toDouble();
  else if (convq.lastError().type() != QSqlError::None)
  {
    if (convq.lastError().databaseText().contains("No exchange rate"))
      sNoConversionRate(0, from, date);
    else
      QMessageBox::critical(0, tr("A System Error occurred at %1::%2.")
			    .arg(__FILE__)
			    .arg(__LINE__),
			    convq.lastError().databaseText());
  }
  return 0.0;
}

void CurrDisplay::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldNameValue, QByteArray("localValue"));
  _mapper=m;
}
