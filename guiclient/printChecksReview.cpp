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

#include "printChecksReview.h"

#include <QSqlError>
#include <QVariant>

#include "guiclient.h"
#include "storedProcErrorLookup.h"

printChecksReview::printChecksReview(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_complete,  SIGNAL(clicked()), this, SLOT(sComplete()));
  connect(_printed,   SIGNAL(clicked()), this, SLOT(sMarkPrinted()));
  connect(_replace,   SIGNAL(clicked()), this, SLOT(sMarkReplaced()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_unmark,    SIGNAL(clicked()), this, SLOT(sUnmark()));
  connect(_voided,    SIGNAL(clicked()), this, SLOT(sMarkVoided()));

  _checks->addColumn(tr("Check Number"),          -1, Qt::AlignLeft );
  _checks->addColumn(tr("Action"),       _itemColumn, Qt::AlignLeft );
}

printChecksReview::~printChecksReview()
{
  // no need to delete child widgets, Qt does it all for us
}

void printChecksReview::languageChange()
{
  retranslateUi(this);
}

static const int ActionUnmark = -1;
static const int ActionPrinted = 1;
static const int ActionVoided = 2;
static const int ActionReplaced = 3;

/*
  TODO: refactor printChecks/printChecksReview so that printChecks
	hands printChecksReview a list of checks to be reviewed
	instead of populating printChecksReview directly.  Then
	printChecksReview could populate its GUI from this list.
	This will allow printChecksReview to have real error reporting
	and recovery: requery the db for the entire passed-in list
	and show the current state of each check so the user can
	decide what to reprocess if there were errors.
*/
void printChecksReview::sComplete()
{
  XSqlQuery checkPrint;
  checkPrint.prepare( "SELECT markCheckAsPrinted(:check_id) AS result;");

  XSqlQuery checkVoid;
  checkVoid.prepare( "SELECT voidCheck(:check_id) AS result;" );

  XSqlQuery checkReplace;
  checkReplace.prepare( "SELECT replaceVoidedCheck(:check_id) AS result;" );

  // no returns in the loop: process as much as possible, regardless of errors
  for (int i = 0; i < _checks->topLevelItemCount(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)(_checks->topLevelItem(i));

    switch(cursor->altId())
    {
      case ActionPrinted:
        checkPrint.bindValue(":check_id", cursor->id());
        checkPrint.exec();
	if (checkPrint.first())
	{
	  int result = checkPrint.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("markCheckPrinted", result), __FILE__, __LINE__);
	}
	else if (checkPrint.lastError().type() != QSqlError::None)
	  systemError(this, checkPrint.lastError().databaseText(), __FILE__, __LINE__);
        break;
      case ActionVoided:
        checkVoid.bindValue(":check_id", cursor->id());
        checkVoid.exec();
	if (checkVoid.first())
	{
	  int result = checkVoid.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
	}
	else if (checkVoid.lastError().type() != QSqlError::None)
	  systemError(this, checkVoid.lastError().databaseText(), __FILE__, __LINE__);
        break;
      case ActionReplaced:
        checkVoid.bindValue(":check_id", cursor->id());
        checkVoid.exec();
	if (checkVoid.first())
	{
	  int result = checkVoid.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("voidCheck", result), __FILE__, __LINE__);
	}
	else if (checkVoid.lastError().type() != QSqlError::None)
	  systemError(this, checkVoid.lastError().databaseText(), __FILE__, __LINE__);
        checkReplace.bindValue(":check_id", cursor->id());
        checkReplace.exec();
	if (checkReplace.first())
	{
	  int result = checkReplace.value("result").toInt();
	  if (result < 0)
	    systemError(this, storedProcErrorLookup("replaceVoidedCheck", result), __FILE__, __LINE__);
	}
	else if (checkReplace.lastError().type() != QSqlError::None)
	  systemError(this, checkReplace.lastError().databaseText(), __FILE__, __LINE__);
        break;
    }
  }
  // TODO: after refactoring, handle any errors in the loop here and *return*

  close();
}

void printChecksReview::sUnmark()
{
  markSelected(ActionUnmark);
}

void printChecksReview::sMarkPrinted()
{
  markSelected(ActionPrinted);
}

void printChecksReview::sMarkVoided()
{
  markSelected(ActionVoided);
}

void printChecksReview::sMarkReplaced()
{
  markSelected(ActionReplaced);
}

void printChecksReview::sSelectAll()
{
  _checks->selectAll();
}

void printChecksReview::markSelected( int actionId )
{
  QString action;
  switch(actionId)
  {
    case ActionPrinted:
      action = tr("Printed");
      break;
    case ActionVoided:
      action = tr("Voided");
      break;
    case ActionReplaced:
      action = tr("Replace");
      break;
    case ActionUnmark:
    default:
      actionId = -1;
      action = "";
  };

  QList<QTreeWidgetItem*> selected = _checks->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
    cursor->setText(1, action);
    cursor->setAltId(actionId);
  }
}
