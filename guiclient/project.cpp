/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "project.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <comment.h>
#include "task.h"

project::project(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_printTasks, SIGNAL(clicked()), this, SLOT(sPrintTasks()));
  connect(_newTask, SIGNAL(clicked()), this, SLOT(sNewTask()));
  connect(_editTask, SIGNAL(clicked()), this, SLOT(sEditTask()));
  connect(_viewTask, SIGNAL(clicked()), this, SLOT(sViewTask()));
  connect(_deleteTask, SIGNAL(clicked()), this, SLOT(sDeleteTask()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sNumberChanged()));

  _prjtask->addColumn( tr("Number"),      _itemColumn, Qt::AlignRight );
  _prjtask->addColumn( tr("Name"),        _itemColumn, Qt::AlignLeft  );
  _prjtask->addColumn( tr("Description"), -1,          Qt::AlignLeft  ); 
}

/*
 *  Destroys the object and frees any allocated resources
 */
project::~project()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void project::languageChange()
{
    retranslateUi(this);
}

enum SetResponse project::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("prj_id", &valid);
  if (valid)
  {
    _prjid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      connect(_prjtask, SIGNAL(valid(bool)), _editTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), _deleteTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));

      q.exec("SELECT NEXTVAL('prj_prj_id_seq') AS prj_id;");
      if (q.first())
        _prjid = q.value("prj_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return UndefinedError;
      }

      _comments->setId(_prjid);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _number->setEnabled(FALSE);

      connect(_prjtask, SIGNAL(valid(bool)), _editTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(valid(bool)), _deleteTask, SLOT(setEnabled(bool)));
      connect(_prjtask, SIGNAL(itemSelected(int)), _editTask, SLOT(animateClick()));
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      
      _number->setEnabled(FALSE);
      _status->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _so->setEnabled(FALSE);
      _wo->setEnabled(FALSE);
      _po->setEnabled(FALSE);

      _newTask->setEnabled(FALSE);
      connect(_prjtask, SIGNAL(itemSelected(int)), _viewTask, SLOT(animateClick()));
      _comments->setReadOnly(TRUE);
    }
  }
    
  return NoError;
}

void project::populate()
{
  q.prepare( "SELECT prj_number, prj_name, prj_descrip,"
             "       prj_so, prj_wo, prj_po, prj_status "
             "FROM prj "
             "WHERE (prj_id=:prj_id);" );
  q.bindValue(":prj_id", _prjid);
  q.exec();
  if (q.first())
  {
    _number->setText(q.value("prj_number").toString());
    _name->setText(q.value("prj_name").toString());
    _descrip->setText(q.value("prj_descrip").toString());
    _so->setChecked(q.value("prj_so").toBool());
    _wo->setChecked(q.value("prj_wo").toBool());
    _po->setChecked(q.value("prj_po").toBool());
    QString status = q.value("prj_status").toString();
    if("P" == status)
      _status->setCurrentItem(0);
    else if("O" == status)
      _status->setCurrentItem(1);
    else if("C" == status)
      _status->setCurrentItem(2);
  }

  sFillTaskList();
  _comments->setId(_prjid);
}

void project::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM prjtask "
               "WHERE (prjtask_prj_id=:prj_id);" );
    q.bindValue(":prj_id", _prjid);
    q.exec();
  }

  reject();
}

void project::sSave()
{
  if (_number->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::warning( this, tr("Cannot Save Project"),
      tr("No Project Number was specified. You must specify a project number.") );
    return;
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO prj "
               "( prj_id, prj_number, prj_name, prj_descrip,"
               "  prj_so, prj_wo, prj_po, prj_status ) "
               "VALUES "
               "( :prj_id, :prj_number, :prj_name, :prj_descrip,"
               "  :prj_so, :prj_wo, :prj_po, :prj_status );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE prj "
               "SET prj_number=:prj_number, prj_name=:prj_name, prj_descrip=:prj_descrip,"
               "    prj_so=:prj_so, prj_wo=:prj_wo, prj_po=:prj_po, prj_status=:prj_status "
               "WHERE (prj_id=:prj_id);" );

  q.bindValue(":prj_id", _prjid);
  q.bindValue(":prj_number", _number->text().stripWhiteSpace().upper());
  q.bindValue(":prj_name", _name->text());
  q.bindValue(":prj_descrip", _descrip->text());
  q.bindValue(":prj_so", QVariant(_so->isChecked(), 0));
  q.bindValue(":prj_wo", QVariant(_wo->isChecked(), 0));
  q.bindValue(":prj_po", QVariant(_po->isChecked(), 0));
  switch(_status->currentItem())
  {
    case 0:
    default:
      q.bindValue(":prj_status", "P");
      break;
    case 1:
      q.bindValue(":prj_status", "O");
      break;
    case 2:
      q.bindValue(":prj_status", "C");
      break;
  }
  q.exec();

  done(_prjid);
}

void project::sPrintTasks()
{
  ParameterList params;

  params.append("prj_id", _prjid);

  orReport report("ProjectTaskList", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void project::sNewTask()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("prj_id", _prjid);

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sEditTask()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillTaskList();
}

void project::sViewTask()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prjtask_id", _prjtask->id());

  task newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void project::sDeleteTask()
{
  q.prepare("DELETE FROM prjtask"
            " WHERE (prjtask_id=:prjtask_id); ");
  q.bindValue(":prjtask_id", _prjtask->id());
  q.exec();
  sFillTaskList();
}

void project::sFillTaskList()
{
  q.prepare( "SELECT prjtask_id, prjtask_number, prjtask_name, firstLine(prjtask_descrip) "
             "FROM prjtask "
             "WHERE (prjtask_prj_id=:prj_id) "
             "ORDER BY prjtask_number;" );
  q.bindValue(":prj_id", _prjid);
  q.exec();
  _prjtask->populate(q);
}

void project::sNumberChanged()
{
  if((cNew == _mode) && (_number->text().length()))
  {
    _number->setText(_number->text().stripWhiteSpace().upper());

    q.prepare( "SELECT prj_id"
               "  FROM prj"
               " WHERE (prj_number=:prj_number);" );
    q.bindValue(":prj_number", _number->text());
    q.exec();
    if(q.first())
    {
      _number->setEnabled(FALSE);
      _prjid = q.value("prj_id").toInt();
      _mode = cEdit;
      populate();
    }
    else
    {
      _number->setEnabled(FALSE);
      _mode = cNew;
    }
  }
}

