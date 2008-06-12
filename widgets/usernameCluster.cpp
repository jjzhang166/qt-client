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

// usernameCluster.cpp
// Copyright (c) 2002-2008, OpenMFG, LLC

#include <QPushButton>
#include <QLabel>
#include <QValidator>
#include <QHBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "usernameList.h"
#include "usernamecluster.h"

UsernameLineEdit::UsernameLineEdit(QWidget * parent, const char * name)
  : XLineEdit(parent, name)
{
  _id = -1;
  _type = UsersAll;

  setMaximumWidth(100);

  connect(this, SIGNAL(lostFocus()), SLOT(sParse()));
}

void UsernameLineEdit::setId(int pId)
{
  QString sql("SELECT usr_username"
              "  FROM usr"
              " WHERE ((usr_id=:id)");

  if(UsersActive == _type)
    sql += " AND (usr_active)";
  else if(UsersInactive == _type)
    sql += " AND (NOT usr_active)";

  sql += " );";

  XSqlQuery query;
  query.prepare(sql);
  query.bindValue(":id", pId);
  query.exec();
  if(query.first())
  {
    _id = pId;
    _valid = true;
    _username = query.value("usr_username").toString();
  }
  else
  {
    _id = -1;
    _valid = false;
    _username = "";
  }

  setText(_username);
  emit newId(_id);
  emit valid(_valid);

  _parsed = true;
}

void UsernameLineEdit::setUsername(const QString & pUsername)
{
  XSqlQuery query;
  query.prepare("SELECT usr_id"
                "  FROM usr"
                " WHERE (usr_username=:username);");
  query.bindValue(":username", pUsername);
  query.exec();
  if(query.first())
    setId(query.value("usr_id").toInt());
  else
    setId(-1);
}

void UsernameLineEdit::clear()
{
  setId(-1);
}

void UsernameLineEdit::sParse()
{
  if(!_parsed)
  {
    _parsed = true;
    setUsername(text());
  }
}

int UsernameLineEdit::id()
{
  if(hasFocus())
    sParse();
  return _id;
}

const QString & UsernameLineEdit::username()
{
  if(hasFocus())
    sParse();
  return _username;
}

UsernameCluster::UsernameCluster(QWidget * parent, const char * name)
  : QWidget(parent, name)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setMargin(0);
  layout->setSpacing(6);

  _label = new QLabel(tr("Username:"), this);
  _label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  layout->addWidget(_label);

  _username = new UsernameLineEdit(this);
  layout->addWidget(_username);

  _list = new QPushButton(tr("..."), this);
  _list->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#else
  _list->setMinimumWidth(60);
  _list->setMinimumHeight(32);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  layout->addWidget(_list);

  QSpacerItem * spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
  layout->addItem(spacer);

  setLayout(layout);

  connect(_list, SIGNAL(clicked()), this, SLOT(sList()));
  connect(_username, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_username, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  setFocusProxy(_username);
}

void UsernameCluster::sList()
{
  ParameterList params;
  params.append("id", _username->id());
  params.append("type", (int)_username->type());

  usernameList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id;
  if((id = newdlg.exec()) != QDialog::Rejected)
    _username->setId(id);
}

void UsernameCluster::setReadOnly(bool pReadOnly)
{
  if(pReadOnly)
    _list->hide();
  else
    _list->show();
  _username->setEnabled(!pReadOnly);
}

QString UsernameCluster::label() const
{
  return _label->text();
}

void UsernameCluster::setLabel(QString t)
{
  _label->setText(t);
}

void UsernameCluster::setUsername(const QString & pUsername)
{
  _username->setUsername(pUsername);
}

