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

#include "OpenMFGWidgets.h"

#include "plugins/addressclusterplugin.h"
#include "plugins/calendarcomboboxplugin.h"
#include "plugins/clineeditplugin.h"
#include "plugins/cmclusterplugin.h"
#include "plugins/commentsplugin.h"
#include "plugins/contactclusterplugin.h"
#include "plugins/crmacctclusterplugin.h"
#include "plugins/currclusterplugin.h"
#include "plugins/currdisplayplugin.h"
#include "plugins/custclusterplugin.h"
#include "plugins/custinfoplugin.h"
#include "plugins/dateclusterplugin.h"
#include "plugins/deptclusterplugin.h"
#include "plugins/dlineeditplugin.h"
#include "plugins/empclusterplugin.h"
#include "plugins/expenseclusterplugin.h"
#include "plugins/expenselineeditplugin.h"
#include "plugins/fileclusterplugin.h"
#include "plugins/glclusterplugin.h"
#include "plugins/imageclusterplugin.h"
#include "plugins/invoicelineeditplugin.h"
#include "plugins/incidentclusterplugin.h"
#include "plugins/itemclusterplugin.h"
#include "plugins/itemlineeditplugin.h"
#include "plugins/lotserialclusterplugin.h"
#include "plugins/orderclusterplugin.h"
#include "plugins/opportunityclusterplugin.h"
#include "plugins/parametergroupplugin.h"
#include "plugins/periodslistviewplugin.h"
#include "plugins/planordclusterplugin.h"
#include "plugins/planordlineeditplugin.h"
#include "plugins/poclusterplugin.h"
#include "plugins/polineeditplugin.h"
#include "plugins/projectclusterplugin.h"
#include "plugins/projectlineeditplugin.h"
#include "plugins/raclusterplugin.h"
#include "plugins/revisionclusterplugin.h"
#include "plugins/screencontrolplugin.h"
#include "plugins/shiftclusterplugin.h"
#include "plugins/shipmentclusterplugin.h"
#include "plugins/shiptoclusterplugin.h"
#include "plugins/shiptoeditplugin.h"
#include "plugins/soclusterplugin.h"
#include "plugins/solineeditplugin.h"
#include "plugins/toclusterplugin.h"
#include "plugins/usernameclusterplugin.h"
#include "plugins/usernamelineeditplugin.h"
#include "plugins/vendorclusterplugin.h"
#include "plugins/vendorinfoplugin.h"
#include "plugins/vendorlineeditplugin.h"
#include "plugins/warehousegroupplugin.h"
#include "plugins/wcomboboxplugin.h"
#include "plugins/woclusterplugin.h"
#include "plugins/wolineeditplugin.h"
#include "plugins/womatlclusterplugin.h"
#include "plugins/workcenterclusterplugin.h"
#include "plugins/workcenterlineeditplugin.h"
#include "plugins/xcheckboxplugin.h"
#include "plugins/xcomboboxplugin.h"
#include "plugins/xlineeditplugin.h"
#include "plugins/xtreewidgetplugin.h"
#include "plugins/xtreeviewplugin.h"
#include "plugins/xurllabelplugin.h"
#include "plugins/xtexteditplugin.h"

