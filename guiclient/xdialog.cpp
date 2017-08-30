/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdialog.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QDebug>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "guiclient.h"
#include "shortcuts.h"

class XDialogPrivate
{
  friend class XDialog;

  public:
    XDialogPrivate(XDialog*);
    ~XDialogPrivate();
};

XDialogPrivate::XDialogPrivate(XDialog *parent)
{
  Q_UNUSED(parent);
}

XDialogPrivate::~XDialogPrivate()
{
}

XDialog::XDialog(QWidget * parent, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    ScriptablePrivate(parent, this),
    _private(0)
{
  connect(this, SIGNAL(destroyed(QObject*)), omfgThis, SLOT(windowDestroyed(QObject*)));
  connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
}

XDialog::XDialog(QWidget * parent, const char * name, bool modal, Qt::WindowFlags flags)
  : QDialog(parent, flags),
    ScriptablePrivate(parent, this),
    _private(0)
{
  if(name)
    setObjectName(name);
  if(modal)
    setModal(modal);

  connect(this, SIGNAL(destroyed(QObject*)), omfgThis, SLOT(windowDestroyed(QObject*)));
  connect(this, SIGNAL(finished(int)), this, SLOT(saveSize()));
}

XDialog::~XDialog()
{
  if(_private)
    delete _private;
}

void XDialog::saveSize()
{
  xtsettingsSetValue(objectName() + "/geometry/size", size());
  xtsettingsSetValue(objectName() + "/geometry/pos", pos());
}

void XDialog::closeEvent(QCloseEvent * event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  callCloseEvent(event);

  if(event->isAccepted())
  {
    saveSize();
    QDialog::closeEvent(event);
  }
  
#ifdef Q_OS_MAC
  omfgThis->removeFromMacDockMenu(this);
#endif
}

void XDialog::showEvent(QShowEvent *event)
{
  if(!_shown)
  {
    _shown = true;

    QRect availableGeometry = QApplication::desktop()->availableGeometry();

    QString objName = objectName();
    QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
    QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();
    QSize currsize = size();

    if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
      resize(lsize);

    // do I want to do this for a dialog?
    //_windowList.append(w);
    QRect r(pos, size());
    if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
      move(pos);
    else if(currsize!=size())
      move(QPoint(1, 1));

    _rememberPos = new QAction(tr("Remember Position"), this);
    _rememberPos->setCheckable(true);
    _rememberPos->setChecked(xtsettingsValue(objectName() + "/geometry/rememberPos", true).toBool());
    connect(_rememberPos, SIGNAL(triggered(bool)), this, SLOT(setRememberPos(bool)));
    _rememberSize = new QAction(tr("Remember Size"), this);
    _rememberSize->setCheckable(true);
    _rememberSize->setChecked(xtsettingsValue(objectName() + "/geometry/rememberSize", true).toBool());
    connect(_rememberSize, SIGNAL(triggered(bool)), this, SLOT(setRememberSize(bool)));

    addAction(_rememberPos);
    addAction(_rememberSize);
    setContextMenuPolicy(Qt::ActionsContextMenu);

    loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  callShowEvent(event);
  QDialog::showEvent(event);
}

void XDialog::setRememberPos(bool b)
{
  xtsettingsSetValue(objectName() + "/geometry/rememberPos", b);
  if(_rememberPos)
    _rememberPos->setChecked(b);
}

void XDialog::setRememberSize(bool b)
{
  xtsettingsSetValue(objectName() + "/geometry/rememberSize", b);
  if(_rememberSize)
    _rememberSize->setChecked(b);
}

enum SetResponse XDialog::set(const ParameterList & pParams)
{
  _lastSetParams = pParams;
  loadScriptEngine();
  QTimer::singleShot(0, this, SLOT(postSet()));
  return NoError;
}

enum SetResponse XDialog::postSet()
{
  return callSet(_lastSetParams);
}

ParameterList XDialog::get() const
{
  return _lastSetParams;
}

int XDialog::exec()
{
#ifdef Q_OS_MAC
  omfgThis->updateMacDockMenu(this);
#endif

  return QDialog::exec();
}

