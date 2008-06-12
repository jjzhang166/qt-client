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
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QTextEdit>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVariant>
#include <QWhatsThis>

#include <parameter.h>

#include "xcombobox.h"
#include "comment.h"

#define cNew  1
#define cEdit 2
#define cView 3

comment::comment( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) :
  QDialog( parent, name, modal, fl )
{
  setCaption(tr("Comment"));

  _commentid = -1;
  _targetId = -1;
  _mode = cNew;

  if (!name)
    setName("comment");

  QHBoxLayout *commentLayout = new QHBoxLayout( this, 5, 7, "commentLayout"); 
  QVBoxLayout *layout11  = new QVBoxLayout( 0, 0, 5, "layout11"); 
  QHBoxLayout *layout9   = new QHBoxLayout( 0, 0, 0, "layout9"); 
  QBoxLayout *layout8    = new QHBoxLayout( 0, 0, 5, "layout8"); 
  QVBoxLayout *Layout181 = new QVBoxLayout( 0, 0, 0, "Layout181"); 
  QVBoxLayout *Layout180 = new QVBoxLayout( 0, 0, 5, "Layout180"); 

  QLabel *_cmnttypeLit = new QLabel(tr("Comment Type:"), this, "_cmnttypeLit");
  layout8->addWidget( _cmnttypeLit );

  _cmnttype = new XComboBox( FALSE, this, "_cmnttype" );
  layout8->addWidget( _cmnttype );
  layout9->addLayout( layout8 );

  QSpacerItem* spacer = new QSpacerItem( 66, 10, QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout9->addItem( spacer );
  layout11->addLayout( layout9 );

  _comment = new QTextEdit( this, "_comment" );
  layout11->addWidget( _comment );
  commentLayout->addLayout( layout11 );

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  Layout180->addWidget( _close );

  _save = new QPushButton(tr("&Save"), this, "_save");
  Layout180->addWidget( _save );
  Layout181->addLayout( Layout180 );
  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
  Layout181->addItem( spacer_2 );
  commentLayout->addLayout( Layout181 );

  resize( QSize(524, 270).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

// signals and slots connections
  connect( _save, SIGNAL( clicked() ), this, SLOT( sSave() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( reject() ) );

// tab order
  setTabOrder( _cmnttype, _comment );
  setTabOrder( _comment, _save );
  setTabOrder( _save, _close );

  _source = Comments::Uninitialized;
  _cmnttype->setAllowNull(TRUE);
}

void comment::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    _source = Comments::Customer;
    _cmnttype->setType(XComboBox::CustomerCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _source = Comments::Vendor;
    _cmnttype->setType(XComboBox::VendorCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source = Comments::Item;
    _cmnttype->setType(XComboBox::ItemCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _source = Comments::ItemSite;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


//  Quotes
  param = pParams.value("quhead_id", &valid);
  if (valid)
  {
    _source = Comments::Quote;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("quitem_id", &valid);
  if (valid)
  {
    _source = Comments::QuoteItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


//  Sales Orders
  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _source = Comments::SalesOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("soitem_id", &valid);
  if (valid)
  {
    _source = Comments::SalesOrderItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Sales Orders
  param = pParams.value("tohead_id", &valid);
  if (valid)
  {
    _source = Comments::TransferOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Return Authorizations
  param = pParams.value("rahead_id", &valid);
  if (valid)
  {
    _source = Comments::ReturnAuth;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("raitem_id", &valid);
  if (valid)
  {
    _source = Comments::ReturnAuthItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

//  Purchase Orders
  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _source = Comments::PurchaseOrder;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    _source = Comments::PurchaseOrderItem;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }


  param = pParams.value("lsdetail_id", &valid);
  if (valid)
  {
    _source = Comments::LotSerial;
    _cmnttype->setType(XComboBox::LotSerialCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("prj_id", &valid);
  if (valid)
  {
    _source = Comments::Project;
    _cmnttype->setType(XComboBox::ProjectCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("warehous_id", &valid);
  if (valid)
  {
    _source = Comments::Warehouse;
    _cmnttype->setType(XComboBox::ProjectCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("addr_id", &valid);
  if (valid)
  {
    _source = Comments::Address;
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _targetId = param.toInt();
  }

  param = pParams.value("sourceType", &valid);
  if (valid)
  {
    _source = (enum Comments::CommentSources)param.toInt();
    switch (_source)
    {
      default:
        _cmnttype->setType(XComboBox::AllCommentTypes);
        break;
    }
  }

  param = pParams.value("source_id", &valid);
  if (valid)
    _targetId = param.toInt();

  param = pParams.value("comment_id", &valid);
  if (valid)
  {
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _commentid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      _comment->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _cmnttype->setEnabled(FALSE);
      _comment->setReadOnly(TRUE);
      _save->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }
}

void comment::sSave()
{
  if (_cmnttype->id() == -1)
  {
    QMessageBox::critical( this, tr("Cannot Post Comment"),
                           tr("<p>You must select a Comment Type for this "
                              "Comment before you may post it.") );
    _cmnttype->setFocus();
    return;
  }

  _query.prepare("SELECT postComment(:cmnttype_id, :source, :source_id, :text) AS result;");
  _query.bindValue(":cmnttype_id", _cmnttype->id());
  _query.bindValue(":source", Comments::_commentMap[_source].ident);
  _query.bindValue(":source_id", _targetId);
  _query.bindValue(":text", _comment->text().stripWhiteSpace());
  _query.exec();
  if (_query.first())
  {
    int result = _query.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical(this, tr("Cannot Post Comment"),
                            tr("<p>A Stored Procedure failed to run "
                               "properly.<br>(%1, %2)<br>")
                              .arg("postComment").arg(result));
      reject();
    }
    done (_query.value("result").toInt());
  }
  else if (_query.lastError().type() != QSqlError::None)
  {
    QMessageBox::critical(this, tr("Cannot Post Comment"),
                          _query.lastError().databaseText());
    reject();
  }
}

void comment::populate()
{
  _query.prepare( "SELECT comment_cmnttype_id, comment_text "
                  "FROM comment "
                  "WHERE (comment_id=:comment_id);" );
  _query.bindValue(":comment_id", _commentid);
  _query.exec();
  if (_query.first())
  {
    _cmnttype->setId(_query.value("comment_cmnttype_id").toInt());
    _comment->setText(_query.value("comment_text").toString());
  }
  else if (_query.lastError().type() != QSqlError::None)
  {
    QMessageBox::critical(this, tr("Error Selecting Comment"),
                          _query.lastError().databaseText());
    return;
  }
}
