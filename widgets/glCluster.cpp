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

//  glCluster.cpp
//  Created 03/01/2003 JSL
//  Copyright (c) 2000-2008, OpenMFG, LLC

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFocusEvent>

#include <xsqlquery.h>
#include <parameter.h>

#include "OpenMFGWidgets.h"
#include "glcluster.h"
#include "accountList.h"
#include "accountSearch.h"

GLCluster::GLCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
//  Create and place the component Widgets
  QHBoxLayout *_layoutMain = new QHBoxLayout(this);
  _layoutMain->setMargin(0);
  _layoutMain->setSpacing(5);
  QWidget * _layoutFieldsWidget = new QWidget(this);
  QHBoxLayout *_layoutFields = new QHBoxLayout(_layoutFieldsWidget);
  _layoutFields->setMargin(0);
  _layoutFields->setSpacing(2);

  _type = cUndefined;

  _company = 0;
  _profit = 0;
  _sub = 0;
  if (_x_metrics && !_x_metrics->boolean("AllowManualGLAccountEntry")) 
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
    {
      _company = new QLineEdit(_layoutFieldsWidget, "_company");
      _company->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      _company->setMaximumWidth(40);
      _company->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _company->setReadOnly(TRUE);
      _company->setPaletteBackgroundColor(QColor("lightgrey"));
      _company->setFocusPolicy(Qt::NoFocus);
      _layoutFields->addWidget(_company);

      QLabel *_sep1Lit = new QLabel(tr("-"), _layoutFieldsWidget, "_sep1Lit");
      _sep1Lit->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _layoutFields->addWidget(_sep1Lit);
    }

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
    {
      _profit = new QLineEdit(_layoutFieldsWidget, "_profit");
      _profit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      _profit->setMaximumWidth(40);
      _profit->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _profit->setReadOnly(TRUE);
      _profit->setPaletteBackgroundColor(QColor("lightgrey"));
      _profit->setFocusPolicy(Qt::NoFocus);
      _layoutFields->addWidget(_profit);

      QLabel *_sep2Lit = new QLabel(tr("-"), _layoutFieldsWidget, "_sep2Lit");
      _sep2Lit->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _layoutFields->addWidget(_sep2Lit);
    }
  }

  _main = new XLineEdit(_layoutFieldsWidget, "_main");
  _main->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _main->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
  if(!(_x_metrics && _x_metrics->boolean("AllowManualGLAccountEntry")))
  {
    _main->setReadOnly(TRUE);
    _main->setPaletteBackgroundColor(QColor("lightgrey"));
    _main->setFocusPolicy(Qt::NoFocus);
  }
  _layoutFields->addWidget(_main);

  if (_x_metrics && !_x_metrics->boolean("AllowManualGLAccountEntry"))
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
    {
      QLabel *_sep3Lit = new QLabel(tr("-"), _layoutFieldsWidget,  "_sep3Lit");
      _sep3Lit->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _layoutFields->addWidget(_sep3Lit);

      _sub = new QLineEdit(_layoutFieldsWidget, "_sub");
      _sub->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      _sub->setMaximumWidth(40);
      _sub->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
      _sub->setReadOnly(TRUE);
      _sub->setPaletteBackgroundColor(QColor("lightgrey"));
      _sub->setFocusPolicy(Qt::NoFocus);
      _layoutFields->addWidget(_sub);
    }
  }

  _layoutFieldsWidget->setLayout(_layoutFields);
  _layoutMain->addWidget(_layoutFieldsWidget);

  _account = new QLineEdit(this, "_account");
  _account->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _account->setReadOnly(TRUE);
  _account->setPaletteBackgroundColor(QColor("lightgrey"));
  _account->setFocusPolicy(Qt::NoFocus);
  _layoutMain->addWidget(_account);

  _list = new QPushButton(tr("..."), this, "_list");
#ifndef Q_WS_MAC
  _list->setMaximumWidth(25);
#endif
  _list->setFocusPolicy(Qt::NoFocus);
  _layoutMain->addWidget(_list);

  setLayout(_layoutMain);

//  Make some internal connections
  connect(_list, SIGNAL(clicked()), this, SLOT(sEllipses()));
  if(_x_metrics && _x_metrics->boolean("AllowManualGLAccountEntry"))
  {
    connect(_main, SIGNAL(lostFocus()), this, SLOT(sParse()));
    connect(_main, SIGNAL(textChanged(const QString &)), this, SLOT(sTextChanged(const QString &)));
    connect(_main, SIGNAL(requestList()), this, SLOT(sList()));
    connect(_main, SIGNAL(requestSearch()), this, SLOT(sSearch()));
  }

  setTabOrder(_main, _list);
  setTabOrder(_list, _main);
  setFocusProxy(_main);

  _accntid = -1;
  _valid = FALSE;
  _parsed = TRUE;

  _showExternal = false;
  
  _mapper = new XDataWidgetMapper(this);
}

void GLCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
    _list->hide();
  else
    _list->show();
  if(_x_metrics && _x_metrics->boolean("AllowManualGLAccountEntry"))
    _main->setReadOnly(pReadOnly);
}

