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

#ifndef CRMACCOUNT_H
#define CRMACCOUNT_H

#include "guiclient.h"
#include <QWidget>
#include "ui_crmaccount.h"

class crmaccount : public QWidget, public Ui::crmaccount
{
    Q_OBJECT

public:
    crmaccount(QWidget* parent = 0, Qt::WFlags fl = 0);
    ~crmaccount();
    static void doDialog(QWidget *, const ParameterList &);
    int	getIncidentId();

public slots:
    virtual enum SetResponse set(const ParameterList&);
    virtual void sPopulate();
    virtual void sPopulateContacts();
    virtual void sPopulateTodo();
    virtual void sPopulateOplist();
    virtual void sPopulateRegistrations();

protected slots:
    virtual void languageChange();

    virtual void sAttach();
    virtual int  saveContact(ContactCluster*);
    virtual void sClose();
    virtual void sCompetitor();
    virtual void sCustomer();
    virtual void sDeleteCharacteristic();
    virtual void sDeleteIncdt();
    virtual void sDeleteReg();
    virtual void sDeleteTodo();
    virtual void sDeleteTodoIncdt();
    virtual void sDetach();
    virtual void sEdit();
    virtual void sEditCharacteristic();
    virtual void sEditIncdt();
    virtual void sEditReg();
    virtual void sEditTodo();
    virtual void sEditTodoIncdt();
    virtual void sGetCharacteristics();
    virtual void sHandleAutoUpdate();
    virtual void sHandleTodoPrivs();
    virtual void sNew();
    virtual void sNewCharacteristic();
    virtual void sNewIncdt();
    virtual void sNewReg();
    virtual void sNewTodo();
    virtual void sPartner();
    virtual void sPopulateMenu(QMenu *);
    virtual void sPopulateTodoMenu(QMenu *);
    virtual void sPopulateOplistMenu(QMenu *);
    virtual void sProspect();
    virtual void sSave();
    virtual void sTaxAuth();
    virtual void sUpdateRelationships();
    virtual void sVendor();
    virtual void sView();
    virtual void sViewIncdt();
    virtual void sViewTodo();
    virtual void sViewTodoIncdt();
    virtual void sCustomerInfo();
    virtual void sHandleButtons();
    virtual void sCustomerToggled();
    virtual void sProspectToggled();
    virtual void sOplistView();
    virtual void sOplistEdit();
    virtual void sCheckNumber();

protected:
    virtual void closeEvent(QCloseEvent*);

private:
    bool	_modal;
    int		_mode;
    int		_crmacctId;
    int		_competitorId;
    int		_custId;
    int		_partnerId;
    int		_prospectId;
    int		_taxauthId;
    int		_vendId;
    int		_cntct1Id;
    int		_cntct2Id;
    int		_myUsrId;
    int         _NumberGen;

    int	saveNoErrorCheck();

};

#endif // CRMACCOUNT_H
