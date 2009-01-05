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

#include "uiforms.h"

#include <QBuffer>
#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>
#include "xuiloader.h"
#include "uiform.h"
#include "guiclient.h"
#include "xmainwindow.h"

uiforms::uiforms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_test, SIGNAL(clicked()), this, SLOT(sTest()));

  _uiform->addColumn(tr("Name"),   _itemColumn, Qt::AlignLeft,  true, "uiform_name");
  _uiform->addColumn(tr("Description"),     -1, Qt::AlignLeft,  true, "uiform_notes");
  _uiform->addColumn(tr("Order"),    _ynColumn, Qt::AlignCenter,true, "uiform_order");
  _uiform->addColumn(tr("Enabled"),  _ynColumn, Qt::AlignCenter,true, "uiform_enabled");
  _uiform->addColumn(tr("Package"),_itemColumn, Qt::AlignLeft,  false,"nspname");

  sFillList();
}

uiforms::~uiforms()
{
  // no need to delete child widgets, Qt does it all for us
}

void uiforms::languageChange()
{
  retranslateUi(this);
}

void uiforms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  uiform *newdlg = new uiform(this, "");
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  sFillList();
}

void uiforms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("uiform_id", _uiform->id());

  uiform *newdlg = new uiform(this, "");
  newdlg->set(params);

  omfgThis->handleNewWindow(newdlg);
  sFillList();
}

void uiforms::sDelete()
{
  q.prepare( "DELETE FROM uiform "
             "WHERE (uiform_id=:uiform_id);" );
  q.bindValue(":uiform_id", _uiform->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void uiforms::sFillList()
{
  q.exec("SELECT uiform.*,"
         "       CASE WHEN nspname='public' THEN ''"
         "            ELSE nspname END AS nspname"
         "  FROM uiform, pg_class, pg_namespace"
         " WHERE ((uiform.tableoid=pg_class.oid)"
         "   AND  (relnamespace=pg_namespace.oid))"
         " ORDER BY uiform_name, uiform_order, uiform_id;" );
  _uiform->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void uiforms::sTest()
{
  q.prepare("SELECT *"
            "  FROM uiform "
            " WHERE(uiform_id=:uiform_id);");
  q.bindValue(":uiform_id", _uiform->id());
  q.exec();
  if (q.first())
    ; // everything's OK
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    return;
  
  XMainWindow * wnd = new XMainWindow();
  wnd->setObjectName(q.value("uiform_name").toString());

  XUiLoader loader;
  QByteArray ba = q.value("uiform_source").toByteArray();
  QBuffer uiFile(&ba);
  if(!uiFile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this, tr("Could not load file"),
        tr("There was an error loading the UI Form from the database."));
    return;
  }
  QWidget *ui = loader.load(&uiFile);
  wnd->setWindowTitle(ui->windowTitle());
  uiFile.close();
  wnd->setCentralWidget(ui);

  omfgThis->handleNewWindow(wnd);
}
