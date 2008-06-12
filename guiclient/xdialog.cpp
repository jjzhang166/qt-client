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

#include "xdialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QWorkspace>
#include <QSettings>
#include <QCloseEvent>
#include <QShowEvent>
#include <QtScript>
#include <QDebug>

#include "guiclient.h"
#include "scripttoolbox.h"

//
// XDialogPrivate
//
class XDialogPrivate
{
  friend class XDialog;

  public:
    XDialogPrivate();
    ~XDialogPrivate();

    bool _shown;
    QScriptEngine * _engine;
};

XDialogPrivate::XDialogPrivate()
{
  _shown = false;
  _engine = 0;
}

XDialogPrivate::~XDialogPrivate()
{
  if(_engine)
    delete _engine;
}

XDialog::XDialog(QWidget * parent, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  _private = new XDialogPrivate();
  ScriptToolbox::setLastWindow(this);
}

XDialog::XDialog(QWidget * parent, const char * name, bool modal, Qt::WindowFlags flags)
  : QDialog(parent, flags)
{
  if(name)
    setObjectName(name);
  if(modal)
    setModal(modal);

  _private = new XDialogPrivate();
}

XDialog::~XDialog()
{
  if(_private)
    delete _private;
}

void XDialog::closeEvent(QCloseEvent *event)
{
/* We currently aren't doing this
 * Some additional code needs to be added elsewhere to allow
 * users to enable/disable memory for individual screens
 * and this closeEvent isn't called consistently for dialogs.
  QString objName = objectName();
  QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
  settings.setValue(objName + "/geometry/size", size());
  settings.setValue(objName + "/geometry/pos", pos());
 */

  QDialog::closeEvent(event);
}

void XDialog::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;

/* We currently aren't doing this
 * The saving portion of this code elsewhere doesn't
 * work consistently and there is no interaction available
 * to users to enable/disable per screen
    QRect availableGeometry = QApplication::desktop()->availableGeometry();

    QSettings settings(QSettings::UserScope, "OpenMFG.com", "OpenMFG");
    QString objName = objectName();
    QPoint pos = settings.value(objName + "/geometry/pos").toPoint();
    QSize lsize = settings.value(objName + "/geometry/size").toSize();

    if(lsize.isValid() && settings.value(objName + "/geometry/rememberSize", true).toBool())
      resize(lsize);

    // do I want to do this for a dialog?
    //_windowList.append(w);
    QRect r(pos, size());
    if(!pos.isNull() && availableGeometry.contains(r) && settings.value(objName + "/geometry/rememberPos", true).toBool())
      move(pos);
 */

    QStringList parts = objectName().split(" ");
    QStringList search_parts;
    QString oName;
    while(!parts.isEmpty())
    {
      search_parts.append(parts.takeFirst());
      oName = search_parts.join(" ");

      // load and run an QtScript that applies to this window
      qDebug() << "Looking for a script on window " << oName;
      q.prepare("SELECT script_source, script_order"
                "  FROM script"
                " WHERE((script_name=:script_name)"
                "   AND (script_enabled))"
                " ORDER BY script_order;");
      q.bindValue(":script_name", oName);
      q.exec();
      while(q.next())
      {
        QString script = q.value("script_source").toString();
        if(!_private->_engine)
        {
          _private->_engine = new QScriptEngine();
          omfgThis->loadScriptGlobals(_private->_engine);
          QScriptValue mywindow = _private->_engine->newQObject(this);
          _private->_engine->globalObject().setProperty("mywindow", mywindow);
        }
  
        QScriptValue result = _private->_engine->evaluate(script);
        if (_private->_engine->hasUncaughtException())
        {
          int line = _private->_engine->uncaughtExceptionLineNumber();
          qDebug() << "uncaught exception at line" << line << ":" << result.toString();
        }
      }
    }
  }
  QDialog::showEvent(event);
}
