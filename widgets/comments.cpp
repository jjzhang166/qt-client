/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QHBoxLayout>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QList>
#include <QTextBrowser>
#include <QDateTime>
#include <QDesktopServices>
#include <QDebug>
#include <QScrollBar>
#if QT_VERSION >= 0x050000
#include <QUrlQuery>
#endif
#include <parameter.h>
#include <xsqlquery.h>

#include "comment.h"
#include "comments.h"

void Comments::showEvent(QShowEvent *event)
{
  if (event)
  {
    QScrollBar * scrbar = _browser->verticalScrollBar();
    scrbar->setValue(0);
  }
}


QMap<QString, struct CommentMap*> Comments::_strMap;
QMap<int,     struct CommentMap*> Comments::_intMap;

/** Add another document type to the map by both key and int.

    Actually there are two parallel maps, one by DocumentSources/integer
    and the other by string key. This simplifies lookups and provides
    backward compatibility with existing UI file definitions.

    @param id    A DocumentSources enum value or a unique extension-supplied int
    @param key   A human-readable abbreviation for the document type
    @param trans A human-readable, translatable term for the document type
    @param param The name of the parameter a window checks for the record id
    @param ui    The application or extension window to create a new record
    @param priv  The privileges required to create a new record

    @return true on success, false if this entry would create a duplicate
            on either id or key.
 */
bool Comments::addToMap(int id,        QString key, QString trans,
                        QString param, QString ui,  QString priv)
{
  if (_strMap.contains(key) || _intMap.contains(id)) {
    qDebug() << "Comments::addToMap(" << id << ", " << key << ") duplicate!";
    return false;
  }

  CommentMap *entry = new CommentMap(id, key, trans, param, ui, priv);
  _strMap.insert(key, entry);
  _intMap.insert(id,  entry);
  return true;
}

