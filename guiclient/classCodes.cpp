/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCodes.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "classCode.h"
#include "storedProcErrorLookup.h"
#include "errorReporter.h"

classCodes::classCodes(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  if (_privileges->check("MaintainClassCodes"))
  {
    connect(_classcode, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_classcode, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_classcode, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_classcode, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(false);
    _deleteUnused->setEnabled(false);
  }

  _classcode->addColumn(tr("Class Code"),  70, Qt::AlignLeft, true, "classcode_code");
  _classcode->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "classcode_descrip");

  sFillList(-1);
}

classCodes::~classCodes()
{
  // no need to delete child widgets, Qt does it all for us
}

void classCodes::languageChange()
{
  retranslateUi(this);
}

void classCodes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  classCode newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void classCodes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("classcode_id", _classcode->id());

  classCode newdlg(this, "", true);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void classCodes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("classcode_id", _classcode->id());

  classCode newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void classCodes::sDelete()
{
  XSqlQuery classDelete;
  classDelete.prepare("SELECT deleteClassCode(:classcode_id) AS result;");
  classDelete.bindValue(":classcode_id", _classcode->id());
  classDelete.exec();
  if (classDelete.first())
  {
    int result = classDelete.value("result").toInt();
    if (result < 0)
    {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Class Code"),
                             storedProcErrorLookup("deleteClassCode", result),
                             __FILE__, __LINE__);
        return;
    }
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Class Code"),
                                classDelete, __FILE__, __LINE__))
  {
    return;
  }
  sFillList(-1);
}

void classCodes::sPrint()
{
  orReport report("ClassCodesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void classCodes::sDeleteUnused()
{
  XSqlQuery classDeleteUnused;
  if ( QMessageBox::warning( this, tr("Delete Unused Class Codes"),
                             tr("<p>Are you sure that you wish to delete all "
                                "unused Class Codes?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    classDeleteUnused.exec("SELECT deleteUnusedClassCodes() AS result;");
    if (classDeleteUnused.first())
    {
      int result = classDeleteUnused.value("result").toInt();
      if (result < 0)
      {
          ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Unused Class Codes"),
                               storedProcErrorLookup("deleteUnusedClassCodes", result),
                               __FILE__, __LINE__);
          return;
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Unused Class Codes"),
                                  classDeleteUnused, __FILE__, __LINE__))
    {
      return;
    }
    sFillList(-1);
  }
}

void classCodes::sFillList(int pId)
{
  _classcode->populate( "SELECT classcode_id, classcode_code, classcode_descrip "
                        "FROM classcode "
                        "ORDER BY classcode_code;", pId  );
}
