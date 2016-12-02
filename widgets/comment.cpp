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
#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QToolTip>
#include <QVBoxLayout>
#include <QVariant>
#include <QWhatsThis>
#include <QVector>
#include <QCheckBox>

#include <parameter.h>

#include "comment.h"
#include "shortcuts.h"
#include "xcombobox.h"
#include "xtextedit.h"
#include "xtsettings.h"

#define cNew  1
#define cEdit 2
#define cView 3

comment::comment( QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl ) :
  QDialog( parent, fl )
{
  setupUi(this);

  if (name)
    setObjectName(name);
  if(modal)
    setWindowModality(Qt::WindowModal);

  _commentid = -1;
  _targetId = -1;
  _mode = cNew;

  if(!(_x_metrics && _x_metrics->boolean("CommentPublicPrivate")))
    _public->hide();
  _public->setChecked(_x_metrics && _x_metrics->boolean("CommentPublicDefault"));

  _comment->setSpellEnable(true);

  _close = buttonBox->button(QDialogButtonBox::Cancel);
  _close->setObjectName("_close");

  _save = buttonBox->button(QDialogButtonBox::Save);
  _save->setObjectName("_save");

  _prev = buttonBox->addButton(tr("&Previous"), QDialogButtonBox::ActionRole);
  _prev->setObjectName("_prev");

  _next = buttonBox->addButton(tr("&Next"), QDialogButtonBox::ActionRole);
  _next->setObjectName("_next");

  _more = buttonBox->addButton(tr("&More"), QDialogButtonBox::ActionRole);
  _more->setObjectName("_more");
  _more->setCheckable(true);

  _comments->setReadOnly(true);
  _comments->findChild<XCheckBox*>("_verbose")->setForgetful(true);
  _comments->findChild<XCheckBox*>("_verbose")->hide();
  _comments->findChild<XCheckBox*>("_verbose")->setChecked(false);
  _comments->_newComment->setVisible(false);
  _comments->setVerboseCommentList(true);
  _comments->setVisible(false);
  _comments->setEditable(false);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_next, SIGNAL(clicked()), this, SLOT(sNextComment()));
  connect(_prev, SIGNAL(clicked()), this, SLOT(sPrevComment()));
  connect(_more, SIGNAL(toggled(bool)), _comments, SLOT(setVisible(bool)));

  _sourcetype = "";

  shortcuts::setStandardKeys(this);

  QSize savedSize = xtsettingsValue("comment-" + objectName() + "/geometry/size").toSize();
  if (savedSize.isValid())
    resize(savedSize);
  connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
}

// ::closeEvent() would be better but it didn't get called
void comment::saveSize()
{
  xtsettingsSetValue("comment-" + objectName() + "/geometry/size", size());
}