// Inconsistencies between here and the rest of the app: S? Q?
QMap<QString, struct CommentMap *> &Comments::commentMap() {
  if (_strMap.isEmpty()) {
    addToMap(Uninitialized,     "",      tr("[Pick a Document Type]")                           );
    addToMap(Address,           "ADDR",  tr("Address")                                          );
    addToMap(BBOMHead,          "BBH",   tr("Breeder BOM Head")                                 );
    addToMap(BBOMItem,          "BBI",   tr("Breeder BOM Item")                                 );
    addToMap(BOMHead,           "BMH",   tr("BOM Head"),        "bomhead_id", "bom"             );
    addToMap(BOMItem,           "BMI",   tr("BOM Item")                                         );
    addToMap(BOOHead,           "BOH",   tr("Router Head")                                      );
    addToMap(BOOItem,           "BOI",   tr("Router Item")                                      );
    addToMap(CRMAccount,        "CRMA",  tr("Account"),         "crmacct_id", "crmaccount"      );
    addToMap(Contact,           "T",     tr("Contact"),         "cntct_id",   "contact"         );
//    addToMap(Contract,          "CNTR",  tr("Contract"),        "contrct_id", "contrct"         );
//    addToMap(CreditMemo,        "CM",    tr("Return"),          "cmhead_id",  "creditMemo"      );
//    addToMap(CreditMemoItem,    "CMI",   tr("Return Item")                                      );
    addToMap(Customer,          "C",     tr("Customer"),        "cust_id",    "customer"        );
    addToMap(Employee,          "EMP",   tr("Employee"),        "emp_id",     "employee"        );
    addToMap(Incident,          "INCDT", tr("Incident"),        "incdt_id",   "incident",     "MaintainPersonalIncidents MaintainAllIncidents" );
//    addToMap(Invoice,           "INV",   tr("Invoice"),         "invchead_id","invoice"         );
//    addToMap(InvoiceItem,       "INVI",  tr("Invoice Item")                                     );
    addToMap(Item,              "I",     tr("Item"),            "item_id",    "item"            );
    addToMap(ItemSite,          "IS",    tr("Item Site")                                        );
    addToMap(ItemSource,        "IR",    tr("Item Source"),     "itemsrc_id", "itemSource"      );
    addToMap(Location,          "L",     tr("Location")                                         );
    addToMap(LotSerial,         "LS",    tr("Lot/Serial"),      "ls_id",      "lotSerial"       );
    addToMap(Opportunity,       "OPP",   tr("Opportunity"),     "ophead_id",  "opportunity",  "MaintainPersonalOpportunities MaintainAllOpportunities" );
    addToMap(Project,           "J",     tr("Project"),         "prj_id",     "project",      "MaintainPersonalProjects MaintainAllProjects" );
    addToMap(PurchaseOrder,     "P",     tr("Purchase Order"),  "pohead_id",  "purchaseOrder"   );
    addToMap(PurchaseOrderItem, "PI",    tr("Purchase Order Item")                              );
    addToMap(ReturnAuth,        "RA",    tr("Return Authorization"), "rahead_id", "returnAuthorization");
    addToMap(ReturnAuthItem,    "RI",    tr("Return Authorization Item")                        );
    addToMap(Quote,             "Q",     tr("Quote"),           "quhead_id",  "salesOrder"      );
    addToMap(QuoteItem,         "QI",    tr("Quote Item")                                       );
    addToMap(SalesOrder,        "S",     tr("Sales Order"),     "sohead_id",  "salesOrder"      );
    addToMap(SalesOrderItem,    "SI",    tr("Sales Order Item")                                 );
//    addToMap(ShipTo,            "SHP",   tr("Ship To"),         "shipto_id",  "shipTo"          );
    addToMap(TimeAttendance,    "TATC",  tr("Time Attendance")                                     );
//    addToMap(TimeExpense,       "TE",    tr("Time Expense")                                     );
    addToMap(TodoItem,          "TD",    tr("To-Do"),           "todoitem_id","todoItem",     "MaintainPersonalToDoItems MaintainAllTodoItems" );
    addToMap(TransferOrder,     "TO",    tr("Transfer Order"),  "tohead_id",  "transferOrder"   );
    addToMap(TransferOrderItem, "TI",    tr("Transfer Order Item")                              );
    addToMap(Vendor,            "V",     tr("Vendor"),          "vend_id",    "vendor"          );
//    addToMap(Voucher,           "VCH",   tr("Voucher"),         "vohead_id",  "voucher"         );
    addToMap(Warehouse,         "WH",    tr("Site")                                             );
    addToMap(WorkOrder,         "W",     tr("Work Order"),      "wo_id",      "workOrder"       );
    addToMap(Task,              "TA",    tr("Project Task"),    "prjtask_id", "projectTask"     );
  }

  return _strMap;
}

