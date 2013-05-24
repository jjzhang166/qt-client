/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "opportunity.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "errorReporter.h"
#include "storedProcErrorLookup.h"
#include "todoItem.h"
#include "salesOrder.h"
#include "salesOrderList.h"
#include "quoteList.h"
#include "printQuote.h"
#include "printSoForm.h"
#include "characteristicAssignment.h"

opportunity::opportunity(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  if(!_privileges->check("EditOwner")) _owner->setEnabled(false);

  connect(_crmacct, SIGNAL(newId(int)), this, SLOT(sHandleCrmacct(int)));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(sCancel()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_deleteTodoItem, SIGNAL(clicked()), this, SLOT(sDeleteTodoItem()));
  connect(_viewTodoItem, SIGNAL(clicked()), this, SLOT(sViewTodoItem()));
  connect(_editTodoItem, SIGNAL(clicked()), this, SLOT(sEditTodoItem()));
  connect(_newTodoItem, SIGNAL(clicked()), this, SLOT(sNewTodoItem()));
  connect(_todoList, SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  connect(_todoList, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*, int)), this, SLOT(sPopulateTodoMenu(QMenu*)));
  connect(_todoList, SIGNAL(valid(bool)), this, SLOT(sHandleTodoPrivs()));
  connect(_deleteSale, SIGNAL(clicked()), this, SLOT(sDeleteSale()));
  connect(_viewSale, SIGNAL(clicked()), this, SLOT(sViewSale()));
  connect(_editSale, SIGNAL(clicked()), this, SLOT(sEditSale()));
  connect(_convertQuote, SIGNAL(clicked()), this, SLOT(sConvertQuote()));
  connect(_printSale, SIGNAL(clicked()), this, SLOT(sPrintSale()));
  connect(_salesList, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateSalesMenu(QMenu*)));
  connect(_salesList, SIGNAL(valid(bool)), this, SLOT(sHandleSalesPrivs()));
  connect(omfgThis,	SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillSalesList()));
  connect(omfgThis,	SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillSalesList()));
  connect(_charass, SIGNAL(itemSelected(int)), _editCharacteristic, SLOT(animateClick()));
  connect(_newCharacteristic, SIGNAL(clicked()), this, SLOT(sNewCharacteristic()));
  connect(_editCharacteristic, SIGNAL(clicked()), this, SLOT(sEditCharacteristic()));
  connect(_deleteCharacteristic, SIGNAL(clicked()), this, SLOT(sDeleteCharacteristic()));
  connect(_assignedTo, SIGNAL(newId(int)), this, SLOT(sHandleAssigned()));

  _probability->setValidator(new QIntValidator(this));
  
  _opheadid = -1;
  _custid = -1;
  _prospectid = -1;
  
  _todoList->addColumn(tr("Active"),   _statusColumn, Qt::AlignRight, true, "todoitem_active");
  _todoList->addColumn(tr("Priority"),   _userColumn, Qt::AlignRight, true, "incdtpriority_name");
  _todoList->addColumn(tr("Owner"),      _userColumn, Qt::AlignLeft, false, "todoitem_owner_username");
  _todoList->addColumn(tr("Assigned"),   _userColumn, Qt::AlignLeft,  true, "todoitem_username" );
  _todoList->addColumn(tr("Name"),               100, Qt::AlignLeft,  true, "todoitem_name" );
  _todoList->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "todoitem_description" );
  _todoList->addColumn(tr("Status"),   _statusColumn, Qt::AlignLeft,  true, "todoitem_status" );
  _todoList->addColumn(tr("Due Date"),   _dateColumn, Qt::AlignLeft,  true, "todoitem_due_date" );

  _salesList->addColumn(tr("Doc #"),       _orderColumn, Qt::AlignLeft,  true, "sale_number" );
  _salesList->addColumn(tr("Type"),                  -1, Qt::AlignLeft,  true, "sale_type" );
  _salesList->addColumn(tr("Date"),         _dateColumn, Qt::AlignLeft,  true, "sale_date" );
  _salesList->addColumn(tr("Ext. Price"),  _priceColumn, Qt::AlignRight, true, "sale_extprice");

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft,  true, "char_name" );
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft,  true, "charass_value" );
  _charass->addColumn(tr("Default"),        _ynColumn,   Qt::AlignCenter,true, "charass_default" );

  _owner->setUsername(omfgThis->username());
  _owner->setType(UsernameLineEdit::UsersActive);

  _assignedTo->setType(UsernameLineEdit::UsersActive);

  _saved = false;
}

/*
 *  Destroys the object and frees any allocated resources
 */
opportunity::~opportunity()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void opportunity::languageChange()
{
  retranslateUi(this);
}

