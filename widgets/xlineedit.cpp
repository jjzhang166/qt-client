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

//  xlineedit.cpp
//  Created 01/03/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include "xlineedit.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QLocale>
#include <QMouseEvent>
#include <QValidator>

#include "format.h"

XLineEdit::XLineEdit(QWidget *parent, const char *name) :
  QLineEdit(parent, name)
{
  setAcceptDrops(FALSE);

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif

  _parsed = TRUE;
  _valid = FALSE;

  _id = -1;
  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

bool XLineEdit::isValid()
{
  sParse();
  return _valid;
}

int XLineEdit::id()
{
  sParse();
  return _id;
}

void XLineEdit::sParse()
{
  if (validator() && validator()->inherits("QDoubleValidator"))
  {
    QRegExp zeroRegex(QString("^[0") + QLocale().groupSeparator() + "]*" +
                      QLocale().decimalPoint() + "0*$");
    if (! text().isEmpty() && toDouble() == 0 && ! text().contains(zeroRegex))
      setText("");
  }
}

double XLineEdit::toDouble(bool *pIsValid)
{
  QString strippedText = text().remove(QLocale().groupSeparator());
  return QLocale().toDouble(strippedText, pIsValid);
}

void XLineEdit::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("text"), QByteArray("defaultText"));
}

void XLineEdit::setText(const QVariant &pVariant)
{
  if (pVariant.type() == QVariant::Double)
  {
    const QValidator *v = validator();
    int prec = 0;

    if (v && v->inherits("QDoubleValidator"))
      prec = ((QDoubleValidator*)v)->decimals();
    else if (v && v->inherits("QIntValidator"))
      prec = 0;

    QLineEdit::setText(formatNumber(pVariant.toDouble(), prec));
  }
  else
    QLineEdit::setText(pVariant.toString());
}

void XLineEdit::setDouble(const double pDouble, const int pPrec)
{
  const QValidator *v = validator();
  int prec = pPrec;

  if (pPrec < 0 && v && v->inherits("QDoubleValidator"))
    prec = ((QDoubleValidator*)v)->decimals();

  QLineEdit::setText(formatNumber(pDouble, prec));
}

void XLineEdit::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QLineEdit::mousePressEvent(event);
}

void XLineEdit::keyPressEvent(QKeyEvent *event)
{
  if (event->state() == Qt::ControlModifier)
  {
    if (event->key() == Qt::Key_L)
    {
      _parsed = TRUE;
      emit (requestList());
    }

    else if (event->key() == Qt::Key_S)
    {
      _parsed = TRUE;
      emit (requestSearch());
    }

    else if (event->key() == Qt::Key_A)
    {
      _parsed = TRUE;
      emit (requestAlias());
    }
  }
  else
    _parsed = FALSE;

  if (_parsed)
    event->accept();
  else
    QLineEdit::keyPressEvent(event);
}

void XLineEdit::focusInEvent(QFocusEvent *pEvent)
{
  if(!text().isEmpty())
    setSelection(0, text().length());

  QLineEdit::focusInEvent(pEvent);
}

