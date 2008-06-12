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

#ifndef currCluster_h
#define currCluster_h

#include <QWidget>
#include <QDateTime>

class QLabel;
class QGridLayout;
class QDoubleValidator;

#include "OpenMFGWidgets.h"
#include "xcombobox.h"
#include "xlineedit.h"

/*
 There are several places where all we need is a widget that shows a currency
 value.  It has to internally hold a value in the base currency but show it
 converted to local.  To do so it must respond to signals that the currency
 to display in or the exchange rate has changed.
 */
class OPENMFGWIDGETS_EXPORT CurrDisplay : public QWidget
{
    Q_OBJECT

    Q_ENUMS(CurrDisplayFormats)

    Q_PROPERTY(CurrDisplayFormats format    READ format                   WRITE setFormat)
    Q_PROPERTY(bool localControl            READ localControl             WRITE setLocalControl)
    Q_PROPERTY(bool enabled                 READ isEnabled                WRITE setEnabled)
    Q_PROPERTY(QString fieldNameValue       READ fieldNameValue           WRITE setFieldNameValue)
    Q_PROPERTY(double localValue            READ localValue               WRITE setLocalValue)
    

    public:
	CurrDisplay(QWidget * parent, const char* name = 0);
	~CurrDisplay();

        // correspond to format.cpp's monetary scales,
        // which can be overridden in the db via locales
        enum CurrDisplayFormats
        {
          Money, SalesPrice, PurchPrice, ExtPrice, Cost
        };

	virtual bool		isZero()	const;
        virtual inline bool     isEmpty()       const { return (New == _state); };
	static QString		baseCurrAbbr();
	virtual QString		currAbbr()	const;
	static QString		currSymbol(const int);

	virtual inline int	id()		const { return _localId; };
	static int		baseId();
	virtual inline double	baseValue()	const { return _valueBase; };
	virtual inline double	localValue()	const { return _valueLocal; };
        virtual        int      decimals()      const { return _decimals; };
        virtual CurrDisplayFormats format()     const { return _format; };
	virtual inline bool	localControl()	const { return _localControl; };
	virtual inline QDate	effective()	const { return _effective; };
	virtual inline bool	isEnabled()	const { return _valueLocalWidget->isEnabled(); };
	virtual inline bool	isBase() const { return _localId == _baseId; };
	static double   	convert(const int, const int, const double, const QDate&);
        virtual QString         fieldNameValue()   const { return _fieldNameValue; };

    public slots:
	void clear();
	void reset();
	void setId(int);
	void setBaseValue(double);
	void setLocalValue(double);
	virtual void setFormat(CurrDisplayFormats = Money);
	void setLocalControl(bool);
	void setEffective(const QDate&);
	inline void setEnabled(bool B) { _valueLocalWidget->setEnabled(B); };
	void setPaletteForegroundColor ( const QColor & );
	void set(const double, const int, const QDate&, const bool = true);
	void setNA(const bool = true);
        virtual void setFieldNameValue(QString p) { _fieldNameValue = p; };

    signals:
	void idChanged(int);
	void valueBaseChanged(const double);
	void valueLocalChanged(const double);
	void valueChanged();
	void effectiveChanged(const QDate&);
	void noConversionRate();

    protected:
	static QString	_baseAbbr;
	static int	_baseId;
	static int	_baseScale;

	QGridLayout*	_grid;
	int		_localId;
	QDate		_effective;
        int             _decimals;
	CurrDisplayFormats _format;
	double		_valueLocal;
	XLineEdit*	_valueLocalWidget;
	double		_valueBase;
	bool		_localControl;
	int		_localScale;

	// if New then display "" instead of 0
	// if NA* then display N/A instead of 0
	enum State	{ New, NANew, Initialized, NAInit };
	State		_state;
	bool		_baseKnown;
	bool		_localKnown;
        
        QString           _fieldNameValue;
        XDataWidgetMapper *_mapper;
	
