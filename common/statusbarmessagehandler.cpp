/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "statusbarmessagehandler.h"

#include <QDebug>
#include <QString>
#include <QTextDocument>

StatusBarMessageHandler::StatusBarMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent),
    _widget(0),
    _timeout(0)
{
}

StatusBarMessageHandler::~StatusBarMessageHandler()
{
}

bool StatusBarMessageHandler::setDestination(QStatusBar *destination)
{
  if (destination) {
    _widget = destination;
  }
  else
    return false;

  return true;
}

bool StatusBarMessageHandler::setTimeout(int timeout)
{
  if (timeout >= 0)
    _timeout = timeout;
  else
    return false;

  return true;
}

void StatusBarMessageHandler::handleMessage(QtMsgType type,
                   const QString &description,
                   const QUrl    &identifier,
                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);
  QTextDocument tmpdoc;
  tmpdoc.setHtml(description);
  QString msg = tmpdoc.toPlainText().trimmed();
  if (type == QtFatalMsg)
    msg = QString("Error: %1").arg(msg);

  if (_widget)
    _widget->showMessage(msg, _timeout);
}

QMessageBox::StandardButton StatusBarMessageHandler::question(const QString &question, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  Q_UNUSED(buttons);

  if (_acceptDefaults)
    return defaultButton;

  // we don't know how to deal with questions
  _widget->showMessage(tr("Status Bar cannot answer this question: %1").arg(question), _timeout);
  return QMessageBox::Cancel;
}
