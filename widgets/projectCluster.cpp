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

//  projectCluster.cpp
//  Copyright (c) 2002-2008, OpenMFG, LLC

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "xcombobox.h"
#include "xlineedit.h"

#include "projectList.h"
#include "projectcluster.h"

#include "../common/format.h"

ProjectLineEdit::ProjectLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _prjType = Undefined;
  _parsed = TRUE;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

ProjectLineEdit::ProjectLineEdit(enum Type pPrjType, QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _prjType = pPrjType;
  _parsed = TRUE;

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
}

void ProjectLineEdit::setId(int pId)
{
  if (pId != -1)
  {
    XSqlQuery prj;
    prj.prepare( "SELECT prj_number AS prjnumber"
                "  FROM prj"
                " WHERE (prj_id=:prj_id); ");
    prj.bindValue(":prj_id", pId);
    prj.exec();
    if (prj.first())
    {
      _id    = pId;
      _valid = TRUE;

      setText(prj.value("prjnumber").toString());

      emit newId(_id);
      emit valid(TRUE);

      _parsed = TRUE;

      return;
    }
  }

  _id    = -1;
  _valid = FALSE;

  setText("");

  emit newId(-1);
  emit valid(FALSE);
    
  _parsed = TRUE;
}

void ProjectLineEdit::sParse()
{
  if (!_parsed)
  {
    if (text().stripWhiteSpace().length() == 0)
      setId(-1);
    else
    {
      bool statusCheck = FALSE;
      QString sql = QString( "SELECT prj_id, prj_number "
                             "FROM prj "
                             "WHERE ((prj_number=:prj_number)"
                             " AND (" );

//  Add in the Status checks
      if (_prjType & SalesOrder)
      {
        sql += "(prj_so)";
        statusCheck = TRUE;
      }

      if (_prjType & WorkOrder)
      {
        if (statusCheck)
          sql += " OR ";
        sql += "(prj_wo)";
        statusCheck = TRUE;
      }

      if (_prjType & PurchaseOrder)
      {
        if (statusCheck)
          sql += " OR ";
        sql += "(prj_po)";
        statusCheck = TRUE;
      }

      if (!statusCheck)
        sql += "TRUE";
      sql += "))";

      XSqlQuery prj;
      prj.prepare(sql);
      prj.bindValue(":prj_number", text().stripWhiteSpace().upper());
      prj.exec();
      if (prj.first())
        setId(prj.value("prj_id").toInt());
      else
        setId(-1);
    }
  }
}


ProjectCluster::ProjectCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();
}

ProjectCluster::ProjectCluster(enum ProjectLineEdit::Type pPrjType, QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();

  _prjNumber->setType(pPrjType);
}

void ProjectCluster::constructor()
{
//  Create the component Widgets
  QHBoxLayout *_prjLayout = new QHBoxLayout(this);
  _prjLayout->setMargin(0);
  _prjLayout->setSpacing(5);

  QLabel *prjNumberLit = new QLabel(tr("Project #:"), this, "prjNumberLit");
  prjNumberLit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  prjNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _prjLayout->addWidget(prjNumberLit);

  _prjNumber = new ProjectLineEdit(this);
  _prjNumber->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _prjLayout->addWidget(_prjNumber);

  _prjList = new QPushButton(tr("..."), this, "_prjList");
  _prjList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
#ifndef Q_WS_MAC
  _prjList->setMaximumWidth(25);
#endif
  _prjList->setFocusPolicy(Qt::NoFocus);
  _prjLayout->addWidget(_prjList);

  setLayout(_prjLayout);

//  Make some internal connections
  connect(_prjNumber, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_prjNumber, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_prjList, SIGNAL(clicked()), SLOT(sProjectList()));
  connect(_prjNumber, SIGNAL(requestList()), SLOT(sProjectList()));

  setFocusProxy(_prjNumber);
}

void ProjectCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _prjNumber->setEnabled(FALSE);
    _prjList->hide();
  }
  else
  {
    _prjNumber->setEnabled(TRUE);
    _prjList->show();
  }
}

void ProjectCluster::setId(int pId)
{
  _prjNumber->setId(pId);
}

void ProjectCluster::sProjectList()
{
  ParameterList params;
  params.append("id", _prjNumber->_id);
  params.append("type", _prjNumber->_prjType);

  projectList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  
  int id = newdlg.exec();
  setId(id);

  if (id != -1)
  {
    _prjNumber->setFocus();
    focusNextPrevChild(TRUE);
  }
}

QString ProjectCluster::projectNumber() const
{
  return _prjNumber->text();
}


