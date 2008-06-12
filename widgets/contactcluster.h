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

#ifndef contactCluster_h
#define contactCluster_h

#include "OpenMFGWidgets.h"
#include "virtualCluster.h"
#include "xcombobox.h"
#include "addresscluster.h"
#include "crmacctcluster.h"
#include "xcheckbox.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QGridLayout;
class ContactCluster;

class OPENMFGWIDGETS_EXPORT ContactList : public VirtualList
{
    Q_OBJECT

    friend class ContactCluster;
    friend class ContactInfo;
    friend class ContactSearch;

    public:
      ContactList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
      virtual void sFillList();
      virtual void sSearch(const QString&);

    protected:
      ContactCluster* _parent;
};

class OPENMFGWIDGETS_EXPORT ContactSearch : public VirtualSearch
{
    Q_OBJECT

    friend class ContactCluster;

    public:
	ContactSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
	virtual void sFillList();

    protected:
	ContactCluster*	_parent;
	XCheckBox*	_searchFirst;
	XCheckBox*	_searchLast;
	XCheckBox*	_searchTitle;
	XCheckBox*	_searchCRMAcct;
	XCheckBox*	_searchPhones;
	XCheckBox*	_searchEmail;
	XCheckBox*	_searchWebAddr;
	XCheckBox*	_searchInactive;

};

class OPENMFGWIDGETS_EXPORT ContactCluster : public VirtualCluster
{
    Q_OBJECT
    
    //Q_PROPERTY(bool     numberVisible         READ numberVisible       		WRITE setNumberVisible)
    Q_PROPERTY(bool     accountVisible        READ accountVisible       	WRITE setAccountVisible)
    Q_PROPERTY(bool     activeVisible         READ activeVisible        	WRITE setActiveVisible)
    Q_PROPERTY(bool     addressVisible        READ addressVisible       	WRITE setAddressVisible)
    Q_PROPERTY(bool     emailVisible          READ emailVisible         	WRITE setEmailVisible)
    Q_PROPERTY(bool     initialsVisible       READ initialsVisible      	WRITE setInitialsVisible)
    Q_PROPERTY(bool     minimalLayout         READ minimalLayout        	WRITE setMinimalLayout)
    Q_PROPERTY(bool     phonesVisible         READ phonesVisible        	WRITE setPhonesVisible)
    Q_PROPERTY(bool     webaddrVisible        READ webaddrVisible       	WRITE setWebaddrVisible)
    Q_PROPERTY(QString  fieldNameChange       READ fieldNameChange      	WRITE setFieldNameChange)   
    Q_PROPERTY(QString  fieldNameNumber       READ fieldNameNumber      	WRITE setFieldNameNumber)   
    Q_PROPERTY(QString  fieldNameActive       READ fieldNameActive      	WRITE setFieldNameActive)
    Q_PROPERTY(QString  fieldNameCrmAccount   READ fieldNameCrmAccount      	WRITE setFieldNameCrmAccount)
    Q_PROPERTY(QString  fieldNameHonorific    READ fieldNameHonorific   	WRITE setFieldNameHonorific)
    Q_PROPERTY(QString  fieldNameFirst        READ fieldNameFirst       	WRITE setFieldNameFirst)
    Q_PROPERTY(QString  fieldNameLast         READ fieldNameLast        	WRITE setFieldNameLast)
    Q_PROPERTY(QString  fieldNameInitials     READ fieldNameInitials    	WRITE setFieldNameInitials)
    Q_PROPERTY(QString  fieldNameTitle        READ fieldNameTitle       	WRITE setFieldNameTitle)
    Q_PROPERTY(QString  fieldNamePhone        READ fieldNamePhone       	WRITE setFieldNamePhone)
    Q_PROPERTY(QString  fieldNamePhone2       READ fieldNamePhone2      	WRITE setFieldNamePhone2)
    Q_PROPERTY(QString  fieldNameFax          READ fieldNameFax         	WRITE setFieldNameFax)
    Q_PROPERTY(QString  fieldNameEmailAddress READ fieldNameEmailAddress 	WRITE setFieldNameEmailAddress)
    Q_PROPERTY(QString  fieldNameWebAddress   READ fieldNameWebAddress  	WRITE setFieldNameWebAddress)
    Q_PROPERTY(QString  fieldNameAddressChange READ fieldNameAddrChange      	WRITE setFieldNameAddrChange)
    Q_PROPERTY(QString  fieldNameAddressNumber READ fieldNameAddrNumber      	WRITE setFieldNameAddrNumber)   
    Q_PROPERTY(QString  fieldNameLine1        READ fieldNameLine1          	WRITE setFieldNameLine1)
    Q_PROPERTY(QString  fieldNameLine2        READ fieldNameLine2	    	WRITE setFieldNameLine2)
    Q_PROPERTY(QString  fieldNameLine3        READ fieldNameLine3          	WRITE setFieldNameLine3)
    Q_PROPERTY(QString  fieldNameCity         READ fieldNameCity           	WRITE setFieldNameCity)
    Q_PROPERTY(QString  fieldNameState        READ fieldNameState          	WRITE setFieldNameState)
    Q_PROPERTY(QString  fieldNamePostalCode   READ fieldNamePostalCode     	WRITE setFieldNamePostalCode)
    Q_PROPERTY(QString  fieldNameCountry      READ fieldNameCountry        	WRITE setFieldNameCountry)
    Q_PROPERTY(QString	number		      READ number		        WRITE setNumber)

