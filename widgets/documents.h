/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef documents_h
#define documents_h

#include <QWidget>

#include <xsqlquery.h>

#include "widgets.h"
#include "guiclientinterface.h"

#include "ui_documents.h"

#define cNew                  1
#define cEdit                 2
#define cView                 3

struct DocumentMap
{
  int     doctypeId;
  QString doctypeStr;
  QString translation;
  QString idParam;
  QString uiname;
  QString newPriv;

  DocumentMap(int     id,       // enum DocumentSources + extensions
              QString key,
              QString trans,
              QString param,
              QString ui    = QString(),
              QString priv  = QString()) {

    doctypeId   = id;
    doctypeStr  = key;
    translation = trans;
    idParam     = param;
    uiname      = ui;
    newPriv     = priv;
  }
};

class XTUPLEWIDGETS_EXPORT Documents : public QWidget, public Ui::documents
{
  Q_OBJECT

  Q_ENUMS(DocumentSources)

  Q_PROPERTY(int type READ type WRITE setType)

  friend class image;
  friend class file;

  public:
    Documents(QWidget *);

    // If you add to this then add to the documentMap() function in documents.cpp
    enum DocumentSources
    {
      Uninitialized,
      Address,          BBOMHead,           BBOMItem,
      BOMHead,          BOMItem,            BOOHead,
      BOOItem,          CRMAccount,         Contact, 
      Contract,         CreditMemo,         CreditMemoItem,
      Customer,         Employee,           Incident,
      Invoice,          InvoiceItem,
      Item,             ItemSite,           ItemSource,
      Location,         LotSerial,
      Opportunity,      Project,            PurchaseOrder,
      PurchaseOrderItem,ReturnAuth,         ReturnAuthItem,
      Quote,            QuoteItem,          SalesOrder,
      SalesOrderItem,   ShipTo,             TimeExpense,
      Todo,             TransferOrder,      TransferOrderItem,
      Vendor,           Voucher,            Warehouse,
      WorkOrder,                            ProjectTask
    };

    static GuiClientInterface *_guiClientInterface;
    inline int  sourceid()             { return _sourceid; }
    int         type() const;

    static QMap<QString, struct DocumentMap*> &documentMap();

  public slots:
    void setType(int sourceType);
    void setType(QString sourceType);
    void setId(int);
    void setReadOnly(bool);
    void sNewDoc(QString type = QString(), QString ui = QString());
    void sNewImage();
    void sInsertDocass(QString, int);
    void sAttachDoc();
    void sViewDoc();
    void sEditDoc();
    void sOpenDoc(QString mode = "edit");
    void sDetachDoc();
    
    void refresh();

  private slots:
    void handleSelection(bool = false);
    void handleItemSelected();

  private:
    static QMap<QString, struct DocumentMap*> _strMap;
    static QMap<int,     struct DocumentMap*> _intMap;
    int                  _sourceid;
    QString              _sourcetype;
    bool                 _readOnly;

    static bool addToMap(int id, QString key, QString trans, QString param = QString(), QString ui = QString(), QString priv = QString());

};

#endif
