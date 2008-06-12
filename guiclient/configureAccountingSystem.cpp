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

#include "configureAccountingSystem.h"

#include <qvariant.h>
#include <q3filedialog.h>
#include "guiclient.h"

/*
 *  Constructs a configureAccountingSystem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureAccountingSystem::configureAccountingSystem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_importPathList, SIGNAL(clicked()), this, SLOT(sImportPathList()));
    connect(_exportPathList, SIGNAL(clicked()), this, SLOT(sExportPathList()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_external, SIGNAL(toggled(bool)), _externalGroup, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureAccountingSystem::~configureAccountingSystem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureAccountingSystem::languageChange()
{
    retranslateUi(this);
}


void configureAccountingSystem::init()
{
#ifndef Q_WS_MAC
  _importPathList->setMaximumWidth(25);
  _exportPathList->setMaximumWidth(25);
#endif

  _accountingSystem->insertItem("PassPort RW2000 v6.7B");
  _accountingSystem->insertItem("RealWorld v9.1");
  _accountingSystem->insertItem("Other");

  if (_metrics->value("AccountingSystem") == "Native")
    _native->setChecked(TRUE);
  else
  {
    _external->setChecked(TRUE);

    if (_metrics->value("AccountingSystem") == "RW2000")
      _accountingSystem->setCurrentItem(0);
    else if (_metrics->value("AccountingSystem") == "RealWorld91")
      _accountingSystem->setCurrentItem(1);
    else if (_metrics->value("AccountingSystem") == "Other")
      _accountingSystem->setCurrentItem(2);

    _importPath->setText(_metrics->value("AccountingSystemImportPath"));
    _exportPath->setText(_metrics->value("AccountingSystemExportPath"));
  }
}

void configureAccountingSystem::sSave()
{
  if (_native->isChecked())
    _metrics->set("AccountingSystem", QString("Native"));
  else
  {
    switch (_accountingSystem->currentItem())
    {
      case 0:
        _metrics->set("AccountingSystem", QString("RW2000"));
        break;
  
      case 1:
        _metrics->set("AccountingSystem", QString("RealWorld91"));
        break;
  
      default:
        _metrics->set("AccountingSystem", QString("Other"));
        break;
    }

    _metrics->set("AccountingSystemImportPath", _importPath->text().stripWhiteSpace());
    _metrics->set("AccountingSystemExportPath", _exportPath->text().stripWhiteSpace());
  }

  _metrics->load();

  accept();
}

void configureAccountingSystem::sImportPathList()
{
  QString path = Q3FileDialog::getExistingDirectory( _importPath->text(), this, "selectImportPath",
                                                    tr("Select Accounting System Import Directory") );
  if (path.length())
    _importPath->setText(path);
}

void configureAccountingSystem::sExportPathList()
{
  QString path = Q3FileDialog::getExistingDirectory( _exportPath->text(), this, "selectExportPath",
                                                    tr("Select Accounting System Export Directory") );
  if (path.length())
    _exportPath->setText(path);
}