enum SetResponse opportunity::set(const ParameterList &pParams)
{
  XSqlQuery opportunityet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("ophead_id", &valid);
  if (valid)
  {
    _opheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
      
      param = pParams.value("crmacct_id", &valid);
      if (valid)
        _crmacct->setId(param.toInt());

      _startDate->setDate(omfgThis->dbDate());

      opportunityet.exec("SELECT NEXTVAL('ophead_ophead_id_seq') AS result;");
      if (opportunityet.first())
      {
        _opheadid = opportunityet.value("result").toInt();
      }
      else if(opportunityet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, opportunityet.lastError().databaseText(), __FILE__, __LINE__);
      }

      opportunityet.exec("SELECT fetchNextNumber('OpportunityNumber') AS result;");
      if (opportunityet.first())
      {
        _number->setText(opportunityet.value("result").toString());
      }
      else if(opportunityet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, opportunityet.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      connect(_charass, SIGNAL(valid(bool)), _editCharacteristic, SLOT(setEnabled(bool)));
      connect(_charass, SIGNAL(valid(bool)), _deleteCharacteristic, SLOT(setEnabled(bool)));

      _crmacct->setEnabled(true);
      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _crmacct->setEnabled(false);
      _owner->setEnabled(false);
      _oppstage->setEnabled(false);
      _oppsource->setEnabled(false);
      _opptype->setEnabled(false);
      _notes->setReadOnly(true);
      _name->setEnabled(false);
      _targetDate->setEnabled(false);
      _actualDate->setEnabled(false);
      _amount->setEnabled(false);
      _probability->setEnabled(false);
      _deleteTodoItem->setEnabled(false);
      _editTodoItem->setEnabled(false);
      _newTodoItem->setEnabled(false);
      _deleteSale->setEnabled(false);
      _editSale->setEnabled(false);
      _printSale->setEnabled(false);
      _newSale->setEnabled(false);
      _attachSale->setEnabled(false);
      _newCharacteristic->setEnabled(FALSE);

      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
      _cntct->setReadOnly(true);
      _comments->setReadOnly(true);
      _documents->setReadOnly(true);
    }
  }

  param = pParams.value("crmacct_id", &valid);
  if (valid)
  {
    _crmacct->setId(param.toInt());
    _crmacct->setEnabled(false);
  }

  sHandleTodoPrivs();
  sHandleSalesPrivs();
  return NoError;
}

