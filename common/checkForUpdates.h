/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef CHECKFORUPDATES_H
#define CHECKFORUPDATES_H

#include <QDialog>

#include "tmp/ui_checkForUpdates.h"
#include <QProgressDialog>
#include <QFile>
#include <QtNetwork>
#include <QMessageBox>

class checkForUpdates : public QDialog, public Ui::checkForUpdates
{
    Q_OBJECT

public:
    checkForUpdates(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~checkForUpdates();

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
    QNetworkAccessManager manager;
    QFile *file;
    QProgressDialog *progressDialog;
    QNetworkReply *reply;
    bool downloadRequestAborted;
    QString serverVersion;
    QString OS;
    QString suffix;
    QString newurl;
    QString filename;
    qint64 filesize;
};

#endif // CHECKFORUPDATES_H
