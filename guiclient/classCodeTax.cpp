/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCodeTax.h"

#include <QMessageBox>
#include <QVariant>

/*
 *  Constructs a classCodeTax as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
classCodeTax::classCodeTax(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _classcodetaxid = -1;
  _classcodeid = -1;
}

/*
 *  Destroys the object and frees any allocated resources
 */
classCodeTax::~classCodeTax()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void classCodeTax::languageChange()
{
  retranslateUi(this);
}

enum SetResponse classCodeTax::set(const ParameterList & pParams)
{
  XSqlQuery classcodetaxet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
    _classcodeid = param.toInt();

  param = pParams.value("classcodetax_id", &valid);
  if (valid)
  {
    _classcodetaxid = param.toInt();
    classcodetaxet.prepare("SELECT classcodetax_classcode_id,"
              "       COALESCE(classcodetax_taxzone_id,-1) AS taxzone_id,"
              "       classcodetax_taxtype_id"
              "  FROM classcodetax"
              " WHERE (classcodetax_id=:classcodetax_id);");
    classcodetaxet.bindValue(":classcodetax_id", _classcodetaxid);
    classcodetaxet.exec();
    if(classcodetaxet.first())
    {
      _classcodeid = classcodetaxet.value("classcodetax_classcode_id").toInt();
      _taxzone->setId(classcodetaxet.value("taxzone_id").toInt());
      _taxtype->setId(classcodetaxet.value("classcodetax_taxtype_id").toInt());
    }
    // TODO: catch any possible errors
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
      _mode = cNew;
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
      _taxzone->setEnabled(false);
      _taxtype->setEnabled(false);
    }
  }

  return NoError;
}

void classCodeTax::sSave()
{
  XSqlQuery classcodetaxSave;
  classcodetaxSave.prepare("SELECT classcodetax_id"
            "  FROM classcodetax"
            " WHERE ((classcodetax_taxzone_id=:taxzone_id)"
            "   AND  (classcodetax_classcode_id=:classcode_id)"
            "   AND  (classcodetax_id != :classcodetax_id))");
  classcodetaxSave.bindValue(":classcode_id", _classcodeid);
  classcodetaxSave.bindValue(":classcodetax_id", _classcodetaxid);
  if(_taxzone->isValid())
    classcodetaxSave.bindValue(":taxzone_id", _taxzone->id());
  classcodetaxSave.exec();
  if(classcodetaxSave.first())
  {
    QMessageBox::warning(this, tr("Tax Zone Already Exists"),
                      tr("The Tax Zone you have choosen already exists for this Class Code."));
    return;
  }

  if(cNew == _mode)
    classcodetaxSave.prepare("INSERT INTO classcodetax"
              "      (classcodetax_classcode_id, classcodetax_taxzone_id, classcodetax_taxtype_id) "
              "VALUES(:classcode_id, :taxzone_id, :taxtype_id)");
  else if(cEdit == _mode)
    classcodetaxSave.prepare("UPDATE classcodetax"
              "   SET classcodetax_taxzone_id=:taxzone_id,"
              "       classcodetax_taxtype_id=:taxtype_id"
              " WHERE (classcodetax_id=:classcodetax_id);");

  classcodetaxSave.bindValue(":classcode_id", _classcodeid);
  classcodetaxSave.bindValue(":classcodetax_id", _classcodetaxid);
  if(_taxzone->isValid())
    classcodetaxSave.bindValue(":taxzone_id", _taxzone->id());
  classcodetaxSave.bindValue(":taxtype_id", _taxtype->id());
  classcodetaxSave.exec();

  accept();
}