void opportunity::sCancel()
{
  XSqlQuery cancelOppo;
  if(cNew == _mode)
  {
    cancelOppo.prepare("SELECT releaseNumber('OpportunityNumber', :number);");
    cancelOppo.bindValue(":number", _number->text().toInt());
    cancelOppo.exec();
    if (cancelOppo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cancelOppo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_saved && cNew == _mode)
  {
    cancelOppo.prepare("SELECT deleteOpportunity(:ophead_id) AS result;");
    cancelOppo.bindValue(":ophead_id", _opheadid);
    cancelOppo.exec();
    if (cancelOppo.first())
    {
      int result = cancelOppo.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteOpportunity", result));
	return;
      }
    }
    else if (cancelOppo.lastError().type() != QSqlError::NoError)
    {
      systemError(this, cancelOppo.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  reject();
}

void opportunity::sSave()
{
  if (! save(false)) // if error
    return;

  done(_opheadid);
}

bool opportunity::save(bool partial)
{
  XSqlQuery opportunityave;
  if (! partial)
  {
    if(_crmacct->id() == -1)
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify the Account that this opportunity is for.") );
      return false;
    }

    if(_name->text().trimmed().isEmpty())
    {
      QMessageBox::critical( this, tr("Incomplete Information"),
	tr("You must specify a Name for this opportunity report.") );
      _name->setFocus();
      return false;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (!opportunityave.exec("BEGIN"))
  {
    rollback.exec();
    systemError(this, opportunityave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (cNew == _mode && !_saved)
    opportunityave.prepare("INSERT INTO ophead"
              "      (ophead_id, ophead_name, ophead_crmacct_id,"
              "       ophead_owner_username,"
              "       ophead_opstage_id, ophead_opsource_id,"
              "       ophead_optype_id, ophead_probability_prcnt,"
              "       ophead_amount, ophead_curr_id, ophead_target_date,"
              "       ophead_actual_date,"
              "       ophead_notes, ophead_active, ophead_cntct_id, "
              "       ophead_username, ophead_start_date, ophead_assigned_date, "
              "       ophead_priority_id, ophead_number) "
              "VALUES(:ophead_id, :ophead_name, :ophead_crmacct_id,"
              "       :ophead_owner_username,"
              "       :ophead_opstage_id, :ophead_opsource_id,"
              "       :ophead_optype_id, :ophead_probability_prcnt,"
              "       :ophead_amount, :ophead_curr_id, :ophead_target_date,"
              "       :ophead_actual_date,"
              "       :ophead_notes, :ophead_active, :ophead_cntct_id, "
              "       :ophead_username, :ophead_start_date, :ophead_assigned_date, "
              "       :ophead_priority_id, :ophead_number); " );
  else if (cEdit == _mode || _saved)
    opportunityave.prepare("UPDATE ophead"
              "   SET ophead_name=:ophead_name,"
              "       ophead_crmacct_id=:ophead_crmacct_id,"
              "       ophead_owner_username=:ophead_owner_username,"
              "       ophead_opstage_id=:ophead_opstage_id,"
              "       ophead_opsource_id=:ophead_opsource_id,"
              "       ophead_optype_id=:ophead_optype_id,"
              "       ophead_probability_prcnt=:ophead_probability_prcnt,"
              "       ophead_amount=:ophead_amount,"
              "       ophead_curr_id=:ophead_curr_id,"
              "       ophead_target_date=:ophead_target_date,"
              "       ophead_actual_date=:ophead_actual_date,"
              "       ophead_notes=:ophead_notes,"
              "       ophead_active=:ophead_active, "
              "       ophead_cntct_id=:ophead_cntct_id, "
              "       ophead_username=:ophead_username, "
              "       ophead_start_date=:ophead_start_date, "
              "       ophead_assigned_date=:ophead_assigned_date, "
              "       ophead_priority_id=:ophead_priority_id "
              " WHERE (ophead_id=:ophead_id); ");

  opportunityave.bindValue(":ophead_id", _opheadid);
  opportunityave.bindValue(":ophead_name", _name->text());
  opportunityave.bindValue(":ophead_active",	QVariant(_active->isChecked()));
  if (_crmacct->id() > 0)
    opportunityave.bindValue(":ophead_crmacct_id", _crmacct->id());
  opportunityave.bindValue(":ophead_owner_username", _owner->username());
  opportunityave.bindValue(":ophead_username", _assignedTo->username());
  if(_oppstage->isValid())
    opportunityave.bindValue(":ophead_opstage_id", _oppstage->id());
  if(_oppsource->isValid())
    opportunityave.bindValue(":ophead_opsource_id", _oppsource->id());
  if(_opptype->isValid())
    opportunityave.bindValue(":ophead_optype_id", _opptype->id());
  if(!_probability->text().isEmpty())
    opportunityave.bindValue(":ophead_probability_prcnt", _probability->text().toInt());
  if(!_amount->isZero())
  {
    opportunityave.bindValue(":ophead_amount", _amount->localValue());
    opportunityave.bindValue(":ophead_curr_id", _amount->id());
  }
  if(_startDate->isValid())
    opportunityave.bindValue(":ophead_start_date", _startDate->date());
  if(_assignDate->isValid())
    opportunityave.bindValue(":ophead_assigned_date", _assignDate->date());
  if(_targetDate->isValid())
    opportunityave.bindValue(":ophead_target_date", _targetDate->date());
  if(_actualDate->isValid())
    opportunityave.bindValue(":ophead_actual_date", _actualDate->date());
  opportunityave.bindValue(":ophead_notes", _notes->toPlainText());
  if (_cntct->isValid())
    opportunityave.bindValue(":ophead_cntct_id", _cntct->id());
  if (_priority->isValid())
    opportunityave.bindValue(":ophead_priority_id", _priority->id());
  opportunityave.bindValue(":ophead_number", _number->text());

  if(!opportunityave.exec() && opportunityave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, opportunityave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  opportunityave.exec("COMMIT;");
  if(opportunityave.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, opportunityave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  _saved = true;
  return true;
}

void opportunity::populate()
{ 
  XSqlQuery opportunitypopulate;
  opportunitypopulate.prepare("SELECT ophead_name,"
            "       ophead_crmacct_id,"
            "       ophead_owner_username,"
            "       ophead_opstage_id, ophead_opsource_id,"
            "       ophead_optype_id,"
            "       ophead_probability_prcnt, ophead_amount,"
            "       COALESCE(ophead_curr_id, basecurrid()) AS curr_id,"
            "       ophead_target_date, ophead_actual_date,"
            "       ophead_notes, ophead_active, ophead_cntct_id, "
            "       ophead_username, ophead_start_date, ophead_assigned_date, "
            "       ophead_number, ophead_priority_id "
            "  FROM ophead"
            " WHERE(ophead_id=:ophead_id); ");
  opportunitypopulate.bindValue(":ophead_id", _opheadid);
  opportunitypopulate.exec();
  if(opportunitypopulate.first())
  {
    _number->setText(opportunitypopulate.value("ophead_number").toString());
    _name->setText(opportunitypopulate.value("ophead_name").toString());
    _active->setChecked(opportunitypopulate.value("ophead_active").toBool());
    _crmacct->setId(opportunitypopulate.value("ophead_crmacct_id").toInt());
    _owner->setUsername(opportunitypopulate.value("ophead_owner_username").toString());
    _assignedTo->setUsername(opportunitypopulate.value("ophead_username").toString());

    _oppstage->setNull();
    if(!opportunitypopulate.value("ophead_opstage_id").toString().isEmpty())
      _oppstage->setId(opportunitypopulate.value("ophead_opstage_id").toInt());

    _priority->setNull();
    if(!opportunitypopulate.value("ophead_priority_id").toString().isEmpty())
      _priority->setId(opportunitypopulate.value("ophead_priority_id").toInt());

    _oppsource->setNull();
    if(!opportunitypopulate.value("ophead_opsource_id").toString().isEmpty())
      _oppsource->setId(opportunitypopulate.value("ophead_opsource_id").toInt());

    _opptype->setNull();
    if(!opportunitypopulate.value("ophead_optype_id").toString().isEmpty())
      _opptype->setId(opportunitypopulate.value("ophead_optype_id").toInt());

    _probability->clear();
    if(!opportunitypopulate.value("ophead_probability_prcnt").toString().isEmpty())
      _probability->setText(opportunitypopulate.value("ophead_probability_prcnt").toDouble());

    _amount->clear();
    _amount->setId(opportunitypopulate.value("curr_id").toInt());
    if(!opportunitypopulate.value("ophead_amount").toString().isEmpty())
      _amount->setLocalValue(opportunitypopulate.value("ophead_amount").toDouble());

    _startDate->clear();
    if(!opportunitypopulate.value("ophead_start_date").toString().isEmpty())
      _startDate->setDate(opportunitypopulate.value("ophead_start_date").toDate());

    _assignDate->clear();
    if(!opportunitypopulate.value("ophead_assigned_date").toString().isEmpty())
      _assignDate->setDate(opportunitypopulate.value("ophead_assigned_date").toDate());

    _targetDate->clear();
    if(!opportunitypopulate.value("ophead_target_date").toString().isEmpty())
      _targetDate->setDate(opportunitypopulate.value("ophead_target_date").toDate());

    _actualDate->clear();
    if(!opportunitypopulate.value("ophead_actual_date").toString().isEmpty())
      _actualDate->setDate(opportunitypopulate.value("ophead_actual_date").toDate());

    _notes->setText(opportunitypopulate.value("ophead_notes").toString());

    _comments->setId(_opheadid);
    _documents->setId(_opheadid);

    _cntct->setId(opportunitypopulate.value("ophead_cntct_id").toInt());

    sFillTodoList();
    sFillSalesList();
    sFillCharList();
  }
}

void opportunity::sNewTodoItem()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);
  if (_crmacct->isValid())
    params.append("crmacct_id", _crmacct->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (!newdlg.exec() == XDialog::Rejected)
    sFillTodoList();
}

void opportunity::sEditTodoItem()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  if (!newdlg.exec() == XDialog::Rejected)
    sFillTodoList();
}

void opportunity::sViewTodoItem()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("todoitem_id", _todoList->id());

  todoItem newdlg(this, 0, true);
  newdlg.set(params);
  newdlg.exec();
}

void opportunity::sDeleteTodoItem()
{
  XSqlQuery opportunityDeleteTodoItem;
  opportunityDeleteTodoItem.prepare("SELECT deleteTodoItem(:todoitem_id) AS result;");
  opportunityDeleteTodoItem.bindValue(":todoitem_id", _todoList->id());
  opportunityDeleteTodoItem.exec();
  if (opportunityDeleteTodoItem.first())
  {
    int result = opportunityDeleteTodoItem.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteTodoItem", result));
      return;
    }
    else
      sFillTodoList();
    }
  else if (opportunityDeleteTodoItem.lastError().type() != QSqlError::NoError)
  {
    systemError(this, opportunityDeleteTodoItem.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void opportunity::sFillTodoList()
{
  XSqlQuery opportunityFillTodoList;
  opportunityFillTodoList.prepare("SELECT todoitem_id, incdtpriority_name, incdtpriority_order, "
            "       todoitem_owner_username, todoitem_username, todoitem_name, "
	    "       firstLine(todoitem_description) AS todoitem_description, "
            "       todoitem_status, todoitem_due_date, todoitem_active, "
            "       CASE "
            "         WHEN (todoitem_status != 'C' AND todoitem_due_date < current_date) THEN "
            "           'error' "
            "         WHEN (todoitem_status != 'C' AND todoitem_due_date > current_date) THEN "
            "           'altemphasis' "
            "       END AS qtforegroundrole "
	    "  FROM todoitem "
            "       LEFT OUTER JOIN incdtpriority ON (incdtpriority_id=todoitem_priority_id) "
            "WHERE ( (todoitem_ophead_id=:ophead_id) ) "
	    "ORDER BY incdtpriority_order, todoitem_username;");

  opportunityFillTodoList.bindValue(":ophead_id", _opheadid);
  opportunityFillTodoList.exec();
  if (opportunityFillTodoList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, opportunityFillTodoList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _todoList->populate(opportunityFillTodoList);
}

void opportunity::sPopulateTodoMenu(QMenu *pMenu)
{
  QAction *menuItem;

  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalToDoItems") ||
       _privileges->check("MaintainAllToDoItems") );

  bool editPriv = (cNew == _mode || cEdit == _mode) && (
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("MaintainPersonalToDoItems")) ||
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("MaintainPersonalToDoItems")) ||
      (_privileges->check("MaintainAllToDoItems")) );

  bool viewPriv =
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("ViewPersonalToDoItems")) ||
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("ViewPersonalToDoItems")) ||
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("MaintainPersonalToDoItems")) ||
      (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("MaintainPersonalToDoItems")) ||
      (_privileges->check("ViewAllToDoItems")) || (_privileges->check("MaintainAllToDoItems"));

  menuItem = pMenu->addAction(tr("New..."), this, SLOT(sNewTodoItem()));
  menuItem->setEnabled(newPriv);

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditTodoItem()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewTodoItem()));
  menuItem->setEnabled(viewPriv);

  menuItem = pMenu->addAction(tr("Delete"), this, SLOT(sDeleteTodoItem()));
  menuItem->setEnabled(editPriv);
}

