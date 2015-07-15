/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureSearchPath.h"

#include <metasql.h>
#include <parameter.h>

#include "errorReporter.h"

class configureSearchPathPrivate {
  public:
    configureSearchPathPrivate()
    {
    }
};

configureSearchPath::configureSearchPath(QWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
  : XAbstractConfigure(parent, fl),
    _private(0)
{
  Q_UNUSED(modal);
  setupUi(this);

  if (name)
    setObjectName(name);

  _excluded->addColumn(tr("Schema"),         -1, Qt::AlignLeft,  true, "nspname");
  _excluded->addColumn(tr("Package"), _ynColumn, Qt::AlignCenter,true, "ispkg");
  _excluded->addColumn(tr("Enabled"), _ynColumn, Qt::AlignCenter,true, "isenabled");

  _included->addColumn(tr("Order"),          -1, Qt::AlignRight, true, "seq");
  _included->addColumn(tr("Schema"),         -1, Qt::AlignLeft,  true, "nspname");
  _included->addColumn(tr("Package"), _ynColumn, Qt::AlignCenter,true, "ispkg");
  _included->addColumn(tr("Enabled"), _ynColumn, Qt::AlignCenter,true, "isenabled");

  connect(_add,         SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_addAll,      SIGNAL(clicked()), this, SLOT(sAddAll()));
  connect(_excluded,  SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_included,  SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_moveDown,    SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_moveUp,      SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_remove,      SIGNAL(clicked()), this, SLOT(sRemove()));
  connect(_removeAll,   SIGNAL(clicked()), this, SLOT(sRemoveAll()));
  connect(_showAll, SIGNAL(toggled(bool)), this, SLOT(sPopulate()));

  _addAll->setEnabled(_privileges->isDba());
  _removeAll->setEnabled(_privileges->isDba());

  sPopulate();
}

configureSearchPath::~configureSearchPath()
{
  if (_private)
  {
    delete _private;
    _private = 0;
  }
}

void configureSearchPath::languageChange()
{
  retranslateUi(this);
}

bool configureSearchPath::sSave()
{
  emit saving();

  XSqlQuery pathq("SELECT buildsearchpath() AS path;");
  if (pathq.first())
    pathq.exec(QString("SET SEARCH_PATH TO %1;")
                  .arg(pathq.value("path").toString()));
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Setting Search Path"),
                           pathq, __FILE__, __LINE__))
    return false;

  return true;
}

void configureSearchPath::sAdd()
{
  XSqlQuery addq;
  addq.prepare("INSERT INTO schemaord (schemaord_name, schemaord_order)"
               " SELECT :name, COALESCE(MAX(schemaord_order), 0) + 1"
               "   FROM schemaord;");
  foreach (XTreeWidgetItem *item, _excluded->selectedItems())
  {
    addq.bindValue(":name", item->text("nspname"));
    addq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Adding to Search Path"),
                             addq, __FILE__, __LINE__))
      return;
  }

  sPopulate();
}

void configureSearchPath::sAddAll()
{
  XSqlQuery addq;
  addq.prepare("INSERT INTO schemaord (schemaord_name, schemaord_order)"
               " SELECT :name, COALESCE(MAX(schemaord_order), 0) + 1"
               "   FROM schemaord;");
  int namecol = _excluded->column("nspname");
  while (XTreeWidgetItem *item =
                dynamic_cast<XTreeWidgetItem*>(_excluded->takeTopLevelItem(0)))
  {
    addq.bindValue(":name", item->text(namecol));
    addq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Adding to Search Path"),
                             addq, __FILE__, __LINE__);
  }

  sPopulate();
}

void configureSearchPath::sHandleButtons()
{
  _add->setEnabled(_excluded->id() >= 0);
  _addAll->setEnabled(_excluded->topLevelItemCount() > 0);
  _moveDown->setEnabled(_included->itemBelow(_included->currentItem()));//exists
  _moveUp->setEnabled(_included->itemAbove(_included->currentItem()));  //exists
  _remove->setEnabled(_included->id() >= 0);
  _removeAll->setEnabled(_included->topLevelItemCount() > 0);
}

