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

#include "shipTo.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include "addresscluster.h"

shipTo::shipTo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_shipToNumber, SIGNAL(lostFocus()), this, SLOT(sPopulateNumber()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_salesRep, SIGNAL(newID(int)), this, SLOT(sPopulateCommission(int)));

    _shipZone->populate( "SELECT shipzone_id, shipzone_name "
			 "FROM shipzone "
			 "ORDER BY shipzone_name;" );

    _commission->setValidator(omfgThis->percentVal());

    if (_metrics->boolean("EnableBatchManager"))
    {
      _ediProfile->append(-2, tr("Use Customer Master"));
      _ediProfile->append(-1, tr("No EDI"));
      q.prepare("SELECT ediprofile_id, ediprofile_name"
  	        "  FROM ediprofile, ediform"
	        " WHERE ((ediform_ediprofile_id=ediprofile_id)"
	        "   AND  (ediform_type='invoice')) "
	        "ORDER BY ediprofile_name; ");
      q.exec();
      while(q.next())
        _ediProfile->append(q.value("ediprofile_id").toInt(), q.value("ediprofile_name").toString());
      if (q.lastError().type() != QSqlError::None)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        return;
      }
    }
    else
    {
      _ediProfileLit->hide();
      _ediProfile->hide();
    }
}

shipTo::~shipTo()
{
    // no need to delete child widgets, Qt does it all for us
}

