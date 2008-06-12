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

#include "financialLayouts.h"

#include <QVariant>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <parameter.h>
#include "financialLayout.h"

/*
 *  Constructs a financialLayouts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
financialLayouts::financialLayouts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

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

  statusBar()->hide();
  
  _flhead->addColumn(tr("Name"),        (_itemColumn*2), Qt::AlignLeft );
  _flhead->addColumn(tr("Description"), -1,          Qt::AlignLeft );
  _flhead->addColumn(tr("Active"), _ynColumn,          Qt::AlignLeft );
  _flhead->addColumn(tr("System"), _ynColumn,          Qt::AlignLeft );

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

  financialLayout newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayouts::sEdit()
{
  ParameterList params;
  q.prepare("SELECT * FROM flhead "
            " WHERE ((flhead_id=:flhead_id)"
            "   AND  (flhead_sys));");
  q.bindValue(":flhead_id", _flhead->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::information(this, tr("System Report"), 
            tr("This is a system report and will be opened in\n"
               "view mode. Only status may be changed."));
    params.append("mode", "view");
  }
  else
    params.append("mode", "edit");
  params.append("flhead_id", _flhead->id());

  financialLayout newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void financialLayouts::sDelete()
{
  q.prepare("SELECT * FROM flhead "
            " WHERE ((flhead_id=:flhead_id)"
            "   AND  (flhead_sys));");
  q.bindValue(":flhead_id", _flhead->id());
  q.exec();
  if (q.first())
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
    q.prepare( "DELETE FROM flcol "
               "WHERE (flcol_flhead_id=:flhead_id);"
               "DELETE FROM flitem "
               "WHERE (flitem_flhead_id=:flhead_id);"
               "DELETE FROM flgrp "
               "WHERE (flgrp_flhead_id=:flhead_id);"
               "DELETE FROM flhead "
               "WHERE (flhead_id=:flhead_id);" );
    q.bindValue(":flhead_id", _flhead->id());
    q.exec();

    sFillList();
  }
}

void financialLayouts::sFillList()
{
  QString query;

  _flhead->clear();
    
  query = ("SELECT flhead_id, flhead_name, flhead_descrip,"
           " formatboolyn(flhead_active),formatboolyn(flhead_sys) "
           "FROM flhead ");
  if (!_showInactive->isChecked())
    query += " WHERE flhead_active=true ";
  query += "ORDER BY flhead_name;";
    
  _flhead->populate(query); 
}

void financialLayouts::sCopy()
{
  bool ok;
  QString text;
  do {
    text = QInputDialog::getText( tr("Copy Financial Report"),
           tr("Target Report:"),
           QLineEdit::Normal,
           text, &ok, this );
    if ( ok ) {
      q.prepare("SELECT copyFinancialLayout(:flhead_id, :name) AS result;");
      q.bindValue(":flhead_id", _flhead->id());
      q.bindValue(":name", text);
      q.exec();
      if(q.first())
      {
        if(q.value("result").toInt() < 0)
        {
          QString message;
          switch(q.value("result").toInt())
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
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
        return;
      }
    }
    else
      return;
  } while ( !ok );
}

