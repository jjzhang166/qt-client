/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "todoCalendarControl.h"
#include "guiclient.h"
#include <parameter.h>
#include <QDebug>
#include <metasql.h>

#include "todoListCalendar.h"

todoCalendarControl::todoCalendarControl(todoListCalendar * parent)
  : CalendarControl(parent)
{
  _list = parent;
}

QString todoCalendarControl::contents(const QDate & date)
{
  QString sql = "SELECT sum(count) AS result "
                " FROM ( "
                "  SELECT count(*) "
                "  FROM todoitem() "
                " WHERE((todoitem_due_date = <? value(\"date\") ?>)"
                "  <? if not exists(\"completed\") ?>"
                "   AND (todoitem_status != 'C')"
                "  <? endif ?>"
                "  <? if exists(\"username\") ?> "
                "   AND (todoitem_username=<? value(\"username\") ?>) "
                "  <? elseif exists(\"usr_pattern\") ?>"
                "   AND (todoitem_username ~ <? value(\"usr_pattern\") ?>) "
                "  <? endif ?>"
                "  <? if exists(\"active\") ?>AND (todoitem_active) <? endif ?>"
                "       ) "
                " UNION ALL "
                "  SELECT count(*) "
                "  FROM prjtask() "
                " WHERE((prjtask_due_date = <? value(\"date\") ?>)"
                "  <? if not exists(\"completed\") ?>"
                "   AND (prjtask_status != 'C')"
                "  <? endif ?>"
                "  <? if exists(\"username\") ?> "
                "   AND (prjtask_username=<? value(\"username\") ?>) "
                "  <? elseif exists(\"usr_pattern\") ?>"
                "   AND (prjtask_username ~ <? value(\"usr_pattern\") ?>) "
                "  <? endif ?>"
                "       ) "    
                " ) data;";     

  ParameterList params;
  params.append("date", date);
  if(_list)
    _list->setParams(params);

  MetaSQLQuery mql(sql);
  XSqlQuery qry = mql.toQuery(params);
  if(qry.first())
  {
    if(qry.value(0).toInt() != 0)
      return qry.value(0).toString();
  }
  return QString::null;
}

