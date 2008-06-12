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

//  custcluster.h
//  Created 02/27/2002 JSL
//  Copyright (c) 2002-2008, OpenMFG, LLC

#ifndef custCluster_h
#define custCluster_h

#include "OpenMFGWidgets.h"
#include "xlineedit.h"

class QLabel;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class CustInfo;

#define __allCustomers    0x01
#define __activeCustomers 0x02

class OPENMFGWIDGETS_EXPORT CLineEdit : public XLineEdit
{
  Q_OBJECT

  Q_ENUMS(CLineEditTypes)

  Q_PROPERTY(bool	   autoFocus READ autoFocus WRITE setAutoFocus	)
  Q_PROPERTY(CLineEditTypes	type READ type	    WRITE setType	)
  Q_PROPERTY(QString     number          READ text          WRITE setNumber)

  friend class CustInfo;

  public:
    CLineEdit(QWidget * = 0, const char * = 0);

    enum CLineEditTypes
    {
      AllCustomers, 		ActiveCustomers,
      AllProspects,		ActiveProspects,
      AllCustomersAndProspects,	ActiveCustomersAndProspects
    };

    inline bool			autoFocus() const { return _autoFocus; }
    inline CLineEditTypes	type()	    const { return _type; };

  public slots:
    void sEllipses();
    void sSearch();
    void sList();
    void setSilentId(int);
    void setId(int);
    void setNumber(const QString& pNumber);
    void setType(CLineEditTypes);
    void sParse();
    void setAutoFocus(bool);

  signals:
    void newId(int);
    void custNumberChanged(const QString &);
    void custNameChanged(const QString &);
    void custAddr1Changed(const QString &);
    void custAddr2Changed(const QString &);
    void custAddr3Changed(const QString &);
    void custCityChanged(const QString &);
    void custStateChanged(const QString &);
    void custZipCodeChanged(const QString &);
    void custCountryChanged(const QString &);
    void creditStatusChanged(const QString &);
    void custAddressChanged(const int);
    void custContactChanged(const int);
    void valid(bool);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void keyPressEvent(QKeyEvent *);
    XDataWidgetMapper *_mapper;

  private:
    bool		_autoFocus;
    bool		_dragging;
    CLineEditTypes	_type;
};

class OPENMFGWIDGETS_EXPORT CustInfoAction
{
  public:
    virtual ~CustInfoAction() {};
    virtual void customerInformation(QWidget* parent, int pCustid) = 0;
};

class OPENMFGWIDGETS_EXPORT CustInfo : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool	   autoFocus READ autoFocus WRITE setAutoFocus	)
  Q_PROPERTY(CLineEdit::CLineEditTypes	type READ type	    WRITE setType	)
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName)

  public:
    CustInfo(QWidget *parent, const char *name = 0);

    void setReadOnly(bool);

    inline bool	      autoFocus() const { return _customerNumber->autoFocus(); }
    inline int		     id()	{ return _customerNumber->_id;    }
    inline bool		isValid()	{ return _customerNumber->_valid; }
    inline CLineEdit::CLineEditTypes type() const { return _customerNumber->type(); }

    static CustInfoAction * _custInfoAction;
      
    QString fieldName()   const { return _fieldName; };

  public slots:
    void setSilentId(int);
    void setId(int);
    void setType(CLineEdit::CLineEditTypes);
    void setAutoFocus(bool);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(QString p) { _fieldName = p; };

  private slots:
    void sInfo();
    void sHandleCreditStatus(const QString &);

  signals:
    void newId(int);
    void numberChanged(const QString &);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);
    void addressChanged(const int);
    void contactChanged(const int);
    void valid(bool);

  private:
    CLineEdit   *_customerNumber;
    QPushButton *_list;
    QPushButton *_info;
    QString     _fieldName;
};


class OPENMFGWIDGETS_EXPORT CustCluster : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool	   autoFocus READ autoFocus WRITE setAutoFocus	)
  Q_PROPERTY(CLineEdit::CLineEditTypes	type READ type	    WRITE setType	)
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName)

  public:
    CustCluster(QWidget *parent, const char *name = 0);

    void setReadOnly(bool);

    inline bool	      autoFocus() const { return _custInfo->autoFocus(); }
    inline int		     id()	{ return _custInfo->id();       }
    inline bool		isValid()	{ return _custInfo->isValid();   }
    inline CLineEdit::CLineEditTypes type() const { return _custInfo->type(); }
    inline QString    fieldName() const { return _custInfo->fieldName(); };

    void setType(CLineEdit::CLineEditTypes);
    
  signals:
    void newId(int);
    void valid(bool);

  public slots:
    void setId(int);
    void setSilentId(int);
    void setAutoFocus(bool);
    void setFieldName(const QString& p) { _custInfo->setFieldName(p); };
    void setDataWidgetMap(XDataWidgetMapper* m) { _custInfo->setDataWidgetMap(m); };

  private:
    CustInfo    *_custInfo;
    QLabel      *_name;
    QLabel      *_address1;
};

#endif