Comments::Comments(QWidget *pParent, const char *name) :
  QWidget(pParent)
{
  setObjectName(name);
  _sourceid = -1;
  _editable = true;
  if (_strMap.isEmpty()) {
    (void)commentMap();
  }

  _verboseCommentList = false;

  QVBoxLayout *vbox = new QVBoxLayout(this);

  QHBoxLayout *hbox = new QHBoxLayout();
  hbox->setMargin(0);
  hbox->setSpacing(7);
  
  _verbose = new XCheckBox(tr("Verbose Text"), this);
  _verbose->setObjectName("_verbose");
  _verboseCommentList = _verbose->isChecked();
  vbox->addWidget(_verbose);
      
  vbox->addLayout(hbox);

  QWidget *buttons = new QWidget(this);
  QVBoxLayout * buttonsLayout = new QVBoxLayout(buttons);
  buttonsLayout->setMargin(0);
  buttonsLayout->setSpacing(0);

  _comment = new XTreeWidget(this);
  _comment->setObjectName("_comment");
  _comment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _comment->addColumn(tr("Date/Time"), _timeDateColumn, Qt::AlignCenter,true, "comment_date");
  _comment->addColumn(tr("Type"),    _itemColumn, Qt::AlignCenter,true, "type");
  _comment->addColumn(tr("Source"),  _itemColumn, Qt::AlignCenter,true, "comment_source");
  _comment->addColumn(tr("User Account"),    _userColumn, Qt::AlignCenter,true, "comment_user");
  _comment->addColumn(tr("Comment"), -1,          Qt::AlignLeft,  true, "first");
  _comment->addColumn(tr("Public"),    _ynColumn, Qt::AlignLeft, false, "comment_public");
  hbox->addWidget(_comment);

  _browser = new QTextBrowser(this);
  _browser->setObjectName("_browser");
  _browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _browser->setOpenLinks(false);
  hbox->addWidget(_browser);

  _newComment = new QPushButton(tr("New"), buttons);
  _newComment->setObjectName("_newComment");
  buttonsLayout->addWidget(_newComment);

  _viewComment = new QPushButton(tr("View"), buttons);
  _viewComment->setObjectName("_viewComment");
  _viewComment->setEnabled(false);
  buttonsLayout->addWidget(_viewComment);

  _editComment = new QPushButton(tr("Edit"), buttons);
  _editComment->setObjectName("_editComment");
  _editComment->setEnabled(false);
  buttonsLayout->addWidget(_editComment);

  QSpacerItem *_buttonSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  buttonsLayout->addItem(_buttonSpacer);
  buttons->setLayout(buttonsLayout);
  hbox->addWidget(buttons);
  
  _editmap = new QMultiMap<int, bool>();
  _editmap2 = new QMultiMap<int, bool>();

  connect(_newComment, SIGNAL(clicked()), this, SLOT( sNew()));
  connect(_viewComment, SIGNAL(clicked()), this, SLOT( sView()));
  connect(_editComment, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_comment, SIGNAL(valid(bool)), this, SLOT(sCheckButtonPriv(bool)));
  connect(_comment, SIGNAL(itemSelected(int)), _viewComment, SLOT(animateClick()));
  connect(_browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));
  connect(_verbose, SIGNAL(toggled(bool)), this, SLOT(setVerboseCommentList(bool)));

  setFocusProxy(_comment);
  setVerboseCommentList(_verboseCommentList);
}

int Comments::type() const
{
  CommentMap *elem = _strMap.value(_sourcetype);
  return elem ? elem->doctypeId : Uninitialized;
}

void Comments::setType(int sourceType)
{
  CommentMap *elem = _intMap.value(sourceType);
  setType(elem ? elem->doctypeStr : "");
}

void Comments::setType(QString sourceType)
{
  _sourcetype = sourceType;
}

void Comments::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Comments::setReadOnly(bool pReadOnly)
{
  _newComment->setDisabled(pReadOnly);
  _editable = !pReadOnly;
}

void Comments::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);

  comment newdlg(this, "", true);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
  {
    emit commentAdded();
    refresh();
  }
}

void Comments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);
  params.append("comment_id", _comment->id());
  params.append("commentIDList", _commentIDList);

  comment newdlg(this, "", true);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.set(params);
  newdlg.exec();
}

