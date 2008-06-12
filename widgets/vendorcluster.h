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

//  vendorcluster.h
//  Created 02/26/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#ifndef vendorcluster_h
#define vendorcluster_h

#include "OpenMFGWidgets.h"
#include "xlineedit.h"

#define __allVendors    0x01
#define __activeVendors 0x02

class QPushButton;
class QLabel;

class VendorCluster;

class OPENMFGWIDGETS_EXPORT VendorLineEdit : public XLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString     number          READ text          WRITE setNumber);

friend class VendorInfo;
friend class VendorCluster;

  public:
    VendorLineEdit(QWidget *, const char * = 0);

  public slots:
    void setId(int);
    void setNumber(const QString &);
    void setType(int);
    void sParse();
    void sEllipses();
    void sSearch();
    void sList();

  signals:
    void valid(bool);
    void newId(int);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    XDataWidgetMapper *_mapper;

  private:
    bool _dragging;
    int  _type;
};

class OPENMFGWIDGETS_EXPORT VendorInfo : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly )
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName);

  public:
    VendorInfo(QWidget *parent, const char *name = 0);

    inline int id()       const { return _vendorNumber->_id;    }
    inline bool isValid() const { return _vendorNumber->_valid; }
    inline bool isReadOnly() const { return _vendorNumber->isEnabled(); }
    
    QString fieldName()   const { return _fieldName; };

  public slots:
    void setId(int);
    void setType(int);
    void setReadOnly(bool);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(QString p) { _fieldName = p; };

  signals:
    void newId(int);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);
    void valid(bool);

  private:
    VendorLineEdit *_vendorNumber;
    QPushButton    *_list;
    QPushButton    *_info;
    QString        _fieldName;
};


class OPENMFGWIDGETS_EXPORT VendorCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName);

  public:
    VendorCluster(QWidget *, const char * = 0);

    inline void setId(int pId)     { _vendorNumber->setId(pId);    }
    inline int id()                { return _vendorNumber->_id;    }
    inline bool isValid()          { return _vendorNumber->_valid; }
    inline QString    fieldName() const { return _fieldName; };

    void setReadOnly(bool);
    void setType(int);
    
  public slots:
    void setFieldName(QString p) { _fieldName = p; };
    void setDataWidgetMap(XDataWidgetMapper* m);

  signals:
    void newId(int);
    void valid(bool);

  private:
    VendorLineEdit *_vendorNumber;
    QPushButton    *_list;
    QLabel         *_vendorName;
    QString        _fieldName;
};

#endif

