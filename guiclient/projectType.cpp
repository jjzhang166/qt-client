/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projectType.h"

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#include <QMessageBox>
#include <QVariant>

projectType::projectType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

}

projectType::~projectType()
{
  // no need to delete child widgets, Qt does it all for us
}

void projectType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse projectType::set(const ParameterList &pParams)
{
  XSqlQuery prType;
  QVariant param;
  bool     valid;

  param = pParams.value("prjtype_id", &valid);
  if (valid)
  {
    _prjtypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      prType.exec("SELECT NEXTVAL('prjtype_prjtype_id_seq') AS prjtype_id;");
      if (prType.first())
        _prjtypeid = prType.value("prjtype_id").toInt();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _typeCode->setEnabled(false);
      _typeDescr->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void projectType::sClose()
{
  XSqlQuery typeClose;
  if (_mode == cNew)
  {
    typeClose.prepare( "DELETE FROM prjtype "
                "WHERE (prjtype_id=:prjtype_id);" );
    typeClose.bindValue(":prjtype_id", _prjtypeid);
    typeClose.exec();
  }

  close();
}

void projectType::sSave()
{
  XSqlQuery typeSave;
  if (_typeCode->text().trimmed().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Project Type"),
                          tr("You must enter a Code for this Project Type before you may save it.") );
    _typeCode->setFocus();
    return;
  }

  if (_mode == cNew)
    typeSave.prepare( "INSERT INTO prjtype "
               "(prjtype_id, prjtype_code, prjtype_descr, prjtype_active) "
               "VALUES "
               "(:prjtype_id, :prjtype_typeCode, :prjtype_typeDescr, :prjtype_active);" );
  else
    typeSave.prepare( "UPDATE prjtype "
               "SET prjtype_code=:prjtype_typeCode, prjtype_descr=:prjtype_typeDescr, prjtype_active=:prjtype_active "
               "WHERE (prjtype_id=:prjtype_id);" );

  typeSave.bindValue(":prjtype_id", _prjtypeid);
  typeSave.bindValue(":prjtype_typeCode", _typeCode->text());
  typeSave.bindValue(":prjtype_typeDescr", _typeDescr->text());
  typeSave.bindValue(":prjtype_active", QVariant(_active->isChecked()));
  typeSave.exec();

  omfgThis->sItemGroupsUpdated(-1, true);

  close();
}

void projectType::populate()
{
  XSqlQuery typepopulate;
  typepopulate.prepare( "SELECT * FROM prjtype "
             "WHERE (prjtype_id=:prjtype_id);" );
  typepopulate.bindValue(":prjtype_id", _prjtypeid);
  typepopulate.exec();
  if (typepopulate.first())
  {
    _typeCode->setText(typepopulate.value("prjtype_code").toString());
    _typeDescr->setText(typepopulate.value("prjtype_descr").toString());
    _active->setChecked(typepopulate.value("prjtype_active").toBool());
  }
}
