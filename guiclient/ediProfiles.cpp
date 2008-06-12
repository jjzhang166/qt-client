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

#include "ediProfiles.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include <openreports.h>
#include "ediProfile.h"

/*
 *  Constructs a ediProfiles as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
ediProfiles::ediProfiles(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_ediprofile, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ediprofile, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ediprofile, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediProfiles::~ediProfiles()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediProfiles::languageChange()
{
    retranslateUi(this);
}


void ediProfiles::init()
{
  statusBar()->hide();
  
  _ediprofile->addColumn(tr("Profile Name"), -1,          Qt::AlignLeft   );

  sFillList();
}

void ediProfiles::sPrint()
{
  orReport report("EDIProfilesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void ediProfiles::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  ediProfile newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfiles::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ediprofile_id", _ediprofile->id());

  ediProfile newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfiles::sDelete()
{
  q.prepare("SELECT deleteEDIProfile(:ediprofile_id) AS RESULT;");
  q.bindValue(":ediprofile_id", _ediprofile->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete EDI Profile"),
          tr("The select EDI Profile cannot be deleted because it is currently\n"
             "being referenced by one or more records.") );
        return;
      default:
        sFillList();
    }
  }
  else
    systemError( this, tr("A System Error occurred at ediProfiles::%1.")
                       .arg(__LINE__) );
}

void ediProfiles::sFillList()
{
  _ediprofile->populate( "SELECT ediprofile_id, ediprofile_name "
                         "  FROM ediprofile "
                         "ORDER BY ediprofile_name" );
}
