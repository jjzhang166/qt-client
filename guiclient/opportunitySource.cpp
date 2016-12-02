/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunitySource.h"

#include <QVariant>
#include <QMessageBox>

#include "errorReporter.h"

/*
 *  Constructs a opportunitySource as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
opportunitySource::opportunitySource(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
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
opportunitySource::~opportunitySource()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunitySource::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunitySource::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("opsource_id", &valid);
  if (valid)
  {
    _opsourceid = param.toInt();
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
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void opportunitySource::sCheck()
{
  XSqlQuery opportunityCheck;
  _name->setText(_name->text().trimmed());

  if ((_mode == cNew) && (_name->text().length() != 0))
  {
    opportunityCheck.prepare( "SELECT opsource_id "
               "FROM opsource "
               "WHERE (UPPER(opsource_name)=UPPER(:opsource_name));" );
    opportunityCheck.bindValue(":opsource_name", _name->text());
    opportunityCheck.exec();
    if (opportunityCheck.first())
    {
      _opsourceid = opportunityCheck.value("opsource_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
  if ((_mode == cEdit) && (_name->text().length() != 0))
  {
    opportunityCheck.prepare( "SELECT opsource_id "
               "FROM opsource "
               "WHERE (UPPER(opsource_name)=UPPER(:opsource_name) "
               " AND (opsource_id <> :opsource_id));" );
    opportunityCheck.bindValue(":opsource_name", _name->text());
    opportunityCheck.bindValue(":opsource_id", _opsourceid);
    opportunityCheck.exec();
    if (opportunityCheck.first())
    {
      QMessageBox::information( this, tr("Invalid Name"),
                              tr("An Opportunity Source with this name already exists.") );
      _name->setText("");
      _name->setFocus();
    }
  }
}

void opportunitySource::sSave()
{
  XSqlQuery opportunitySave;
  _name->setText(_name->text().trimmed().toUpper());
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Name"),
                              tr("You must enter a valid Name for this Opportunity Source.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    opportunitySave.exec("SELECT NEXTVAL('opsource_opsource_id_seq') AS opsource_id");
    if (opportunitySave.first())
      _opsourceid = opportunitySave.value("opsource_id").toInt();
    else
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Opportunity Source"),
                           opportunitySave, __FILE__, __LINE__);
      return;
    }

    opportunitySave.prepare( "INSERT INTO opsource "
               "( opsource_id, opsource_name, opsource_descrip)"
               "VALUES "
               "( :opsource_id, :opsource_name, :opsource_descrip );" );
  }
  else if (_mode == cEdit)
    opportunitySave.prepare( "UPDATE opsource "
               "   SET opsource_name=:opsource_name,"
               "       opsource_descrip=:opsource_descrip "
               " WHERE(opsource_id=:opsource_id);" );

  opportunitySave.bindValue(":opsource_id", _opsourceid);
  opportunitySave.bindValue(":opsource_name", _name->text());
  opportunitySave.bindValue(":opsource_descrip", _description->text().trimmed());
  opportunitySave.exec();

  done(_opsourceid);
}

void opportunitySource::populate()
{
  XSqlQuery opportunitypopulate;
  opportunitypopulate.prepare( "SELECT opsource_name, opsource_descrip "
             "  FROM opsource "
             " WHERE(opsource_id=:opsource_id);" );
  opportunitypopulate.bindValue(":opsource_id", _opsourceid);
  opportunitypopulate.exec();
  if (opportunitypopulate.first())
  {
    _name->setText(opportunitypopulate.value("opsource_name"));
    _description->setText(opportunitypopulate.value("opsource_descrip"));
  }
} 

