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


#include <QHBoxLayout>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "comment.h"
#include "comments.h"


const Comments::CommentMap Comments::_commentMap[] =
  {
    CommentMap( Uninitialized,     " "   ),
    CommentMap( Address,           "ADDR"),
    CommentMap( BBOMHead,          "BBH" ),
    CommentMap( BBOMItem,          "BBI" ),
    CommentMap( BOMHead,           "BMH" ),
    CommentMap( BOMItem,           "BMI" ),
    CommentMap( BOOHead,           "BOH" ),
    CommentMap( BOOItem,           "BOI" ),
    CommentMap( CRMAccount,        "CRMA"),
    CommentMap( Contact,           "T"   ),
    CommentMap( Customer,          "C"   ),
    CommentMap( Employee,          "EMP" ),
    CommentMap( Incident,          "INCDT"),
    CommentMap( Item,              "I"   ),
    CommentMap( ItemSite,          "IS"  ),
    CommentMap( ItemSource,        "IR"  ),
    CommentMap( Location,          "L"   ),
    CommentMap( LotSerial,         "LS"   ),
    CommentMap( Opportunity,       "OPP" ),
    CommentMap( Project,           "J"   ),
    CommentMap( PurchaseOrder,     "P"   ),
    CommentMap( PurchaseOrderItem, "PI"  ),
    CommentMap( ReturnAuth,        "RA"  ),
    CommentMap( ReturnAuthItem,    "RI"  ),
    CommentMap( Quote,             "Q"   ),
    CommentMap( QuoteItem,         "QI"  ),
    CommentMap( SalesOrder,        "S"   ),
    CommentMap( SalesOrderItem,    "SI"  ),
    CommentMap( TransferOrder,     "TO"  ),
    CommentMap( TransferOrderItem, "TI"  ),
    CommentMap( Vendor,            "V"   ),
    CommentMap( Warehouse,         "WH"  ),
    CommentMap( WorkOrder,         "W"   ),
  };

Comments::Comments(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  _source = Uninitialized;
  _sourceid = -1;

  QHBoxLayout *main = new QHBoxLayout(this);
  main->setMargin(0);
  main->setSpacing(7);

  QWidget *buttons = new QWidget(this);
  QVBoxLayout * buttonsLayout = new QVBoxLayout(buttons);
  buttonsLayout->setMargin(0);
  buttonsLayout->setSpacing(0);

  _comment = new XTreeWidget(this);
  _comment->setObjectName("_comment");
  _comment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _comment->addColumn(tr("Date/Time"), _timeDateColumn, Qt::AlignCenter,true, "comment_date");
  _comment->addColumn(tr("Type"),    _itemColumn, Qt::AlignCenter,true, "type");
  _comment->addColumn(tr("User"),    _userColumn, Qt::AlignCenter,true, "comment_user");
  _comment->addColumn(tr("Comment"), -1,          Qt::AlignLeft,  true, "first");
  main->addWidget(_comment);

  _newComment = new QPushButton(tr("New"), buttons, "_newComment");
  buttonsLayout->addWidget(_newComment);

  _viewComment = new QPushButton(tr("View"), buttons, "_viewComment");
  _viewComment->setEnabled(FALSE);
  buttonsLayout->addWidget(_viewComment);

  QSpacerItem *_buttonSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  buttonsLayout->addItem(_buttonSpacer);
  buttons->setLayout(buttonsLayout);
  main->addWidget(buttons);

  setLayout(main);

  connect(_newComment, SIGNAL(clicked()), this, SLOT( sNew()));
  connect(_viewComment, SIGNAL(clicked()), this, SLOT( sView()));
  connect(_comment, SIGNAL(valid(bool)), _viewComment, SLOT(setEnabled(bool)));
  connect(_comment, SIGNAL(itemSelected(int)), _viewComment, SLOT(animateClick()));

  setFocusProxy(_comment);
}

void Comments::setType(enum CommentSources pSource)
{
  _source = pSource;
}

void Comments::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Comments::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
    _newComment->setEnabled(FALSE);
  else
    _newComment->setEnabled(TRUE);
}

void Comments::sNew()
{ 
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  comment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Comments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("comment_id", _comment->id());

  comment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Comments::refresh()
{
  if(-1 == _sourceid)
  {
    _comment->clear();
    return;
  }

  XSqlQuery comment;
  if(_source != CRMAccount)
  {
    comment.prepare( "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first "
                     "FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     "WHERE ( (comment_source=:source)"
                     " AND (comment_source_id=:sourceid) ) "
                     "ORDER BY comment_date;" );
  }
  else
  {
    // If it's CRMAccount we want to do some extra joining in our SQL
    comment.prepare( "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first "
                     "  FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:source)"
                     "   AND (comment_source_id=:sourceid) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)) "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceCust)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_cust_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)) "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceVend)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_vend_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)) "
                     "  FROM cntct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceContact)"
                     "   AND (cntct_crmacct_id=:sourceid)"
                     "   AND (comment_source_id=cntct_id) ) "
                     "ORDER BY comment_date;" );
    comment.bindValue(":sourceCust", _commentMap[Customer].ident);
    comment.bindValue(":sourceContact", _commentMap[Contact].ident);
    comment.bindValue(":sourceVend", _commentMap[Vendor].ident);
  }
  comment.bindValue(":none", tr("None"));
  comment.bindValue(":source", _commentMap[_source].ident);
  comment.bindValue(":sourceid", _sourceid);
  comment.exec();
  _comment->populate(comment);
}
