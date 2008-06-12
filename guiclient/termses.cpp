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

#include "termses.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <openreports.h>
#include "terms.h"

/*
 *  Constructs a termses as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
termses::termses(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_terms, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_terms, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
termses::~termses()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void termses::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void termses::init()
{
  statusBar()->hide();
  
  _terms->addColumn(tr("Code"),        _itemColumn, Qt::AlignLeft   );
  _terms->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _terms->addColumn(tr("Type"),        _dateColumn, Qt::AlignCenter );
  _terms->addColumn(tr("A/P"),         _ynColumn,   Qt::AlignCenter );
  _terms->addColumn(tr("A/R"),         _ynColumn,   Qt::AlignCenter );

  if (_privileges->check("MaintainTerms"))
  {
    connect(_terms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_terms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_terms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_terms, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void termses::sFillList()
{
  q.prepare( "SELECT terms_id, terms_code, terms_descrip,"
             "       CASE WHEN (terms_type = 'D') THEN :days"
             "            WHEN (terms_type = 'P') THEN :proximo"
             "            ELSE '?'"
             "       END,"
             "       formatBoolYN(terms_ap), formatBoolYN(terms_ar) "
             "FROM terms "
             "ORDER BY terms_code;" );
  q.bindValue(":days", tr("Days"));
  q.bindValue(":proximo", tr("Proximo"));
  q.exec();
  _terms->populate(q);
}

void termses::sDelete()
{
  q.prepare( "SELECT cust_id "
             "FROM cust "
             "WHERE (cust_terms_id=:terms_id);" );
  q.bindValue(":terms_id", _terms->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Terms Code"),
                           tr( "You may not delete the selected Terms Code are there are one or more Customers assigned to it.\n"
                               "You must reassign these Customers before you may delete the selected Terms Code." ) );
    return;
  }

  q.prepare( "SELECT vend_id "
             "FROM vend "
             "WHERE (vend_terms_id=:terms_id);" );
  q.bindValue(":terms_id", _terms->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Terms Code"),
                           tr( "You may not delete the selected Terms Code are there are one or more Vendors assigned to it.\n"
                               "You must reassign these Vendors before you may delete the selected Terms Code." ) );
    return;
  }

  q.prepare( "DELETE FROM terms "
             "WHERE (terms_id=:terms_id);" );
  q.bindValue(":terms_id", _terms->id());
  q.exec();

  sFillList();
}

void termses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  terms newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void termses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("terms_id", _terms->id());

  terms newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void termses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("terms_id", _terms->id());

  terms newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void termses::sPopulateMenu( QMenu * )
{

}

void termses::sPrint()
{
  orReport report("TermsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

