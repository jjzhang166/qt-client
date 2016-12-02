/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkForUpdates.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QSqlError>
#include <QTranslator>
#include <QUrl>
#include <QXmlQuery>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

#include <parameter.h>

#include "xsqlquery.h"

#define QT_NO_URL_CAST_FROM_STRING

#define DEBUG false

class Sudoable : public QProcess {
  public:
    // TODO: try sudo without -S and feed it password if it asks?
    void start(const QString password, const QString &program, OpenMode mode = ReadWrite)
    {
      if (DEBUG)
        qDebug() << "starting" << QString(password.isEmpty() ? "" : "sudo")
                 << program;
      if (password.isEmpty())
        QProcess::start(program, mode);
      else
      {
        QProcess::start("sudo -S " + program, mode);
        waitForStarted();
        write(password.toLocal8Bit());
        write("\n");
      }
      if (DEBUG) qDebug() << "started";
    }

    QString getPassword(bool *ok)
    {
      QString password;
      bool retry = false;
      do {
        password = QInputDialog::getText(0, tr("Password Required"),
                                          tr("Enter System Password:"),
                                          QLineEdit::Password, password, ok);
        start(password, "echo testing");
        if (! waitForFinished(1000))
          terminate();
        retry = QString(readAllStandardError()).contains(tr("Password:"));
      } while (retry);
      return password;
    }
};

class checkForUpdatesPrivate {
  public:
    checkForUpdatesPrivate(checkForUpdates *parent, QString serverVersion)
      : _serverVersion(serverVersion)
    {
      Q_UNUSED(parent);

#ifdef Q_OS_MAC
      _newExePath   = "xTuple-" + _serverVersion + ".app/Contents/MacOS/xtuple";
      _downloadFile = "xTuple-" + _serverVersion + "-MACi386.dmg";
      _destdir = ".";
#endif
#ifdef Q_OS_WIN
      _newExePath   = "xTuple-" + _serverVersion + "-Windows/xtuple.exe";
      _downloadFile = "xTuple-" + _serverVersion + "-Windows.zip";
      _destdir      = "..";
#endif
#ifdef Q_OS_LINUX
      _newExePath   = "../xTuple-" + _serverVersion + "-Linux/xtuple";
      #ifdef Q_PROCESSOR_X86_64
        _downloadFile = "xTuple-"    + _serverVersion + "-Linux64.tar.bz2";
      #else
        _downloadFile = "xTuple-"    + _serverVersion + "-Linux.tar.bz2";
      #endif
      _destdir = "..";
#endif
    }

    bool ensureWritePriv(QFileInfo dirinfo) {
      if (dirinfo.isWritable())
        return true;
      return false;
    }

    /* TODO: QNetworkAccessManager gets RemoteHostClosedError from SF during
             download redirects. must(?) use QTcpSocket to switch.
    return QString("http://sourceforge.net/projects/postbooks/files/")
         + "02%20PostBooks-GUIclient-only/"
         + _serverVersion + "/" + _downloadFile + "?use_mirror=autoselect";
     */
    QString urlString() {
      return "http://updates.xtuple.com/updates/" + _downloadFile;
    }

    bool makeExecutable(QString filename, QString &errmsg)
    {
      QString filepath = QFileInfo(filename).absoluteFilePath();
      Sudoable proc;
      proc.start(_password, QString("chmod a+x %1").arg(filepath));
      if (! proc.waitForFinished(1000))
      {
        proc.terminate();
        errmsg = proc.readAllStandardError();
        return false;
      }
      return true;
    }

    QString _destdir;
    QString _downloadFile;
    QString _newExePath;
    QString _password;
    QString _serverVersion;
};

