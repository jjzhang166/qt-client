/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardJournal.h"

#include <QVariant>
#include <QMessageBox>
#include "standardJournalItem.h"
#include "errorReporter.h"

standardJournal::standardJournal(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _stdjrnlid = -1;

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_stdjrnlitem, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(this, SIGNAL(rejected()), this, SLOT(sReject()));

  _stdjrnlitem->addColumn(tr("Account"), 200,          Qt::AlignLeft,   true,  "account"  );
  _stdjrnlitem->addColumn(tr("Notes"),   -1,           Qt::AlignLeft,   true,  "note"  );
  _stdjrnlitem->addColumn(tr("Debit"),   _priceColumn, Qt::AlignRight,  true,  "debit" );
  _stdjrnlitem->addColumn(tr("Credit"),  _priceColumn, Qt::AlignRight,  true,  "credit" );

  _debits->setValidator(omfgThis->moneyVal());
  _credits->setValidator(omfgThis->moneyVal());
}

standardJournal::~standardJournal()
{
  destroy();
  // no need to delete child widgets, Qt does it all for us
}

void standardJournal::languageChange()
{
  retranslateUi(this);
}

void standardJournal::sReject()
{
  XSqlQuery standardReject;
  if (_mode == cNew)
  {
    standardReject.prepare( "DELETE FROM stdjrnlitem "
               "WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id);" );
    standardReject.bindValue(":stdjrnl_id", _stdjrnlid);
    standardReject.exec();
  }
}

