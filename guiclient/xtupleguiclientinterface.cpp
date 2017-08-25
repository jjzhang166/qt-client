/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guiclient.h"
#include "scripttoolbox.h"
#include "xdialog.h"
#include "xmainwindow.h"
#include "xtupleguiclientinterface.h"
#include "xwidget.h"

/** @class xTupleGuiClientInterface
    @brief A concrete implementation of the GuiClientInterface allowing widgets
           to request services from the main application.

    The primary use of this class/interface is for individual widgets to open
    application-level windows.
  */

xTupleGuiClientInterface::xTupleGuiClientInterface(QObject *pParent)
  : GuiClientInterface(pParent)
{
  if (pParent)
    connect(pParent, SIGNAL(dbConnectionLost()), this, SIGNAL(dbConnectionLost()));
}

QWidget* xTupleGuiClientInterface::openWindow(const QString      pname,
                                              ParameterList      pparams,
                                              QWidget           *parent,
                                              Qt::WindowModality modality,
                                              Qt::WindowFlags    flags)
{
  ScriptToolbox toolbox(0);
  QWidget *w = toolbox.openWindow(pname, parent, modality, flags);

  if (w)
  {
    if (w->inherits("XDialog"))
    {
      XDialog* xdlg = (XDialog*)w;
      xdlg->set(pparams);
      w = (QWidget*)xdlg;
    }
    else if (w->inherits("XMainWindow"))
    {
      XMainWindow* xwind = (XMainWindow*)w;
      xwind->set(pparams);
      w = (QWidget*)xwind;
      w->show();
    }
    else if (w->inherits("XWidget"))
    {
      XWidget* xwind = (XWidget*)w;
      xwind->set(pparams);
      w = (QWidget*)xwind;
      w->show();
    }
  }
  return w;
}

QAction* xTupleGuiClientInterface::findAction(const QString pname)
{
 return omfgThis->findChild<QAction*>(pname);
}

void xTupleGuiClientInterface::addDocumentWatch(QString path, int id)
{
  omfgThis->addDocumentWatch(path, id);
}

void xTupleGuiClientInterface::removeDocumentWatch(QString path)
{
  omfgThis->removeDocumentWatch(path);
}

bool xTupleGuiClientInterface::hunspell_ready()
{
  return omfgThis->hunspell_ready();
}

int xTupleGuiClientInterface::hunspell_check(const QString word)
{
  return omfgThis->hunspell_check(word);
}

const QStringList xTupleGuiClientInterface::hunspell_suggest(const QString word)
{
  return omfgThis->hunspell_suggest(word);
}

int xTupleGuiClientInterface::hunspell_add(const QString word)
{
  return omfgThis->hunspell_add(word);
}

int xTupleGuiClientInterface::hunspell_ignore(const QString word)
{
  return omfgThis->hunspell_ignore(word);
}

Metrics *xTupleGuiClientInterface::getMetrics()
{
  return _metrics;
}

Metricsenc *xTupleGuiClientInterface::getMetricsenc()
{
  return _metricsenc;
}

Preferences *xTupleGuiClientInterface::getPreferences()
{
  return _preferences;
}

Privileges *xTupleGuiClientInterface::getPrivileges()
{
  return _privileges;
}