checkForUpdates::checkForUpdates(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl),
      reply(0)
{
  Q_UNUSED(name);

  setupUi(this);
  progressDialog = new QProgressDialog(this);
  _ok     = _buttonBox->button(QDialogButtonBox::Ok);
  _ignore = _buttonBox->button(QDialogButtonBox::Ignore);
  connect(_ok,        SIGNAL(clicked()),  this, SLOT(downloadButtonPressed()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_ignore,    SIGNAL(clicked()),  this, SLOT(accept()));

  XSqlQuery metric;
  metric.exec("SELECT fetchMetricText('ServerVersion') AS dbver,"
              "       fetchMetricBool('DisallowMismatchClientVersion') as disallowed,"
              "       fetchMetricBool('AutoVersionUpdate') as allow;");

  QString serverVersion;
  if(metric.first())
  {
    serverVersion = metric.value("dbver").toString();

    _label->setText(tr("Your client does not match the server version: %1. "
                       "Would you like to update?").arg(serverVersion));
    _ignore->setEnabled(!metric.value("disallowed").toBool());
    _ok->setEnabled(metric.value("allow").toBool());
  }
  else if (metric.lastError().type() != QSqlError::NoError) {
    QMessageBox::warning(parent, tr("System Error"),
                         tr("Could not find version information: %1")
                           .arg(metric.lastError().text()));
  }

  _private = new checkForUpdatesPrivate(this, serverVersion);
}

void checkForUpdates::downloadButtonPressed()
{
  // TODO: http://qt-project.org/doc/qt-5/qstandardpaths.html
  QUrl url(_private->urlString());
  QString filename = _private->_downloadFile;
  QString tempfile = QDir::tempPath() + QDir::separator() + filename;

  if (QFile::exists(tempfile))
  {
    QRegExp splitname("^(.*)\\.([^.]*$)");
    if (DEBUG) qDebug() << splitname.capturedTexts();
    if (splitname.indexIn(filename) == -1) {
      QMessageBox::information(this, tr("Download Failed"),
                               tr("Could not find extension on %1.")
                                 .arg(filename));
      return;
    }
    QString newname  = QDir::tempPath() + QDir::separator()
                     + splitname.cap(1) + "-%1." + splitname.cap(2);
    int i = 1;
    while (QFile::exists(newname.arg(i)))
      i++;
    tempfile = newname.arg(i);
  }
  if (DEBUG) qDebug() << "downloading to" << tempfile;

  file = new QFile(tempfile);
  if (!file->open(QIODevice::WriteOnly))
  {
    QMessageBox::information(this, tr("Download Failed"),
                             tr("Unable to save the file %1: %2.")
                               .arg(filename, file->errorString()));
    delete file;
    file = 0;
    return;
  }

  downloadRequestAborted = false;
  reply = manager.get(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()),  this, SLOT(downloadFinished()));
  connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
  connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

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
  if (DEBUG) qDebug() << "downloadFinished() entered";
  if(!downloadRequestAborted)
  {
    downloadReadyRead();
    file->flush();

    if(reply->error())
      QMessageBox::information(this, tr("Download Failed"),
                               tr("Failed: %1").arg(reply->errorString()));
    else
      startUpdate();
  }
  reply->deleteLater();
  reply = 0;
  progressDialog->hide();
  _ok->setEnabled(true);

  if(file)
  {
    file->close();
    file->remove();
    delete file;
    file = 0;
  }

  if (DEBUG) qDebug() << "downloadFinished() returning";
}