    friend class ContactInfo;
    friend class ContactList;
    friend class ContactSearch;

    public:
	// AccountLimits may be ORed together
	enum AccountLimits { Employee = 1,	Customer =  2,	Vendor     = 4,
			     Partner  = 8,	Prospect = 16,	Competitor = 32};

	ContactCluster(QWidget*, const char* = 0);
	virtual QString name() const;
	virtual QString number()		const { return _number->text(); };
	inline virtual bool    numberVisible()  const { return _number->isVisible(); };
	inline virtual bool    active()		const { return _active->isChecked(); };
	inline virtual bool    activeVisible()  const { return _active->isVisible(); };
	inline virtual bool    accountVisible()	const { return _crmAcct->isVisible(); };
	inline virtual bool    addressVisible() const { return _address->isVisible(); };
	inline virtual AddressCluster* addressWidget() const { return _address; };
	inline virtual int     addressId()	const { return _address->id(); };
        inline virtual QString change()         const { return _change->text(); };
	inline virtual int     crmAcctId()	const { return _crmAcct->id(); };
	inline virtual QString description()    const { return ""; };
	inline virtual QString emailAddress()	const { return _email->text(); };
	inline virtual bool    emailVisible()   const { return _email->isVisible(); };
	inline virtual QString fax()		const { return _fax->text(); };
	inline virtual QString first()		const { return _first->text(); };
	inline virtual QString honorific()	const { return _honorific->currentText(); };
	inline virtual int     id()	        const { return _id; };
	inline virtual QString initials()	const { return _initials->text(); };
	inline virtual bool   initialsVisible()	const { return _initials->isVisible(); };
	inline virtual bool    isValid()        const { return _valid; };
	inline virtual QString label()	        const { return _label->text(); };
	inline virtual QString last()		const { return _last->text(); };
	inline virtual bool    minimalLayout()  const { return _minimalLayout; };
	inline virtual QString notes()	        const { return _notes; };
	inline virtual QString phone()		const { return _phone->text(); };
	inline virtual QString phone2()		const { return _phone2->text(); };
	inline virtual bool    phonesVisible()  const { return _phone->isVisible(); };
        inline virtual QString title()          const { return _title->text(); };
	inline virtual int     searchAcct()	const { return _searchAcctId; };
	inline virtual QString webAddress()	const { return _webaddr->text(); };
	inline virtual bool    webaddrVisible() const { return _webaddr->isVisible(); };

