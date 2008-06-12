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

#include "scriptquery.h"

#include <QSqlError>
#include <QScriptEngine>
#include <QRegExp>
#include <QDateTime>

#include "scripttoolbox.h"

ScriptQuery::ScriptQuery(QScriptEngine * engine)
  : QObject(engine)
{
  _engine = engine;
}

ScriptQuery::~ScriptQuery()
{
}

XSqlQuery ScriptQuery::query() const
{
  return _query;
}

void ScriptQuery::setQuery(XSqlQuery query)
{
  _query = query;
}

bool ScriptQuery::isActive()
{
  return _query.isActive();
}

bool ScriptQuery::isValid()
{
  return _query.isValid();
}

bool ScriptQuery::isForwardOnly()
{
  return _query.isForwardOnly();
}

bool ScriptQuery::isSelect()
{
  return _query.isSelect();
}

bool ScriptQuery::first()
{
  return _query.first();
}

bool ScriptQuery::last()
{
  return _query.last();
}

bool ScriptQuery::next()
{
  return _query.next();
}

bool ScriptQuery::previous()
{
  return _query.previous();
}

int ScriptQuery::size()
{
  return _query.size();
}

int ScriptQuery::numRowsAffected()
{
  return _query.numRowsAffected();
}

QScriptValue ScriptQuery::value(int index)
{
  return ScriptToolbox::variantToScriptValue(_engine, _query.value(index));
}

QScriptValue ScriptQuery::value(const QString & field)
{
  return ScriptToolbox::variantToScriptValue(_engine, _query.value(field));
}

QVariantMap ScriptQuery::lastError()
{
  QVariantMap m;
  QSqlError err = _query.lastError();
  m.insert("databaseText", err.databaseText());
  m.insert("driverText", err.driverText());
  m.insert("text", err.text());
  m.insert("number", err.number());
  m.insert("type", err.type());
  m.insert("isValid", QVariant(err.isValid(), 0));

  return m;
}

