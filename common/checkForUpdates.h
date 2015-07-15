/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHECKFORUPDATES_H
#define CHECKFORUPDATES_H

#include <QDialog>
#include <QNetworkAccessManager>

#include "tmp/ui_checkForUpdates.h"

class QFile;
class QNetworkReply;
class QProgressDialog;
class QPushButton;
class checkForUpdatesPrivate;

class checkForUpdates : public QDialog, public Ui::checkForUpdates
{
    Q_OBJECT

public:
    checkForUpdates(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = 0);
    ~checkForUpdates();

    QPushButton* _ok;
    QPushButton* _cancel;
    QPushButton* _ignore;

public slots:
    void downloadButtonPressed();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();
    void cancelDownload();
    void startUpdate();

protected slots:
    virtual void languageChange();

private:
    checkForUpdatesPrivate *_private;
    QNetworkAccessManager   manager;
    QFile                *file;
    QProgressDialog      *progressDialog;
    QNetworkReply        *reply;
    bool                  downloadRequestAborted;
    qint64                filesize;
};

#endif // CHECKFORUPDATES_H
