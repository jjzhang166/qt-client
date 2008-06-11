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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
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
 * Powered by PostBooks, an open source solution from xTuple
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

#ifndef __SCRIPTTOOLBOX_H__
#define __SCRIPTTOOLBOX_H__

#include <QObject>
#include <QVariant>
#include <QScriptValue>

#include <parametergroup.h>

#include "parameter.h"
#include "guiclient.h"

class QWidget;
class QLayout;
class QGridLayout;
class QBoxLayout;
class QStackedLayout;
class QScriptEngine;

Q_DECLARE_METATYPE(ParameterList)
Q_DECLARE_METATYPE(XSqlQuery);
Q_DECLARE_METATYPE(enum SetResponse)
Q_DECLARE_METATYPE(enum ParameterGroupStates);
Q_DECLARE_METATYPE(enum ParameterGroupTypes);
Q_DECLARE_METATYPE(enum Qt::WindowModality);

QScriptValue ParameterListtoScriptValue(QScriptEngine *engine, const ParameterList &params);
void ParameterListfromScriptValue(const QScriptValue &obj, ParameterList &params);

QScriptValue XSqlQuerytoScriptValue(QScriptEngine *engine, const XSqlQuery &qry);
void XSqlQueryfromScriptValue(const QScriptValue &obj, XSqlQuery &qry);

QScriptValue SetResponsetoScriptValue(QScriptEngine *engine, const enum SetResponse &sr);
void SetResponsefromScriptValue(const QScriptValue &obj, enum SetResponse &sr);

QScriptValue ParameterGroupStatestoScriptValue(QScriptEngine *engine, const enum ParameterGroupStates &en);
void ParameterGroupStatesfromScriptValue(const QScriptValue &obj, enum ParameterGroupStates &en);

QScriptValue ParameterGroupTypestoScriptValue(QScriptEngine *engine, const enum ParameterGroupTypes &en);
void ParameterGroupTypesfromScriptValue(const QScriptValue &obj, enum ParameterGroupTypes &en);

QScriptValue QtWindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &en);
void QtWindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &en);

class ScriptToolbox : public QObject
{
  Q_OBJECT

  public:
    ScriptToolbox(QScriptEngine * engine);
    virtual ~ScriptToolbox();

    static QScriptValue variantToScriptValue(QScriptEngine *, QVariant);
    static void setLastWindow(QWidget * lw);

  public slots:

    QObject * executeQuery(const QString & query, const ParameterList & params);

    QObject * widgetGetLayout(QWidget * w);
    void layoutBoxInsertWidget(QObject *, int index, QWidget *, int stretch = 0, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int row, int column, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int fromRow, int fromColumn, int rowSpan, int columnSpan, int alignment = 0);
    void layoutStackedInsertWidget(QObject *, int index, QWidget *);

    QObject * menuAddAction(QObject * menu, const QString & text);
    QObject * menuAddMenu(QObject * menu, const QString & text, const QString & name = QString());

    QWidget * tabWidget(QWidget * tab, int idx);
    int       tabInsertTab(QWidget * tab, int idx, QWidget * page, const QString & text);
    void      tabRemoveTab(QWidget * tab, int idx);
    void      tabSetTabText(QWidget * tab, int idx, const QString & text);
    QString   tabtabText(QWidget * tab, int idx);

    QWidget * createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString());
    QWidget * lastWindow() const;

    bool printReport(const QString & name, const ParameterList & params);

    int messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons = 0x00000400, int defaultButton = 0x00000000);
  private:
    QScriptEngine * _engine;
    static QWidget * _lastWindow;
};

#endif // __SCRIPTTOOLBOX_H__
