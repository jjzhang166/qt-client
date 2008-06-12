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

//  inputManager.h
//  Created 05/13/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#ifndef inputManager_h
#define inputManager_h

#include <QObject>
#include <QEvent>

class InputManagerPrivate;

#define	cBCWorkOrder           0x00000010
#define	cBCWorkOrderMaterial   0x00000020
#define	cBCWorkOrderOperation  0x00000040
#define	cBCSalesOrder          0x00000100
#define cBCSalesOrderLineItem  0x00000200
#define cBCItemSite            0x00000400
#define cBCItem                0x00000800
#define cBCUPCCode             0x00001000
#define cBCEANCode             0x00002000
#define cBCCountTag            0x00004000
#define cBCLocation            0x00010000
#define cBCLocationIssue       0x00020000
#define cBCLocationContents    0x00040000
#define cBCUser                0x00100000
#define	cBCTransferOrder          0x00200000
#define cBCTransferOrderLineItem  0x00400000
#define cBCLotSerialNumber	  0x00800000

class InputManager : public QObject
{
  Q_OBJECT

  public:
    InputManager();

    void notify(int, QObject *, QObject *, const QString &);

  public slots:
    void sRemove(QObject *);

  signals:
    void readWorkOrder(int);
    void readWorkOrderMaterial(int);
    void readWorkOrderOperation(int);
    void readSalesOrder(int);
    void readSalesOrderLineItem(int);
    void readTransferOrder(int);
    void readTransferOrderLineItem(int);
    void readItemSite(int);
    void readItem(int);
    void readCountTag(int);
    void readLocation(int);
    void readLocationIssue(int);
    void readLocationContents(int);
    void readLotSerialNumber(QString);
    void readUser(int);

  protected:
    bool eventFilter(QObject *, QEvent *);

  private:
    InputManagerPrivate *_private;

    void dispatchWorkOrder();
    void dispatchWorkOrderOperation();
    void dispatchSalesOrder();
    void dispatchSalesOrderLineItem();
    void dispatchTransferOrder();
    void dispatchTransferOrderLineItem();
    void dispatchItemSite();
    void dispatchItem();
    void dispatchUPCCode();
    void dispatchCountTag();
    void dispatchLocation();
    void dispatchLocationIssue();
    void dispatchLocationContents();
    void dispatchLotSerialNumber();
    void dispatchUser();
};

#endif