void comment::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("comment_id", &valid);
  if (valid)
  {
    _cmnttype->setType(XComboBox::AllCommentTypes);
    _commentid = param.toInt();
    populate();
  }

  param = pParams.value("commentIDList", &valid);
  if (valid)
  {
    _commentIDList = param.toList();
    _commentLocation = _commentIDList.indexOf(_commentid);

    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 

    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      
      _cmnttype->setFocus();
      _next->setVisible(false);
      _prev->setVisible(false);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _next->setVisible(false);
      _prev->setVisible(false);
      _cmnttype->setEnabled(false);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _next->setVisible(true);
      _prev->setVisible(true);
      _cmnttype->setEnabled(false);
      _comment->setReadOnly(true);
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setShortcut(QKeySequence::Close);
      _more->hide();

      _close->setFocus();
    }
  }

  param = pParams.value("sourceType", &valid);
  if (valid)
  {
    _sourcetype = param.toString();
    if(!(_mode == cEdit || _mode == cView))
    {
      //      CommentMap *map = Comments::commentMap().value(_sourcetype);
      //      _cmnttype->setType(map ? (XComboBox::XComboBoxTypes)map->doctypeId : XComboBox::AllCommentTypes);
      if (_sourcetype == "ADDR")
        _cmnttype->setType(XComboBox::AddressCommentTypes);
      else if (_sourcetype == "BBH")
        _cmnttype->setType(XComboBox::BBOMHeadCommentTypes);
      else if (_sourcetype == "BBI")
        _cmnttype->setType(XComboBox::BBOMItemCommentTypes);
      else if (_sourcetype == "BMH")
        _cmnttype->setType(XComboBox::BOMHeadCommentTypes);
      else if (_sourcetype == "BMI")
        _cmnttype->setType(XComboBox::BOMItemCommentTypes);
      else if (_sourcetype == "BOH")
        _cmnttype->setType(XComboBox::BOOHeadCommentTypes);
      else if (_sourcetype == "BOI")
        _cmnttype->setType(XComboBox::BOOItemCommentTypes);
      else if (_sourcetype == "CRMA")
        _cmnttype->setType(XComboBox::CRMAccountCommentTypes);
      else if (_sourcetype == "T")
        _cmnttype->setType(XComboBox::ContactCommentTypes);
      else if (_sourcetype == "C")
        _cmnttype->setType(XComboBox::CustomerCommentTypes);
      else if (_sourcetype == "EMP")
        _cmnttype->setType(XComboBox::EmployeeCommentTypes);
      else if (_sourcetype == "FX")
        _cmnttype->setType(XComboBox::ExchangeRateCommentTypes);
      else if (_sourcetype == "INCDT")
        _cmnttype->setType(XComboBox::IncidentCommentTypes);
      else if (_sourcetype == "I")
        _cmnttype->setType(XComboBox::ItemCommentTypes);
      else if (_sourcetype == "IS")
        _cmnttype->setType(XComboBox::ItemSiteCommentTypes);
      else if (_sourcetype == "IR")
        _cmnttype->setType(XComboBox::ItemSourceCommentTypes);
      else if (_sourcetype == "L")
        _cmnttype->setType(XComboBox::LocationCommentTypes);
      else if (_sourcetype == "LS")
        _cmnttype->setType(XComboBox::LotSerialCommentTypes);
      else if (_sourcetype == "OPP")
        _cmnttype->setType(XComboBox::OpportunityCommentTypes);
      else if (_sourcetype == "J")
        _cmnttype->setType(XComboBox::ProjectCommentTypes);
      else if (_sourcetype == "P")
        _cmnttype->setType(XComboBox::PurchaseOrderCommentTypes);
      else if (_sourcetype == "PI")
        _cmnttype->setType(XComboBox::PurchaseOrderItemCommentTypes);
      else if (_sourcetype == "RA")
        _cmnttype->setType(XComboBox::ReturnAuthCommentTypes);
      else if (_sourcetype == "RI")
        _cmnttype->setType(XComboBox::ReturnAuthItemCommentTypes);
      else if (_sourcetype == "Q")
        _cmnttype->setType(XComboBox::QuoteCommentTypes);
      else if (_sourcetype == "QI")
        _cmnttype->setType(XComboBox::QuoteItemCommentTypes);
      else if (_sourcetype == "S")
        _cmnttype->setType(XComboBox::SalesOrderCommentTypes);
      else if (_sourcetype == "SI")
        _cmnttype->setType(XComboBox::SalesOrderItemCommentTypes);
      else if (_sourcetype == "TA")
        _cmnttype->setType(XComboBox::TaskCommentTypes);
      else if (_sourcetype == "TATC")
        _cmnttype->setType(XComboBox::TimeAttendanceCommentTypes);
      else if (_sourcetype == "TD")
        _cmnttype->setType(XComboBox::TodoItemCommentTypes);
      else if (_sourcetype == "TO")
        _cmnttype->setType(XComboBox::TransferOrderCommentTypes);
      else if (_sourcetype == "TI")
        _cmnttype->setType(XComboBox::TransferOrderItemCommentTypes);
      else if (_sourcetype == "V")
        _cmnttype->setType(XComboBox::VendorCommentTypes);
      else if (_sourcetype == "WH")
        _cmnttype->setType(XComboBox::WarehouseCommentTypes);
      else if (_sourcetype == "W")
        _cmnttype->setType(XComboBox::WorkOrderCommentTypes);
      else
        _cmnttype->setType(XComboBox::AllCommentTypes);
    }
  }

  param = pParams.value("source_id", &valid);
  if (valid)
  {
    _targetId = param.toInt();
  }

  _comments->setType(_sourcetype);
  _comments->setId(_targetId);
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

  int result = -1;
  if(_mode == cNew)
  {
    _query.prepare("SELECT postComment(:cmnttype_id, :source_type, :source_id, :text, :public) AS result;");
    _query.bindValue(":cmnttype_id", _cmnttype->id());
    _query.bindValue(":source_type", _sourcetype);
    _query.bindValue(":source_id", _targetId);
    _query.bindValue(":text", _comment->toPlainText().trimmed());
    _query.bindValue(":public", _public->isChecked());
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
      }
    }
    else if (_query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("Cannot Post Comment"),
                            _query.lastError().databaseText());
    }
  }
  else if(_mode == cEdit)
  {
    result = _commentid;
    _query.prepare("UPDATE comment SET comment_text=:text, comment_public=:public WHERE comment_id=:comment_id");
    _query.bindValue(":text", _comment->toPlainText().trimmed());
    _query.bindValue(":comment_id", _commentid);
    _query.bindValue(":public", _public->isChecked());
    _query.exec();
    if(_query.lastError().type() != QSqlError::NoError)
    {
      QMessageBox::critical(this, tr("Cannot Post Comment"),
                            _query.lastError().databaseText());
      return;
    }
  }

  if(result < 0)
    done(result);
  else
    reject();
}

void comment::populate()
{
  _query.prepare( "SELECT comment_cmnttype_id, comment_text, comment_public "
                  "FROM comment "
                  "WHERE (comment_id=:comment_id);" );
  _query.bindValue(":comment_id", _commentid);
  _query.exec();
  if (_query.first())
  {
    _cmnttype->setId(_query.value("comment_cmnttype_id").toInt());
    _comment->setText(_query.value("comment_text").toString());
    _public->setChecked(_query.value("comment_public").toBool());
  }
  else if (_query.lastError().type() != QSqlError::NoError)
  {
    QMessageBox::critical(this, tr("Error Selecting Comment"),
                          _query.lastError().databaseText());
    return;
  }
}

void comment::sNextComment()
{
  if((_commentLocation+1) < _commentIDList.size())
  {
    _commentLocation++;
    _commentid = _commentIDList[_commentLocation].toInt();
    populate();
    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 

    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 
  }
}

void comment::sPrevComment()
{
  if((_commentLocation-1) >= 0)
  {
    _commentLocation--;
    _commentid = _commentIDList[_commentLocation].toInt();
    populate();
    if((_commentLocation-1) >= 0)
      _prev->setEnabled(true); 
    else
      _prev->setEnabled(false); 

    if((_commentLocation+1) < _commentIDList.size())
      _next->setEnabled(true); 
    else
      _next->setEnabled(false); 
  }
}
