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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "postProduction.h"

#include <QMessageBox>
#include <QValidator>
#include <QVariant>

#include "inputManager.h"
#include "closeWo.h"
#include "distributeInventory.h"
#include "distributeBreederProduction.h"
#include "scrapWoMaterialFromWIP.h"

/*
 *  Constructs a postProduction as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postProduction::postProduction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_wo, SIGNAL(qtyOrderedChanged(const QString&)), _qtyOrdered, SLOT(setText(const QString&)));
    connect(_wo, SIGNAL(qtyReceivedChanged(const QString&)), _qtyReceived, SLOT(setText(const QString&)));
    connect(_wo, SIGNAL(qtyBalanceChanged(const QString&)), _qtyBalance, SLOT(setText(const QString&)));
    connect(_wo, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_backflush, SIGNAL(toggled(bool)), _nonPickItems, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_scrap, SIGNAL(clicked()), this, SLOT(sScrap()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sHandleWoid(int)));
    connect(_wo, SIGNAL(newItemid(int)), _transferWarehouse, SLOT(findItemsites(int)));
    connect(_immediateTransfer, SIGNAL(toggled(bool)), _transferWarehouse, SLOT(setEnabled(bool)));
    connect(_backflushOperations, SIGNAL(toggled(bool)), this, SLOT(sBackflushOperationsToggled(bool)));

    _captive = false;

    _wo->setType(cWoExploded | cWoReleased | cWoIssued);

    omfgThis->inputManager()->notify(cBCWorkOrder, this, this, SLOT(sCatchWoid(int)));

    _closeWo->setEnabled(_privleges->check("CloseWorkOrders"));
    _nonPickItems->setEnabled(_privleges->check("ChangeNonPickItems"));

    _qty->setValidator(omfgThis->qtyVal());
    _fromWOTC = false;
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _immediateTransfer->hide();
      _transferWarehouse->hide();
    }
    
    if (!_metrics->boolean("Routings"))
    {
      _backflushOperations->setChecked(FALSE);
      _backflushOperations->hide();
      _setupUser->hide();
      _runUser->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
postProduction::~postProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse postProduction::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
    _qty->setFocus();
  }

  param = pParams.value("usr_id", &valid);
  if (valid)
  {
    _setupUser->setId(param.toInt());
    _runUser->setId(param.toInt());
  }

  param = pParams.value("backflush", &valid);
  if (valid)
    _backflush->setChecked(param.toBool());

  param = pParams.value("fromWOTC", &valid);
  if (valid)
  {
    _scrap->setHidden(param.toBool());
    _fromWOTC = true;
  }

  return NoError;
}

void postProduction::sHandleWoid(int pWoid)
{
  q.prepare( "SELECT womatl_issuemethod "
             "FROM womatl "
             "WHERE (womatl_wo_id=:womatl_wo_id);" );
  q.bindValue(":womatl_wo_id", pWoid);
  q.exec();
  if (q.first())
  {
    if (q.findFirst("womatl_issuemethod", "L") != -1)
    {
      _backflush->setEnabled(FALSE);
      _backflush->setChecked(TRUE);
    }
    else if (q.findFirst("womatl_issuemethod", "M") != -1)
    {
      _backflush->setEnabled(TRUE);
      _backflush->setChecked(TRUE);
    }
    else
    {
      _backflush->setEnabled(FALSE);
      _backflush->setChecked(FALSE);
    }
  }

  if (_metrics->boolean("Routings"))
  {
    q.prepare( "SELECT wooper_id "
             "FROM wooper "
             "WHERE (wooper_wo_id=:wo_id);" );
    q.bindValue(":wo_id", pWoid);
    q.exec();
    _backflushOperations->setEnabled(q.first());
    _backflushOperations->setChecked(q.first());
  }
}

void postProduction::sReadWorkOrder(int pWoid)
{
  _wo->setId(pWoid);
}

void postProduction::sScrap()
{
    ParameterList params;
    params.append("wo_id", _wo->id());

    scrapWoMaterialFromWIP newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
}

void postProduction::sPost()
{
  if (_qty->toDouble() == 0.0 && _fromWOTC)
  {
    if (QMessageBox::question(this, tr("Zero Quantity To Post"),
			    tr("Is the Quantity of Production really 0?\n"
			       "If so, you will be clocked out but nothing "
			       "else will be posted."),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    {
      _qty->setFocus();
      return;
    }
    accept();	// nothing to post, but accept() to let woTimeClock clock out the user
  }
  else if (_qty->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter Quantity to Post"),
                           tr("You must enter a quantity of production to Post.") );
    _qty->setFocus();
    return;
  }
  else
  {
    q.prepare("SELECT postProduction(:wo_id, :qty, :backflushMaterials, :backflushOperations, 0, :suUser, :rnUser) AS result;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":qty", _qty->toDouble());
    q.bindValue(":backflushMaterials", QVariant(_backflush->isChecked(), 0));
    q.bindValue(":backflushOperations", QVariant(_backflushOperations->isChecked(), 0));
    q.bindValue(":suUser", _setupUser->username());
    q.bindValue(":rnUser", _runUser->username());
    q.exec();
    if (q.first())
    {
      int itemlocSeries = q.value("result").toInt();

      omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);

      if (itemlocSeries < 0)
      {
	systemError( this, tr("A System Error occurred at postProduction::%1, Work Order ID #%2, Error #%3.")
			   .arg(__LINE__)
			   .arg(_wo->id())
			   .arg(itemlocSeries) );
	return;
      }
      else
      {
	if (_productionNotes->text().stripWhiteSpace().length())
	{
	  q.prepare( "UPDATE wo "
		     "SET wo_prodnotes=(wo_prodnotes || :productionNotes || '\n') "
		     "WHERE (wo_id=:wo_id);" );
	  q.bindValue(":productionNotes", _productionNotes->text().stripWhiteSpace());
	  q.bindValue(":wo_id", _wo->id());
	  q.exec();
	}

	distributeInventory::SeriesAdjust(itemlocSeries, this);

	if (_immediateTransfer->isChecked())
	{
	  q.prepare( "SELECT itemsite_warehous_id "
		     "FROM itemsite, wo "
		     "WHERE ( (wo_itemsite_id=itemsite_id)"
		     " AND (wo_id=:wo_id) );" );
	  q.bindValue(":wo_id", _wo->id());
	  q.exec();
	  if (q.first())
	  {
	    if (q.value("itemsite_warehous_id").toInt() == _transferWarehouse->id())
	      QMessageBox::warning( this, tr("Cannot Post Immediate Transfer"),
				    tr( "OpenMFG cannot post an immediate transfer for the newly posted production as the\n"
					"transfer Warehouse is the same as the production Warehouse.  You must manually\n"
					"transfer the production to the intended Warehouse." ) );
	    else
	    {
	      q.prepare( "SELECT interWarehouseTransfer( itemsite_item_id, itemsite_warehous_id, :to_warehous_id, :qty,"
			 "                               'W', formatWoNumber(wo_id), 'Immediate Transfer from Production Posting' ) AS result "
			 "FROM wo, itemsite "
			 "WHERE ( (wo_itemsite_id=itemsite_id)"
			 " AND (wo_id=:wo_id) );" );
	      q.bindValue(":wo_id", _wo->id());
	      q.bindValue(":to_warehous_id", _transferWarehouse->id());
	      q.bindValue(":qty", _qty->toDouble());
	      q.exec();
	      if (q.first())
		distributeInventory::SeriesAdjust(q.value("result").toInt(), this);
	    }
	  }
	  else
	  {
	    systemError( this, tr("A System Error occurred at postProduction::%1, Work Order ID #%2.")
			 .arg(__LINE__)
			 .arg(_wo->id()) );
	    return;
	  }
	}

  // If this is a breeder item then distribute the production
	q.prepare( "SELECT item_type "
		   "FROM wo, itemsite, item "
		   "WHERE ( (wo_itemsite_id=itemsite_id)"
		   " AND (itemsite_item_id=item_id)"
		   " AND (wo_id=:wo_id) );" );
	q.bindValue(":wo_id", _wo->id());
	q.exec();
	if (q.first())
	{
	  if (q.value("item_type").toString() == "B")
	  {
	    ParameterList params;
	    params.append("mode", "new");
	    params.append("wo_id", _wo->id());

	    distributeBreederProduction newdlg(this, "", TRUE);
	    newdlg.set(params);
	    newdlg.exec();
	  }
	}
	else
	  systemError( this, tr("A System Error occurred at postProduction::%1, Work Order ID #%2.")
			     .arg(__LINE__)
			     .arg(_wo->id()) );

	if (_closeWo->isChecked())
	{
	  ParameterList params;
	  params.append("wo_id", _wo->id());

	  closeWo newdlg(this, "", TRUE);
	  newdlg.set(params);
	  newdlg.exec();
	}
      }
    }
    else
      systemError( this, tr("A System Error occurred at postProduction::%1, Work Order ID #%2.")
			 .arg(__LINE__)
			 .arg(_wo->id()) );


    if (_captive)
      accept();
    else
    {
      _wo->setId(-1);
      _qty->clear();
      _productionNotes->clear();
      _immediateTransfer->setChecked(FALSE);
      _closeWo->setChecked(FALSE);
      _close->setText(tr("&Close"));

      _wo->setFocus();
    }
  }
}

void postProduction::sCatchWoid(int pWoid)
{
  _wo->setId(pWoid);
  _qty->setFocus();
}

void postProduction::sBackflushOperationsToggled( bool yes )
{
  _setupUser->setReadOnly(!yes);
  _runUser->setReadOnly(!yes);
}