void shipTo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse shipTo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _custid = param.toInt();

  param = pParams.value("shipto_id", &valid);
  if (valid)
  {
    _shiptoid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      XSqlQuery cust;
      cust.prepare( "SELECT cust_number, cust_name, cust_taxauth_id, "
                 "       cust_salesrep_id, cust_shipform_id, cust_shipvia "
                 "FROM custinfo "
                 "WHERE (cust_id=:cust_id);" );
      cust.bindValue(":cust_id", _custid);
      cust.exec();
      if (cust.first())
      {
        _custNum->setText(cust.value("cust_number").toString());
        _custName->setText(cust.value("cust_name").toString());
        _salesRep->setId(cust.value("cust_salesrep_id").toInt());
        _shipform->setId(cust.value("cust_shipform_id").toInt());
        _taxauth->setId(cust.value("cust_taxauth_id").toInt());

	//  Handle the free-form Ship Via
        _shipVia->setId(-1);
        QString shipvia = cust.value("cust_shipvia").toString().stripWhiteSpace();
        if (shipvia.length())
        {
          for (int counter = 0; counter < _shipVia->count(); counter++)
            if (_shipVia->text(counter) == shipvia)
              _shipVia->setCurrentItem(counter);

          if (_shipVia->id() == -1)
          {
            _shipVia->insertItem(shipvia);
            _shipVia->setCurrentItem(_shipVia->count() - 1);
          }
        }
      }
      if (cust.lastError().type() != QSqlError::None)
      {
	systemError(this, cust.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _shipToNumber->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _shipToNumber->setEnabled(FALSE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _shipToNumber->setEnabled(FALSE);
      _active->setEnabled(FALSE);
      _default->setEnabled(FALSE);
      _name->setEnabled(FALSE);
      _contact->setEnabled(FALSE);
      _address->setEnabled(FALSE);
      _salesRep->setEnabled(FALSE);
      _commission->setEnabled(FALSE);
      _shipZone->setEnabled(FALSE);
      _taxauth->setEnabled(FALSE);
      _shipVia->setEnabled(FALSE);
      _shipform->setEnabled(FALSE);
      _shipchrg->setEnabled(FALSE);
      _ediProfile->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _shippingComments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

// similar code in address, customer, shipto, vendor, vendorAddress, warehouse
int shipTo::saveContact(ContactCluster* pContact)
{
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
		    tr("<p>There are multiple Contacts sharing this address (%1). "
		       "What would you like to do?")
		    .arg(pContact->label()),
		    tr("Change This One"),
		    tr("Change Address for All"),
		    tr("Cancel"),
		    2, 2);
  else if (-10 == saveResult)
    answer = QMessageBox::question(this,
		    tr("Question Saving %1").arg(pContact->label()),
		    tr("<p>Would you like to update the existing Contact or "
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

void shipTo::sSave()
{
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  if (! q.exec("BEGIN"))
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (saveContact(_contact) < 0)
  {
    rollback.exec();
    _contact->setFocus();
    return;
  }

  int saveResult = _address->save(AddressCluster::CHECK);
  if (-2 == saveResult)
  {
    int answer = QMessageBox::question(this,
		    tr("Question Saving Address"),
		    tr("<p>There are multiple uses of this Ship-To "
		       "Address. What would you like to do?"),
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
    systemError(this, tr("<p>There was an error saving this address (%1). "
			 "Check the database server log for errors.")
		      .arg(saveResult), __FILE__, __LINE__);
    rollback.exec();
    _address->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('shipto_shipto_id_seq') AS shipto_id;");
    if (q.first())
      _shiptoid = q.value("shipto_id").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      rollback.exec();
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO shiptoinfo "
               "( shipto_id, shipto_cust_id, shipto_active, shipto_default,"
               "  shipto_num, shipto_name, shipto_cntct_id, shipto_commission,"
               "  shipto_comments, shipto_shipcomments,"
               "  shipto_taxauth_id, shipto_salesrep_id, shipto_shipzone_id,"
               "  shipto_shipvia, shipto_shipform_id, shipto_shipchrg_id, "
	       "  shipto_ediprofile_id, shipto_addr_id ) "
               "VALUES "
               "( :shipto_id, :shipto_cust_id, :shipto_active, :shipto_default,"
               "  :shipto_num, :shipto_name, :shipto_cntct_id, :shipto_commission,"
               "  :shipto_comments, :shipto_shipcomments,"
               "  :shipto_taxauth_id, :shipto_salesrep_id, :shipto_shipzone_id,"
               "  :shipto_shipvia, :shipto_shipform_id, :shipto_shipchrg_id, "
	       "  :shipto_ediprofile_id , :shipto_addr_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE shiptoinfo "
               "SET shipto_active=:shipto_active, shipto_default=:shipto_default,"
               "    shipto_name=:shipto_name, shipto_cntct_id=:shipto_cntct_id,"
               "    shipto_commission=:shipto_commission,"
               "    shipto_comments=:shipto_comments, shipto_shipcomments=:shipto_shipcomments,"
               "    shipto_taxauth_id=:shipto_taxauth_id, shipto_salesrep_id=:shipto_salesrep_id, shipto_shipzone_id=:shipto_shipzone_id,"
               "    shipto_shipvia=:shipto_shipvia, shipto_shipform_id=:shipto_shipform_id, shipto_shipchrg_id=:shipto_shipchrg_id,"
               "    shipto_ediprofile_id=:shipto_ediprofile_id, "
	       "    shipto_addr_id=:shipto_addr_id "
               "WHERE (shipto_id=:shipto_id);" );

  q.bindValue(":shipto_id", _shiptoid);
  q.bindValue(":shipto_active", QVariant(_active->isChecked(), 0));
  q.bindValue(":shipto_default", QVariant(_default->isChecked(), 0));
  q.bindValue(":shipto_cust_id", _custid);
  q.bindValue(":shipto_num", _shipToNumber->text().stripWhiteSpace());
  q.bindValue(":shipto_name", _name->text());
  if (_contact->id() > 0)
    q.bindValue(":shipto_cntct_id", _contact->id());
  if (_address->id() > 0)
    q.bindValue(":shipto_addr_id", _address->id());
  q.bindValue(":shipto_commission", (_commission->toDouble() / 100));
  q.bindValue(":shipto_comments", _comments->text());
  q.bindValue(":shipto_shipcomments", _shippingComments->text());
  q.bindValue(":shipto_shipvia", _shipVia->currentText());
  if (_taxauth->isValid())
    q.bindValue(":shipto_taxauth_id",  _taxauth->id());
  q.bindValue(":shipto_salesrep_id", _salesRep->id());
  q.bindValue(":shipto_shipzone_id", _shipZone->id());
  q.bindValue(":shipto_shipform_id", _shipform->id());
  q.bindValue(":shipto_shipchrg_id", _shipchrg->id());
  q.bindValue(":shipto_ediprofile_id", _ediProfile->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");

  done(_shiptoid);
}

void shipTo::populate()
{
  q.prepare( "SELECT cust_number, cust_name, shipto_active, shipto_default,"
             "       shipto_cust_id,"
             "       shipto_num, shipto_name, shipto_cntct_id,"
             "       shipto_shipvia, formatScrap(shipto_commission) AS commission,"
             "       shipto_comments, shipto_shipcomments,"
             "       shipto_salesrep_id, shipto_taxauth_id, shipto_shipzone_id,"
             "       shipto_shipform_id, shipto_shipchrg_id, "
	     "       shipto_ediprofile_id, shipto_addr_id "
             "FROM shiptoinfo LEFT OUTER JOIN cust ON (shipto_cust_id=cust_id) "
             "WHERE (shipto_id=:shipto_id);" );
  q.bindValue(":shipto_id", _shiptoid);
  q.exec();
  if (q.first())
  {
    QString commission = q.value("commission").toString();
    _custid = q.value("shipto_cust_id").toInt();
    _custNum->setText(q.value("cust_number").toString());
    _custName->setText(q.value("cust_name").toString());
    _active->setChecked(q.value("shipto_active").toBool());
    _default->setChecked(q.value("shipto_default").toBool());
    _shipToNumber->setText(q.value("shipto_num"));
    _name->setText(q.value("shipto_name"));
    _contact->setId(q.value("shipto_cntct_id").toInt());
    _comments->setText(q.value("shipto_comments").toString());
    _shippingComments->setText(q.value("shipto_shipcomments").toString());
    _taxauth->setId(q.value("shipto_taxauth_id").toInt());
    _shipZone->setId(q.value("shipto_shipzone_id").toInt());
    _shipform->setId(q.value("shipto_shipform_id").toInt());
    _shipchrg->setId(q.value("shipto_shipchrg_id").toInt());
    _ediProfile->setId(q.value("shipto_ediprofile_id").toInt());
    _address->setId(q.value("shipto_addr_id").toInt());

    //  Handle the free-form Ship Via
    _shipVia->setCurrentItem(-1);
    QString shipvia = q.value("shipto_shipvia").toString();
    if (shipvia.stripWhiteSpace().length() != 0)
    {
      for (int counter = 0; counter < _shipVia->count(); counter++)
        if (_shipVia->text(counter) == shipvia)
          _shipVia->setCurrentItem(counter);

      if (_shipVia->id() == -1)
      {
        _shipVia->insertItem(shipvia);
        _shipVia->setCurrentItem(_shipVia->count() - 1);
      }
    }

    _salesRep->setId(q.value("shipto_salesrep_id").toInt());
    _commission->setText(commission);
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void shipTo::sPopulateNumber()
{
  if (_shipToNumber->text().length() == 0)
  {
    q.prepare( "SELECT (COALESCE(MAX(CAST(shipto_num AS INTEGER)), 0) + 1) AS n_shipto_num "
               "  FROM shipto "
               " WHERE ((shipto_cust_id=:cust_id)"
               "   AND  (shipto_num~'^[0-9]*$') )" );
    q.bindValue(":cust_id", _custid);
    q.exec();
    if (q.first())
      _shipToNumber->setText(q.value("n_shipto_num"));
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else
  {
    q.prepare( "SELECT shipto_id "
               "FROM shipto "
               "WHERE ( (shipto_cust_id=:cust_id)"
               " AND (UPPER(shipto_num)=UPPER(:shipto_num)) );" );
    q.bindValue(":cust_id", _custid);
    q.bindValue(":shipto_num", _shipToNumber->text());
    q.exec();
    if (q.first())
    {
      _mode = cEdit;
      _shiptoid = q.value("shipto_id").toInt();
      populate();

      _shipToNumber->setEnabled(FALSE);
      _name->setFocus();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void shipTo::sPopulateCommission(int pSalesrepid)
{
  if (_mode != cView)
  {
    q.prepare( "SELECT formatScrap(salesrep_commission) AS commission "
               "FROM salesrep "
               "WHERE (salesrep_id=:salesrep_id);" );
    q.bindValue(":salesrep_id", pSalesrepid);
    q.exec();
    if (q.first())
      _commission->setText(q.value("commission"));
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
