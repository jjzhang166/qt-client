/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postPoReturnCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include "errorReporter.h"

postPoReturnCreditMemo::postPoReturnCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));

  _qty->setPrecision(omfgThis->qtyVal());

  _porejectid = -1;
}

postPoReturnCreditMemo::~postPoReturnCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void postPoReturnCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postPoReturnCreditMemo::set(const ParameterList & pParams)
{
  XSqlQuery postet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("poreject_id", &valid);
  if(valid)
  {
    _porejectid = param.toInt();
    postet.prepare("SELECT pohead_curr_id,"
              "       COALESCE(item_number, poitem_vend_item_number) AS itemnumber,"
              "       poreject_qty,"
              "       (COALESCE(recv_purchcost, poitem_unitprice) * poreject_qty) AS itemAmount"
              "  FROM poreject JOIN poitem ON (poitem_id=poreject_poitem_id)"
              "                JOIN pohead ON (pohead_id=poitem_pohead_id)"
              "                LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id)"
              "                LEFT OUTER JOIN item ON (itemsite_item_id=item_id)"
              "                LEFT OUTER JOIN recv ON (recv_id=poreject_recv_id) "
              " WHERE (poreject_id=:poreject_id);");
    postet.bindValue(":poreject_id", _porejectid);
    postet.exec();
    if(postet.first())
    {
      _item->setText(postet.value("itemNumber").toString());
      _qty->setDouble(postet.value("poreject_qty").toDouble());
      _amount->set(postet.value("itemAmount").toDouble(), postet.value("pohead_curr_id").toInt(), QDate::currentDate(), false);
    }
  }

  return NoError;
}

void postPoReturnCreditMemo::sPost()
{
  XSqlQuery postPost;
  postPost.prepare("SELECT postPoReturnCreditMemo(:poreject_id, :amount) AS result;");
  postPost.bindValue(":poreject_id", _porejectid);
  postPost.bindValue(":amount", _amount->localValue());
  if(!postPost.exec())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Credit Memo"),
                         postPost, __FILE__, __LINE__);
    return;
  }

  accept();
}
