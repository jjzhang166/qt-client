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

//  itemcluster.h
//  Created 03/07/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#ifndef itemCluster_h
#define itemCluster_h

#include "xlineedit.h"

#include "qstringlist.h"

class QPushButton;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class OPENMFGWIDGETS_EXPORT ItemLineEdit : public XLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString     number          READ text          WRITE setItemNumber);

friend class ItemCluster;

  public:
    ItemLineEdit(QWidget *, const char * = 0);

    enum Type {
      cUndefined           = 0x00,

      // Specific Item Types
      cPurchased           = 0x00000001,
      cManufactured        = 0x00000002,
      cPhantom             = 0x00000004,
      cBreeder             = 0x00000008,
      cCoProduct           = 0x00000010,
      cByProduct           = 0x00000020,
      cReference           = 0x00000040,
      cCosting             = 0x00000080,
      cTooling             = 0x00000100,
      cOutsideProcess      = 0x00000200,
      cPlanning            = 0x00000400,
      cJob                 = 0x00000800,

      // The first 16 bits are reserved for individual item types and we
      // have this mask defined here for convenience.
      cAllItemTypes_Mask   = 0x0000FFFF,

      // Groups of Item Types
      cGeneralManufactured = cManufactured | cPhantom | cBreeder,
      cGeneralPurchased    = cPurchased | cOutsideProcess,
      cGeneralComponents   = cManufactured | cPhantom | cCoProduct | cPurchased | cOutsideProcess,

      // Planning Systems
      cPlanningMRP         = 0x00100000,
      cPlanningMPS         = 0x00200000,
      cPlanningNone        = 0x00400000,

      cPlanningAny         = cPlanningMRP | cPlanningMPS | cPlanningNone,

      // Misc. Options
      cItemActive          = 0x04000000,
      cSold                = 0x08000000,

      // Lot/Serial and Location Controlled
      cLocationControlled  = 0x10000000,
      cLotSerialControlled = 0x20000000,
      cDefaultLocation     = 0x40000000,

      // Active ItemSite
      cActive         = 0x80000000
    };

    inline void setType(unsigned int pType)            { _type = pType;                 } 
    inline void setQuery(const QString &pSql) { _sql = pSql; _useQuery = TRUE; }
    inline void setValidationQuery(const QString &pSql) { _validationSql = pSql; _useValidationQuery = TRUE; }
    inline int queryUsed() const              { return _useQuery;              }
    inline int validationQueryUsed() const    { return _useValidationQuery;    }

    void addExtraClause(const QString &);
    inline QStringList getExtraClauseList() const { return _extraClauses; }
    inline void clearExtraClauseList() { _extraClauses.clear(); }

    QString itemNumber();
    QString uom();
    QString itemType();
    bool    isConfigured();

  public slots:
    void sEllipses();
    void sList();
    void sSearch();
    void sAlias();

    void silentSetId(int);
    void setId(int);
    void setItemNumber(QString);
    void setItemsiteid(int);
    void sParse();

  signals:
    void privateIdChanged(int);
    void newId(int);
    void aliasChanged(const QString &);
    void uomChanged(const QString &);
    void descrip1Changed(const QString &);
    void descrip2Changed(const QString &);
    void typeChanged(const QString &);
    void configured(bool);
    void warehouseIdChanged(int);
    void valid(bool);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    QPoint dragStartPosition;
    XDataWidgetMapper *_mapper;
  private:
    void constructor();

    QString _sql;
    QString _validationSql;
    QString _itemNumber;
    QString _uom;
    QString _itemType;
    QStringList _extraClauses;
    unsigned int _type;
    bool    _configured;
    bool    _useQuery;
    bool    _useValidationQuery;
};

class OPENMFGWIDGETS_EXPORT ItemCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName);
  
  public:
    ItemCluster(QWidget *, const char * = 0);

    void setReadOnly(bool);
    void setEnabled(bool);
    void setDisabled(bool);

    inline void setType(unsigned int pType) { _itemNumber->setType(pType); } 
    inline void setQuery(const QString &pSql) { _itemNumber->setQuery(pSql);                }
    inline void setValidationQuery(const QString &pSql) { _itemNumber->setValidationQuery(pSql); }
    inline QString itemNumber() const         { return _itemNumber->itemNumber();           }
    inline QString itemType() const           { return _itemNumber->itemType();             }
    inline bool isConfigured()                { return _itemNumber->isConfigured();         }
    Q_INVOKABLE inline int id()               { return _itemNumber->id();                   }
    inline int isValid()                      { return _itemNumber->isValid();              }
    inline QString uom()                      { return _itemNumber->uom();                  }
    inline QString fieldName()   const        { return _fieldName;                          }

    inline void addExtraClause(const QString & pClause) { _itemNumber->addExtraClause(pClause); }
    inline QStringList getExtraClauseList() const { return _itemNumber->getExtraClauseList(); }
    inline void clearExtraClauseList() { _itemNumber->clearExtraClauseList(); }

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(QString p) { _fieldName = p; };
    void setId(int);
    void setItemNumber(QString);
    void setItemsiteid(int);
    void silentSetId(int);

  signals:
    void privateIdChanged(int);
    void newId(int);
    void aliasChanged(const QString &);
    void warehouseIdChanged(int);
    void typeChanged(const QString &);
    void configured(bool);
    void valid(bool);

  private:
    ItemLineEdit *_itemNumber;
    QPushButton  *_itemList;
    QLabel       *_uom;
    QLabel       *_descrip1;
    QLabel       *_descrip2;
    QString _fieldName;
};

#endif