void opportunity::sHandleTodoPrivs()
{
  bool newPriv = (cNew == _mode || cEdit == _mode) &&
      (_privileges->check("MaintainPersonalToDoItems") ||
       _privileges->check("MaintainAllToDoItems") );

  bool editPriv = false;
  bool viewPriv = false;

  if(_todoList->currentItem())
  {
    editPriv = (cNew == _mode || cEdit == _mode) && (
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("MaintainAllToDoItems")) );

    viewPriv =
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("ViewPersonalToDoItems")) ||
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_username") && _privileges->check("MaintainPersonalToDoItems")) ||
        (omfgThis->username() == _todoList->currentItem()->rawValue("todoitem_owner_username") && _privileges->check("MaintainPersonalToDoItems")) ||
        (_privileges->check("ViewAllToDoItems")) || (_privileges->check("MaintainAllToDoItems"));
  }

  _newTodoItem->setEnabled(newPriv);
  _editTodoItem->setEnabled(editPriv && _todoList->id() > 0);
  _viewTodoItem->setEnabled((editPriv || viewPriv) && _todoList->id() > 0);
  _deleteTodoItem->setEnabled(editPriv && _todoList->id() > 0);

  if (editPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_viewTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _editTodoItem, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    disconnect(_todoList,SIGNAL(itemSelected(int)),_editTodoItem, SLOT(animateClick()));
    connect(_todoList,	SIGNAL(itemSelected(int)), _viewTodoItem, SLOT(animateClick()));
  }
}

