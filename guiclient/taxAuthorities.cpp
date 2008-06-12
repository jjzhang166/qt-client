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

#include "taxAuthorities.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "taxAuthority.h"

/*
 *  Constructs a taxAuthorities as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
taxAuthorities::taxAuthorities(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_taxauth, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList(int)));

  _taxauth->addColumn(tr("Code"), 70, Qt::AlignLeft );
  _taxauth->addColumn(tr("Name"), -1, Qt::AlignLeft );

  if (_privileges->check("MaintainTaxAuthorities"))
  {
    connect(_taxauth, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_taxauth, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_taxauth, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_taxauth, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  sFillList(-1);
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxAuthorities::~taxAuthorities()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxAuthorities::languageChange()
{
    retranslateUi(this);
}

void taxAuthorities::sDelete()
{
  q.prepare("SELECT deleteTaxAuthority(:taxauth_id) AS result;");
  q.bindValue(":taxauth_id", _taxauth->id());
  q.exec();
  if (q.first())
  {
    if(q.value("result").toInt() < 0)
    {
      QMessageBox::warning( this, tr("Cannot Delete Tax Authority"),
                            tr( "You cannot delete the selected Tax Authority because there are currently items assigned to it.\n"
                                "You must first re-assign these items before deleting the selected Tax Authority." ) );
      return;
    }

    omfgThis->sTaxAuthsUpdated(_taxauth->id());
    sFillList(-1);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void taxAuthorities::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxAuthorities::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxauth_id", _taxauth->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxAuthorities::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxauth_id", _taxauth->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxAuthorities::sPrint()
{
  orReport report("TaxAuthoritiesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void taxAuthorities::sFillList(int pId)
{
  _taxauth->populate( "SELECT taxauth_id, taxauth_code, taxauth_name "
                      "FROM taxauth "
                      "ORDER BY taxauth_code;", pId );
}

