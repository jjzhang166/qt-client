/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XABSTRACTMESSAGEHANDLER_H__
#define __XABSTRACTMESSAGEHANDLER_H__

#include <QAbstractMessageHandler>
#include <QMessageBox>

class XAbstractMessageHandler : public QAbstractMessageHandler
{
  public:
    XAbstractMessageHandler(QObject *parent = 0);
    virtual ~XAbstractMessageHandler();

    virtual QMessageBox::StandardButton question(const QString &question, QMessageBox::StandardButtons buttons = QMessageBox::Ok, QMessageBox::StandardButton defaultButton = QMessageBox::NoButton) = 0;

    bool acceptDefaults() const;
    bool setAcceptDefaults(const bool accept);

  protected:
    bool _acceptDefaults;
};

#endif
