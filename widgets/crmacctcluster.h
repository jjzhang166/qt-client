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

// crmaccttcluster.h
// Created 01/31/2006 GJM
// Copyright (c) 2006-2008, OpenMFG, LLC

#ifndef _crmaccttCluster_h
#define _crmaccttCluster_h

#include "virtualCluster.h"
#include "xcheckbox.h"

class XComboBox;

class OPENMFGWIDGETS_EXPORT CRMAcctInfoAction
{
  public:
    virtual ~CRMAcctInfoAction() {};
    virtual void crmacctInformation(QWidget* parent, int pCustid) = 0;
};

class OPENMFGWIDGETS_EXPORT CRMAcctLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
	CRMAcctLineEdit(QWidget*, const char* = 0);

	enum CRMAcctSubtype { Crmacct,	Competitor,	Cust,	Partner,
			      Prospect,	Taxauth,	Vend,
			      CustAndProspect };

	static CRMAcctInfoAction *_crmacctInfoAction;

	virtual void		setSubtype(const CRMAcctSubtype);
	virtual CRMAcctSubtype	subtype()	const;

    protected slots:
	virtual void	sInfo();
	VirtualList*	listFactory();
	VirtualSearch*	searchFactory();

    protected:
	CRMAcctSubtype _subtype;
};

class OPENMFGWIDGETS_EXPORT CRMAcctList : public VirtualList
{
    Q_OBJECT

    friend class CRMAcctCluster;
    friend class CRMAcctLineEdit;

    public:
	CRMAcctList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
	virtual void sFillList();
	virtual void sSearch(const QString&);
	virtual void setId(const int);
	virtual void setShowInactive(const bool);
	virtual void setSubtype(const CRMAcctLineEdit::CRMAcctSubtype);

    protected:
	QWidget* _parent;
	QString	 _query;
	bool	 _showInactive;
	enum CRMAcctLineEdit::CRMAcctSubtype _subtype;
};

class OPENMFGWIDGETS_EXPORT CRMAcctSearch : public VirtualSearch
{
    Q_OBJECT

    friend class CRMAcctCluster;
    friend class CRMAcctLineEdit;

    public:
	CRMAcctSearch(QWidget*, Qt::WindowFlags = 0);
	virtual void setId(const int);
	virtual void setShowInactive(const bool);
	virtual void setSubtype(const CRMAcctLineEdit::CRMAcctSubtype);

    public slots:
	virtual void sFillList();

    protected:
	QLabel*		_addressLit;
	QWidget*	_parent;
	XCheckBox*	_searchContact;
	XCheckBox*	_searchPhone;
	XCheckBox*	_searchStreet;
	XCheckBox*	_searchCity;
	XCheckBox*	_searchState;
	XCheckBox*	_searchPostalCode;
	XCheckBox*	_searchCountry;
	QCheckBox*	_showInactive;
        XCheckBox*      _searchCombo;
        XComboBox*      _comboCombo;

	enum CRMAcctLineEdit::CRMAcctSubtype _subtype;

    private:
};

class OPENMFGWIDGETS_EXPORT CRMAcctCluster : public VirtualCluster
{
    Q_OBJECT

    public:
	CRMAcctCluster(QWidget*, const char* = 0);
	virtual void				setSubtype(CRMAcctLineEdit::CRMAcctSubtype const);
	virtual CRMAcctLineEdit::CRMAcctSubtype subtype() const;

};

#endif
