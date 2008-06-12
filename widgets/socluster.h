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

//  socluster.h
//  Created 03/04/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#ifndef soCluster_h
#define soCluster_h

#include "xlineedit.h"
#include "xdatawidgetmapper.h"
#include "OpenMFGWidgets.h"
#include <QLabel>

class QLabel;
class QPushButton;

class SoCluster;

//  Possible Customer Order Status
#define cSoOpen               0x01
#define cSoClosed             0x02
#define cSoAtShipping         0x04
#define cSoReleased           0x08
#define cSoCustomer           0x16

class OPENMFGWIDGETS_EXPORT SoLineEdit : public XLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString     number          READ text()          WRITE setNumber);

friend class SoCluster;

  public:
    SoLineEdit(QWidget *, const char * = 0);

  public slots:
    void setId(int);
    void setCustId(int);
    void setNumber(QString);
    void clear();
    void sParse();

  signals:
    void valid(bool);
    void newId(int);
    void custidChanged(int);
    void custNameChanged(const QString &);
    void numberChanged(int);
    void numberChanged(const QString &);
    
  protected:
    XDataWidgetMapper *_mapper;

  private:
    int  _custid;
    int  _type;
    QString  _number;

};


class OPENMFGWIDGETS_EXPORT SoCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString label       READ label       WRITE setLabel);
  Q_PROPERTY(bool    readOnly    READ isReadOnly  WRITE setReadOnly);
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName);

  public:
    SoCluster(QWidget *, const char * = 0);
    SoCluster(int, QWidget *);
		
    inline int id()                { return _soNumber->_id;      }
    inline int custid()            { return _soNumber->_custid;  } 
    inline bool isValid()          { return _soNumber->_valid;   }
    inline QString  number()           { return _soNumber->text();  }
    //inline void setType(int pType) { _soNumber->_type = pType;   };
    inline virtual QString label() const { return _label; }
    inline bool isReadOnly()       { return _readOnly; } 
    QString fieldName() const 	{ return _fieldName; };

  public slots:
    void setReadOnly(bool);
    void setId(int);
    void setCustId(int);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(QString p) { _fieldName = p; };
    void setNumber(QString);
    void setLabel(const QString p);
    void setType(int pType) { _soNumber->_type = pType;   };

  signals:
    void newId(int);
    void newCustid(int);
    void valid(bool);

  private slots:
    void sList();

  protected:
    QLabel* _soNumberLit;

  private:
    void constructor();

    SoLineEdit   *_soNumber;
    QPushButton  *_list;
    QLabel       *_custName;
    bool         _readOnly;
    QString      _fieldName;
    QString      _label;
    
};

#endif