void Comments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);
  params.append("comment_id", _comment->id());
  params.append("commentIDList", _commentIDList);

  comment newdlg(this, "", true);
  newdlg.setWindowModality(Qt::WindowModal);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Comments::refresh()
{
  _browser->document()->clear();
  _editmap->clear();
  _editmap2->clear();
  if(-1 == _sourceid)
  {
    _comment->clear();
    return;
  }

  XSqlQuery comment;
  if(_sourcetype != "CRMA")
  {
    _comment->hideColumn(2);
    comment.prepare( "SELECT comment_id, comment_date, comment_source,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first,"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_public, "
                     "       comment_user=getEffectiveXtUser() AS self "
                     "FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     "WHERE ( (comment_source=:source)"
                     " AND (comment_source_id=:sourceid) ) "
                     "ORDER BY comment_date DESC;" );
  }
  else
  {
    // If it's CRMAccount we want to do some extra joining in our SQL
    _comment->showColumn(2);
    comment.prepare( "SELECT comment_id, comment_date, comment_source,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first,"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_public, "
                     "       comment_user=getEffectiveXtUser() AS self "
                     "  FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:source)"
                     "   AND (comment_source_id=:sourceid) ) "
                     " UNION "
                     "SELECT comment_id, comment_date, comment_source,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_public, "
                     "       comment_user=getEffectiveXtUser() AS self "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceCust)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_cust_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date, comment_source,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_public, "
                     "       comment_user=getEffectiveXtUser() AS self "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceVend)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_vend_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date, comment_source,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_public, "
                     "       comment_user=getEffectiveXtUser() AS self "
                     "  FROM cntct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceContact)"
                     "   AND (cntct_crmacct_id=:sourceid)"
                     "   AND (comment_source_id=cntct_id) ) "
                     "ORDER BY comment_date DESC;" );
    comment.bindValue(":sourceCust", "C");
    comment.bindValue(":sourceContact", "T");
    comment.bindValue(":sourceVend", "V");
  }
  comment.bindValue(":none", tr("None"));
  comment.bindValue(":source", _sourcetype);
  comment.bindValue(":sourceid", _sourceid);
  comment.exec();

  QString lclHtml = "<body>";
  QRegExp br("\r?\n");
  _commentIDList.clear();
  while(comment.next())
  {
    _editmap->insert(comment.value("comment_id").toInt(),comment.value("editable").toBool());
    _editmap2->insert(comment.value("comment_id").toInt(),comment.value("self").toBool());
    
    int cid = comment.value("comment_id").toInt();
    _commentIDList.push_back(cid);
    lclHtml += comment.value("comment_date").toDateTime().toString();
    lclHtml += " ";
    lclHtml += comment.value("type").toString();
    lclHtml += " ";
    lclHtml += comment.value("comment_user").toString();
    if(_x_metrics && _x_metrics->boolean("CommentPublicPrivate"))
    {
      lclHtml += " (";
      if(comment.value("comment_public").toBool())
        lclHtml += "Public";
      else
        lclHtml += "Private";
      lclHtml += ")";
    }
    if(userCanEdit(cid))
    {
      lclHtml += " <a href=\"edit?id=";
      lclHtml += QString::number(cid);
      lclHtml += "\">edit</a>";
    }
    lclHtml += "<p>\n<blockquote>";
    lclHtml += comment.value("comment_text").toString().replace("<", "&lt;").replace(br,"<br>\n");
    lclHtml += "</pre></blockquote>\n<hr>\n";
  }
  lclHtml += "</body>";

  _browser->document()->setHtml(lclHtml);
  comment.first();
  _comment->populate(comment);
}

void Comments::setVerboseCommentList(bool vcl)
{
  _verboseCommentList = vcl;
  _comment->setVisible(!vcl);
  _viewComment->setVisible(!vcl);
  _editComment->setVisible(!vcl);
  _browser->setVisible(vcl);
}

void Comments::anchorClicked(const QUrl & url)
{
  if(url.host().isEmpty() && url.path() == "edit")
  {
    #if QT_VERSION >= 0x050000
    int cid = QUrlQuery(url).queryItemValue("id").toInt();
    #else
    int cid = url.queryItemValue("id").toInt();
    #endif
    if(userCanEdit(cid))
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("sourceType", _sourcetype);
      params.append("source_id", _sourceid);
      params.append("comment_id", cid);
      params.append("commentIDList", _commentIDList);

      comment newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();
      refresh();
    }
  }
  else
  {
    QDesktopServices::openUrl(url);
  }
}

void Comments::sCheckButtonPriv(bool pValid)
{
  _viewComment->setEnabled(pValid);
  if(pValid)
  {
    _editComment->setEnabled(userCanEdit(_comment->id()) && _editable);
  }
  else
    _editComment->setEnabled(false);
}

bool Comments::userCanEdit(int id)
{
  QList<bool> values = _editmap->values(id);
  QList<bool> values2 = _editmap2->values(id);
  
  if(values.at(0))
  {
    if(_x_privileges && _x_privileges->check("EditOthersComments") && _editable)
      return true;
    if(_x_privileges && _x_privileges->check("EditOwnComments") && values2.at(0) && _editable)
      return true;
  }
  return false;
}
