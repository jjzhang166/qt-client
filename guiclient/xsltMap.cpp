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

#include "xsltMap.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"

bool xsltMap::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

int xsltMap::exec()
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

xsltMap::xsltMap(QWidget* parent, Qt::WFlags fl)
    : XDialog(parent, fl)
{
  setupUi(this);

  connect(_export, SIGNAL(textChanged(const QString &)), this, SLOT(sHandleExport()));
  connect(_import, SIGNAL(textChanged(const QString &)), this, SLOT(sHandleImport()));
  connect(_save,   SIGNAL(clicked()),     this, SLOT(sSave()));

  QString filter = tr("XSLT Files (*.xsl *.xslt)");
  _import->setFilter(filter);
  _export->setFilter(filter);

  _mode		= cNew;
  _xsltmapId	= -1;
}

xsltMap::~xsltMap()
{
  // no need to delete child widgets, Qt does it all for us
}

void xsltMap::languageChange()
{
  retranslateUi(this);
}

enum SetResponse xsltMap::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("xsltmap_id", &valid);
  if (valid)
  {
    _xsltmapId = param.toInt();
    sPopulate();
  }

  param = pParams.value("defaultLinuxDir", &valid);
  if (valid)
    _xsltLinuxDir = param.toString();

  param = pParams.value("defaultMacDir", &valid);
  if (valid)
    _xsltMacDir = param.toString();

  param = pParams.value("defaultWindowsDir", &valid);
  if (valid)
    _xsltWindowsDir = param.toString();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _doctype->setEnabled(false);
      _system->setEnabled(false);
      _import->setEnabled(false);
      _export->setEnabled(false);

      _save->hide();

      _cancel->setFocus();
    }
  }

  return NoError;
}

void xsltMap::sSave()
{
  QString errorCaption = tr("Cannot Save XSLT Map");

  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _name->text().stripWhiteSpace().isEmpty(),
      tr("<p>You must enter a name for this XSLT Map before saving it."),
      _name
    },
    { _doctype->text().stripWhiteSpace().isEmpty() &&
      _system->text().stripWhiteSpace().isEmpty(),
      tr("<p>You must enter a Document Type, a System Identifier, or both "
	 "before saving this XSLT Map."),
      _doctype->text().stripWhiteSpace().isEmpty() ? _doctype : _system
    },
    { _import->text().stripWhiteSpace().isEmpty(),
      tr("<p>You must enter an Import XSLT File Name before saving this XSLT Map."),
      _import
    },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, errorCaption,
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }

  q.prepare( "SELECT xsltmap_name "
	     "FROM xsltmap "
	     "WHERE ((xsltmap_name=:name)"
	     " AND   (xsltmap_id<>:xsltmap_id) );" );

  q.bindValue(":xsltmap_id",	_xsltmapId);
  q.bindValue(":name",		_name->text());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(this, errorCaption,
			  tr("<p>This Name is already in use by another "
			     "XSLT Map."));
    _name->setFocus();
    return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (cNew == _mode)
  {
    q.exec("SELECT NEXTVAL('xsltmap_xsltmap_id_seq') AS result;");
    if (q.first())
      _xsltmapId = q.value("result").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    q.prepare("INSERT INTO xsltmap ("
	      "    xsltmap_id, xsltmap_name, xsltmap_doctype,"
	      "    xsltmap_system, xsltmap_import, xsltmap_export"
	      ") VALUES ("
	      "    :id, :name, :doctype,"
	      "    :system, :import, :export"
	      ");");
  }
  else if (cEdit == _mode)
    q.prepare("UPDATE xsltmap SET"
	      "    xsltmap_name=:name,"
	      "    xsltmap_doctype=:doctype,"
	      "    xsltmap_system=:system,"
	      "    xsltmap_import=:import,"
	      "    xsltmap_export=:export "
	      "WHERE (xsltmap_id=:id);");

  q.bindValue(":id",      _xsltmapId);
  q.bindValue(":name",    _name->text());
  q.bindValue(":doctype", _doctype->text());
  q.bindValue(":system",  _system->text());
  q.bindValue(":import",  _import->text());
  q.bindValue(":export",  _export->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void xsltMap::sPopulate()
{
  if (_xsltmapId <= 0)
  {
    _name->clear();
    _doctype->clear();
    _system->clear();
    _import->clear();
    _export->clear();
  }
  else
  {
    q.prepare("SELECT * FROM xsltmap WHERE (xsltmap_id=:id);");
    q.bindValue(":id", _xsltmapId);
    q.exec();
    if (q.first())
    {
      _name->setText(q.value("xsltmap_name").toString());
      _doctype->setText(q.value("xsltmap_doctype").toString());
      _system->setText(q.value("xsltmap_system").toString());
      _import->setText(q.value("xsltmap_import").toString());
      _export->setText(q.value("xsltmap_export").toString());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void xsltMap::sHandleExport()
{
  if (!_xsltLinuxDir.isEmpty() && _export->text().startsWith(_xsltLinuxDir))
    _export->setText(_export->text().remove(0, _xsltLinuxDir.length() + 1));
  else if (!_xsltMacDir.isEmpty() && _export->text().startsWith(_xsltMacDir))
    _export->setText(_export->text().remove(0, _xsltMacDir.length() + 1));
  else if (!_xsltWindowsDir.isEmpty() && _export->text().startsWith(_xsltWindowsDir))
    _export->setText(_export->text().remove(0, _xsltWindowsDir.length() + 1));
}

void xsltMap::sHandleImport()
{
  if (!_xsltLinuxDir.isEmpty() && _import->text().startsWith(_xsltLinuxDir))
    _import->setText(_import->text().remove(0, _xsltLinuxDir.length() + 1));
  else if (!_xsltMacDir.isEmpty() && _import->text().startsWith(_xsltMacDir))
    _import->setText(_import->text().remove(0, _xsltMacDir.length() + 1));
  else if (!_xsltWindowsDir.isEmpty() && _import->text().startsWith(_xsltWindowsDir))
    _import->setText(_import->text().remove(0, _xsltWindowsDir.length() + 1));
}
