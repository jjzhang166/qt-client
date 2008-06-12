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

#ifndef addressCluster_h
#define addressCluster_h

#include "OpenMFGWidgets.h"
#include "virtualCluster.h"
#include "xcheckbox.h"
#include "xcombobox.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QGridLayout;
class AddressCluster;

class OPENMFGWIDGETS_EXPORT AddressList : public VirtualList
{
    Q_OBJECT

    friend class AddressCluster;
    friend class AddressInfo;
    friend class AddressSearch;

    public:
	AddressList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
	virtual void sFillList();
	virtual void sSearch(const QString& pTarget);

    protected:
	AddressCluster*	_parent;
	QLabel*		_searchLit;
	QLineEdit*	_search;
	QLabel*		_titleLit;
	QPushButton*	_close;
	QPushButton*	_select;
	XTreeWidget*	_list;

    private:
};

class OPENMFGWIDGETS_EXPORT AddressSearch : public VirtualSearch
{
    Q_OBJECT

    friend class AddressCluster;

    public:
	AddressSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
	virtual void sFillList();

    protected:
	AddressCluster*	_parent;
	XCheckBox*	_searchStreet;
	XCheckBox*	_searchCity;
	XCheckBox*	_searchState;
	XCheckBox*	_searchCountry;
	XCheckBox*	_searchPostalCode;
	XCheckBox*	_searchInactive;
};

class OPENMFGWIDGETS_EXPORT AddressCluster : public VirtualCluster
{
    Q_OBJECT
    Q_PROPERTY(bool     activeVisible 	      	READ activeVisible      WRITE setActiveVisible)
    Q_PROPERTY(QString  fieldNameAddressChange READ fieldNameAddrChange     WRITE setFieldNameAddrChange)
    Q_PROPERTY(QString  fieldNameNumber        READ fieldNameNumber         WRITE setFieldNameNumber)
    Q_PROPERTY(QString  fieldNameActive        READ fieldNameActive         WRITE setFieldNameActive)
    Q_PROPERTY(QString  fieldNameLine1         READ fieldNameLine1          WRITE setFieldNameLine1)
    Q_PROPERTY(QString  fieldNameLine2         READ fieldNameLine2	    WRITE setFieldNameLine2)
    Q_PROPERTY(QString  fieldNameLine3         READ fieldNameLine3          WRITE setFieldNameLine3)
    Q_PROPERTY(QString  fieldNameCity          READ fieldNameCity           WRITE setFieldNameCity)
    Q_PROPERTY(QString  fieldNameState         READ fieldNameState          WRITE setFieldNameState)
    Q_PROPERTY(QString  fieldNamePostalCode    READ fieldNamePostalCode     WRITE setFieldNamePostalCode)
    Q_PROPERTY(QString  fieldNameCountry       READ fieldNameCountry        WRITE setFieldNameCountry)

    friend class AddressInfo;
    friend class AddressList;
    friend class AddressSearch;

    public:
	enum SaveFlags { CHECK = 0, CHANGEONE = 1, CHANGEALL = 2 }; 

	AddressCluster(QWidget*, const char* = 0);
	
	inline virtual bool    activeVisible() const { return _active->isVisible(); };
        inline virtual QString addrChange()  const { return _addrChange->text(); };
	inline virtual QString city()        const { return _city->text(); };
	inline virtual QString country()     const { return _country->currentText(); };
	inline virtual QString description() const { return ""; };
	inline virtual bool    isValid()     const { return _valid; };
	inline virtual QString label()	     const { return _label->text(); };
	inline virtual QString line1()       const { return _addr1->text(); };
	inline virtual QString line2()       const { return _addr2->text(); };
	inline virtual QString line3()       const { return _addr3->text(); };
	inline virtual int     id()	     const { return _id; };
	inline virtual QString notes()	     const { return _notes; };
               virtual QString number()      const { return QString(); };
	inline virtual QString postalCode()  const { return _postalcode->text(); };
	inline virtual QString state()       const { return _state->currentText(); };
	
	// Return data map values
	virtual QString  fieldNameAddrChange()  const { return _fieldNameAddrChange; };
	virtual QString  fieldNameNumber()	const { return _fieldNameNumber; };
	virtual QString  fieldNameActive()	const { return _fieldNameActive; };
	virtual QString  fieldNameLine1() 	const { return _fieldNameLine1; };
	virtual QString  fieldNameLine2()  	const { return _fieldNameLine2; };
	virtual QString  fieldNameLine3()   	const { return _fieldNameLine3; };
	virtual QString  fieldNameCity()   	const { return _fieldNameCity; };
	virtual QString  fieldNamePostalCode()  const { return _fieldNamePostalCode; };
	virtual QString  fieldNameState()  	const { return _fieldNameState; };
	virtual QString  fieldNameCountry() 	const { return _fieldNameCountry; };

