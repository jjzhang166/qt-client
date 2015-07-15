/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunityStage.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a opportunityStage as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
opportunityStage::opportunityStage(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunityStage::~opportunityStage()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunityStage::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunityStage::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("opstage_id", &valid);
  if (valid)
  {
    _opstageid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(false);
      _description->setEnabled(false);
      _opInactive->setEnabled(false);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void opportunityStage::sCheck()
{
  XSqlQuery opportunityCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    opportunityCheck.prepare( "SELECT opstage_id "
               "FROM opstage "
               "WHERE (UPPER(opstage_name)=UPPER(:opstage_name));" );
    opportunityCheck.bindValue(":opstage_name", _name->text());
    opportunityCheck.exec();
    if (opportunityCheck.first())
    {
      _opstageid = opportunityCheck.value("opstage_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void opportunityStage::sSave()
{
  XSqlQuery opportunitySave;
  _name->setText(_name->text().trimmed().toUpper());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Opportunity Stage.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    opportunitySave.exec("SELECT NEXTVAL('opstage_opstage_id_seq') AS opstage_id");
    if (opportunitySave.first())
      _opstageid = opportunitySave.value("opstage_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    opportunitySave.prepare( "INSERT INTO opstage "
               "( opstage_id, opstage_name, opstage_descrip, opstage_opinactive)"
               "VALUES "
               "( :opstage_id, :opstage_name, :opstage_descrip, :opstage_opinactive );" );
  }
  else if (_mode == cEdit)
    opportunitySave.prepare( "UPDATE opstage "
               "   SET opstage_name=:opstage_name,"
               "       opstage_descrip=:opstage_descrip,"
			   "       opstage_opinactive=:opstage_opinactive "
               " WHERE(opstage_id=:opstage_id);" );

  opportunitySave.bindValue(":opstage_id", _opstageid);
  opportunitySave.bindValue(":opstage_name", _name->text());
  opportunitySave.bindValue(":opstage_descrip", _description->text().trimmed());
  opportunitySave.bindValue(":opstage_opinactive", QVariant(_opInactive->isChecked()));
  opportunitySave.exec();

  done(_opstageid);
}

void opportunityStage::populate()
{
  XSqlQuery opportunitypopulate;
  opportunitypopulate.prepare( "SELECT * "
             "  FROM opstage "
             " WHERE(opstage_id=:opstage_id);" );
  opportunitypopulate.bindValue(":opstage_id", _opstageid);
  opportunitypopulate.exec();
  if (opportunitypopulate.first())
  {
    _name->setText(opportunitypopulate.value("opstage_name"));
    _description->setText(opportunitypopulate.value("opstage_descrip"));
    _opInactive->setChecked(opportunitypopulate.value("opstage_opinactive").toBool());
  }
} 

