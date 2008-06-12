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

#include "taxRegistration.h"

#include <QSqlError>
#include <QVariant>

taxRegistration::taxRegistration(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_number,  SIGNAL(textChanged(QString)), this, SLOT(sHandleButtons()));
  connect(_save,    SIGNAL(clicked()),		  this, SLOT(sSave()));
  connect(_taxauth, SIGNAL(newID(int)),		  this, SLOT(sHandleButtons()));

  _taxregid = -1;
  _reltype  = "";
  _relid    = -1;
  _mode     = cNew;
}

taxRegistration::~taxRegistration()
{
  // no need to delete child widget, Qt does it all for us
}

void taxRegistration::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxRegistration::set(const ParameterList pParams)
{
  QVariant param;
  bool	   valid;

  param = pParams.value("taxreg_id", &valid);
  if (valid)
  {
    _taxregid = param.toInt();
    sPopulate();
  }

  param = pParams.value("taxreg_rel_type", &valid);
  if (valid)
    _reltype = param.toString();

  param = pParams.value("taxreg_rel_id", &valid);
  if (valid)
    _relid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _cust->setEnabled(false);
      _vend->setEnabled(false);
      _taxauth->setEnabled(false);
      _number->setEnabled(false);

      _close->setText(tr("Close"));
      _save->hide();
    }
  }

  if (handleReltype() < 0)
    return UndefinedError;

  sHandleButtons();

  return NoError;
}

void taxRegistration::sSave()
{
  if (cNew == _mode)
  {
    q.prepare("INSERT INTO taxreg ("
	      "    taxreg_rel_id, taxreg_rel_type, "
	      "    taxreg_taxauth_id, taxreg_number "
	      " ) VALUES ("
	      "    :taxreg_rel_id, :taxreg_rel_type, "
	      "    :taxreg_taxauth_id, :taxreg_number "
	      " );");
  }
  else
  {
    q.prepare("UPDATE taxreg SET "
	      "    taxreg_rel_id=:taxreg_rel_id, "
	      "    taxreg_rel_type=:taxreg_rel_type, "
	      "    taxreg_taxauth_id=:taxreg_taxauth_id, "
	      "    taxreg_number=:taxreg_number "
	      "WHERE (taxreg_id=:taxreg_id);");
    q.bindValue(":taxreg_id", _taxregid);
  }
  q.bindValue(":taxreg_rel_id", _relid); //_reltype == "C" ? _cust->id() : _vend->id());
  if(!_reltype.isEmpty())
    q.bindValue(":taxreg_rel_type", _reltype);
  q.bindValue(":taxreg_taxauth_id", _taxauth->id());
  q.bindValue(":taxreg_number", _number->text());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  accept();
}

void taxRegistration::sPopulate()
{
  q.prepare("SELECT * FROM taxreg WHERE (taxreg_id=:taxreg_id);");
  q.bindValue(":taxreg_id", _taxregid);
  q.exec();
  if (q.first())
  {
    _taxregid	= q.value("taxreg_id").toInt();
    _reltype	= q.value("taxreg_rel_type").toString();
    _relid	= q.value("taxreg_rel_id").toInt();
    _number->setText(q.value("taxreg_number").toString());
    _taxauth->setId(q.value("taxreg_taxauth_id").toInt());
    if (handleReltype() < 0)
      return;
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void taxRegistration::sHandleButtons()
{
  _save->setEnabled( (! _number->text().isEmpty() && _taxauth->isValid()) );
}

int taxRegistration::handleReltype()
{
  if (_reltype == "C")
  {
    _cust->setId(_relid);
    _cust->setVisible(true);
    _vend->setVisible(false);
  }
  else if (_reltype == "V")
  {
    _vend->setId(_relid);
    _cust->setVisible(false);
    _vend->setVisible(true);
  }
  else
  {
    _cust->setVisible(false);
    _vend->setVisible(false);
  }

  return 0;
}
