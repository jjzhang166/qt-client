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

#include "configureIE.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "xsltMap.h"

bool configureIE::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

int configureIE::exec()
{
  if (userHasPriv())
    return XDialog::exec();
  else
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    return XDialog::Rejected;
  }
}

configureIE::configureIE(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_deleteMap,	SIGNAL(clicked()), this, SLOT(sDeleteMap()));
  connect(_editMap,	SIGNAL(clicked()), this, SLOT(sEditMap()));
  connect(_map, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditMap()));
  connect(_map,	  SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_newMap,	SIGNAL(clicked()), this, SLOT(sNewMap()));
  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));

  _map->addColumn(tr("Name"),			-1, Qt::AlignLeft );
  _map->addColumn(tr("Document Type"),		-1, Qt::AlignLeft );
  _map->addColumn(tr("System Identifier"),	-1, Qt::AlignLeft );
  _map->addColumn(tr("Import XSLT File"),	-1, Qt::AlignLeft );
  _map->addColumn(tr("Export XSLT File"),	-1, Qt::AlignLeft );

  // TODO: fix these when support for an internal XSLT processor is enabled
  _internal->setEnabled(false);
  _internal->setVisible(false);
  _external->setVisible(false);
  _tabs->removePage(_tabs->page(2));
  
  resize(minimumSize());

  sPopulate();
}

configureIE::~configureIE()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureIE::languageChange()
{
  retranslateUi(this);
}

void configureIE::sSave()
{
  if (_renameFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Rename"));
  else if (_deleteFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Delete"));
  else if (_moveFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Move"));
  else if (_doNothing->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("None"));
  else
  {
    QMessageBox::critical(this, tr("Incomplete Data"),
			  tr("<p>Please choose whether to delete or rename "
			     "XML files after they have been successfully "
			     "imported."));
    _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
    _renameFiles->setFocus();
    return;
  }

  if (_internal->isChecked())
    _metrics->set("XSLTLibrary",	  _internal->isChecked());
  else if (_external->isChecked())
  {
    if (_linuxCmd->text().isEmpty() && _macCmd->text().isEmpty() &&
	_windowsCmd->text().isEmpty())
    {
      QMessageBox::critical(this, tr("Incomplete Data"),
			    tr("<p>Please enter the XSLT processor command "
			       "line for at least one platform."));
      _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
      _linuxCmd->setFocus();
      return;
    }
    else
      _metrics->set("XSLTLibrary",	  ! _external->isChecked());
  }
  else
  {
    QMessageBox::critical(this, tr("Incomplete Data"),
			  tr("<p>Please choose whether to use the internal "
			     "XSLT processor or an external XSLT processor."));
    _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
    _internal->setFocus();
    return;
  }

  _metrics->set("XSLTDefaultDirLinux",	  _xsltLinuxDir->text());
  _metrics->set("XSLTDefaultDirMac",	  _xsltMacDir->text());
  _metrics->set("XSLTDefaultDirWindows",  _xsltWindowsDir->text());


  _metrics->set("XSLTProcessorLinux",	  _linuxCmd->text());
  _metrics->set("XSLTProcessorMac",	  _macCmd->text());
  _metrics->set("XSLTProcessorWindows",	  _windowsCmd->text());

  _metrics->set("XMLDefaultDirLinux",	  _linuxDir->text());
  _metrics->set("XMLDefaultDirMac",	  _macDir->text());
  _metrics->set("XMLDefaultDirWindows",	  _windowsDir->text());

  _metrics->set("XMLSuccessSuffix",	  _renameSuffix->text());
  _metrics->set("XMLSuccessDir",	  _moveDir->text());

  accept();
}

void configureIE::sPopulate()
{
  _xsltLinuxDir->setText(_metrics->value("XSLTDefaultDirLinux"));
  _xsltMacDir->setText(_metrics->value("XSLTDefaultDirMac"));
  _xsltWindowsDir->setText(_metrics->value("XSLTDefaultDirWindows"));

  /*  Will implement this if/when Xalan is embedded
  if (! _metrics->value("XSLTLibrary").isEmpty())
  {
    _internal->setChecked(_metrics->boolean("XSLTLibrary"));
    _external->setChecked(! _metrics->boolean("XSLTLibrary"));
  }
  */

  _external->setChecked(TRUE);

  _linuxCmd->setText(_metrics->value("XSLTProcessorLinux"));
  _macCmd->setText(_metrics->value("XSLTProcessorMac"));
  _windowsCmd->setText(_metrics->value("XSLTProcessorWindows"));

  _linuxDir->setText(_metrics->value("XMLDefaultDirLinux"));
  _macDir->setText(_metrics->value("XMLDefaultDirMac"));
  _windowsDir->setText(_metrics->value("XMLDefaultDirWindows"));

  if (_metrics->value("XMLSuccessTreatment") == "Rename")
    _renameFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "Delete")
    _deleteFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "Move")
    _moveFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "None")
    _doNothing->setChecked(true);

  _renameSuffix->setText(_metrics->value("XMLSuccessSuffix"));
  _moveDir->setText(_metrics->value("XMLSuccessDir"));

  sFillList();
}

void configureIE::sFillList()
{
  q.prepare("SELECT * FROM xsltmap ORDER BY xsltmap_name;");
  q.exec();
  _map->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void configureIE::sDeleteMap()
{
  if (QMessageBox::question(this, tr("Delete Map?"),
		    tr("Are you sure you want to delete this XSLT Map?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("DELETE FROM xsltmap WHERE (xsltmap_id=:mapid);");
    q.bindValue(":mapid", _map->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteXSLTMap", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void configureIE::sEditMap()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("xsltmap_id", _map->id());
  params.append("defaultLinuxDir",   _xsltLinuxDir->text());
  params.append("defaultMacDir",     _xsltMacDir->text());
  params.append("defaultWindowsDir", _xsltWindowsDir->text());

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sNewMap()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("defaultLinuxDir",   _xsltLinuxDir->text());
  params.append("defaultMacDir",     _xsltMacDir->text());
  params.append("defaultWindowsDir", _xsltWindowsDir->text());

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("New..."),  this, SLOT(sNewMap()),    0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditMap()),   0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());

  menuItem = pMenu->insertItem(tr("Delete"),  this, SLOT(sDeleteMap()), 0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());
}
