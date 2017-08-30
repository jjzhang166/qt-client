/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "guimessagehandler.h"

#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextBrowser>

class GuiMessageHandlerPrivate
{
  public:
    GuiMessageHandlerPrivate()
      : debugDest(0),
        warningDest(0),
        criticalDest(0),
        fatalDest()
    {
    }

    QWidget *debugDest;
    QWidget *warningDest;
    QWidget *criticalDest;
    QWidget *fatalDest;
};

/* By default, all messages will be displayed in QMessageBoxes. Override this
   with calls to setDestination(QtMsgType, QWidget*).
 */
GuiMessageHandler::GuiMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent)
{
  _p = new GuiMessageHandlerPrivate();
}

GuiMessageHandler::~GuiMessageHandler()
{
  delete _p;
}

/* Messages with the given QtMsgType will be written to the destination widget,
   or QMessageBoxes if destination is NULL.
 */
bool GuiMessageHandler::setDestination(QtMsgType type, QWidget *destination)
{
  switch (type)
  {
    case QtDebugMsg:     _p->debugDest    = destination; break;
    case QtWarningMsg:   _p->warningDest  = destination; break;
    case QtFatalMsg:     _p->fatalDest    = destination; break;
    case QtCriticalMsg:
    default:             _p->criticalDest = destination; break;
  }

  // keep this in sync with handleMessage
  bool result = (qobject_cast<QTextBrowser*>(destination) ||
                 qobject_cast<QStatusBar  *>(destination) ||
                 qobject_cast<QLabel      *>(destination));
  return result; 
}

void GuiMessageHandler::handleMessage(QtMsgType type,
                   const QString &description,
                   const QUrl    &identifier,
                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);

  QWidget *dest = 0;
  switch (type)
  {
    case QtDebugMsg:     dest = _p->debugDest;    break;
    case QtWarningMsg:   dest = _p->warningDest;  break;
    case QtFatalMsg:     dest = _p->fatalDest;    break;
    case QtCriticalMsg:
    default:             dest = _p->criticalDest; break;
  }

  QTextBrowser *text   = qobject_cast<QTextBrowser*>(dest);
  QStatusBar   *status = qobject_cast<QStatusBar  *>(dest);
  QLabel       *label  = qobject_cast<QLabel      *>(dest);
  QWidget      *parent = qobject_cast<QWidget *>(dest); //(parent());

  if (text)        text->append(description);
  else if (status) status->showMessage(description);
  else if (label)  label->setText(description);
  else if (type == QtDebugMsg)   QMessageBox::information(parent, QString(), description);
  else if (type == QtWarningMsg) QMessageBox::warning(parent,     QString(), description);
  else                           QMessageBox::critical(parent,    QString(), description);
}

QMessageBox::StandardButton GuiMessageHandler::question(const QString &question, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  if (_acceptDefaults)
    return defaultButton;

  QWidget *dest = _p->warningDest;
  QWidget *parent = qobject_cast<QWidget *>(dest); //(parent());
  return QMessageBox::question(parent, QString(), question, buttons, defaultButton);
}
