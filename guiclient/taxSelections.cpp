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

#include "taxSelections.h"

#include <QSqlError>

#include "metasql.h"
#include "parameter.h"

#include "guiclient.h"
#include "taxSelection.h"

taxSelections::taxSelections(QWidget* parent, const char* name, Qt::WFlags fl)
  : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_delete,	SIGNAL(clicked()),	 this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	 this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	 this, SLOT(sNew()));
  connect(_showAll,	SIGNAL(toggled(bool)),	 this, SLOT(sFillList()));
  connect(_taxsel,	SIGNAL(valid(bool)),	_view, SLOT(setEnabled(bool)));
  connect(_taxauth,	SIGNAL(newID(int)),	 this, SLOT(sFillList()));
  connect(_view,	SIGNAL(clicked()),	 this, SLOT(sView()));

  if (_privileges->check("MaintainTaxSel"))
  {
    connect(_taxsel,	SIGNAL(valid(bool)),  _delete, SLOT(setEnabled(bool)));
    connect(_taxsel,	SIGNAL(valid(bool)),	_edit, SLOT(setEnabled(bool)));
    connect(_taxsel, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_taxsel, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _taxsel->addColumn(tr("Tax Authority"),	 -1,	Qt::AlignLeft  );
  _taxsel->addColumn(tr("Tax Type"),		100,	Qt::AlignLeft  );
  _taxsel->addColumn(tr("Tax Code"),		100,	Qt::AlignLeft  );

  sFillList();
}

taxSelections::~taxSelections()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxSelections::languageChange()
{
  retranslateUi(this);
}

void taxSelections::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxsel_id", _taxsel->id());

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxsel_id", _taxsel->id());

  taxSelection newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxSelections::sDelete()
{
  q.prepare("DELETE FROM taxsel WHERE (taxsel_id=:taxsel_id);");
  q.bindValue(":taxsel_id", _taxsel->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void taxSelections::sFillList()
{
  ParameterList params;
  if (! _showAll->isChecked())
    params.append("taxauth_id", _taxauth->id());
  params.append("any", tr("~Any~"));

  QString sql("SELECT taxsel_id, "
	      "       COALESCE(taxauth_code, <? value(\"any\") ?>) AS taxauth, "
	      "       COALESCE(taxtype_name, <? value(\"any\") ?>) AS taxtype, "
	      "       COALESCE(tax_code, '') AS tax "
	      "FROM taxsel LEFT OUTER JOIN "
	      "     taxauth ON (taxsel_taxauth_id=taxauth_id) LEFT OUTER JOIN "
	      "     taxtype ON (taxsel_taxtype_id=taxtype_id) LEFT OUTER JOIN "
	      "     tax     ON (taxsel_tax_id=tax_id)"
	      "<? if exists(\"taxauth_id\") ?>"
	      "WHERE ((taxsel_taxauth_id = <? value(\"taxauth_id\") ?>) "
	      "  OR   (taxsel_taxauth_id IS NULL))"
	      "<? endif ?>"
	      "ORDER BY taxauth_id, taxtype_id, tax"
	      ";");
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _taxsel->clear();
  _taxsel->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