    public slots:
	inline virtual void clearExtraClause()	{ };
	inline virtual void setExtraClause(const QString&)  { };
	virtual void        setActiveVisible(const bool p);
        virtual void        setAddrChange(QString p);
	inline virtual void setCity(const QString& p)	{ _city->setText(p); };
	inline virtual void setCountry(const QString& p) { _country->setEditText(p); };
	inline virtual void setDescription(const QString&) { };
	inline virtual void setLabel(const QString& p)  { _label->setText(p); _label->setHidden(_label->text().isEmpty()); };
	inline virtual void setLine1(const QString& p)	{ _addr1->setText(p); };
	inline virtual void setLine2(const QString& p)	{ _addr2->setText(p); };
	inline virtual void setLine3(const QString& p)	{ _addr3->setText(p); };
	inline virtual void setNotes(const QString& p)  { _notes = p; };
	virtual void setNumber(QString p);
	inline virtual void setNumber(const int)	{ };
	inline virtual void setPostalCode(const QString& p) { _postalcode->setText(p); };
	inline virtual void setState(const QString& p)	{ _state->setEditText(p); };
	virtual void	clear();
	virtual void	sEllipses();
	virtual void	sInfo();
	virtual void	sList();
	virtual void	sSearch();
	virtual void	setId(const int);
        virtual int	save(enum SaveFlags = CHECK);
        virtual void    check();

	// Set data map values      
        virtual void 	setDataWidgetMap(XDataWidgetMapper* m);
	virtual void  	setFieldNameAddrChange(QString p)   { _fieldNameAddrChange = p ; };
	virtual void  	setFieldNameNumber(QString p)       { _fieldNameNumber = p ; };
	virtual void  	setFieldNameActive(QString p)       { _fieldNameActive = p ; };
	virtual void  	setFieldNameLine1(QString p)        { _fieldNameLine1 = p ; };
	virtual void  	setFieldNameLine2(QString p)        { _fieldNameLine2 = p ; };
	virtual void  	setFieldNameLine3(QString p)        { _fieldNameLine3 = p ; };
	virtual void  	setFieldNameCity(QString p)         { _fieldNameCity = p ; };
	virtual void  	setFieldNamePostalCode(QString p)   { _fieldNamePostalCode = p ; };
	virtual void  	setFieldNameState(QString p)        { _fieldNameState = p ; };
	virtual void  	setFieldNameCountry(QString p)      { _fieldNameCountry = p ; };

    signals:
	void newId(int);
	void changed();

    protected:
	QString		_query;	
	QString		_extraClause;
        QLineEdit*      _addrChange;
	QLineEdit*      _number;
	QLabel*		_addrLit;
	QLineEdit*	_addr1;
	QLineEdit*	_addr2;
	QLineEdit*	_addr3;
	QLabel*		_cityLit;
	QLineEdit*	_city;
	QLabel*		_stateLit;
	XComboBox*	_state;
	QLabel*		_postalcodeLit;
	QLineEdit*	_postalcode;
	QLabel*		_countryLit;
	XComboBox*	_country;
	QCheckBox*	_active;

    private:
	virtual void	init();
	virtual void	populateStateComboBox();
	virtual void	populateCountryComboBox();
	virtual void	silentSetId(const int);
	int		_id;
	QString		_notes;
	bool		_selected;
	QString		_titlePlural;
	QString		_titleSingular;
	bool		_valid;
	XDataWidgetMapper* _mapper;

	// cached values
	QString         c_number;
	QString		c_addr1;
	QString		c_addr2;
	QString		c_addr3;
	QString		c_city;
	QString		c_state;
	QString		c_postalcode;
	QString		c_country;
	bool		c_active;
	QString		c_notes;

	// data map values
	QString  _fieldNameAddrChange;
	QString  _fieldNameNumber;
	QString  _fieldNameActive;
	QString  _fieldNameLine1;
	QString  _fieldNameLine2;
	QString  _fieldNameLine3;
	QString  _fieldNameCity;
	QString  _fieldNamePostalCode;
	QString  _fieldNameState;
	QString  _fieldNameCountry;
};

#endif
