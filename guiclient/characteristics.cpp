/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "characteristics.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "characteristic.h"
#include "errorReporter.h"
#include "storedProcErrorLookup.h"

characteristics::characteristics(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_char, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainCharacteristics"))
  {
    connect(_char, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),     _delete, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _edit, SLOT(setEnabled(bool)));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }
  else
  {
    _new->setEnabled(false);
    connect(_char, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_char, SIGNAL(valid(bool)),       _view, SLOT(setEnabled(bool)));
  }

  _char->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _char->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "char_notes");

  sFillList();
}

characteristics::~characteristics()
{
  // no need to delete child widgets, Qt does it all for us
}

void characteristics::languageChange()
{
  retranslateUi(this);
}

void characteristics::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  characteristic newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void characteristics::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("char_id", _char->id());

  characteristic newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void characteristics::sDelete()
{
  XSqlQuery characteristicsDelete;
  characteristicsDelete.prepare("SELECT deleteCharacteristic(:char_id) AS result;");
  characteristicsDelete.bindValue(":char_id", _char->id());
  characteristicsDelete.exec();
  if (characteristicsDelete.first())
  {
    sFillList();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Cannot Delete Characteristic"),
                                characteristicsDelete, __FILE__, __LINE__))
  {
    return;
  }
}

void characteristics::sPrint()
{
  orReport report("CharacteristicsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void characteristics::sFillList()
{
  XSqlQuery characteristicsFillList;
  characteristicsFillList.prepare("SELECT char_id, char_name, char_notes "
            "FROM char "
            "ORDER BY char_name;");
  characteristicsFillList.exec();
  _char->populate(characteristicsFillList);
  if (characteristicsFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, characteristicsFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void characteristics::sPopulateMenu(QMenu *)
{
}
