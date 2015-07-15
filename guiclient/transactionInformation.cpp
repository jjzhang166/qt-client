/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "transactionInformation.h"

#include "errorReporter.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

transactionInformation::transactionInformation(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _item->setReadOnly(true);

  _transactionQty->setPrecision(omfgThis->qtyVal());
  _qohBefore->setPrecision(omfgThis->qtyVal());
  _qohAfter->setPrecision(omfgThis->qtyVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

transactionInformation::~transactionInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void transactionInformation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse transactionInformation::set(const ParameterList &pParams)
{
  XSqlQuery transactionet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("invhist_id", &valid);
  if (valid)
  {
    _invhistid = param.toInt();

    transactionet.prepare( "SELECT *, "
               "       CASE WHEN (invhist_transtype IN ('EX', 'IM', 'SH', 'SI')) THEN (invhist_invqty * -1.0)"
               "            ELSE invhist_invqty"
               "       END AS adjinvqty "
               "FROM invhist "
               "WHERE (invhist_id=:invhist_id);" );
    transactionet.bindValue(":invhist_id", _invhistid);
    transactionet.exec();
    if (transactionet.first())
    {
      _analyze->setChecked(transactionet.value("invhist_analyze").toBool());
      _analyzeInit = transactionet.value("invhist_analyze").toBool();
      _transactionType->setText(transactionet.value("invhist_transtype").toString());
      _transactionDate->setDate(transactionet.value("invhist_transdate").toDate());
      _createdDate->setDate(transactionet.value("invhist_created").toDate());
      _username->setText(transactionet.value("invhist_user").toString());
      _item->setItemsiteid(transactionet.value("invhist_itemsite_id").toInt());
      _transactionQty->setText(formatQty(transactionet.value("adjinvqty").toDouble()));
      _qohBefore->setText(formatQty(transactionet.value("invhist_qoh_before").toDouble()));
      _qohAfter->setText(formatQty(transactionet.value("invhist_qoh_after").toDouble()));
      _notes->setText(transactionet.value("invhist_comments").toString());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Transaction Information"),
                           transactionet, __FILE__, __LINE__))
    {
      return UndefinedError;
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "edit")
      _mode = cEdit;
    else if (param.toString() == "view")
    {
      _mode = cView;

      _analyze->setEnabled(false);

      _save->hide();
      _close->setText(tr("&Close"));
    }
  }

  return NoError;
}

void transactionInformation::sSave()
{
  // #19160 Only save if the Analyze flag has been edited
  if (_analyze->isChecked() != _analyzeInit)
  {
    XSqlQuery transactionSave;
    transactionSave.prepare( "UPDATE invhist "
             "SET invhist_analyze=:invhist_analyze "
             "WHERE (invhist_id=:invhist_id);" );
    transactionSave.bindValue(":invhist_analyze", QVariant(_analyze->isChecked()));
    transactionSave.bindValue(":invhist_id", _invhistid);
    transactionSave.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Transaction Information"),
                           transactionSave, __FILE__, __LINE__))
    {
      return;
    }
  }
  accept();
}
