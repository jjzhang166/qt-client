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

//  wocluster.h
//  Created 03/08/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#ifndef woCluster_h
#define woCluster_h

#include "OpenMFGWidgets.h"
#include <QDate>
#include "xlineedit.h"
#include <xsqlquery.h>

class QLabel;
class QPushButton;
class WoCluster;
class XComboBox;

//  Possible Work Order Status
#define cWoOpen     0x01
#define cWoExploded 0x02
#define cWoIssued   0x04
#define cWoReleased 0x08
#define cWoClosed   0x10


class OPENMFGWIDGETS_EXPORT WoLineEdit : public XLineEdit
{
  Q_OBJECT;

friend class WoCluster;

  public:
    WoLineEdit(QWidget *, const char * = 0);
    WoLineEdit(int, QWidget *, const char * = 0);

    inline void setQuery(const QString  &pSql) { _sql = pSql; _useQuery = TRUE; }
    inline void setType(int pWoType)           { _woType = pWoType;           }
    inline void setWarehouse(int pWarehouseid) { _warehouseid = pWarehouseid; }

  public slots:
    void setId(int);
    void sParse();

  private:
    bool    _useQuery;
    QString _sql;
    int     _woType;
    int     _warehouseid;
    QChar   _status;
    double  _qtyOrdered;
    double  _qtyReceived;

  signals:
    void newId(int);
    void newItemid(int);
    void warehouseChanged(const QString &);
    void itemNumberChanged(const QString &);
    void uomChanged(const QString &);
    void itemDescrip1Changed(const QString &);
    void itemDescrip2Changed(const QString &);
    void startDateChanged(const QDate &);
    void dueDateChanged(const QDate &);
    void qtyOrderedChanged(const QString &);
    void qtyReceivedChanged(const QString &);
    void qtyBalanceChanged(const QString &);
    void statusChanged(const QString &);
    void valid(bool);
};

class OPENMFGWIDGETS_EXPORT WoCluster : public QWidget
{
  Q_OBJECT

  public:
    WoCluster(QWidget *, const char * = 0);
    WoCluster(int, QWidget *, const char * = 0);

    QString woNumber() const;

    inline void setQuery(const QString &pSql)  { _woNumber->_sql = pSql; _woNumber->_useQuery = TRUE; }
    inline void setType(int pWoType)           { _woNumber->_woType = pWoType;           }
    inline void setWarehouse(int pWarehouseid) { _woNumber->_warehouseid = pWarehouseid; }
    inline int id()                            { return _woNumber->_id;                  }
    inline bool isValid()                      { return _woNumber->_valid;               }
    inline char status()                       { return _woNumber->_status.toAscii();    }
    inline double qtyOrdered()                 { return _woNumber->_qtyOrdered;          }
    inline double qtyReceived()                { return _woNumber->_qtyReceived;         }

    inline double qtyBalance()
    {
      if (_woNumber->_qtyOrdered <= _woNumber->_qtyReceived)
        return 0;
      else
        return (_woNumber->_qtyOrdered - _woNumber->_qtyReceived);
    }

  public slots:
    void setId(int);
    void setReadOnly(bool);
    void sWoList();

  private:
    void constructor();

    WoLineEdit  *_woNumber;
    QPushButton *_woList;
    QLabel      *_warehouse;
    QLabel      *_itemNumber;
    QLabel      *_uom;
    QLabel      *_descrip1;
    QLabel      *_descrip2;
    QLabel      *_status;

  signals:
    void newId(int);
    void newItemid(int);
    void startDateChanged(const QDate &);
    void dueDateChanged(const QDate &);
    void qtyOrderedChanged(const QString &);
    void qtyReceivedChanged(const QString &);
    void qtyBalanceChanged(const QString &);

    void valid(bool);
};

class OPENMFGWIDGETS_EXPORT WomatlCluster : public QWidget
{
  Q_OBJECT

  public:
    enum SourceTypes
    {
      WorkOrder  = 0x01,
      WoMaterial = 0x02,
      Wooper     = 0x04
    };

    enum IssueTypes
    {
      Pull  = 0x01,
      Push  = 0x02,
      Mixed = 0x04
    };

    WomatlCluster(QWidget *, const char * = 0);
    WomatlCluster(WoCluster *, QWidget *, const char * = 0);

    void setReadOnly(bool);

    inline int id()             { return _id;       }
    inline int woid()           { return _woid;     }
    inline bool isValid()       { return _valid;    }
    inline double qtyRequired() { return _required; }
    inline double qtyIssued()   { return _issued;   }

  signals:
    void newId(int);
    void valid(bool);
    void newQtyRequired(const QString &);
    void newQtyIssued(const QString &);
    void newQtyScrappedFromWIP(const QString &);

  public slots:
    void setId(int);
    void setType(int);
    void setWooperid(int);
    void setWoid(int);
    void sPopulateInfo(int);
    void sRefresh();

  private:
    void constructor();

    XComboBox   *_itemNumber;
    QLabel      *_uom;
    QLabel      *_descrip1;
    QLabel      *_descrip2;
    QLabel      *_qtyPer;
    QLabel      *_scrap;
    QLabel      *_qtyRequired;
    QLabel      *_qtyIssued;

    XSqlQuery _womatl;
    int      _id;
    int      _woid;
    int      _source;
    int      _type;
    int      _sourceId;
    bool     _valid;
    double   _required;
    double   _issued;
};


#endif