void checkForUpdates::startUpdate()
{
  if (DEBUG) qDebug() << "startUpdate() entered";

  if (file->exists())
  {
    QFileInfo   downloadinfo(*file);
    QStringList options;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QFileInfo destdirinfo(QDir::currentPath() + QDir::separator() + _private->_destdir);
    Sudoable sh;
    if (! destdirinfo.isWritable())
    {
      bool ok;
      _private->_password = sh.getPassword(&ok);
      if (! ok) {
        QMessageBox::information(this, tr("Download Saved"),
                                 tr("The new version has been saved to %1. "
                                    "Install it manually or delete it.")
                                 .arg(downloadinfo.absoluteFilePath()));
        reject();
      }
    }
    if (DEBUG) qDebug() << "empty pw?" << _private->_password.isEmpty();

#ifdef Q_OS_MAC
    // mount(2) gave errors trying to mount .dmg as block device
    sh.start(_private->_password,
             QString("hdiutil attach ") + downloadinfo.absoluteFilePath());
    if (! sh.waitForFinished())
    {
      QMessageBox::critical(this, tr("Mounting Error"),
                            tr("<p>Please open %1 and copy xtuple.app to %2.</p>"
                              "<p>Could not mount .dmg: %3</p>")
                              .arg(downloadinfo.absoluteFilePath())
                              .arg(destdirinfo.absoluteFilePath())
                              .arg(QString(sh.readAllStandardError())));
      reject();
      return;
    }

    QString output = sh.readAllStandardOutput();
    QRegExp regexp("(/dev/\\S+)\\s+Apple_HFS\\s+(\\S+)");
    if (DEBUG) qDebug() << regexp.capturedTexts();
    if (regexp.indexIn(output) == -1) {
      QMessageBox::critical(this, tr("Unpacking Error"),
                            tr("Could not find the .dmg mount point.dmg."));
      reject();
      return;
    }
    QString unpack = QString("cp -R ") + regexp.cap(2) + "/xtuple.app "
                   + destdirinfo.absoluteFilePath()
                   + "/xTuple-" + _private->_serverVersion + ".app";
#endif  // MAC
#ifdef Q_OS_LINUX
    QString unpack = QString("tar -xf %1 -C %2")
                            .arg(downloadinfo.absoluteFilePath())
                            .arg(destdirinfo.absoluteFilePath());
#endif // LINUX
    sh.start(_private->_password, unpack);
    if (! sh.waitForFinished())
    {
      sh.terminate();
      QMessageBox::critical(this, tr("Unpacking error"),
                            tr("Could not unpack %1 (%2): %3")
                              .arg(downloadinfo.absoluteFilePath())
                              .arg(sh.exitCode())
                              .arg(QString(sh.readAllStandardError())));
      reject();
      return;
    }
    if (DEBUG) qDebug() << "finished unpacking";
    sh.close();

#ifdef Q_OS_MAC
    sh.start(_private->_password, QString("hdiutil detach ") + regexp.cap(2));
    (void)sh.waitForFinished();
#endif

    QString errmsg;
    if (! _private->makeExecutable(_private->_newExePath, errmsg)) {
      QMessageBox::critical(this, tr("Permissions Error"),
                            tr("Couldn't set execute permissions on %1: %2")
                              .arg(_private->_newExePath)
                              .arg(errmsg));
      reject();
    }
#endif  // both MAC & LINUX
#ifdef Q_OS_WIN
    QString script("zipFile = \"%1\"\n"
                   "destDir = \"%2\"\n"
                   "set fs  = CreateObject(\"Scripting.FileSystemObject\")\n"
                   "set app = CreateObject(\"Shell.Application\")\n"
                   "set zipAsDir = app.NameSpace(zipFile)\n"
                   "if (zipAsDir is nothing) then\n"
                   "  WScript.echo(\"Could not open \" & zipFile)\n"
                   "else\n"
                   "  set destAsDir = app.NameSpace(destDir)\n"
                   "  if (destAsDir is nothing) then\n"
                   "    WScript.echo(\"Could not open \" & destDir)\n"
                   "  else\n"
                   "    destAsDir.CopyHere zipAsDir.Items(), 256\n"
                   "  end if\n"
                   "end if\n"
                  ); // copyHere: 256 => show progress dialog
    script = script.arg(QDir::toNativeSeparators(downloadinfo.absoluteFilePath()))
                   .arg(QDir::toNativeSeparators(QFileInfo(_private->_destdir).absoluteFilePath()));

    QString scriptName = QDir::tempPath() + QDir::separator() + "unzip.vbs";
    QFile   scriptFile(scriptName);
    if (DEBUG) qDebug() << scriptName << ":" << script;
    if (scriptFile.open(QIODevice::WriteOnly))
    {
      if (DEBUG) qDebug() << "writing to" << scriptName;
      scriptFile.write(script.toLatin1().data());
      scriptFile.close();

      QProcess scriptProc;
      if (DEBUG) qDebug() << "starting" << scriptName;
      scriptProc.start(QString("cscript %1").arg(scriptName));
      (void)scriptProc.waitForFinished();
      scriptFile.remove();
    }

    QString newExePath = QFileInfo(_private->_destdir).absoluteFilePath() +
                         "/" + _private->_newExePath;
    if (DEBUG) qDebug() << "looking for" << newExePath;
    if (! QFile::exists(newExePath))
    {
      QMessageBox::critical(this, tr("Could Not Unpack"),
                            tr("The file was downloaded but could not be "
                               "unpacked. Try installing %1 manually.")
                            .arg(downloadinfo.absoluteFilePath()));
      reject();
    }
#endif
    if (QProcess::startDetached(_private->_newExePath, options))
      reject();
  }
}

checkForUpdates::~checkForUpdates()
{
  if (progressDialog) {
    delete progressDialog;
    progressDialog = 0;
  }
  if (_private) {
    delete _private;
    _private = 0;
  }
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}

