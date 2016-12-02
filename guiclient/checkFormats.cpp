/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkFormats.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "checkFormat.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

checkFormats::checkFormats(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  if (_privileges->check("MaintainCheckFormats"))
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  else
    _new->setEnabled(false);
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_form, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(sHandleButtons()));

  _form->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "form_name");
  _form->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "form_descrip");

  sFillList();
}

checkFormats::~checkFormats()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkFormats::languageChange()
{
  retranslateUi(this);
}

void checkFormats::sHandleButtons()
{
  XTreeWidgetItem *selected = (XTreeWidgetItem*)_form->currentItem();
  if (selected && _privileges->check("MaintainCheckFormats"))
  {
    _edit->setEnabled(true);
    _delete->setEnabled(true);
  }
  else
  {
    _edit->setEnabled(false);
    _delete->setEnabled(false);
  }
}

void checkFormats::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  checkFormat newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void checkFormats::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("form_id", _form->id());

  checkFormat newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void checkFormats::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("form_id", _form->id());

  checkFormat newdlg(this, "", true);
  newdlg.set(params);

  newdlg.exec();
}

void checkFormats::sDelete()
{
  XSqlQuery checkDelete;
  checkDelete.prepare("SELECT deleteForm(:form_id) AS result;");
  checkDelete.bindValue(":form_id", _form->id());
  checkDelete.exec();
  if (checkDelete.first())
  {
    int result = checkDelete.value("result").toInt();
    if (result < 0)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Check Format"),
                           storedProcErrorLookup("deleteForm", result),
                           __FILE__, __LINE__);
      return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Check Format"),
                                checkDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void checkFormats::sFillList()
{
  _form->populate( "SELECT form_id, form_name, form_descrip "
                   "FROM form "
                   "WHERE form_key='Chck' "
                   "ORDER BY form_name;" );
}
