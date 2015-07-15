/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef comments_h
#define comments_h

#include <QMultiMap>

#include <xsqlquery.h>

#include "xtreewidget.h"
#include "xcheckbox.h"

class QPushButton;
class QTextBrowser;
class XTreeWidget;
class Comment;

struct CommentMap
{
  int     doctypeId;
  QString doctypeStr;
  QString translation;
  QString idParam;
  QString uiname;
  QString newPriv;

  CommentMap(int     id,       // enum DocumentSources + extensions
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

class XTUPLEWIDGETS_EXPORT Comments : public QWidget
{
  Q_OBJECT

  Q_ENUMS(CommentSources)

  Q_PROPERTY(int type READ type WRITE setType)
  
  friend class comment;

  public:
    Comments(QWidget *, const char * = 0);

    // if you add to this then add to the _commentMap[] below
    enum CommentSources
    {
      Uninitialized,
      Address,
      BBOMHead,		BBOMItem,
      BOMHead,		BOMItem,
      BOOHead,		BOOItem,
      CRMAccount,	Contact,	Customer,
      Employee,         Incident,
      Item,		ItemSite,	ItemSource,
      Location,		LotSerial,
      Opportunity,
      Project,		PurchaseOrder,	PurchaseOrderItem,
      ReturnAuth, ReturnAuthItem, Quote, QuoteItem,
      SalesOrder,	SalesOrderItem, Task, TimeAttendance,
      TodoItem,      TransferOrder,	TransferOrderItem,
      Vendor,
      Warehouse,	WorkOrder
    };

    inline int sourceid()             { return _sourceid; }
    int         type() const;
  
    static QMap<QString, struct CommentMap*> &commentMap();

    bool userCanEdit(int);

  public slots:
    void setType(int sourceType);
    void setType(QString sourceType);
    void setId(int);
    void setReadOnly(bool);
    void setVerboseCommentList(bool);
    void setEditable(bool p) {_editable = p;}

    void sNew();
    void sView();
    void sEdit();
    void refresh();

    void anchorClicked(const QUrl &);
    void sCheckButtonPriv(bool); 

  signals:
    void commentAdded();

  private:
    void showEvent(QShowEvent *event);
  
    static QMap<QString, struct CommentMap*> _strMap;
    static QMap<int,     struct CommentMap*> _intMap;
    QString              _sourcetype;
  
    static bool addToMap(int id, QString key, QString trans, QString param = QString(), QString ui = QString(), QString priv = QString());

    int                 _sourceid;
    QList<QVariant> _commentIDList;
    bool _verboseCommentList;
    bool _editable;

    QTextBrowser *_browser;
    XTreeWidget *_comment;
    QPushButton *_newComment;
    QPushButton *_viewComment;
    QPushButton *_editComment;
    QMultiMap<int, bool> *_editmap;
    QMultiMap<int, bool> *_editmap2;
    XCheckBox *_verbose;
};

#endif