OpenMFGPlugin::OpenMFGPlugin(QObject * parent) : QObject(parent)
{
  m_plugins.append(new AddressClusterPlugin(this));
  m_plugins.append(new CalendarComboBoxPlugin(this));
  m_plugins.append(new CLineEditPlugin(this));
  m_plugins.append(new CmClusterPlugin(this));
  m_plugins.append(new CommentsPlugin(this));
  m_plugins.append(new ContactClusterPlugin(this));
  m_plugins.append(new CRMAcctClusterPlugin(this));
  m_plugins.append(new CurrClusterPlugin(this));
  m_plugins.append(new CurrDisplayPlugin(this));
  m_plugins.append(new CustClusterPlugin(this));
  m_plugins.append(new CustInfoPlugin(this));
  m_plugins.append(new DateClusterPlugin(this));
  m_plugins.append(new DeptClusterPlugin(this));
  m_plugins.append(new DLineEditPlugin(this));
  m_plugins.append(new EmpClusterPlugin(this));
  m_plugins.append(new ExpenseClusterPlugin(this));
  m_plugins.append(new ExpenseLineEditPlugin(this));
  m_plugins.append(new FileClusterPlugin(this));
  m_plugins.append(new GLClusterPlugin(this));
  m_plugins.append(new ImageClusterPlugin(this));
  m_plugins.append(new InvoiceLineEditPlugin(this));
  m_plugins.append(new IncidentClusterPlugin(this));
  m_plugins.append(new ItemClusterPlugin(this));
  m_plugins.append(new ItemLineEditPlugin(this));
  m_plugins.append(new LotserialClusterPlugin(this));
  m_plugins.append(new OrderClusterPlugin(this));
  m_plugins.append(new OpportunityClusterPlugin(this));
  m_plugins.append(new ParameterGroupPlugin(this));
  m_plugins.append(new PeriodsListViewPlugin(this));
  m_plugins.append(new PlanOrdClusterPlugin(this));
  m_plugins.append(new PlanOrdLineEditPlugin(this));
  m_plugins.append(new PoClusterPlugin(this));
  m_plugins.append(new PoLineEditPlugin(this));
  m_plugins.append(new ProjectClusterPlugin(this));
  m_plugins.append(new ProjectLineEditPlugin(this));
  m_plugins.append(new RaClusterPlugin(this));
  m_plugins.append(new RevisionClusterPlugin(this));
  m_plugins.append(new ScreenControlPlugin(this));
  m_plugins.append(new ShiftClusterPlugin(this));
  m_plugins.append(new ShipmentClusterPlugin(this));
  m_plugins.append(new ShiptoClusterPlugin(this));
  m_plugins.append(new ShiptoEditPlugin(this));
  m_plugins.append(new SoClusterPlugin(this));
  m_plugins.append(new SoLineEditPlugin(this));
  m_plugins.append(new ToClusterPlugin(this));
  m_plugins.append(new UsernameClusterPlugin(this));
  m_plugins.append(new UsernameLineEditPlugin(this));
  m_plugins.append(new VendorClusterPlugin(this));
  m_plugins.append(new VendorInfoPlugin(this));
  m_plugins.append(new VendorLineEditPlugin(this));
  m_plugins.append(new WarehouseGroupPlugin(this));
  m_plugins.append(new WComboBoxPlugin(this));
  m_plugins.append(new WoClusterPlugin(this));
  m_plugins.append(new WoLineEditPlugin(this));
  m_plugins.append(new WomatlClusterPlugin(this));
  m_plugins.append(new WorkCenterClusterPlugin(this));
  m_plugins.append(new WorkCenterLineEditPlugin(this));
  m_plugins.append(new XCheckBoxPlugin(this));
  m_plugins.append(new XComboBoxPlugin(this));
  m_plugins.append(new XLineEditPlugin(this));
  m_plugins.append(new XTreeWidgetPlugin(this));
  m_plugins.append(new XTreeViewPlugin(this));
  m_plugins.append(new XURLLabelPlugin(this));
  m_plugins.append(new XTextEditPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> OpenMFGPlugin::customWidgets() const
{
  return m_plugins;
}

#ifndef QT_STATICPLUGIN
Q_EXPORT_PLUGIN(OpenMFGPlugin)
#else
Q_EXPORT_STATIC_PLUGIN(OpenMFGPlugin)
#endif

Preferences *_x_preferences = 0;
Metrics     *_x_metrics = 0;
QWorkspace  *_x_workspace = 0;
Privileges  *_x_privileges = 0;

void initializePlugin(Preferences *pPreferences, Metrics *pMetrics, Privileges *pPrivileges, QWorkspace *pWorkspace)
{
  _x_preferences = pPreferences;
  _x_metrics = pMetrics;
  _x_workspace = pWorkspace;
  _x_privileges = pPrivileges;
}