void configureSearchPath::sMoveDown()
{
  XSqlQuery moveq;
  moveq.prepare("SELECT moveUpDown(:id, 'public', 'schemaord',"
                "                  'schemaord_order', NULL,"
                "                  NULL, 'DOWN');");
  moveq.bindValue(":id", _included->id());
  moveq.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Changing Order"),
                       moveq, __FILE__, __LINE__);

  sPopulate();
}

void configureSearchPath::sMoveUp()
{
  XSqlQuery moveq;
  moveq.prepare("SELECT moveUpDown(:id, 'public', 'schemaord',"
                "                  'schemaord_order', NULL,"
                "                  NULL, 'UP');");
  moveq.bindValue(":id", _included->id());
  moveq.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Changing Order"),
                       moveq, __FILE__, __LINE__);

  sPopulate();
}

void configureSearchPath::sPopulate()
{
  MetaSQLQuery emql("SELECT pg_namespace.oid, nspname,"
                    "       (pkghead_id IS NOT NULL) AS ispkg,"
                    "       (pkghead IS NULL OR packageisenabled(pkghead_id))"
                    "                        AS isenabled,"
                    "       CASE nspname WHEN 'public' THEN 0"
                    "                    WHEN 'api'    THEN 1"
                    "                    ELSE 2"
                    "       END AS seq"
                    "  FROM pg_namespace"
                    "  LEFT OUTER JOIN pkghead ON (nspname=pkghead_name)"
                    " WHERE (nspname NOT IN (SELECT schemaord_name"
                    "                          FROM schemaord)"
                    " <? if not exists('showall') ?>"
                    "   AND ((nspname IN ('public', 'api'))"
                    "     OR (pkghead_id IS NOT NULL))"
                    " <? endif ?>"
                    " )"
                    " ORDER BY seq, nspname;");
  ParameterList params;
  if (_showAll->isChecked())
    params.append("showall");

  XSqlQuery eqry = emql.toQuery(params);
  _excluded->populate(eqry);
  ErrorReporter::error(QtCriticalMsg, this, tr("Getting Excluded Schemas"),
                       eqry, __FILE__, __LINE__);

  XSqlQuery iqry("SELECT schemaord_id, schemaord_name AS nspname,"
                 "       (pkghead_id IS NOT NULL) AS ispkg,"
                 "       (pkghead IS NULL OR packageisenabled(pkghead_id))"
                 "                        AS isenabled,"
                 "       schemaord_order AS seq"
                 "  FROM schemaord"
                 "  LEFT OUTER JOIN pkghead ON (schemaord_name=pkghead_name)"
                 " ORDER BY seq;");
  _included->populate(iqry);
  ErrorReporter::error(QtCriticalMsg, this, tr("Getting Included Schemas"),
                       iqry, __FILE__, __LINE__);

  XSqlQuery pathq("SELECT buildsearchpath() AS path;");
  if (pathq.first())
    _result->setPlainText(pathq.value("path").toString());
  ErrorReporter::error(QtCriticalMsg, this, tr("Showing New Search Path"),
                       pathq, __FILE__, __LINE__);
}

void configureSearchPath::sRemove()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM schemaord WHERE schemaord_id=:id;");
  foreach (XTreeWidgetItem *item, _included->selectedItems())
  {
    delq.bindValue(":id", item->id());
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Removing Schema"),
                         delq, __FILE__, __LINE__);
  }

  sPopulate();
}

void configureSearchPath::sRemoveAll()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM schemaord WHERE schemaord_id=:id;");
  while (XTreeWidgetItem *item =
                dynamic_cast<XTreeWidgetItem*>(_included->takeTopLevelItem(0)))
  {
    delq.bindValue(":id", item->id());
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Removing Schema"),
                         delq, __FILE__, __LINE__);
  }

  sPopulate();
}
