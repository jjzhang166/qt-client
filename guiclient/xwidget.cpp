/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2013 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xwidget.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QShowEvent>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QDebug>
#include <QStyleOptionTitleBar>

#include "xcheckbox.h"
#include "xtsettings.h"
#include "guiclient.h"
#include "scriptablePrivate.h"
#include "shortcuts.h"

#define DEBUG false
//
// XWidgetPrivate
//
class XWidgetPrivate : public ScriptablePrivate
{
  friend class XWidget;

  public:
    XWidgetPrivate(XWidget *parent);
    virtual ~XWidgetPrivate();

    XWidget * _parent;
    bool _shown;
};

XWidgetPrivate::XWidgetPrivate(XWidget *parent) : ScriptablePrivate(false, parent), _parent(parent)
{
  _shown = false;
}

XWidgetPrivate::~XWidgetPrivate()
{
}

XWidget::XWidget(QWidget * parent, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags )
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
  }

  _private = new XWidgetPrivate(this);
}

XWidget::XWidget(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : QWidget(parent,
            ((flags & (Qt::Dialog | Qt::Window)) && parent && parent->isModal()) ? (flags | Qt::Dialog)
                  : flags )
{
  if(parent && parent->isModal() && (flags & (Qt::Dialog | Qt::Window)))
  {
    setWindowModality(Qt::ApplicationModal);
    //setWindowFlags(windowFlags() | Qt::Dialog);
  }

  if(name)
    setObjectName(name);

  _private = new XWidgetPrivate(this);
}

XWidget::~XWidget()
{
  if(_private)
    delete _private;
}

void XWidget::closeEvent(QCloseEvent *event)
{
  event->accept(); // we have no reason not to accept and let the script change it if needed
  _private->callCloseEvent(event);

  if(event->isAccepted())
  {
    QString objName = objectName();
    xtsettingsSetValue(objName + "/geometry/size", size());
    if(omfgThis->showTopLevel() || isModal())
      xtsettingsSetValue(objName + "/geometry/pos", pos());
    else
      xtsettingsSetValue(objName + "/geometry/pos", parentWidget()->pos());
  }
}

void XWidget::showEvent(QShowEvent *event)
{
  if(!_private->_shown)
  {
    _private->_shown = true;
    if (windowFlags() & (Qt::Window | Qt::Dialog))
    {
      QRect availableGeometry = QApplication::desktop()->availableGeometry();
      if(!omfgThis->showTopLevel() && !isModal())
        availableGeometry = QRect(QPoint(0, 0), omfgThis->workspace()->size());

      QString objName = objectName();
      QPoint pos = xtsettingsValue(objName + "/geometry/pos").toPoint();
      QSize lsize = xtsettingsValue(objName + "/geometry/size").toSize();

      setAttribute(Qt::WA_DeleteOnClose);
      if(omfgThis->showTopLevel() || isModal())
      {
        if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
          resize(lsize);
        omfgThis->_windowList.append(this);
        QRect r(pos, size());
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool())
          move(pos);
      }
      else
      {
        QWidget * fw = focusWidget();
        QMdiSubWindow *subwin = omfgThis->workspace()->addSubWindow(this);
        omfgThis->workspace()->setActiveSubWindow(subwin);
        //begin re-size hack
        QStyle * wStyle = subwin->style();
        QStyleOptionTitleBar so;
        so.titleBarState = 1;
        so.titleBarFlags = Qt::Window;
        int titleBarHeight = wStyle->pixelMetric(QStyle::PM_TitleBarHeight, &so, this);
        titleBarHeight += 4;  //inexplicably off by 4 pixels?!
        if(DEBUG)
        {
        qDebug() << "lsize= " << lsize << "titleBarHeight= " << titleBarHeight; //before pixel tweak
        }
        if(lsize.isValid())
        {
        lsize.rheight() += titleBarHeight;
        lsize.rwidth() += 8;  //width compensation happened to be consistently 8 pixels
        }
        if(DEBUG)
        qDebug() << "lsize2= " << lsize << "titleBarHeight= " << titleBarHeight; //after pixel tweak
        //end re-size hack
        connect(this, SIGNAL(destroyed(QObject*)), subwin, SLOT(close()));
        if(lsize.isValid() && xtsettingsValue(objName + "/geometry/rememberSize", true).toBool())
          subwin->resize(lsize);
        QRect r(pos, lsize);
        if(!pos.isNull() && availableGeometry.contains(r) && xtsettingsValue(objName + "/geometry/rememberPos", true).toBool() && parentWidget())
          parentWidget()->move(pos);
        // This originally had to be after the show? Will it work here?
        if(fw)
          fw->setFocus();
      }
    }

    _private->loadScriptEngine();

    QList<XCheckBox*> allxcb = findChildren<XCheckBox*>();
    for (int i = 0; i < allxcb.size(); ++i)
      allxcb.at(i)->init();

    shortcuts::setStandardKeys(this);
  }

  _private->callShowEvent(event);
  QWidget::showEvent(event);
}

enum SetResponse XWidget::set( const ParameterList & pParams )
{
  _lastSetParams = pParams;

  _private->loadScriptEngine();

  QTimer::singleShot(0, this, SLOT(postSet()));

  return NoError;
}

enum SetResponse XWidget::postSet()
{
  return _private->callSet(_lastSetParams);
}

ParameterList XWidget::get() const
{
  return _lastSetParams;
}

QScriptEngine *XWidget::engine()
{
  _private->loadScriptEngine();
  return _private->_engine;
}