enum SetResponse standardJournal::set(const ParameterList &pParams)
{
  XSqlQuery standardet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("stdjrnl_id", &valid);
  if (valid)
  {
    _stdjrnlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      standardet.exec("SELECT NEXTVAL('stdjrnl_stdjrnl_id_seq') AS _stdjrnl_id");
      if (standardet.first())
        _stdjrnlid = standardet.value("_stdjrnl_id").toInt();

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_stdjrnlitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      connect(_stdjrnlitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _new->setEnabled(false);
      _name->setEnabled(false);
      _descrip->setEnabled(false);
      _notes->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void standardJournal::sSave()
{
  XSqlQuery standardSave;
  if (_name->text().length() == 0)
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal"),
                          tr("You must enter a valid Name.") );
    return;
  }
  
  standardSave.prepare( "SELECT stdjrnl_id"
             "  FROM stdjrnl "
             " WHERE((UPPER(stdjrnl_name)=UPPER(:stdjrnl_name))"
             "   AND (stdjrnl_id != :stdjrnl_id));" );
  standardSave.bindValue(":stdjrnl_name", _name->text());
  standardSave.bindValue(":stdjrnl_id", _stdjrnlid);
  standardSave.exec();
  if (standardSave.first())
  {
    QMessageBox::warning( this, tr("Cannot Save Standard Journal"),
                          tr("The Name you have entered for this Standard Journal already exists. "
                             "Please enter in a different Name for this Standard Journal."));
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
    standardSave.prepare( "INSERT INTO stdjrnl "
               "(stdjrnl_id, stdjrnl_name, stdjrnl_descrip, stdjrnl_notes) "
               "VALUES "
               "(:stdjrnl_id, :stdjrnl_name, :stdjrnl_descrip, :stdjrnl_notes);" );
  else if (_mode == cEdit)
    standardSave.prepare( "UPDATE stdjrnl "
               " SET stdjrnl_name=:stdjrnl_name, stdjrnl_descrip=:stdjrnl_descrip, stdjrnl_notes=:stdjrnl_notes "
               " WHERE (stdjrnl_id=:stdjrnl_id);" );

  standardSave.bindValue(":stdjrnl_id", _stdjrnlid);
  standardSave.bindValue(":stdjrnl_name", _name->text());
  standardSave.bindValue(":stdjrnl_descrip", _descrip->text());
  standardSave.bindValue(":stdjrnl_notes", _notes->toPlainText());
  standardSave.exec();

  done(_stdjrnlid);
}

void standardJournal::sCheck()
{
  XSqlQuery standardCheck;
  _name->setText(_name->text().trimmed());
  if ((_mode == cNew) && (_name->text().length()))
  {
    standardCheck.prepare( "SELECT stdjrnl_id"
               "  FROM stdjrnl"
               " WHERE((UPPER(stdjrnl_name)=UPPER(:stdjrnl_name))"
               "   AND (stdjrnl_id != :stdjrnl_id));" );
    standardCheck.bindValue(":stdjrnl_name", _name->text());
    standardCheck.bindValue(":stdjrnl_id", _stdjrnlid);
    standardCheck.exec();
    if (standardCheck.first())
    {
      _stdjrnlid = standardCheck.value("stdjrnl_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(false);
    }
  }
}

void standardJournal::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("stdjrnl_id", _stdjrnlid);

  standardJournalItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardJournal::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdjrnlitem_id", _stdjrnlitem->id());

  standardJournalItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void standardJournal::sDelete()
{
  XSqlQuery standardDelete;
  standardDelete.prepare( "DELETE FROM stdjrnlitem "
             "WHERE (stdjrnlitem_id=:stdjrnlitem_id);" );
  standardDelete.bindValue(":stdjrnlitem_id", _stdjrnlitem->id());
  standardDelete.exec();
  sFillList();
}

void standardJournal::sFillList()
{
  XSqlQuery standardFillList;
  standardFillList.prepare( "SELECT stdjrnlitem_id,"
             "       CASE WHEN(accnt_id IS NOT NULL) THEN (formatGLAccount(accnt_id) || '-' || accnt_descrip)"
             "            ELSE 'ERROR - NO ACCOUNT SPECIFIED'"
             "       END AS account,"
             "       firstLine(stdjrnlitem_notes) AS note,"
             "       CASE WHEN (stdjrnlitem_amount < 0) THEN (stdjrnlitem_amount * -1)"
             "            ELSE NULL"
             "       END AS debit,"
             "       CASE WHEN (stdjrnlitem_amount > 0) THEN (stdjrnlitem_amount)"
             "            ELSE NULL"
             "       END AS credit,"
             "       'curr' AS debit_xtnumericrole,"
             "       'curr' AS credit_xtnumericrole,"
             "       '' AS debit_xtnullrole,"
             "       '' AS credit_xtnullrole "
             "  FROM stdjrnlitem LEFT OUTER JOIN accnt ON (stdjrnlitem_accnt_id=accnt_id)"
             " WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id) "
             " ORDER BY accnt_number, accnt_profit, accnt_sub;" );
  standardFillList.bindValue(":stdjrnl_id", _stdjrnlid);
  standardFillList.exec();
  _stdjrnlitem->populate(standardFillList);

  standardFillList.prepare( "SELECT SUM( CASE WHEN (stdjrnlitem_amount < 0) THEN (stdjrnlitem_amount * -1)"
             "                 ELSE 0"
             "            END ) AS debit,"
             "       SUM( CASE WHEN (stdjrnlitem_amount > 0) THEN stdjrnlitem_amount"
             "                 ELSE 0"
             "            END ) AS credit,"
             "       (SUM(stdjrnlitem_amount) <> 0) AS oob "
             "FROM stdjrnlitem "
             "WHERE (stdjrnlitem_stdjrnl_id=:stdjrnl_id);" );
  standardFillList.bindValue(":stdjrnl_id", _stdjrnlid);
  standardFillList.exec();
  if (standardFillList.first())
  {
    _debits->setDouble(standardFillList.value("debit").toDouble());
    _credits->setDouble(standardFillList.value("credit").toDouble());

    QString stylesheet;
    if (standardFillList.value("oob").toBool())
      stylesheet = QString("* { color: %1; }").arg(namedColor("error").name());
    _debits->setStyleSheet(stylesheet);
    _credits->setStyleSheet(stylesheet);
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Journal Information"),
                       standardFillList, __FILE__, __LINE__);
}

void standardJournal::populate()
{
  XSqlQuery standardpopulate;
  standardpopulate.prepare( "SELECT stdjrnl_name, stdjrnl_descrip, stdjrnl_notes "
             "FROM stdjrnl "
             "WHERE (stdjrnl_id=:stdjrnl_id);" );
  standardpopulate.bindValue(":stdjrnl_id", _stdjrnlid);
  standardpopulate.exec();
  if (standardpopulate.first())
  {
    _name->setText(standardpopulate.value("stdjrnl_name").toString());
    _descrip->setText(standardpopulate.value("stdjrnl_descrip").toString());
    _notes->setText(standardpopulate.value("stdjrnl_notes").toString());

    sFillList();
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Journal Information"),
                       standardpopulate, __FILE__, __LINE__);
}

