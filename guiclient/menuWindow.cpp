/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "xtsettings.h"
#include "guiclient.h"

#include "menuWindow.h"

class MenuWindowPrivate
{
  public:
    MenuWindowPrivate() :
      _cascade(0),    _closeActive(0), _closeAll(0),     _geometryMenu(0),
      _lastActive(0), _rememberPos(0), _rememberSize(0), _tile(0)
    {
    }

    Action    *_cascade;
    Action    *_closeActive;
    Action    *_closeAll;
    QMenu     *_geometryMenu;
    QWidget   *_lastActive;
    Action    *_rememberPos;
    Action    *_rememberSize;
    Action    *_tile;
};

menuWindow::menuWindow(GUIClient *pParent) :
  QObject(pParent),
  _parent(pParent), _windowMenu(0), _data(0)
{
  setObjectName("winModule");
  _data = new MenuWindowPrivate();

  _windowMenu = new QMenu(tr("&Window"), _parent);
  _windowMenu->setObjectName("menu.window");

  _data->_closeActive = new Action(_parent, "window.closeActiveWindow",
                           tr("Close &Active Window"), this,
                           SLOT(sCloseActive()), _windowMenu, true);
  _data->_closeAll    = new Action(_parent, "window.closeAllWindows",
                           tr("Close A&ll Windows"), this, SLOT(sCloseAll()),
                           _windowMenu, true);

  _data->_cascade = new Action(_parent, "window.cascade", tr("&Cascade"), this,
                               SLOT(sCascade()), _windowMenu, true);
  _data->_tile = new Action(_parent, "window.tile", tr("&Tile"), this,
                            SLOT(sTile()), _windowMenu, true);

  _data->_rememberPos = new Action(_parent, "window.rememberPositionToggle",
                            tr("Remember Position"), this,
                            SLOT(sRememberPositionToggle()), _windowMenu, true);
  _data->_rememberPos->setCheckable(true);

  _data->_rememberSize = new Action(_parent, "window.rememberSizeToggle",
                             tr("Remember Size"), this,
                             SLOT(sRememberSizeToggle()), _windowMenu, true);
  _data->_rememberSize->setCheckable(true);

  (void)_parent->menuBar()->addMenu(_windowMenu);

  connect(_windowMenu, SIGNAL(aboutToShow()), this, SLOT(sPrepareWindowMenu()));
  connect(_windowMenu, SIGNAL(aboutToHide()), this, SLOT(sHideWindowMenu()));

  _parent->populateCustomMenu(_windowMenu, "Window");
}

menuWindow::~menuWindow()
{
  if (_data)
    delete _data;
  _data = 0;
}

void menuWindow::sPrepareWindowMenu()
{
  _windowMenu->clear();

  if (! _parent->showTopLevel())
  {
    _windowMenu->addAction(_data->_cascade);
    _windowMenu->addAction(_data->_tile);

    _windowMenu->addSeparator();
  }

  _windowMenu->addAction(_data->_closeActive);
  _windowMenu->addAction(_data->_closeAll);

  QWidgetList windows = _parent->windowList();

  bool b;
  if(_preferences->value("InterfaceWindowOption") != "TopLevel")
  {
  b = windows.isEmpty();
  }
  else
  {
  b = !windows.isEmpty();
  }

  _data->_cascade->setEnabled(b);
  _data->_tile->setEnabled(b);

  _data->_closeActive->setEnabled(b);
  _data->_closeAll->setEnabled(b);

  _windowMenu->addSeparator();

  if (_parent->showTopLevel())
    _data->_lastActive = QApplication::activeWindow();
  else if (_parent->workspace() && _parent->workspace()->activeSubWindow())
    _data->_lastActive = _parent->workspace()->activeSubWindow()->widget();
  else
    _data->_lastActive = 0;

  if (_data->_lastActive)
  {
    if(!_data->_geometryMenu)
      _data->_geometryMenu = new QMenu();

    _data->_geometryMenu->clear();
    _data->_geometryMenu->setTitle(_data->_lastActive->windowTitle());

    QString objName = _data->_lastActive->objectName();
    
    _data->_rememberPos->setChecked(xtsettingsValue(objName + "/geometry/rememberPos", true).toBool());
    _data->_geometryMenu->addAction(_data->_rememberPos);
    _data->_rememberSize->setChecked(xtsettingsValue(objName + "/geometry/rememberSize", true).toBool());
    _data->_geometryMenu->addAction(_data->_rememberSize);

    _windowMenu->addMenu(_data->_geometryMenu);
    _windowMenu->addSeparator();
  }

  QAction *m = 0;
  foreach (QWidget *wind, windows)
  {
    m = _windowMenu->addAction(wind->windowTitle(), this, SLOT(sActivateWindow()));
    if(m)
    {
      m->setData(qVariantFromValue(wind));
      m->setCheckable(true);
      m->setChecked((_data->_lastActive == wind));
    }
  }
}

void menuWindow::sHideWindowMenu()
{
  _data->_cascade->setEnabled(true);
  _data->_tile->setEnabled(true);
  _data->_closeActive->setEnabled(true);
  _data->_closeAll->setEnabled(true);
}

void menuWindow::sActivateWindow()
{
  QWidget *wind = 0;
  if (QAction *m = qobject_cast<QAction*>(sender())) // not ==
    //wind = qVariantValue<QWidget*>(m->data());
      wind = m->data().value<QWidget*>();

  if (wind)
  {
    if (_parent->showTopLevel())
      wind->activateWindow();
    else
      wind->setFocus();
  }
}

void menuWindow::sCascade()
{
  _parent->workspace()->cascadeSubWindows();
  _parent->workspace()->setViewMode(QMdiArea::SubWindowView);
  _parent->workspace()->setOption(QMdiArea::DontMaximizeSubWindowOnActivation,
                                  true);
}

void menuWindow::sCloseActive()
{
  if (_parent->showTopLevel())
  {
    if (_parent->windowList().contains(QApplication::activeWindow()))
      QApplication::activeWindow()->close();
  }
  else
    _parent->workspace()->closeActiveSubWindow();
}

void menuWindow::sCloseAll()
{
  if (_parent->showTopLevel())
  {
    foreach(QWidget *w, _parent->windowList())
      w->close();
  }
  else
    _parent->workspace()->closeAllSubWindows();
}

void menuWindow::sRememberPositionToggle()
{
  if (! _data->_lastActive)
    return;

  xtsettingsSetValue(_data->_lastActive->objectName() + "/geometry/rememberPos",
                     _data->_rememberPos->isChecked());
}

void menuWindow::sRememberSizeToggle()
{
  if (!_data->_lastActive)
    return;

  xtsettingsSetValue(_data->_lastActive->objectName() + "/geometry/rememberSize",
                     _data->_rememberSize->isChecked());
}

void menuWindow::sTile()
{
  _parent->workspace()->tileSubWindows();
  _parent->workspace()->setViewMode(QMdiArea::SubWindowView);
  _parent->workspace()->setOption(QMdiArea::DontMaximizeSubWindowOnActivation,
                                  true);
}