void GLCluster::setId(int pId)
{
  XSqlQuery _query;
  if (_showExternal)
    _query.prepare("SELECT *, formatGLAccount(accnt_id) AS f_accnt "
                   "  FROM accnt "
                   " WHERE (accnt_id=:accnt_id);" );
  else
    _query.prepare("SELECT *, formatGLAccount(accnt_id) AS f_accnt "
                   "  FROM accnt LEFT OUTER JOIN company ON (accnt_company=company_number)"
                   " WHERE (NOT COALESCE(company_external, false) AND (accnt_id=:accnt_id));" );
  _query.bindValue(":accnt_id", pId);
  _query.exec();
  if (_query.first())
  {
    if (_company)
      _company->setText(_query.value("accnt_company").toString());

    if (_profit)
      _profit->setText(_query.value("accnt_profit").toString());

    if(_x_metrics && _x_metrics->boolean("AllowManualGLAccountEntry"))
      _main->setText(_query.value("f_accnt").toString());
    else
      _main->setText(_query.value("accnt_number").toString());

    if (_sub)
      _sub->setText(_query.value("accnt_sub").toString());

    _account->setText(_query.value("accnt_descrip").toString());
    _accntid = pId;
    _number = _query.value("f_accnt").toString();
    _valid = TRUE;

    if (_mapper->model() &&
      _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != _number)
        _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), _number);
  }
  else
  {
    if (_company)
      _company->setText("");

    if (_profit)
      _profit->setText("");

    if (_sub)
      _sub->setText("");

    _main->setText("");
    _account->setText("");
    _accntid = -1;
    _valid = FALSE;
    _number = "";
  }

  _parsed = true;
}

void GLCluster::sParse()
{
  if(!_parsed && _x_metrics && _x_metrics->boolean("AllowManualGLAccountEntry"))
  {
    _parsed = true;

    if(_main->text().length() == 0)
    {
      setId(-1);
      return;
    }

    XSqlQuery qgl;
    if (_showExternal)
      qgl.prepare("SELECT DISTINCT accnt_id"
                  "  FROM accnt"
                  " WHERE (formatGLAccount(accnt_id)=:searchString);");
    else
      qgl.prepare("SELECT DISTINCT accnt_id"
                  "  FROM accnt JOIN company ON (accnt_company=company_number)"
                  " WHERE (NOT company_external"
                  "   AND  (formatGLAccount(accnt_id)=:searchString));");
    qgl.bindValue(":searchString", _main->text().trimmed());
    qgl.exec();
    if(qgl.first())
      setId(qgl.value("accnt_id").toInt());
    else
    {
      setId(-1);
      focusNextPrevChild(FALSE);
      QMessageBox::warning( this, tr("Invalid Account Number"),
        tr("<p>The Account Number you entered is Invalid<p>") );
    }
  }
}

void GLCluster::setEnabled(bool pEnabled)
{
  if (pEnabled)
    _list->show();
  else
    _list->hide();
  QWidget::setEnabled(pEnabled);
}

void GLCluster::setNumber(QString number)
{
  if (_number==number)
    return;
    
  if (number.isEmpty())
  {
    setId(-1);
    return;
  }
    
  XSqlQuery qgl;
  if (_showExternal)
    qgl.prepare("SELECT DISTINCT accnt_id"
                "  FROM accnt"
                " WHERE (formatGLAccount(accnt_id)=:number);");
  else
    qgl.prepare("SELECT DISTINCT accnt_id"
                "  FROM accnt JOIN company ON (accnt_company=company_number)"
                " WHERE (NOT company_external"
                "   AND  (formatGLAccount(accnt_id)=:number));");
  qgl.bindValue(":number", number);
  qgl.exec();
  if(qgl.first())
    setId(qgl.value("accnt_id").toInt());
  else
  {
    setId(-1);
    focusNextPrevChild(FALSE);
    QMessageBox::warning( this, tr("Invalid Account Number"),
      tr("<p>The Account Number you entered is Invalid<p>") );
  }

  _parsed = true;
}

void GLCluster::sEllipses()
{
  if(_x_preferences)
  {
    if(_x_preferences->value("DefaultEllipsesAction") == "search")
    {
      sSearch();
      return;
    }
  }

  sList();
}

void GLCluster::sSearch()
{
  ParameterList params;
  params.append("accnt_id", _accntid);
  params.append("type", _type);
  if (_showExternal)
    params.append("showExternal");

  accountSearch newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  setId(newdlg.exec());
}

void GLCluster::sList()
{
  ParameterList params;
  params.append("accnt_id", _accntid);
  params.append("type", _type);
  if (_showExternal)
    params.append("showExternal");

  accountList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  setId(newdlg.exec());
}

bool GLCluster::isValid()
{
  sParse();
  return _valid;
}

int GLCluster::id()
{
  sParse();
  return _accntid;
}

void GLCluster::keyPressEvent(QKeyEvent *event)
{
  if (event->state() == Qt::ControlModifier)
  {
    if (event->key() == Qt::Key_L)
    {
      _parsed = TRUE;
      sList();
    }

    else if (event->key() == Qt::Key_S)
    {
      _parsed = TRUE;
      sSearch();
    }

    else if (event->key() == Qt::Key_A)
    {
      _parsed = TRUE;
      sEllipses();
    }
  }
  else
    _parsed = FALSE;

  QWidget::keyPressEvent(event);
}

void GLCluster::focusInEvent(QFocusEvent *pEvent)
{
  if(!_main->text().isEmpty())
    _main->setSelection(0, _main->text().length());

  QWidget::focusInEvent(pEvent);
}

void GLCluster::sTextChanged(const QString &)
{
  _parsed = false;
}

void GLCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
}
