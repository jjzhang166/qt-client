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


#include "fixSerial.h"

#include <QApplication>
#include <QCursor>
#include <QSqlError>

#include <metasql.h>

fixSerial::fixSerial(QWidget* parent, Qt::WindowFlags fl)
    : XMainWindow(parent, fl)
{
  setupUi(this);

  connect(_fix,		SIGNAL(clicked()),	this, SLOT(sFix()));
  connect(_fixAll,	SIGNAL(clicked()), 	this, SLOT(sFixAll()));
  connect(_query,	SIGNAL(clicked()),	this, SLOT(sFillList()));
  connect(_serial,	SIGNAL(valid(bool)),	this, SLOT(sHandleSerial()));
  connect(_serial,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
			    this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_showProblems, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _serial->addColumn(tr("Table Name"),		-1,	Qt::AlignLeft);
  _serial->addColumn(tr("Column Name"),		-1,	Qt::AlignLeft);
  _serial->addColumn(tr("Sequence Name"),	-1,	Qt::AlignLeft);
  _serial->addColumn(tr("Largest Key Used"),	-1,	Qt::AlignRight);
  _serial->addColumn(tr("Next Key"),		-1,	Qt::AlignRight);
}

fixSerial::~fixSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void fixSerial::languageChange()
{
  retranslateUi(this);
}

void fixSerial::sFillList()
{
  _serial->clear();

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  QString sql = "SELECT relname, attname,"
		"	TRIM(quote_literal('\"''') FROM"
		"	  SUBSTRING(pg_catalog.pg_get_expr(d.adbin, d.adrelid)"
		"	  FROM '[' || quote_literal('\"''') || "
		"               '].*[' || quote_literal('\"''') || ' ]')) AS seq"
		"  FROM pg_catalog.pg_attribute a, pg_catalog.pg_class,"
		"       pg_catalog.pg_attrdef d"
		"  WHERE a.attnum > 0"
		"    AND NOT a.attisdropped"
		"    AND a.attnotnull"
		"    AND a.attrelid = pg_class.oid"
		"    AND d.adrelid = a.attrelid"
		"    AND d.adnum = a.attnum"
		"    AND pg_catalog.pg_get_expr(d.adbin, d.adrelid) ~* 'nextval'"
		"    AND a.atthasdef "
		"ORDER BY relname;" ;

  XSqlQuery relq;
  relq.prepare(sql);

  QString maxStr = "SELECT MAX(<? literal(\"attname\" ?>) AS maxval "
		   "FROM <? literal(\"relname\") ?>;" ;
  XSqlQuery maxq;

  QString seqStr = "SELECT last_value AS currval FROM <? literal(\"seq\") ?>;" ;
  XSqlQuery seqq;

  XTreeWidgetItem *last = 0;
  int rows	= 0;
  int maxval	= 0;
  int currval	= 0;
  int errors	= 0;

  relq.exec();
  while (relq.next())
  {
    ParameterList params;
    params.append("attname",	relq.value("attname").toString());
    params.append("relname",	relq.value("relname").toString());
    params.append("seq",	relq.value("seq").toString());

    MetaSQLQuery maxMql = MetaSQLQuery(maxStr);
    maxq = maxMql.toQuery(params);
    if (maxq.first())
      maxval = maxq.value("maxval").toInt();
    else if (maxq.lastError().type() != QSqlError::None)
    {
      systemError(this, maxq.lastError().databaseText(), __FILE__, __LINE__);
      continue;
    }

    MetaSQLQuery seqMql = MetaSQLQuery(seqStr);
    seqq = seqMql.toQuery(params);
    if (seqq.first())
      currval = seqq.value("currval").toInt();
    else if (seqq.lastError().type() != QSqlError::None)
    {
      systemError(this, seqq.lastError().databaseText(), __FILE__, __LINE__);
      continue;
    }

    rows++;

    if (maxval > currval)
      errors++;

    if ((_showProblems->isChecked() && maxval > currval) ||
	! _showProblems->isChecked())
    {
      last = new XTreeWidgetItem(_serial, last, rows, maxval > currval ? 1 : 0,
				 relq.value("relname"),
				 relq.value("attname"),
				 relq.value("seq"),
				 maxval,
				 currval);

      if (maxval > currval)
	last->setTextColor("red");
    }
  }

  QApplication::restoreOverrideCursor();
  if (relq.lastError().type() != QSqlError::None)
  {
    systemError(this, relq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (errors > 0)
    _statusLit->setText(QObject::tr("Found %1 tables with mismatched serial values.")
			.arg(errors));
  else
    _statusLit->setText(QObject::tr("No problems found"));

  _fixAll->setEnabled(errors > 0);
}

void fixSerial::sHandleSerial()
{
  _fix->setEnabled(_serial->altId() > 0);
}

bool fixSerial::fixOne(XTreeWidgetItem *pItem)
{
  q.prepare("SELECT SETVAL(:sequence, :value);");
  q.bindValue(":sequence",	pItem->text(2));
  q.bindValue(":value",		pItem->text(3));

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  return true;
}

void fixSerial::sFix()
{
  if (fixOne(static_cast<XTreeWidgetItem*>(_serial->currentItem())))
    sFillList();
}

void fixSerial::sFixAll()
{
  QList<QTreeWidgetItem*> all = _serial->findItems("", Qt::MatchContains);
  for (int i = 0; i < all.size(); i++)
  {
    XTreeWidgetItem *currItem = static_cast<XTreeWidgetItem*>(all[i]);
    if (currItem->altId() > 0)
      fixOne(currItem);
  }
  sFillList();
}

void fixSerial::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Fix"), this, SLOT(sFix()));
  pMenu->setItemEnabled(menuItem,
		        (static_cast<XTreeWidgetItem*>(pItem))->altId() > 0 &&
			 _privileges->check("FixSerial"));

  menuItem = pMenu->insertItem(tr("Fix All"), this, SLOT(sFixAll()));
  pMenu->setItemEnabled(menuItem, _privileges->check("FixSerial"));
}
