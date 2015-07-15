/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "siteType.h"

#include <QVariant>
#include <QMessageBox>

siteType::siteType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _sitetypeid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

siteType::~siteType()
{
  // no need to delete child widgets, Qt does it all for us
}

void siteType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse siteType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("sitetype_id", &valid);
  if (valid)
  {
    _sitetypeid = param.toInt();
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
      _description->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
    }
  }

  return NoError;
}

void siteType::sCheck()
{
  XSqlQuery siteCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().trimmed().length()))
  {
    siteCheck.prepare( "SELECT sitetype_id"
               "  FROM sitetype"
               " WHERE((UPPER(sitetype_name)=UPPER(:sitetype_name))"
               "   AND (sitetype_id != :sitetype_id));" );
    siteCheck.bindValue(":sitetype_name", _code->text());
    siteCheck.bindValue(":sitetype_id", _sitetypeid);
    siteCheck.exec();
    if (siteCheck.first())
    {
      _sitetypeid = siteCheck.value("sitetype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(false);
    }
  }
}

void siteType::sSave()
{
  XSqlQuery siteSave;
  if (_code->text().length() == 0)
  {
    QMessageBox::critical( this, tr("Cannot Save Site Type"),
                           tr( "You must uniquely name this Site Type\n"
                               "before you may save it." ) );
    _code->setFocus();
    return;
  }
  
  siteSave.prepare( "SELECT sitetype_id"
             "  FROM sitetype"
             " WHERE((UPPER(sitetype_name)=UPPER(:sitetype_name))"
             "   AND (sitetype_id != :sitetype_id));" );
  siteSave.bindValue(":sitetype_name", _code->text());
  siteSave.bindValue(":sitetype_id", _sitetypeid);
  siteSave.exec();
  if (siteSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Save Site Type"),
                           tr( "The new Site Type information cannot be saved as the new Site Type Name that you\n"
                               "entered conflicts with an existing Site Type.  You must uniquely name this Site Type\n"
                               "before you may save it." ) );
    _code->setFocus();
    return;
  }


  if (_mode == cNew)
  {
    siteSave.exec("SELECT NEXTVAL('sitetype_sitetype_id_seq') AS _sitetype_id;");
    if (siteSave.first())
      _sitetypeid = siteSave.value("_sitetype_id").toInt();

    siteSave.prepare( "INSERT INTO sitetype "
               "(sitetype_id, sitetype_name, sitetype_descrip) "
               "VALUES "
               "(:sitetype_id, :sitetype_name, :sitetype_descrip);" );
  }
  else if (_mode == cEdit)
  {
    siteSave.prepare( "UPDATE sitetype "
               "SET sitetype_name=:sitetype_name, sitetype_descrip=:sitetype_descrip "
               "WHERE (sitetype_id=:sitetype_id);" );
  }

  siteSave.bindValue(":sitetype_id", _sitetypeid);
  siteSave.bindValue(":sitetype_name", _code->text().trimmed());
  siteSave.bindValue(":sitetype_descrip", _description->text().trimmed());
  siteSave.exec();

  done(_sitetypeid);
}

void siteType::populate()
{
  XSqlQuery sitepopulate;
  sitepopulate.prepare( "SELECT sitetype_name, sitetype_descrip "
             "FROM sitetype "
             "WHERE (sitetype_id=:sitetype_id);" );
  sitepopulate.bindValue(":sitetype_id", _sitetypeid);
  sitepopulate.exec();
  if (sitepopulate.first()) 
  {
    _code->setText(sitepopulate.value("sitetype_name").toString());
    _description->setText(sitepopulate.value("sitetype_descrip").toString());
  }
}
