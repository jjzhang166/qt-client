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

#include "configurePD.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "storedProcErrorLookup.h"
#include "guiclient.h"

/*
 *  Constructs a configurePD as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configurePD::configurePD(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  if (_metrics->value("TrackMachineOverhead") == "M")
    _machineOverhead->setChecked(TRUE);
  else
    _generalOverhead->setChecked(TRUE);

  _inactiveBomItems->setChecked(_metrics->boolean("AllowInactiveBomItems"));
  _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));
  _changeLog->setChecked(_metrics->boolean("ItemChangeLog"));

  QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
  if (issueMethod == "S")
    _issueMethod->setCurrentItem(0);
  else if (issueMethod == "L")
    _issueMethod->setCurrentItem(1);
  else if (issueMethod == "M")
    _issueMethod->setCurrentItem(2);
    
  
  if (_metrics->value("Application") != "OpenMFG")
  {
    _routings->hide();
    _routings->setChecked(FALSE);
    _bbom->hide();
  }
  else
  {
    q.exec("SELECT booitem_id FROM booitem "
             "UNION "
             "SELECT wooper_id FROM wooper, wo "
             "WHERE ((wo_id=wooper_wo_id) "
             "AND (wo_status <> 'C')) "
             "LIMIT 1;");
    if (q.first())
    {
      _routings->setCheckable(FALSE);
      _routings->setTitle("Work Center Routings");
    } 
    else
      _routings->setChecked(_metrics->boolean("Routings"));
      
    q.exec("SELECT item_id FROM item WHERE (item_type IN ('B','C','Y')) LIMIT 1;");
    if (q.first())
    {
      _bbom->setChecked(TRUE);
      _bbom->setEnabled(FALSE);
    }
    else
      _bbom->setChecked(_metrics->boolean("BBOM"));
  }
  
  
  if ( (_metrics->value("Application") != "OpenMFG")
    && (_metrics->value("Application") != "xTupleERP") )
  {
    _revControl->hide();
    _transforms->hide();
  }
  else
  {
    q.exec("SELECT * FROM itemtrans LIMIT 1;");
    if (q.first())
    {
      _transforms->setChecked(TRUE);
      _transforms->setEnabled(FALSE);
    }
    else 
      _transforms->setChecked(_metrics->boolean("Transforms"));

    q.exec("SELECT * FROM rev LIMIT 1;");
    if (q.first())
    {
      _revControl->setChecked(TRUE);
      _revControl->setEnabled(FALSE);
    }
    else 
      _revControl->setChecked(_metrics->boolean("RevControl"));
  }
  
  this->setCaption("Products Configuration");

  resize(minimumSize());
}

/*
 *  Destroys the object and frees any allocated resources
 */
configurePD::~configurePD()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configurePD::languageChange()
{
  retranslateUi(this);
}

void configurePD::sSave()
{
  if (!_metrics->boolean("RevControl") && (_revControl->isChecked()))
  {
    if (QMessageBox::warning(this, tr("Enable Revision Control"),
	  tr("Enabling revision control will create control records "
	     "for products that contain revision number data.  This "
		 "change can not be undone.  Do you wish to proceed?"),
	    QMessageBox::Yes | QMessageBox::Default,
	    QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
	{
      _metrics->set("RevControl", TRUE);
	  q.exec("SELECT createbomrev(bomhead_item_id,bomhead_revision) AS result "
		      "FROM bomhead "
		      "WHERE ((COALESCE(bomhead_revision,'') <> '') "
			  "AND (bomhead_rev_id=-1))"
			  "UNION "
              "SELECT createboorev(boohead_item_id,boohead_revision) "
		      "FROM boohead "
			  "WHERE ((COALESCE(boohead_revision,'') <> '') "
			  "AND (boohead_rev_id=-1));");
	  if (q.first())
	    if (q.value("result").toInt() < 0)
	    {
	      systemError(this, storedProcErrorLookup("CreateRevision", q.value("result").toInt()),
			  __FILE__, __LINE__);
         _metrics->set("RevControl", FALSE);
	      return;
	    }
	  if (q.lastError().type() != QSqlError::None)
	  {
	    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
	      .arg(__FILE__)
		  .arg(__LINE__),
		  q.lastError().databaseText());
		_metrics->set("RevControl", FALSE);
	    return;
	  }
	}
	else
	  return;
  }

  _metrics->set("TrackMachineOverhead", ((_machineOverhead->isChecked()) ? QString("M") : QString("G")));
  _metrics->set("Routings", ((_routings->isChecked()) || (!_routings->isCheckable())));
  _metrics->set("BBOM", ((_bbom->isChecked()) && (!_bbom->isHidden())));
  _metrics->set("Transforms", ((_transforms->isChecked()) && (!_transforms->isHidden())));
  _metrics->set("RevControl", ((_revControl->isChecked()) && (!_revControl->isHidden())));
  _metrics->set("AllowInactiveBomItems", _inactiveBomItems->isChecked());
  _metrics->set("DefaultSoldItemsExclusive", _exclusive->isChecked());
  _metrics->set("ItemChangeLog", _changeLog->isChecked());
  
  if (_issueMethod->currentItem() == 0)
    _metrics->set("DefaultWomatlIssueMethod", QString("S"));
  else if (_issueMethod->currentItem() == 1)
    _metrics->set("DefaultWomatlIssueMethod", QString("L"));
  else if (_issueMethod->currentItem() == 2)
    _metrics->set("DefaultWomatlIssueMethod", QString("M"));

  _metrics->load();
  _privileges->load();
  omfgThis->saveToolbarPositions();
  _preferences->load();
  omfgThis->initMenuBar();

  accept();
}
