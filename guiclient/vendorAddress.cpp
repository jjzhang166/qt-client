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

#include "vendorAddress.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <vendorcluster.h>

vendorAddress::vendorAddress(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
}

vendorAddress::~vendorAddress()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorAddress::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorAddress::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vendid = param.toInt();

  param = pParams.value("vendaddr_id", &valid);
  if (valid)
  {
    _vendaddrid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();

      q.prepare("SELECT vend_taxauth_id"
                "  FROM vendinfo"
                " WHERE (vend_id=:vend_id);");
      q.bindValue(":vend_id", _vendid);
      q.exec();
      if(q.first())
        _taxauth->setId(q.value("vend_taxauth_id").toInt());
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _notes->setReadOnly(TRUE);
      _taxauth->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}


// similar code in address, customer, shipto, vendor, vendorAddress, warehouse
int vendorAddress::saveContact(ContactCluster* pContact)
{
//  pContact->setAccount(_crmacctid);

  int answer = 2;	// Cancel
  int saveResult = pContact->save(AddressCluster::CHECK);

  if (-1 == saveResult)
    systemError(this, tr("There was an error saving a Contact (%1, %2).\n"
			 "Check the database server log for errors.")
		      .arg(pContact->label()).arg(saveResult),
		__FILE__, __LINE__);
  else if (-2 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple Contacts sharing this address (%1).\n"
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %").arg(pContact->label()),
		    tr("Would you like to update the existing Contact or "
		       "create a new one?"),
		    tr("Create New"),
		    tr("Change Existing"),
		    tr("Cancel"),
		    2, 2);
  if (0 == answer)
    return pContact->save(AddressCluster::CHANGEONE);
  else if (1 == answer)
    return pContact->save(AddressCluster::CHANGEALL);

  return saveResult;
}

void vendorAddress::sSave()
{
  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("There are multiple uses of this Vendor "
		       "Address.\nWhat would you like to do?"),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
    if (0 == answer)
      saveResult = _address->save(AddressCluster::CHANGEONE);
    else if (1 == answer)
      saveResult = _address->save(AddressCluster::CHANGEALL);
  }
  if (saveResult < 0)	// not else-if: this is error check for CHANGE{ONE,ALL}
  {
    systemError(this, tr("There was an error saving this address (%1).\n"
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    rollback.exec();
    _address->setFocus();
    return;
  }

  if (saveContact(_contact) < 0)
  {
    rollback.exec();
    _contact->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('vendaddr_vendaddr_id_seq') AS vendaddr_id;");
    if (q.first())
      _vendaddrid = q.value("vendaddr_id").toInt();
    else
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO vendaddrinfo "
               "( vendaddr_id, vendaddr_vend_id,"
               "  vendaddr_code, vendaddr_name,"
               "  vendaddr_cntct_id, vendaddr_comments, vendaddr_addr_id,"
               "  vendaddr_taxauth_id ) "
               "VALUES "
               "( :vendaddr_id, :vendaddr_vend_id,"
               "  :vendaddr_code, :vendaddr_name,"
               "  :vendaddr_cntct_id, :vendaddr_comments, :vendaddr_addr_id,"
               "  :vendaddr_taxauth_id );");
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE vendaddrinfo "
               "SET vendaddr_code=:vendaddr_code, vendaddr_name=:vendaddr_name,"
               "    vendaddr_cntct_id=:vendaddr_cntct_id,"
	       "    vendaddr_comments=:vendaddr_comments, "
	       "    vendaddr_addr_id=:vendaddr_addr_id,"
               "    vendaddr_taxauth_id=:vendaddr_taxauth_id "
               "WHERE (vendaddr_id=:vendaddr_id);" );

  q.bindValue(":vendaddr_id", _vendaddrid);
  q.bindValue(":vendaddr_vend_id", _vendid);
  q.bindValue(":vendaddr_code", _number->text().stripWhiteSpace());
  q.bindValue(":vendaddr_name", _name->text().stripWhiteSpace());
  if (_contact->id() > 0)
    q.bindValue(":vendaddr_cntct_id", _contact->id());
  if (_address->id() > 0)
    q.bindValue(":vendaddr_addr_id", _address->id());
  q.bindValue(":vendaddr_comments", _notes->text().stripWhiteSpace());
  if(_taxauth->isValid())
    q.bindValue(":vendaddr_taxauth_id", _taxauth->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  done(_vendaddrid);
}

void vendorAddress::sCheck()
{
  if ((_mode == cNew) && (_number->text().length()))
  {
    q.prepare( "SELECT vendaddr_id "
               "FROM vendaddr "
               "WHERE ( (vendaddr_vend_id=:vend_id)"
               " AND (UPPER(vendaddr_code)=UPPER(:vendaddr_code)) );" );
    q.bindValue(":vend_id", _vendid);
    q.bindValue(":vendaddr_code", _number->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      _vendaddrid = q.value("vendaddr_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(FALSE);
    }
  }
}

void vendorAddress::populate()
{
  q.prepare( "SELECT * "
             "FROM vendaddrinfo "
             "WHERE (vendaddr_id=:vendaddr_id);" );
  q.bindValue(":vendaddr_id", _vendaddrid);
  q.exec();
  if (q.first())
  {
    _vendid = q.value("vendaddr_vend_id").toInt();
    _number->setText(q.value("vendaddr_code"));
    _name->setText(q.value("vendaddr_name"));
    _contact->setId(q.value("vendaddr_cntct_id").toInt());
    _address->setId(q.value("vendaddr_addr_id").toInt());
    _taxauth->setId(q.value("vendaddr_taxauth_id").toInt());
    _notes->setText(q.value("vendaddr_comments").toString());
  }
}