void opportunity::sPrintSale()
{
  if (_salesList->altId() == 0)
    sPrintQuote();
  else
    sPrintSalesOrder();
}

void opportunity::sEditSale()
{
  if (_salesList->altId() == 0)
    sEditQuote();
  else
    sEditSalesOrder();
}

void opportunity::sViewSale()
{
  if (_salesList->altId() == 0)
    sViewQuote();
  else
    sViewSalesOrder();
}

void opportunity::sDeleteSale()
{
  if (_salesList->altId() == 0)
    sDeleteQuote();
  else
    sDeleteSalesOrder();
}

void opportunity::sPrintQuote()
{
  printQuote newdlg(this);

  ParameterList params;
  params.append("quhead_id", _salesList->id());
  params.append("persistentPrint");

  newdlg.set(params);

  if (! newdlg.isSetup())
  {
    if (newdlg.exec() != QDialog::Rejected)
      newdlg.setSetup(true);
  }
}

void opportunity::sConvertQuote()
{
  if (QMessageBox::question(this, tr("Convert Selected Quote"),
			    tr("<p>Are you sure that you want to convert "
			       "the selected Quote to a Sales Order?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    int quheadid = _salesList->id();
    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    XSqlQuery prospectq;
    prospectq.prepare("SELECT convertProspectToCustomer(quhead_cust_id) AS result "
                      "FROM quhead "
                      "WHERE (quhead_id=:quhead_id);");

    bool tryagain = false;
    do
	{
	  int soheadid = -1;
      convert.bindValue(":quhead_id", quheadid);
      convert.exec();
      if (convert.first())
      {
        soheadid = convert.value("sohead_id").toInt();
        if (soheadid == -3)
        {
          if ((_metrics->value("DefaultSalesRep").toInt() > 0) &&
              (_metrics->value("DefaultTerms").toInt() > 0) &&
			  (_metrics->value("DefaultCustType").toInt() > 0) && 
              (_metrics->value("DefaultShipFormId").toInt() > 0)  && 
              (_privileges->check("MaintainCustomerMasters"))) 
          {
            if (QMessageBox::question(this, tr("Quote for Prospect"),
											tr("<p>This Quote is for a Prospect, not "
                                               "a Customer. Do you want to convert "
                                               "the Prospect to a Customer using global "
                                               "default values?"),
                                      QMessageBox::Yes,
                                      QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
            {
              prospectq.bindValue(":quhead_id", quheadid);
              prospectq.exec();
              if (prospectq.first())
              {
                int result = prospectq.value("result").toInt();
                if (result < 0)
                {
                  systemError(this, storedProcErrorLookup("convertProspectToCustomer", result), __FILE__, __LINE__);
				  continue;
                }
                convert.exec();
                if (convert.first())
                {
                  soheadid = convert.value("sohead_id").toInt();
                  if (soheadid < 0)
                  {
                    QMessageBox::warning(this, tr("Cannot Convert Quote"), storedProcErrorLookup("convertQuote", soheadid)
                                        .arg(quheadid));
					continue;
                  }
				}
			  }
              else if (prospectq.lastError().type() != QSqlError::NoError)
              {
                systemError(this, prospectq.lastError().databaseText(), __FILE__, __LINE__);
                continue;
			  }
			}
            else
            {
              QMessageBox::information(this, tr("Quote for Prospect"),
                                             tr("<p>The prospect must be manually "
												"converted to customer from either the "
                                                "CRM Account or Customer windows before "
                                                "coverting this quote."));
              continue;
			}
		  }
          else
          {
            QMessageBox::information(this, tr("Quote for Prospect"),
                                           tr("<p>The prospect must be manually "
                                              "converted to customer from either the "
                                              "CRM Account or Customer windows before "
                                              "coverting this quote."));
            continue;
		  }
		}
        else if (soheadid < 0)
        {
          QMessageBox::warning(this, tr("Cannot Convert Quote"), storedProcErrorLookup("convertQuote", soheadid)
							   .arg(quheadid));
          continue;
        }
        ParameterList params;
        params.append("mode", "edit");
        params.append("sohead_id", soheadid);
    
        salesOrder *newdlg = new salesOrder();
        newdlg->setWindowModality(Qt::WindowModal);
        newdlg->set(params);
        omfgThis->handleNewWindow(newdlg);
	  }
      else if (convert.lastError().type() != QSqlError::NoError)
      {
        systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);
        continue;
	  }
    } while (tryagain);
  } // if user wants to convert
}

void opportunity::sNewQuote()
{
  if (! save(false))
    return;

  ParameterList params;
  params.append("mode", "newQuote");
  if (_custid > -1)
    params.append("cust_id", _custid);
  else
    params.append("cust_id", _prospectid);
  params.append("ophead_id", _opheadid);

  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillSalesList();
}

void opportunity::sAttachQuote()
{
  XSqlQuery opportunityAttachQuote;
  ParameterList params;
  params.append("cust_id", _custid);
  params.append("openOnly", true);
  
  quoteList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
  {
    opportunityAttachQuote.prepare("SELECT attachQuoteToOpportunity(:quhead_id, :ophead_id) AS result;");
    opportunityAttachQuote.bindValue(":quhead_id", id);
    opportunityAttachQuote.bindValue(":ophead_id", _opheadid);
    opportunityAttachQuote.exec();
    if (opportunityAttachQuote.first())
    {
      int result = opportunityAttachQuote.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("attachQuoteToOpportunity", result),
                    __FILE__, __LINE__);
        return;
      }
      else
        sFillSalesList();
    }
    else if (opportunityAttachQuote.lastError().type() != QSqlError::NoError)
    {
      systemError(this, opportunityAttachQuote.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void opportunity::sEditQuote()
{
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _salesList->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillSalesList();
}

void opportunity::sViewQuote()
{
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _salesList->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void opportunity::sDeleteQuote()
{
  XSqlQuery opportunityDeleteQuote;
  if ( QMessageBox::warning( this, tr("Delete Selected Quote"),
                             tr("Are you sure that you want to delete the selected Quote?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    opportunityDeleteQuote.prepare("SELECT deleteQuote(:quhead_id) AS result;");
    opportunityDeleteQuote.bindValue(":quhead_id", _salesList->id());
    opportunityDeleteQuote.exec();
    if (opportunityDeleteQuote.first())
    {
      int result = opportunityDeleteQuote.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("deleteQuote", result),
                    __FILE__, __LINE__);
        return;
      }
      else
        sFillSalesList();
    }
    else if (opportunityDeleteQuote.lastError().type() != QSqlError::NoError)
    {
      systemError(this, opportunityDeleteQuote.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void opportunity::sPrintSalesOrder()
{
  ParameterList params;
  params.append("sohead_id", _salesList->id());

  printSoForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void opportunity::sNewSalesOrder()
{
  if (! save(false))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("cust_id", _custid);
  params.append("ophead_id", _opheadid);

  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillSalesList();
}

void opportunity::sAttachSalesOrder()
{
  XSqlQuery opportunityAttachSalesOrder;
  ParameterList params;
  params.append("soType", (cSoOpen | cSoCustomer));
  params.append("cust_id", _custid);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
  {
    opportunityAttachSalesOrder.prepare("SELECT attachSalesOrderToOpportunity(:cohead_id, :ophead_id) AS result;");
    opportunityAttachSalesOrder.bindValue(":cohead_id", id);
    opportunityAttachSalesOrder.bindValue(":ophead_id", _opheadid);
    opportunityAttachSalesOrder.exec();
    if (opportunityAttachSalesOrder.first())
    {
      int result = opportunityAttachSalesOrder.value("result").toInt();
      if (result < 0)
      {
        systemError(this, storedProcErrorLookup("attachSalesOrderToOpportunity", result),
                    __FILE__, __LINE__);
        return;
      }
      else
        sFillSalesList();
    }
    else if (opportunityAttachSalesOrder.lastError().type() != QSqlError::NoError)
    {
      systemError(this, opportunityAttachSalesOrder.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void opportunity::sEditSalesOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _salesList->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
  sFillSalesList();
}

void opportunity::sViewSalesOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sohead_id", _salesList->id());
    
  salesOrder *newdlg = new salesOrder();
  newdlg->setWindowModality(Qt::WindowModal);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void opportunity::sDeleteSalesOrder()
{
  (void)salesOrder::deleteSalesOrder(_salesList->id());
}

void opportunity::sFillSalesList()
{
  XSqlQuery opportunityFillSalesList;
  opportunityFillSalesList.prepare("SELECT id, alt_id, sale_type, sale_number, sale_date, sale_extprice, "
            "       'curr' AS sale_extprice_xtnumericrole, "
			"       0 AS sale_extprice_xttotalrole "
            "FROM ( "
            "SELECT quhead_id AS id, 0 AS alt_id, :quote AS sale_type, "
            "       quhead_number AS sale_number, quhead_quotedate AS sale_date, "
            "       calcQuoteAmt(quhead_id) AS sale_extprice "
            "FROM quhead "
            "WHERE ((quhead_ophead_id=:ophead_id) "
            "  AND  (quhead_status='O'))"
            "UNION "
            "SELECT cohead_id AS id, 1 AS alt_id, :salesorder AS sale_type, "
            "       cohead_number AS sale_number, cohead_orderdate AS sale_date,  "
            "       calcSalesOrderAmt(cohead_id) AS sale_extprice "
            "FROM cohead "
            "WHERE (cohead_ophead_id=:ophead_id) "
            "     ) AS data "
            "ORDER BY sale_date;");
  opportunityFillSalesList.bindValue(":ophead_id", _opheadid);
  opportunityFillSalesList.bindValue(":quote", tr("Quote"));
  opportunityFillSalesList.bindValue(":salesorder", tr("Sales Order"));
  opportunityFillSalesList.exec();
  if (opportunityFillSalesList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, opportunityFillSalesList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _salesList->populate(opportunityFillSalesList, true);
}

void opportunity::sPopulateSalesMenu(QMenu *pMenu)
{
  bool editPriv = false;
  bool viewPriv = false;
  bool convertPriv = false;

  if(_salesList->currentItem())
  {
    editPriv = (cNew == _mode || cEdit == _mode) && (
      (0 == _salesList->currentItem()->altId() && _privileges->check("MaintainQuotes")) ||
      (1 == _salesList->currentItem()->altId() && _privileges->check("MaintainSalesOrders")) );

    viewPriv = (cNew == _mode || cEdit == _mode) && (
      (0 == _salesList->currentItem()->altId() && _privileges->check("ViewQuotes")) ||
      (1 == _salesList->currentItem()->altId() && _privileges->check("ViewSalesOrders")) );

    convertPriv = (cNew == _mode || cEdit == _mode) &&
      (0 == _salesList->currentItem()->altId() && _privileges->check("ConvertQuotes"));
  }

  QAction *menuItem;
  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEditSale()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sViewSale()));
  menuItem->setEnabled((editPriv || viewPriv));

  menuItem = pMenu->addAction(tr("Delete..."), this, SLOT(sDeleteSale()));
  menuItem->setEnabled(editPriv);

  menuItem = pMenu->addAction(tr("Print..."), this, SLOT(sPrintSale()));
  menuItem->setEnabled((editPriv || viewPriv));
}

void opportunity::sHandleSalesPrivs()
{
  bool isCustomer = (_custid > -1);
  
  bool newQuotePriv = ((cNew == _mode || cEdit == _mode) && _privileges->check("MaintainQuotes"));
  bool newSalesOrderPriv = ((cNew == _mode || cEdit == _mode) && isCustomer && _privileges->check("MaintainSalesOrders"));

  bool editPriv = false;
  bool viewPriv = false;
  bool convertPriv = false;

  QAction *menuItem;
  QMenu * newMenu = new QMenu;
  menuItem = newMenu->addAction(tr("Quote"), this, SLOT(sNewQuote()));
  menuItem->setEnabled(newQuotePriv);
  menuItem = newMenu->addAction(tr("Sales Order"), this, SLOT(sNewSalesOrder()));
  menuItem->setEnabled(newSalesOrderPriv);
  _newSale->setMenu(newMenu);

  QMenu * attachMenu = new QMenu;
  menuItem = attachMenu->addAction(tr("Quote"), this, SLOT(sAttachQuote()));
  menuItem->setEnabled(newQuotePriv);
  menuItem = attachMenu->addAction(tr("Sales Order"), this, SLOT(sAttachSalesOrder()));
  menuItem->setEnabled(newSalesOrderPriv);
  _attachSale->setMenu(attachMenu);

  if(_salesList->currentItem())
  {
    editPriv = (cNew == _mode || cEdit == _mode) && (
      (0 == _salesList->currentItem()->altId() && _privileges->check("MaintainQuotes")) ||
      (1 == _salesList->currentItem()->altId() && _privileges->check("MaintainSalesOrders")) );

    viewPriv = (cNew == _mode || cEdit == _mode) && (
      (0 == _salesList->currentItem()->altId() && _privileges->check("ViewQuotes")) ||
      (1 == _salesList->currentItem()->altId() && _privileges->check("ViewSalesOrders")) );

    convertPriv = (cNew == _mode || cEdit == _mode) &&
      (0 == _salesList->currentItem()->altId() && _privileges->check("ConvertQuotes"));
  }

  _convertQuote->setEnabled(convertPriv && _salesList->id() > 0);
  _editSale->setEnabled(editPriv && _salesList->id() > 0);
  _viewSale->setEnabled((editPriv || viewPriv) && _salesList->id() > 0);
  _printSale->setEnabled((editPriv || viewPriv) && _salesList->id() > 0);
  _deleteSale->setEnabled(editPriv && _salesList->id() > 0);

  if (editPriv)
  {
    disconnect(_salesList,SIGNAL(itemSelected(int)),_viewSale, SLOT(animateClick()));
    connect(_salesList,	SIGNAL(itemSelected(int)), _editSale, SLOT(animateClick()));
  }
  else if (viewPriv)
  {
    disconnect(_salesList,SIGNAL(itemSelected(int)),_editSale, SLOT(animateClick()));
    connect(_salesList,	SIGNAL(itemSelected(int)), _viewSale, SLOT(animateClick()));
  }
}

void opportunity::sNewCharacteristic()
{
  if (! save(true))
    return;

  ParameterList params;
  params.append("mode", "new");
  params.append("ophead_id", _opheadid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sEditCharacteristic()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharList();
}

void opportunity::sDeleteCharacteristic()
{
  XSqlQuery opportunityDeleteCharacteristic;
  opportunityDeleteCharacteristic.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  opportunityDeleteCharacteristic.bindValue(":charass_id", _charass->id());
  opportunityDeleteCharacteristic.exec();

  sFillCharList();
}

void opportunity::sFillCharList()
{
  XSqlQuery opportunityFillCharList;
  opportunityFillCharList.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value, "
             "charass_default "
             "FROM charass, char "
             "WHERE ( (charass_target_type='OPP')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:ophead_id) ) "
             "ORDER BY char_order, char_name;" );
  opportunityFillCharList.bindValue(":ophead_id", _opheadid);
  opportunityFillCharList.exec();
  _charass->populate(opportunityFillCharList);
}

void opportunity::sHandleCrmacct(int pCrmacctid)
{
  XSqlQuery crmacct;
  crmacct.prepare( "SELECT COALESCE(crmacct_cust_id, -1) AS cust_id, "
                   "       COALESCE(crmacct_prospect_id, -1) AS prospect_id "
                   "FROM crmacct "
                   "WHERE (crmacct_id=:crmacct_id);" );
  crmacct.bindValue(":crmacct_id", pCrmacctid);
  crmacct.exec();
  if (crmacct.first())
  {
    _custid = crmacct.value("cust_id").toInt();
	_prospectid = crmacct.value("prospect_id").toInt();
  }
  if (_custid == -1 && _prospectid == -1)
    _salesTab->setEnabled(false);
  else
    _salesTab->setEnabled(true);

  sHandleSalesPrivs();
}

void opportunity::sHandleAssigned()
{
  if (_assignedTo->isValid() && !_assignDate->isValid())
    _assignDate->setDate(omfgThis->dbDate());
}

