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

#include "massExpireComponent.h"

#include <qvariant.h>
#include <QMessageBox>

/*
 *  Constructs a massExpireComponent as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
massExpireComponent::massExpireComponent(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
massExpireComponent::~massExpireComponent()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void massExpireComponent::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlQuery>

void massExpireComponent::init()
{
  _captive = FALSE;

  statusBar()->hide();

  _original->setType(ItemLineEdit::cGeneralComponents);

  _expireAsOf->setNullString(tr("Immediate"));
  _expireAsOf->setNullDate(omfgThis->startOfTime());
  _expireAsOf->setAllowNullDate(TRUE);
  _expireAsOf->setNull();

  _original->setFocus();
}

enum SetResponse massExpireComponent::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _original->setId(param.toInt());

  return NoError;
}

void massExpireComponent::sExpire()
{
  if ( (_original->isValid()) && (_expireAsOf->isValid()) )
  {
    if (_metrics->boolean("RevControl"))
    {
      q.prepare("SELECT * "
	  	      "FROM bomitem, rev "
	 		  "WHERE ( (bomitem_rev_id=rev_id) "
			  "AND (rev_status='P') "
			  "AND (bomitem_item_id=:item_id) ) "
			  "LIMIT 1;");
	  q.bindValue(":item_id", _original->id());
	  q.exec();
	  if (q.first())
        QMessageBox::information( this, tr("Mass Expire"),
                          tr("<p>This process will only affect active revisions. "
						  "Items on pending revisions must be expired manually.")  );
    }
    QSqlQuery expire;

    if (_expireAsOf->isNull())
      expire.prepare("SELECT massExpireBomitem(:item_id, CURRENT_DATE, :ecn);");
    else
      expire.prepare("SELECT massExpireBomitem(:item_id, :expire_date, :ecn);");

    expire.bindValue(":item_id", _original->id());
    expire.bindValue(":expire_date", _expireAsOf->date());
    expire.bindValue(":ecn", _ecn->text());

    expire.exec();

    if (_captive)
      close();
    {
      _close->setText(tr("&Close"));
      _original->setId(-1);
      _ecn->clear();
      _original->setFocus();
    }
  }
}
