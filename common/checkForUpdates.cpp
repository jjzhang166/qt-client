/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkForUpdates.h"

#include <QFile>
#include <QSqlError>
#include <QUrl>
#include <QXmlQuery>
#include <QDesktopServices>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTranslator>
#include <QDialog>
#include <QProcess>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif
#include "../guiclient/guiclient.h"
#include <parameter.h>


#define DEBUG false
#define QT_NO_URL_CAST_FROM_STRING

checkForUpdates::checkForUpdates(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  QString url = "http://updates.xtuple.com/updates";
  //intended http://updates.xtuple.com/updates/xTuple-4.0.1-linux-installer.run

  setupUi(this);
  progressDialog = new QProgressDialog(this);
  _ok = _buttonBox->button(QDialogButtonBox::Ok);
  _ignore = _buttonBox->button(QDialogButtonBox::Ignore);
  connect(_ok, SIGNAL(clicked()), this, SLOT(downloadButtonPressed()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_ignore, SIGNAL(clicked()), this, SLOT (accept()));

#ifdef Q_OS_MACX
OS = "osx";
suffix = "tar.gz";
#endif
#ifdef Q_OS_WIN
OS = "windows";
suffix = "exe";
#endif
#ifdef Q_OS_LINUX
OS = "linux";
suffix = "run";
#endif

  XSqlQuery versions, metric;
  versions.exec("SELECT metric_value AS dbver"
                "  FROM metric"
                " WHERE (metric_name = 'ServerVersion');");

  if(versions.first())
  {
    serverVersion = versions.value("dbver").toString();
    newurl = url + "/xTuple-" + serverVersion + "-" + OS + "-installer." + suffix;

    _label->setText(tr("Your client does not match the server version: %1. Would you like to update?").arg(serverVersion));

    metric.exec("SELECT fetchMetricBool('DisallowMismatchClientVersion') as disallowed;");
    metric.first();
    _ignore->setEnabled(!metric.value("disallowed").toBool());

    metric.exec("SELECT fetchMetricBool('AutoVersionUpdate') as allow;");
    metric.first();
    _ok->setEnabled(metric.value("allow").toBool());
  }
  else if (versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);
  if (DEBUG)
  {
    qDebug() << "serverVersion= " << serverVersion;
    qDebug() << "newurl= " << newurl;
  }
}
void checkForUpdates::downloadButtonPressed()
{
      this->close();
      QUrl url(newurl);
      filename = "xTuple-" + serverVersion + "-" + OS + "-installer."+ suffix;

      if(QFile::exists(filename))
      {
          if(QMessageBox::question(this, tr("Update"),
              tr("There already exists a file called %1 in "
                  "the current directory. Overwrite?").arg(filename),
                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                  == QMessageBox::No)
                  return;
          QFile::remove(filename);
      }

      file = new QFile(filename);
      if(!file->open(QIODevice::WriteOnly))
      {
          QMessageBox::information(this, "Update",
              tr("Unable to save the file %1: %2.")
              .arg(filename).arg(file->errorString()));
          delete file;
          file = NULL;
          return;
      }

      downloadRequestAborted = false;
      reply = manager.get(QNetworkRequest(url));
      connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
      connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
      connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
      connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
      //connect(reply, SIGNAL(downloadComplete()), this, SLOT(startUpdate()));

      progressDialog->setLabelText(tr("Downloading %1...").arg(filename));
      _ok->setEnabled(false);
      progressDialog->exec();
}
void checkForUpdates::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(downloadRequestAborted)
        return;
    progressDialog->setMaximum(bytesTotal);
    filesize = bytesTotal;
    progressDialog->setValue(bytesReceived);
}
void checkForUpdates::downloadReadyRead()
{
    if(file)
        file->write(reply->readAll());
}
void checkForUpdates::cancelDownload()
{
    downloadRequestAborted = true;
    reply->abort();
    _ok->setEnabled(true);
}
void checkForUpdates::downloadFinished()
{
    if(downloadRequestAborted)
    {
        if(file)
        {
            file->close();
            file->remove();
            delete file;
            file = NULL;
        }
        reply->deleteLater();
        progressDialog->hide();
        _ok->setEnabled(true);
        return;
    }

    downloadReadyRead();
    progressDialog->hide();
    _ok->setEnabled(true);
    file->flush();
    file->close();

    if(reply->error())
    {
        //Download failed
        QMessageBox::information(this, "Download failed", tr("Failed: %1").arg(reply->errorString()));
    }

    reply->deleteLater();
    reply = NULL;
    delete file;
    file = NULL;
    startUpdate();
}
void checkForUpdates::startUpdate()
{
    QFile *updater = new QFile(filename);
    if(updater->exists())
    {
        QStringList options;
        QProcess *installer = new QProcess(this);
        #ifdef Q_OS_MAC
        QProcess sh;
        sh.start("tar -xvf " + filename);
        sh.waitForFinished();
        sh.close();
        filename = "xTuple-" + serverVersion + "-" + OS + "-installer.app";
        QFileInfo *path2 = new QFileInfo(filename);
        QString filepath = path2->absoluteFilePath() + "/Contents/MacOS/osx-intel";
        if(installer->startDetached(filepath, options))
            reject();
        #endif
        #ifdef Q_OS_LINUX
        QFile launch(filename);
        launch.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|QFile::ReadOther|QFile::WriteOther|QFile::ExeOther);
        QFileInfo *path = new QFileInfo(filename);
        if(installer->startDetached(path->absoluteFilePath(), options))
             reject();
        #endif
        #ifdef Q_OS_WIN
        int result = (int)::ShellExecuteA(0, "open", filename.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
        if (SE_ERR_ACCESSDENIED== result)
        {
            result = (int)::ShellExecuteA(0, "runas", filename.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
            reject();
        }
        if (result <= 32)
            QMessageBox::information(this, "Download failed", tr("Failed: %1").arg(result));
        #endif
    }
}

checkForUpdates::~checkForUpdates()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}

