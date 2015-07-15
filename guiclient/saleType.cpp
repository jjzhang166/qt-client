/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "saleType.h"

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#include <QVariant>
#include <QMessageBox>

saleType::saleType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _saletypeid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_default, SIGNAL(clicked()), this, SLOT(sDefaultChecked()));
}

saleType::~saleType()
{
  // no need to delete child widgets, Qt does it all for us
}

void saleType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse saleType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("saletype_id", &valid);
  if (valid)
  {
    _saletypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(false);
      _active->setEnabled(false);
      _default->setEnabled(false);
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void saleType::sSave()
{
  XSqlQuery saleTypeSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_code->text().length() == 0, _code,
                          tr("You must enter a valid Sale Type Code "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Sale Type"), errors))
    return;

  MetaSQLQuery mql = mqlLoad("saletype", "table");
  ParameterList params;
  if (_mode == cNew)
    params.append("NewMode");
  else
  {
    params.append("EditMode");
    params.append("saletype_id", _saletypeid);
  }
  params.append("saletype_code", _code->text());
  params.append("saletype_descr", _description->text());
  params.append("saletype_active", QVariant(_active->isChecked()));
  params.append("saletype_default", QVariant(_default->isChecked()));
  saleTypeSave = mql.toQuery(params);
  if (saleTypeSave.first() && _mode == cNew)
    _saletypeid = saleTypeSave.value("saletype_id").toInt();

  done(_saletypeid);
}

void saleType::sCheck()
{
  _code->setText(_code->text().trimmed());
  if (_code->text().length())
  {
    MetaSQLQuery mql = mqlLoad("saletype", "table");
    ParameterList params;
    params.append("CheckMode");
    params.append("saletype_id", _saletypeid);
    params.append("saletype_code", _code->text());
    XSqlQuery saleTypeCheck = mql.toQuery(params);
    if (saleTypeCheck.first())
    {
      _saletypeid = saleTypeCheck.value("saletype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
    }
  }
}

void saleType::populate()
{
  MetaSQLQuery mql = mqlLoad("saletype", "table");
  ParameterList params;
  params.append("ViewMode");
  params.append("saletype_id", _saletypeid);
  XSqlQuery saleTypePopulate = mql.toQuery(params);
  if (saleTypePopulate.first())
  {
    _code->setText(saleTypePopulate.value("saletype_code"));
    _description->setText(saleTypePopulate.value("saletype_descr"));
    _active->setChecked(saleTypePopulate.value("saletype_active").toBool());
    _default->setChecked(saleTypePopulate.value("saletype_default").toBool());
  }
}

void saleType::sDefaultChecked()
{
  XSqlQuery setDefault;
  setDefault.prepare("UPDATE saletype SET saletype_default = FALSE WHERE saletype_id <> :saletype_id;");
  setDefault.bindValue(":saletype_id", _saletypeid);
  setDefault.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Defaults"),
                              setDefault, __FILE__, __LINE__))
    return;
}

