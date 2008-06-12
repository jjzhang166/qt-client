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

//  glcluster.h
//  Created 03/01/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#ifndef glCluster_h
#define glCluster_h

#include <QWidget>

#include "OpenMFGWidgets.h"
#include "xlineedit.h"

class QLineEdit;
class QPushButton;
class QKeyEvent;
class QFocusEvent;

class OPENMFGWIDGETS_EXPORT GLCluster : public QWidget
{
  Q_OBJECT

  public:
    GLCluster(QWidget *parent, const char *name = 0);

    enum Type {
      cUndefined  = 0x00,

      cAsset      = 0x01,
      cLiability  = 0x02,
      cExpense    = 0x04,
      cRevenue    = 0x08,
      cEquity     = 0x10
    };

    inline void setType(unsigned int pType) { _type = pType; }
    inline unsigned int type() const { return _type; }

    void setReadOnly(bool);

    int id();
    bool isValid();

  public slots:
    void setId(int);
    void setEnabled(bool);

  protected:
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);

  private slots:
    void sEllipses();
    void sSearch();
    void sList();
    void sParse();
    void sTextChanged(const QString &);

  signals:
    void newId(int);
    void valid(bool);

  private:
    int  _accntid;
    bool _valid;
    bool _parsed;
    unsigned int _type;

    QLineEdit   *_company;
    QLineEdit   *_profit;
    XLineEdit   *_main;
    QLineEdit   *_sub;
    QLineEdit   *_account;
    QPushButton *_list;
};

#endif

