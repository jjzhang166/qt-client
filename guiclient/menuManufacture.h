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

//  menuManufacture.h
//  Created 08/22/2000 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#ifndef menuManufacture_H
#define menuManufacture_H

#include <QObject>
#include <QPixmap>

class QToolBar;
class QMenu;
class GUIClient;

class menuManufacture : public QObject
{
  Q_OBJECT

  struct actionProperties {
    const char*		actionName;
    const QString	actionTitle;
    const char*		slot;
    QMenu*		menu;
    bool		priv;
    QPixmap*		pixmap;
    QToolBar*		toolBar;
    bool		visible;
    const QString   toolTip;
  };

  public:
    menuManufacture(GUIClient *);

  public slots:
    void sNewWorkOrder();
    void sExplodeWorkOrder();
    void sImplodeWorkOrder();
    void sWoTimeClock();
    void sCloseWorkOrder();

    void sPrintTraveler();
    void sPrintPickList();
    void sPrintRouting();

    void sReleaseWorkOrdersByPlannerCode();
    void sReprioritizeWorkOrder();
    void sRescheduleWorkOrder();
    void sChangeWorkOrderQty();
    void sPurgeClosedWorkOrders();

    void sCreateWoMaterialRequirement();
    void sDeleteWoMaterialRequirement();
    void sMaintainWoMaterials();
    void sIssueWoMaterialBatch();
    void sIssueWoMaterialItem();
    void sReturnWoMaterialBatch();
    void sReturnWoMaterialItem();
    void sScrapWoMaterialFromWo();
    void sPostProduction();
    void sPostMiscProduction();
    void sCorrectProductionPosting();

    void sCreateWoOperation();
    void sMaintainWoOperations();
    void sPostOperations();
    void sCorrectOperationsPosting();

    void sDspWoScheduleByItem();
    void sDspWoScheduleByItemGroup();
    void sDspWoScheduleByClassCode();
    void sDspWoScheduleByPlannerCode();
    void sDspWoScheduleByWorkCenter();
    void sDspWoScheduleByWorkOrder();
    void sDspWoBufferStatusByItem();
    void sDspWoBufferStatusByItemGroup();
    void sDspWoBufferStatusByClassCode();
    void sDspWoBufferStatusByPlannerCode();
    void sDspWoHistoryByItem();
    void sDspWoHistoryByNumber();
    void sDspWoHistoryByClassCode();
    void sDspWoMaterialsByComponentItem();
    void sDspWoMaterialsByWo();
    void sDspInventoryAvailabilityByWorkOrder();
    void sDspWoOperationsByWorkCenter();
    void sDspWoOperationsByWo();
    void sDspWoOperationBufrStsByWorkCenter();
    void sDspPendingAvailability();
	void sDspJobCosting();
    void sDspWoEffortByUser();
    void sDspWoEffortByWorkOrder();
    void sDspMaterialUsageVarianceByBOMItem();
    void sDspMaterialUsageVarianceByItem();
    void sDspMaterialUsageVarianceByComponentItem();
    void sDspMaterialUsageVarianceByWorkOrder();
    void sDspMaterialUsageVarianceByWarehouse();
    void sDspLaborVarianceByBOOItem();
    void sDspLaborVarianceByItem();
    void sDspLaborVarianceByWorkCenter();
    void sDspLaborVarianceByWorkOrder();
    void sDspBreederDistributionVarianceByItem();
    void sDspBreederDistributionVarianceByWarehouse();
    void sDspWoSoStatus();
    void sDspWoSoStatusMismatch();

    void sPrintWorkOrderForm();
    void sPrintProductionEntrySheet();

  private:
    GUIClient *parent;

    QToolBar    *toolBar;
    QMenu  *mainMenu;
    QMenu  *ordersMenu;
    QMenu  *formsMenu;
    QMenu  *materialsMenu;
    QMenu  *materialsIssueMenu;
    QMenu  *materialsReturnMenu;
    QMenu  *operationsMenu;
    QMenu  *transactionsMenu;
    QMenu  *reportsMenu;
    QMenu  *reportsScheduleMenu;
    QMenu  *reportsBufrStsMenu;
    QMenu  *reportsHistoryMenu;
    QMenu  *reportsMatlReqMenu;
    QMenu  *reportsOperationsMenu;
    QMenu  *reportsWoTcMenu;
    QMenu  *reportsMatlUseVarMenu;
    QMenu  *reportsLaborVarMenu;
    QMenu  *reportsBrdrDistVarMenu;
    QMenu  *reportsOpenWoMenu;
    QMenu  *utilitiesMenu;

    void	addActionsToMenu(actionProperties [], unsigned int);
};

#endif
