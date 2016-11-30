/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "adjustInvValue.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"

adjustInvValue::adjustInvValue(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_altAccnt,                       SIGNAL(toggled(bool)), this, SLOT(sToggleAltAccnt(bool)));
  connect(_post,                           SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_site,                           SIGNAL(newID(int)), this, SLOT(sValidateSite(int)));
  connect(_item,                           SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_newValue,                       SIGNAL(textChanged(QString)), this, SLOT(sUpdateCost()));

  _captive = false;

  _item->setType(ItemLineEdit::cActive);
  _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
  _item->addExtraClause( QString("(itemsite_costmethod='A')") );
  _item->addExtraClause( QString("(itemsite_qtyonhand <> 0.0)") );
  _site->setType(WComboBox::AllActiveInventory);
  _newValue->setValidator(omfgThis->costVal());

  if (!_metrics->boolean("MultiWhs"))
  {
    _siteLit->hide();
    _site->hide();
  }

  _itemsiteid = -1;
  _qtyonhand = -0;
  sValidateSite(_site->id());
}

adjustInvValue::~adjustInvValue()
{
  // no need to delete child widgets, Qt does it all for us
}

void adjustInvValue::languageChange()
{
  retranslateUi(this);
}

enum SetResponse adjustInvValue::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = true;

    _item->setItemsiteid(param.toInt());
    _item->setEnabled(false);
    _site->setEnabled(false);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(true);
      _site->setEnabled(false);
      _post->hide();
      _close->setText(tr("&Close"));
    }
  }

  return NoError;
}

void adjustInvValue::sPopulate()
{
  XSqlQuery adjustPopulate;
  if (_item->id() == -1)
  {
    _itemsiteid = -1;
    _qoh->setText("0.0");
    _qtyonhand = 0.0;
    _currentValue->setText("0.0");
    _currentCost->setText("0.0");
    _newCost->setText("0.0");
    _post->setEnabled(false);
  }
  else
  {
    QString sql = "SELECT itemsite_id, itemsite_qtyonhand, "
             "       formatQty(itemsite_qtyonhand) AS f_qoh, "
             "       formatMoney(itemsite_value) AS f_value, "
             "       CASE WHEN itemsite_qtyonhand > 0 THEN "
             "         formatCost(itemsite_value / itemsite_qtyonhand) "
             "       ELSE formatCost(0) END AS f_cost "
             "FROM itemsite "
             "WHERE ((itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
             "  AND (itemsite_item_id=<? value(\"item_id\") ?>))";

    ParameterList params;
    params.append("warehous_id", _site->id());
    params.append("item_id", _item->id());
    MetaSQLQuery mql(sql);
    adjustPopulate = mql.toQuery(params);
    if (adjustPopulate.first())
    {
      _itemsiteid = adjustPopulate.value("itemsite_id").toInt();
      _qtyonhand = adjustPopulate.value("itemsite_qtyonhand").toDouble();
      _qoh->setText(adjustPopulate.value("f_qoh").toString());
      _currentValue->setText(adjustPopulate.value("f_value").toString());
      _currentCost->setText(adjustPopulate.value("f_cost").toString());
      _post->setEnabled(true);
    }
    else
    {
      _itemsiteid = -1;
      _qoh->setText("0.0");
      _qtyonhand = 0.0;
      _currentValue->setText("0.0");
      _currentCost->setText("0.0");
      _newCost->setText("0.0");
      _post->setEnabled(false);
    }
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Inventory Values"),
                                  adjustPopulate, __FILE__, __LINE__))
    {
        return;
    }
  }
}

void adjustInvValue::sPost()
{
  XSqlQuery adjustPost;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_itemsiteid == -1, _item,
                          tr("You must select a valid Itemsite before posting this transaction.") )
         << GuiErrorCheck(_newValue->text().length() == 0, _newValue,
                          tr("<p>You must enter a valid New Value before posting this transaction.") )
         << GuiErrorCheck(_altAccnt->isChecked() && ! _accnt->isValid(), _accnt,
                          tr("<p>You must enter a valid Alternate Ledger Account before posting this transaction.") )
         << GuiErrorCheck(_qtyonhand <= 0.0, _item,
                          tr("You must select an Itemsite with a positive Qty on Hand before posting this transaction.") )
    ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Post Transaction"), errors))
    return;

  ParameterList params;
  params.append("newValue", _newValue->toDouble());
  params.append("itemsite_id", _itemsiteid);
  if (_altAccnt->isChecked() && _accnt->id() != -1)
    params.append("accnt_id", _accnt->id());

  QString sql = "SELECT adjustInvValue(<? value('itemsite_id') ?>, "
                "                      <? value('newValue') ?>, "
                "                      <? value('accnt_id') ?>) AS result;";

  MetaSQLQuery mql(sql);
  adjustPost = mql.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Posting Adjustment"),
                                adjustPost, __FILE__, __LINE__))
  {
      return;
  }

  QMessageBox::information( this, tr("Post Successful"),
                         tr( "<p>Value successfully updated. ") );
  sPopulate();
  _newValue->clear();
}

void adjustInvValue::sUpdateCost()
{
  XSqlQuery adjustUpdateCost;
  QString sql = "SELECT "
           "       CASE WHEN itemsite_qtyonhand > 0 THEN "
           "         formatCost(<? value(\"newvalue\") ?> / itemsite_qtyonhand) "
           "       ELSE formatCost(0) END AS f_cost "
           "FROM itemsite "
           "WHERE ((itemsite_warehous_id=<? value(\"warehous_id\") ?>)"
           "  AND (itemsite_item_id=<? value(\"item_id\") ?>))";

  ParameterList params;
  params.append("warehous_id", _site->id());
  params.append("item_id", _item->id());
  params.append("newvalue", _newValue->toDouble());
  MetaSQLQuery mql(sql);
  adjustUpdateCost = mql.toQuery(params);
  if (adjustUpdateCost.first())
  {
    _newCost->setText(adjustUpdateCost.value("f_cost").toString());
  }
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Cost"),
                                adjustUpdateCost, __FILE__, __LINE__))
  {
      return;
  }
}

void adjustInvValue::sValidateSite(int siteid)
{
  if (siteid != -1)
  {
    _item->setEnabled(true);
  }
  else
  {
    _item->setId(-1);
    _item->setEnabled(false);
  }
  sPopulate();
}

void adjustInvValue::sToggleAltAccnt(bool checked)
{
  _accnt->setReadOnly(!checked);
}
