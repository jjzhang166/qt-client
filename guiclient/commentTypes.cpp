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

#include "commentTypes.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include "commentType.h"

/*
 *  Constructs a commentTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
commentTypes::commentTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_cmnttype, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(sHandleButtons()));

  statusBar()->hide();
  
  _cmnttype->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft   );
  _cmnttype->addColumn(tr("Sys."),        _ynColumn,   Qt::AlignCenter );
  _cmnttype->addColumn(tr("Description"), -1,          Qt::AlignLeft   );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
commentTypes::~commentTypes()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void commentTypes::languageChange()
{
  retranslateUi(this);
}

void commentTypes::sFillList()
{
  q.prepare( "SELECT cmnttype_id, cmnttype_name,"
             "       CASE WHEN (cmnttype_sys) THEN :yes"
             "            ELSE :no"
             "       END,"
             "       cmnttype_descrip "
             "FROM cmnttype "
             "ORDER BY cmnttype_name;" );
  q.bindValue(":yes", tr("Yes"));
  q.bindValue(":no", tr("No"));
  q.exec();
  _cmnttype->populate(q);
}

void commentTypes::sHandleButtons()
{
  bool enableButtons;

  XTreeWidgetItem *selected = (XTreeWidgetItem*)_cmnttype->currentItem();
  if (selected)
    enableButtons = (selected->text(1) == tr("No"));
  else
    enableButtons = FALSE;

  _edit->setEnabled(enableButtons);
  _delete->setEnabled(enableButtons);
}

void commentTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  commentType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void commentTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("cmnttype_id", _cmnttype->id());

  commentType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void commentTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cmnttype_id", _cmnttype->id());

  commentType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void commentTypes::sDelete()
{
  q.prepare( "SELECT comment_id "
             "FROM comment "
             "WHERE (comment_cmnttype_id=:cmnttype_id) "
             "LIMIT 1;" );
  q.bindValue(":cmnttype_id", _cmnttype->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Comment Type"),
                           tr("The selected Comment Type cannot be deleted because there are Comments that are assigned to it.\n") );
    return;
  }
  else
  {
    q.prepare( "DELETE FROM cmnttype "
               "WHERE (cmnttype_id=:cmnttype_id);" );
    q.bindValue(":cmnttype_id", _cmnttype->id());
    q.exec();

    sFillList();
  }
}

void commentTypes::sPrint()
{
  orReport report("CommentTypes");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