	//Return Data Mapping values
  	virtual QString  fieldNameChange()        const { return _fieldNameChange; };
	virtual QString  fieldNameNumber()        const { return _fieldNameNumber; };
        virtual QString  fieldNameActive()        const { return _fieldNameActive; };
        virtual QString  fieldNameCrmAccount()    const { return _fieldNameCrmAccount; };
        virtual QString  fieldNameHonorific()     const { return _fieldNameHonorific; };
        virtual QString  fieldNameFirst()         const { return _fieldNameFirst; };
        virtual QString  fieldNameLast()          const { return _fieldNameLast; };
        virtual QString  fieldNameInitials()      const { return _fieldNameInitials; };
        virtual QString  fieldNameTitle()         const { return _fieldNameTitle; };
        virtual QString  fieldNamePhone()         const { return _fieldNamePhone; };
        virtual QString  fieldNamePhone2()        const { return _fieldNamePhone2; };
        virtual QString  fieldNameFax()           const { return _fieldNameFax; };
        virtual QString  fieldNameEmailAddress()  const { return _fieldNameEmailAddress; };
        virtual QString  fieldNameWebAddress()    const { return _fieldNameWebAddress; };
	virtual QString  fieldNameAddrChange() 	  const { return _fieldNameAddrChange; };
	virtual QString  fieldNameAddrNumber() 	  const { return _fieldNameAddrNumber; };
	virtual QString  fieldNameLine1() 	  const { return _fieldNameLine1; };
	virtual QString  fieldNameLine2()  	  const { return _fieldNameLine2; };
	virtual QString  fieldNameLine3()   	  const { return _fieldNameLine3; };
	virtual QString  fieldNameCity()   	  const { return _fieldNameCity; };
	virtual QString  fieldNameState()  	  const { return _fieldNameState; };
	virtual QString  fieldNamePostalCode()    const { return _fieldNamePostalCode; };
	virtual QString  fieldNameCountry() 	  const { return _fieldNameCountry; };

    public slots:
    	virtual void setNumber(QString p);
	inline virtual void clearExtraClause()	{ };
	inline virtual void setExtraClause(const QString&) { };
	inline virtual void setAddress(const int p)     { _address->setId(p); };
        virtual void        setChange(QString p);
	inline virtual void setDescription(const QString&) { };
        inline virtual void setEmailAddress(const QString& p) { _email->setText(p); };
	inline virtual void setFax(const QString& p)	{ _fax->setText(p); };
	inline virtual void setFirst(const QString& p)	{ _first->setText(p); };
	inline virtual void setHonorific(const QString& p) { _honorific->setEditText(p); };
	inline virtual void setLabel(const QString& p)  { _label->setText(p); _label->setHidden(_label->text().isEmpty()); };
	inline virtual void setLast(const QString& p)	{ _last->setText(p); };
	inline virtual void setNotes(const QString& p)  { _notes = p; };
	inline virtual void setPhone(const QString& p)	{ _phone->setText(p); };
	inline virtual void setPhone2(const QString& p)	{ _phone2->setText(p); };
	inline virtual void setTitle(const QString& p)	{ _title->setText(p); };
        inline virtual void setWebAddress(const QString& p) { _webaddr->setText(p); };

	virtual void	clear();
	virtual void	check();
	virtual void	sEllipses();
	virtual void	sInfo();
	virtual void	sList();
	virtual void	sSearch();
	virtual int	save(AddressCluster::SaveFlags = AddressCluster::CHECK);
	virtual void	setAccount(const int);
	virtual void    setNumberVisible(const bool);
	virtual void	setAccountVisible(const bool);
	virtual void	setActiveVisible(const bool);
	virtual void	setAddressVisible(const bool);
	virtual void	setEmailVisible(const bool);
	virtual void	setId(const int);
	virtual void	setInitialsVisible(const bool);
	virtual void	setMinimalLayout(const bool);
	virtual void	setName(const QString& p);
	virtual void	setPhonesVisible(const bool);
	virtual void	setSearchAcct(const int);
	virtual void	setWebaddrVisible(const bool);
	
