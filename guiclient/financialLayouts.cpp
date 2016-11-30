/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "financialLayouts.h"

#include <QVariant>
#include <QMessageBox>
#include <QInputDialog>
//#include <QStatusBar>
#include <parameter.h>
#include "financialLayout.h"
#include "errorReporter.h"

/*
 *  Constructs a financialLayouts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
financialLayouts::financialLayouts(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_flhead, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
  connect(_flhead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_flhead, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_flhead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

//  statusBar()->hide();
  
  _flhead->addColumn(tr("Name"),        (_itemColumn*2), Qt::AlignLeft, true, "flhead_name"  );
  _flhead->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "flhead_descrip" );
  _flhead->addColumn(tr("Active"), _ynColumn,          Qt::AlignLeft, true, "active" );
  _flhead->addColumn(tr("System"), _ynColumn,          Qt::AlignLeft, true, "system" );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
financialLayouts::~financialLayouts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void financialLayouts::languageChange()
{
    retranslateUi(this);
}

void financialLayouts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  financialLayout newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayouts::sEdit()
{
  XSqlQuery financialEdit;
  ParameterList params;
  financialEdit.prepare("SELECT * FROM flhead "
            " WHERE ((flhead_id=:flhead_id)"
            "   AND  (flhead_sys));");
  financialEdit.bindValue(":flhead_id", _flhead->id());
  financialEdit.exec();
  if (financialEdit.first())
  {
    QMessageBox::information(this, tr("System Report"), 
            tr("This is a system report and will be opened in "
               "view mode. Only status and notes may be changed."));
    params.append("mode", "view");
  }
  else
    params.append("mode", "edit");
  params.append("flhead_id", _flhead->id());

  financialLayout newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayouts::sDelete()
{
  XSqlQuery financialDelete;
  financialDelete.prepare("SELECT * FROM flhead "
            " WHERE ((flhead_id=:flhead_id)"
            "   AND  (flhead_sys));");
  financialDelete.bindValue(":flhead_id", _flhead->id());
  financialDelete.exec();
  if (financialDelete.first())
  {
    QMessageBox::critical(this, tr("System Report"), 
            tr("You may not delete a system report,\n"
               "but you may deactivate it."));
    return;
  }

  if(QMessageBox::question( this, tr("Confirm Delete"),
       tr("You are about to delete the selected Financial Report and all of its items.\n"
          "Are you sure you want to continue?"),
       QMessageBox::Yes, QMessageBox::No | QMessageBox::Escape | QMessageBox::Default) == QMessageBox::Yes)
  {
    financialDelete.prepare( "DELETE FROM flcol "
               "WHERE (flcol_flhead_id=:flhead_id);"
               "DELETE FROM flitem "
               "WHERE (flitem_flhead_id=:flhead_id);"
               "DELETE FROM flgrp "
               "WHERE (flgrp_flhead_id=:flhead_id);"
               "DELETE FROM flhead "
               "WHERE (flhead_id=:flhead_id);" );
    financialDelete.bindValue(":flhead_id", _flhead->id());
    financialDelete.exec();

    sFillList();
  }
}

void financialLayouts::sFillList()
{
  QString query;

  _flhead->clear();
    
  query = ("SELECT flhead_id, flhead_name, flhead_descrip,"
           " formatboolyn(flhead_active) AS active,formatboolyn(flhead_sys) AS system "
           "FROM flhead ");
  if (!_showInactive->isChecked())
    query += " WHERE flhead_active=true ";
  query += "ORDER BY flhead_name;";
    
  _flhead->populate(query); 
}

void financialLayouts::sCopy()
{
  XSqlQuery financialCopy;
  bool ok;
  QString text;
  do {
    text = QInputDialog::getText(this, tr("Copy Financial Report"),
                                 tr("Target Report:"), QLineEdit::Normal,
                                 text, &ok);
    if ( ok ) {
      financialCopy.prepare("SELECT copyFinancialLayout(:flhead_id, :name) AS result;");
      financialCopy.bindValue(":flhead_id", _flhead->id());
      financialCopy.bindValue(":name", text);
      financialCopy.exec();
      if(financialCopy.first())
      {
        if(financialCopy.value("result").toInt() < 0)
        {
          QString message;
          switch(financialCopy.value("result").toInt())
          {
            case -1:
              message = tr("The record you are trying to copy is no longer on the database.");
              break;
            case -2:
              message = tr("You must specify a name.");
              ok = false;
              break;
            case -3:
              message = tr("The name you specified is already in use. Please choose a different name.");
              ok = false;
              break;
            default:
              message = tr("There was an unknown error encountered while copying this report.");
          }
          QMessageBox::critical(this, tr("Error Encountered"), message);
        }
        else
          sFillList();
      }
      else
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Copy Error"),
                             tr("%1: An error occurred copying financial layout "
                                "information").arg(windowTitle()),__FILE__,__LINE__);
        return;
      }
    }
    else
      return;
  } while ( !ok );
}

