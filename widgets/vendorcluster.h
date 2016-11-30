/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef VENDORCLUSTER_H
#define VENDORCLUSTER_H

#include "parameter.h"
#include "widgets.h"
#include "virtualCluster.h"
#include "crmacctcluster.h"

class QLabel;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

//class VendInfo;

#define __allVendors    0x01
#define __activeVendorss 0x02

#define CRMACCT_ID      6

class XTUPLEWIDGETS_EXPORT VendorLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  Q_ENUMS(VendorLineEditTypes)

  Q_PROPERTY(VendorLineEditTypes type READ type WRITE setType )

//  friend class VendInfo;

  public:
    VendorLineEdit(QWidget * = 0, const char * = 0);

    enum VendorLineEditTypes
    {
      AllVendors, 		ActiveVendors
    };

    inline VendorLineEditTypes	type()	    const { return _type;       }

    bool   canOpen();

    bool   canEdit();
    void   setCanEdit(bool);

    bool   editMode();

  public slots:
    bool setEditMode(bool);
    void sNew();
    void setId(const int);
    void setType(VendorLineEditTypes);

  protected slots:
    VirtualList*    listFactory();
    VirtualSearch*  searchFactory();
    void            sParse();
    void            sUpdateMenu();

  protected:
    QAction* _modeSep;
    QAction* _modeAct;

  signals:
    void newCrmacctId(int);
    void editable(bool);

  private:
    VendorLineEditTypes	_type;
    CRMAcctLineEdit::CRMAcctSubtype _subtype;
    int                 _crmacctId;
    bool                _canEdit;
    bool                _editMode;
};

class XTUPLEWIDGETS_EXPORT VendorCluster : public VirtualCluster
{
  Q_OBJECT

  Q_PROPERTY(bool canEdit READ canEdit WRITE setCanEdit )
  Q_PROPERTY(VendorLineEdit::VendorLineEditTypes	type           READ type	  WRITE setType                          )

  public:
    VendorCluster(QWidget *parent, const char *name = 0);

    inline VendorLineEdit::VendorLineEditTypes type()  const { return static_cast<VendorLineEdit*>(_number)->type();          };

    Q_INVOKABLE void   setType(VendorLineEdit::VendorLineEditTypes);

    bool canEdit() const { return static_cast<VendorLineEdit*>(_number)->canEdit(); }
    void setCanEdit(bool p) const { static_cast<VendorLineEdit*>(_number)->setCanEdit(p); }

    Q_INVOKABLE bool editMode() { return static_cast<VendorLineEdit*>(_number)->editMode(); }
    Q_INVOKABLE bool setEditMode(bool p) const;

  private slots:
   void sHandleEditMode(bool);

  signals:
    void newCrmacctId(int);
    void editable(bool);
    void editingFinished();
};

#endif