        //Set Data Mapping 
	virtual void setDataWidgetMap(XDataWidgetMapper* m);
        virtual void setFieldNameChange(QString p)	  { _fieldNameChange = p ; };
	virtual void setFieldNameNumber(QString p)	  { _fieldNameNumber = p ; };
        virtual void setFieldNameActive(QString p)        { _fieldNameActive = p ; };
	virtual void setFieldNameCrmAccount(QString p)    { _fieldNameCrmAccount = p ; };
        virtual void setFieldNameHonorific(QString p)     { _fieldNameHonorific = p ; };
        virtual void setFieldNameFirst(QString p)         { _fieldNameFirst = p ; };
        virtual void setFieldNameLast(QString p)          { _fieldNameLast = p ; };
        virtual void setFieldNameInitials(QString p)      { _fieldNameInitials = p ; };
        virtual void setFieldNameTitle(QString p)         { _fieldNameTitle = p ; };
        virtual void setFieldNamePhone(QString p)         { _fieldNamePhone = p ; };
        virtual void setFieldNamePhone2(QString p)        { _fieldNamePhone2 = p ; };
        virtual void setFieldNameFax(QString p)           { _fieldNameFax = p ; };
        virtual void setFieldNameEmailAddress(QString p)  { _fieldNameEmailAddress = p ; };
        virtual void setFieldNameWebAddress(QString p)    { _fieldNameWebAddress = p ; };
	virtual void setFieldNameAddrChange(QString p)    { _fieldNameAddrChange = p ; };
	virtual void setFieldNameAddrNumber(QString p)    { _fieldNameAddrNumber = p ; };
	virtual void setFieldNameLine1(QString p)         { _fieldNameLine1 = p ; };
	virtual void setFieldNameLine2(QString p)         { _fieldNameLine2 = p ; };
	virtual void setFieldNameLine3(QString p)         { _fieldNameLine3 = p ; };
	virtual void setFieldNameCity(QString p)          { _fieldNameCity = p ; };
	virtual void setFieldNamePostalCode(QString p)    { _fieldNamePostalCode = p ; };
	virtual void setFieldNameState(QString p)         { _fieldNameState = p ; };
	virtual void setFieldNameCountry(QString p)       { _fieldNameCountry = p ; };
        
    private slots:
        void sCheck();

    signals:
	void changed();
	void newId(int);

    protected:
	QHBoxLayout*	_nameBox;
	QHBoxLayout*	_titleBox;
	QHBoxLayout*	_buttonBox;
        QLineEdit*      _change;
	QLineEdit*	_number;
	QLabel*		_firstLit;
	XComboBox*	_honorific;
	QLabel*		_numberLit;
	QLineEdit*	_first;
	QLabel*		_lastLit;
	QLineEdit*	_last;
	QLabel*		_initialsLit;
	QLineEdit*	_initials;
	QLabel*		_crmAcctLit;
	CRMAcctCluster*	_crmAcct;
	QLabel*		_titleLit;
	QLineEdit*	_title;
	QLabel*		_phoneLit;
	QLineEdit*	_phone;
	QLabel*		_phone2Lit;
	QLineEdit*	_phone2;
	QLabel*		_faxLit;
	QLineEdit*	_fax;
	QLabel*		_emailLit;
	QLineEdit*	_email;
	QLabel*		_webaddrLit;
	QLineEdit*	_webaddr;
	QCheckBox*	_active;
	QString         _addressChange;
	AddressCluster*	_address;

	QString	_extraClause;
	QString	_query;	
	int	_searchAcctId;
	QString _titleSingular;
	QString _titlePlural;

        bool    _ignoreSignals;

    private:
	virtual void	init();
	virtual void	layout();
	virtual void	silentSetId(const int);
        XDataWidgetMapper* _mapper;

	int		_id;
	bool		_layoutDone;
	int		_limits;
	bool		_minimalLayout;
	QString		_notes;
	bool		_valid;
	
        //Data Mapping Values
        QString  _fieldNameChange;
	QString  _fieldNameNumber;
        QString  _fieldNameActive;
	QString  _fieldNameCrmAccount;
        QString  _fieldNameHonorific;
        QString  _fieldNameFirst;
        QString  _fieldNameLast;
        QString  _fieldNameInitials;
        QString  _fieldNameTitle;
        QString  _fieldNamePhone;
        QString  _fieldNamePhone2;
        QString  _fieldNameFax;
        QString  _fieldNameEmailAddress;
        QString  _fieldNameWebAddress;
	QString  _fieldNameAddrChange;
	QString  _fieldNameAddrNumber;
	QString  _fieldNameLine1;
	QString  _fieldNameLine2;
	QString  _fieldNameLine3;
	QString  _fieldNameCity;
	QString  _fieldNamePostalCode;
	QString  _fieldNameState;
	QString  _fieldNameCountry;
};

#endif
