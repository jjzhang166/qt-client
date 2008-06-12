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

#include "scrapWoMaterialFromWIP.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <QSqlError>
#include <qvalidator.h>
#include "inputManager.h"
#include "distributeInventory.h"
#include "returnWoMaterialItem.h"

/*
 *  Constructs a scrapWoMaterialFromWIP as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
scrapWoMaterialFromWIP::scrapWoMaterialFromWIP(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
    connect(_scrap, SIGNAL(clicked()), this, SLOT(sScrap()));
    connect(_scrapComponent, SIGNAL(toggled(bool)), _qtyScrappedFromWIPLit, SLOT(setEnabled(bool)));
    connect(_scrapComponent, SIGNAL(toggled(bool)), _qty, SLOT(setEnabled(bool)));
    connect(_scrapComponent, SIGNAL(toggled(bool)), _womatl, SLOT(setEnabled(bool)));
    connect(_scrapComponent, SIGNAL(toggled(bool)), _qtyScrappedFromWIPLit, SLOT(setEnabled(bool)));
    connect(_scrapComponent, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
    connect(_scrapComponent, SIGNAL(toggled(bool)), _qtyLit, SLOT(setEnabled(bool)));
    connect(_scrapTopLevel, SIGNAL(toggled(bool)), _topLevelQtyLit, SLOT(setEnabled(bool)));
    connect(_scrapTopLevel, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
    connect(_scrapTopLevel, SIGNAL(toggled(bool)), _topLevelQty, SLOT(setEnabled(bool)));
    connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
    connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
    connect(_wo, SIGNAL(newId(int)), _womatl, SLOT(setWoid(int)));
    connect(_wo, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
    connect(_womatl, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
    connect(_womatl, SIGNAL(valid(bool)), _scrap, SLOT(setEnabled(bool)));
    connect(_womatl, SIGNAL(newQtyScrappedFromWIP(const QString&)), _qtyScrappedFromWIP, SLOT(setText(const QString&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
scrapWoMaterialFromWIP::~scrapWoMaterialFromWIP()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void scrapWoMaterialFromWIP::languageChange()
{
    retranslateUi(this);
}


void scrapWoMaterialFromWIP::init()
{
  _captive = FALSE;
  _fromWOTC = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  //_womatl->setType(WomatlCluster::Push);
  _qty->setValidator(omfgThis->qtyVal());
  _topLevelQty->setValidator(omfgThis->qtyVal());
}

enum SetResponse scrapWoMaterialFromWIP::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _womatl->setId(param.toInt());
    _qty->setFocus();
  }

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  param = pParams.value("allowTopLevel", &valid);
  if (valid)
    _scrapTopLevel->setEnabled(param.toBool());

  param = pParams.value("fromWOTC", &valid);
  if (valid)
    _fromWOTC = TRUE;

  param = pParams.value("wooper_id", &valid);
  if (valid)
    _womatl->setWooperid(param.toInt());

  return NoError;
}

void scrapWoMaterialFromWIP::sScrap()
{
  if (_scrapComponent->isChecked() && _qty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the selected W/O Material to Scrap." ) );
    _qty->setFocus();
    return;
  }
  else if (_scrapTopLevel->isChecked() && _topLevelQty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the W/O Top Level Item to Scrap." ) );
    _topLevelQty->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  if (_scrapComponent->isChecked())
  {
    q.prepare("SELECT scrapWoMaterial(:womatl_id, :qty, :issueRepl) AS result;");
    q.bindValue(":womatl_id", _womatl->id());
    q.bindValue(":qty", _qty->toDouble());
  }
  else if (_scrapTopLevel->isChecked())
  {
    q.prepare("SELECT scrapTopLevel(:wo_id, :qty, :issueRepl) AS result;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":qty",   _topLevelQty->toDouble());
  }
  //q.bindValue(":issueRepl", QVariant(_fromWOTC, 0));
  q.bindValue(":issueRepl", QVariant(false, 0));

  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred scrapping material for "
			    "Work Order ID #%1, Error #%2.")
			   .arg(_wo->id())
			   .arg(q.value("result").toInt()),
		   __FILE__, __LINE__);
       return;
    }
    else
    {
      // scrapWoMaterial() returns womatlid, not itemlocSeries
      if (_scrapTopLevel->isChecked())
      {
        if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Scrap Work Order Material"), tr("Transaction Canceled") );
          return;
        }
      }

      q.exec("COMMIT;");

      if (_captive)
        accept();
      else
      {
        _qty->clear();
        _womatl->setId(-1);
        _womatl->setFocus();
      }
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred scrapping material for "
			  "Work Order ID #%1\n\n%2")
			  .arg(_wo->id())
			  .arg(q.lastError().databaseText()),
		 __FILE__, __LINE__ );
    return;
  }
}

void scrapWoMaterialFromWIP::sHandleButtons()
{
  _scrap->setEnabled(_wo->isValid() && (
                     (_scrapTopLevel->isChecked() && _topLevelQty->toDouble()) ||
		     (_scrapComponent->isChecked() && _qty->toDouble() &&
		      _womatl->isValid()) ));
}

