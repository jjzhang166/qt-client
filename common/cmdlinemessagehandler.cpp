/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <stdio.h>

#include "cmdlinemessagehandler.h"

#include <QDebug>
#include <QString>
#include <QTextDocument>
#include <QTextStream>

class CmdLineMessageHandlerPrivate
{
  public:
    CmdLineMessageHandlerPrivate()
      : input(stdin, QIODevice::ReadOnly),
        output(stdout, QIODevice::WriteOnly)
    {
      if (buttonMap.isEmpty()) {
        buttonMap.insert(QMessageBox::Ok,              QObject::tr("Ok"));
        buttonMap.insert(QMessageBox::Open,            QObject::tr("Open"));
        buttonMap.insert(QMessageBox::Save,            QObject::tr("Save"));
        buttonMap.insert(QMessageBox::Cancel,          QObject::tr("Cancel"));
        buttonMap.insert(QMessageBox::Close,           QObject::tr("Close"));
        buttonMap.insert(QMessageBox::Discard,         QObject::tr("Discard"));
        buttonMap.insert(QMessageBox::Apply,           QObject::tr("Apply"));
        buttonMap.insert(QMessageBox::Reset,           QObject::tr("Reset"));
        buttonMap.insert(QMessageBox::RestoreDefaults, QObject::tr("Restore Defaults"));
        buttonMap.insert(QMessageBox::Help,            QObject::tr("Help"));
        buttonMap.insert(QMessageBox::SaveAll,         QObject::tr("SaveAll"));
        buttonMap.insert(QMessageBox::Yes,             QObject::tr("Yes"));
        buttonMap.insert(QMessageBox::YesToAll,        QObject::tr("Yes To All"));
        buttonMap.insert(QMessageBox::No,              QObject::tr("No"));
        buttonMap.insert(QMessageBox::NoToAll,         QObject::tr("No To All"));
        buttonMap.insert(QMessageBox::Abort,           QObject::tr("Abort"));
        buttonMap.insert(QMessageBox::Retry,           QObject::tr("Retry"));
        buttonMap.insert(QMessageBox::Ignore,          QObject::tr("Ignore"));
      }
    }

    static QMap<QMessageBox::StandardButton, QString> buttonMap;
    QTextStream input;
    QTextStream output;
};

QMap<QMessageBox::StandardButton, QString> CmdLineMessageHandlerPrivate::buttonMap;

/* By default, all messages will be displayed in QMessageBoxes. Override this
   with calls to setDestination(QtMsgType, QWidget*).
 */
CmdLineMessageHandler::CmdLineMessageHandler(QObject *parent)
  : XAbstractMessageHandler(parent)
{
  _p = new CmdLineMessageHandlerPrivate();
}

CmdLineMessageHandler::~CmdLineMessageHandler()
{
  delete _p;
}

void CmdLineMessageHandler::handleMessage(QtMsgType type,
                   const QString &description,
                   const QUrl    &identifier,
                   const QSourceLocation &sourceLocation)
{
  Q_UNUSED(identifier);
  Q_UNUSED(sourceLocation);
  QTextDocument tmpdoc;
  tmpdoc.setHtml(description);
  QString msg = tmpdoc.toPlainText().trimmed();
  switch (type)
  {
    case QtFatalMsg:     _p->output << "Error:" << msg << endl; break;
    case QtDebugMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
    default:             _p->output << msg << endl; break;
  }
}

QMessageBox::StandardButton CmdLineMessageHandler::question(const QString &question, QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
{
  if (_acceptDefaults)
    return defaultButton;

  QMap<int, QMessageBox::StandardButton> choice;
  int defaultIdx = 0;
  int idx = 1;
  QTextDocument tmpdoc;
  tmpdoc.setHtml(question);
  QString msg = tmpdoc.toPlainText().trimmed();
  foreach (const QMessageBox::StandardButton &key, _p->buttonMap.keys())
  {
    if (buttons & key)
    {
      choice.insert(idx, key);
      if (key == defaultButton)
        defaultIdx = idx;
      idx++;
    }
  }

  QString qsprompt;
  foreach (int key, choice.keys())
  {
    qsprompt = qsprompt.append("\n")
                       .append(tr("%1:\t%2)")
                           .arg(key)
                           .arg(_p->buttonMap.value(choice.value(key))));
  }
  qsprompt = qsprompt.append("\n")
                     .append(tr("[%1]").arg(defaultIdx));

  int  selection = -1;
  bool valid;
  QString answer;
  while (! choice.contains(selection))
  {
    _p->output << msg << "\n" << qsprompt << " ";
    _p->output.flush();
    answer = _p->input.readLine();

    if (answer.isEmpty())
      selection = defaultIdx;
    else
    {
      selection = answer.toInt(&valid);
      if (! valid)
        selection = -1;
    }
  }

  return choice.value(selection);
}