    protected slots:
	virtual void sReformat() const;
	virtual void sValueBaseChanged();
	virtual void sValueBaseChanged(double);
	virtual void sValueLocalChanged();
	virtual void sValueLocalChanged(double);
        virtual void setDataWidgetMap(XDataWidgetMapper* m);
};

/*
 The currCluster looks like so:
    +-----------+------------+
    | xcombobox | textfield  |
    |     ABR-S | valueInBase|
    +-----------+------------+

Property descriptions:
    currencyVisible whether the "Currency" label and xcombobox are shown
    baseVisible	  whether the base currency abbreviation and value are shown;
    		  they are always hidden if the xcombobox shows the base
		  currency is selected, but are usually visible if the current
		  currency selected is NOT the base.  If baseVisible is FALSE
		  then the base value is never shown
    format        locale format used to determine the number of decimal places
		  displayed for currency values
    allowNegative whether input may include negative numbers or only >= 0
    localControl  if TRUE then changes to the xcombobox leave the local value
    		  unchanged and reconverts the local value to base; if FALSE
		  then changes to the xcombobox leave the base value alone
		  and converts the base value to local

*/

class OPENMFGWIDGETS_EXPORT CurrCluster : public CurrDisplay
{
    Q_OBJECT
    Q_PROPERTY(bool allowNegative  READ allowNegative WRITE setAllowNegative)
    Q_PROPERTY(bool baseVisible	   READ baseVisible   WRITE setBaseVisible)
    Q_PROPERTY(bool currencyDisabled READ currencyDisabled
						      WRITE setCurrencyDisabled)
    Q_PROPERTY(bool currencyEditable READ currencyEnabled
						      WRITE setCurrencyEnabled)
    Q_PROPERTY(bool currencyEnabled READ currencyEnabled
						      WRITE setCurrencyEnabled)
    Q_PROPERTY(bool currencyVisible READ currencyVisible
						      WRITE setCurrencyVisible)
    Q_PROPERTY(bool enabled        READ isEnabled     WRITE setEnabled)
    Q_PROPERTY(QString fieldNameCurr  READ fieldNameCurr  WRITE setFieldNameCurr);

    public:
	CurrCluster(QWidget * parent, const char* name = 0);
	inline bool	allowNegative() const { return _validator->bottom() < 0; };
	inline bool	baseVisible()	const { return _baseVisible; };
	inline QString	currAbbr()	const { return _currency->currentText(); };
	inline bool	currencyDisabled() const { return ! _currencyEnabled; };
	inline bool	currencyEnabled() const { return _currencyEnabled; };
	inline bool	currencyVisible() const { return !_currency->isHidden(); };
	inline int	id()		const { return _currency->id(); };
	inline bool	isBase()	const { return _currency->id() == _baseId; };
	inline bool	isEnabled()	const { return _valueLocalWidget->isEnabled(); };
        QString         fieldNameCurr() const { return _fieldNameCurr; };

    public slots:
	void clear();
	void set(const double, const int, const QDate&, const bool = true);
	void setAllowNegative(bool);
	void setBaseVisible(bool);
	void setCurrencyDisabled(bool);
	void setCurrencyEditable(bool); // deprecated: use setCurrencyEnabled
	void setCurrencyEnabled(bool);
	void setCurrencyVisible(bool);
	void setEnabled(bool);
	virtual void setFormat(CurrDisplayFormats);
	void setId(int);
	void setPaletteForegroundColor ( const QColor & );
	void sLostFocus();
        void setDataWidgetMap(XDataWidgetMapper* m);
        void setFieldNameCurr(QString p) { _fieldNameCurr = p; };

    signals:
	void lostFocus();

    protected:
	bool	     _baseVisible;
	QDoubleValidator*	_validator;
	bool         _currencyEnabled;
	QLabel*      _valueBaseLit;
	QLabel*      _valueBaseWidget;
	XComboBox*   _currency;

    protected slots:
	virtual void sId(int);
	virtual void sReformat() const;
        
    private:
    	QString _fieldNameCurr;
};

#endif
