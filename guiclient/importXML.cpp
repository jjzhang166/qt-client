/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "importXML.h"

#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QProcess>
#include <QSqlError>
#include <QTemporaryFile>
#include <QVariant>

#include "configureIE.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

bool importXML::userHasPriv()
{
  return _privileges->check("ImportXML");
}

void importXML::setVisible(bool visible)
{
  if (! visible)
    XWidget::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
                tr("You do not have sufficient privilege to view this window"),
                __FILE__, __LINE__);
    close();
  }
  else if (_metrics->value("XMLSuccessTreatment").isEmpty() ||
           _metrics->value("XSLTLibrary").isEmpty()) // not configured properly
  {
    if (! configureIE::userHasPriv())
    {
      systemError(this,
                  tr("The application is not set up to perform XML Import. "
                     "Have an administrator configure XML Import before "
                     "trying to import data."),
                  __FILE__, __LINE__);
      deleteLater();
    }
    else if (QMessageBox::question(this, tr("Setup required"),
                              tr("<p>You must set up the application to "
                                 "import XML data before trying to import "
                                 "data. Would you like to do this now?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes &&
             configureIE(this, "", true).exec() == XDialog::Accepted)
      XWidget::setVisible(true);
    else
      deleteLater();
  }
  else
    XWidget::setVisible(true);
}

importXML::importXML(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_add,            SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_autoUpdate,  SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_clearStatus,    SIGNAL(clicked()), this, SLOT(sClearStatus()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_file, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_importAll,      SIGNAL(clicked()), this, SLOT(sImportAll()));
  connect(_importSelected, SIGNAL(clicked()), this, SLOT(sImportSelected()));
  connect(_resetList,      SIGNAL(clicked()), this, SLOT(sFillList()));

  _file->addColumn(tr("File Name"),     -1, Qt::AlignLeft   );
  _file->addColumn(tr("Status"), _ynColumn, Qt::AlignCenter );

  _defaultXMLDir = _metrics->value(
#if defined Q_WS_MACX
                                      "XMLDefaultDirMac"
#elif defined Q_WS_WIN
                                      "XMLDefaultDirWindows"
#elif defined Q_WS_X11
                                      "XMLDefaultDirLinux"
#endif
      );
  if (_defaultXMLDir.isEmpty())
    _defaultXMLDir = ".";

  _defaultXSLTDir = _metrics->value(
#if defined Q_WS_MACX
                                      "XSLTDefaultDirMac"
#elif defined Q_WS_WIN
                                      "XSLTDefaultDirWindows"
#elif defined Q_WS_X11
                                      "XSLTDefaultDirLinux"
#endif
      );
  _externalCmd = _metrics->value(
#if defined Q_WS_MACX
                                      "XSLTProcessorMac"
#elif defined Q_WS_WIN
                                      "XSLTProcessorWindows"
#elif defined Q_WS_X11
                                      "XSLTProcessorLinux"
#endif
      );

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
}

importXML::~importXML()
{
  // no need to delete child widgets, Qt does it all for us
}

void importXML::languageChange()
{
  retranslateUi(this);
}

void importXML::sFillList()
{
  _file->clear();
  if (! _defaultXMLDir.isEmpty())
  {
    QStringList filters;
    filters << "*.xml" << "*.XML";
    QDirIterator iterator(_defaultXMLDir, filters);
    XTreeWidgetItem *last = 0;
    for (int i = 0; iterator.hasNext(); i++)
      last = new XTreeWidgetItem(_file, last, i, QVariant(iterator.next()));
  }
}

void importXML::sAdd()
{
  QFileDialog newdlg(this, tr("Select File(s) to Import"), _defaultXMLDir);
  newdlg.setFileMode(QFileDialog::ExistingFiles);
  QStringList filters;
  filters << tr("XML files (*.xml *.XML)") << tr("Any Files (*)");
  newdlg.setFilters(filters);
  if (newdlg.exec())
  {
    QStringList files = newdlg.selectedFiles();
    XTreeWidgetItem *last = 0;
    for (int i = 0; i < files.size(); i++)
      last = new XTreeWidgetItem(_file, last, i, QVariant(files[i]));
  }
}

void importXML::sClearStatus()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    selected[i]->setData(1, Qt::DisplayRole, tr(""));
}

void importXML::sDelete()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    _file->takeTopLevelItem(_file->indexOfTopLevelItem(selected[i]));
}

void importXML::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Import Selected"),  this, SLOT(sImportSelected()), 0);
  menuItem = pMenu->insertItem(tr("Clear Status"),     this, SLOT(sClearStatus()), 0);
  menuItem = pMenu->insertItem(tr("Delete From List"), this, SLOT(sDelete()), 0);
}

void importXML::sImportAll()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  for (int i = 0; i < _file->topLevelItemCount(); i++)
  {
    QTreeWidgetItem* pItem = _file->topLevelItem(i);
    if (pItem->data(1, Qt::DisplayRole).toString().isEmpty())
    {
      if (importOne(pItem->data(0, Qt::DisplayRole).toString()))
        pItem->setData(1, Qt::DisplayRole, tr("Done"));
      else
        pItem->setData(1, Qt::DisplayRole, tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

void importXML::sImportSelected()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (selected[i]->data(1, Qt::DisplayRole).toString().isEmpty())
    {
      if (importOne(selected[i]->data(0, Qt::DisplayRole).toString()))
        selected[i]->setData(1, Qt::DisplayRole, tr("Done"));
      else
        selected[i]->setData(1, Qt::DisplayRole, tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

bool importXML::openDomDocument(const QString &pFileName, QDomDocument &pDoc)
{
  QFile file(pFileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    systemError(this, tr("<p>Could not open file %1 (error %2)")
                      .arg(pFileName).arg(file.error()));
    return false;
  }

  QString errMsg;
  int errLine;
  int errColumn;
  if (!pDoc.setContent(&file, false, &errMsg, &errLine, &errColumn))
  {
    file.close();
    systemError(this, tr("Problem reading %1, line %2 column %3:<br>%4")
                      .arg(pFileName).arg(errLine).arg(errColumn).arg(errMsg));
    return false;
  }

  file.close();

  return true;
}

bool importXML::importOne(const QString &pFileName)
{

  QDomDocument doc(pFileName);
  if (!openDomDocument(pFileName, doc))
    return false;

  QString tmpfileName; // only set if we translate the file with XSLT
  if (DEBUG)
      qDebug("importXML::importOne(%s) doctype = %s",
             qPrintable(pFileName), qPrintable(doc.doctype().name()));
  if (doc.doctype().name() != "xtupleimport")
  {
    QString xsltfile;
    q.prepare("SELECT * FROM xsltmap "
              "WHERE ((xsltmap_doctype=:doctype OR xsltmap_doctype='')"
              "   AND (xsltmap_system=:system   OR xsltmap_system=''));");
    q.bindValue(":doctype", doc.doctype().name());
    q.bindValue(":system", doc.doctype().systemId());
    q.exec();
    if (q.first())
    {
      xsltfile = q.value("xsltmap_import").toString();
      //TODO: what if more than one row is found?
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
    else
    {
      systemError(this,
                  tr("<p>Could not find a map for doctype '%1' and "
                     "system id '%2'. "
                     "Write an XSLT stylesheet to convert this to valid xtuple"
                     "import XML and add it to the Map of XSLT Import Filters.")
                      .arg(doc.doctype().name()).arg(doc.doctype().systemId()));
      return false;
    }

    // TODO: switch to use ExportHelper::XSLTConvert()?
    QTemporaryFile tmpfile(_defaultXMLDir + QDir::separator() +
                           doc.doctype().name() + "TOxtupleimport");
    tmpfile.setAutoRemove(false);
    if (! tmpfile.open())
    {
      systemError(this, tr("<p>Could not create a temporary file."));
      return false;
    }
    tmpfileName = tmpfile.fileName();
    tmpfile.close();

    if (_metrics->boolean("XSLTLibrary"))
    {
      systemError(this, "XSLT via internal library not yet supported");
      return false;
    }
    else
    {
      QStringList args = _externalCmd.split(" ", QString::SkipEmptyParts);
      QString command = args[0];
      args.removeFirst();
      args.replaceInStrings("%f", pFileName);
      if (QFile::exists(xsltfile))
        args.replaceInStrings("%x", xsltfile);
      else if (QFile::exists(_defaultXSLTDir + QDir::separator() + xsltfile))
        args.replaceInStrings("%x", _defaultXSLTDir + QDir::separator() + xsltfile);
      else
      {
        systemError(this, tr("Cannot find the XSLT file as either %1 or %2")
                          .arg(xsltfile)
                          .arg(_defaultXSLTDir + QDir::separator() + xsltfile));
        return false;
      }

      QProcess xslt(this);
      xslt.setStandardOutputFile(tmpfileName);
      xslt.start(command, args);
      QString commandline = command + " " + args.join(" ");
      QString errOutput;
      /* TODO: make the entire file-processing asynchronous
         this will keep the UI snappy and handle spurious errors
         like the occasional waitForFinished failure if the processing
         runs faster than expected.
       */
      if (! xslt.waitForStarted())
        errOutput = tr("Error starting XSLT Processing: %1\n%2")
                          .arg(commandline)
                          .arg(QString(xslt.readAllStandardError()));
      if (! xslt.waitForFinished())
        errOutput = tr("The XSLT Processor encountered an error: %1\n%2")
                          .arg(commandline)
                          .arg(QString(xslt.readAllStandardError()));
      if (xslt.exitStatus() !=  QProcess::NormalExit)
        errOutput = tr("The XSLT Processor did not exit normally: %1\n%2")
                          .arg(commandline)
                          .arg(QString(xslt.readAllStandardError()));
      if (xslt.exitCode() != 0)
        errOutput = tr("The XSLT Processor returned an error code: %1\nreturned %2\n%3")
                          .arg(commandline)
                          .arg(xslt.exitCode())
                          .arg(QString(xslt.readAllStandardError()));

      if (! errOutput.isEmpty())
      {
        systemError(this, errOutput);
        return false;
      }

      if (! openDomDocument(tmpfileName, doc))
        return false;
    }
  }

  /* xtupleimport format is very straightforward:
      top level element is xtupleimport
        second level elements are all table/view names (default to api schema)
          third level elements are all column names
     and there are no text nodes until third level

     wrap the import of an entire file in a single transaction so
     we can reimport files which have failures. however, if a
     view-level element has the ignore attribute set to true then
     rollback just that view-level element if it generates an error.
  */

  // the silent attribute provides the user the option to turn off 
  // the interactive message for the view-level element


  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  for (QDomElement viewElem = doc.documentElement().firstChildElement();
       ! viewElem.isNull();
       viewElem = viewElem.nextSiblingElement())
  {
    QStringList columnNameList;
    QStringList columnValueList;

    bool ignoreErr = (viewElem.attribute("ignore", "false").isEmpty() ||
                      viewElem.attribute("ignore", "false") == "true");

    bool silent = (viewElem.attribute("silent", "false").isEmpty() ||
                   viewElem.attribute("silent", "false") == "true");

    QString mode = viewElem.attribute("mode", "insert");
    QStringList keyList;
    if (! viewElem.attribute("key").isEmpty())
      keyList = viewElem.attribute("key").split(QRegExp(",\\s*"));

    QString viewName = viewElem.tagName();
    if (viewName.indexOf(".") > 0)
      ; // viewName contains . so accept that it's schema-qualified
    else if (! viewElem.attribute("schema").isEmpty())
      viewName = viewElem.attribute("schema") + "." + viewName;
    else // backwards compatibility - must be in the api schema
      viewName = "api." + viewName;

    // TODO: fix QtXML classes so they read default attribute values from the DTD
    // then remove this code
    if (mode.isEmpty())
      mode = "insert";
    else if (mode == "update" && keyList.isEmpty())
    {
      if (! viewElem.namedItem(viewName + "_number").isNull())
        keyList.append(viewName + "_number");
      else if (! viewElem.namedItem("order_number").isNull())
        keyList.append("order_number");
      else if (! ignoreErr)
      {
        rollback.exec();
        systemError(this, tr("Cannot process %1 element without a key attribute"));
        return false;
      }
      if (! viewElem.namedItem("line_number").isNull())
        keyList.append("line_number");
    }
    // end of code to remove

    QString savepointName = viewName;
    savepointName.remove(".");
    if (ignoreErr)
      q.exec("SAVEPOINT " + savepointName + ";");

    QRegExp apos("\\\\*'");

    for (QDomElement columnElem = viewElem.firstChildElement();
         ! columnElem.isNull();
         columnElem = columnElem.nextSiblingElement())
    {
      QString value = columnElem.attribute("value").isEmpty() ?
                              columnElem.text() : columnElem.attribute("value");

      columnNameList.append(columnElem.tagName());

      if (value.trimmed() == "[NULL]")
        columnValueList.append("NULL");
      else if (value.trimmed().startsWith("SELECT"))
        columnValueList.append("(" + value.trimmed() + ")");
      else if (columnElem.attribute("quote") == "false")
        columnValueList.append(value);
      else
        columnValueList.append("'" + value.replace(apos, "''") + "'");
    }

    QString sql;
    if (mode == "update")
    {
      QStringList whereList;
      for (int i = 0; i < keyList.size(); i++)
        whereList.append("(" + keyList[i] + "=" +
                         columnValueList[columnNameList.indexOf(keyList[i])] + ")");

      for (int i = 0; i < columnNameList.size(); i++)
        columnNameList[i].append("=" + columnValueList[i]);

      sql = "UPDATE " + viewName + " SET " +
            columnNameList.join(", ") +
            " WHERE (" + whereList.join(" AND ") + ");";
    }
    else if (mode == "insert")
      sql = "INSERT INTO " + viewName + " (" +
            columnNameList.join(", ") +
            " ) SELECT " +
            columnValueList.join(", ") + ";" ;
    else
    {
      if (ignoreErr)
        q.exec("ROLLBACK TO SAVEPOINT " + savepointName + ";");
      else
      {
        rollback.exec();
        systemError(this, tr("Could not process %1: invalid mode %2")
                            .arg(viewElem.tagName()).arg(mode));
        return false;
      }
    }

    if (DEBUG) qDebug("About to run this: %s", qPrintable(sql));
    q.exec(sql);
    if (q.lastError().type() != QSqlError::NoError)
    {
      if (ignoreErr)
      {
        QString warning = q.lastError().databaseText();
        q.exec("ROLLBACK TO SAVEPOINT " + savepointName + ";");
        if (! silent)
        {
          QMessageBox::warning(this, tr("Ignoring Error"),
                             tr("Ignoring database error while importing %1:\n\n%2")
                              .arg(viewElem.tagName())
                              .arg(warning));
        }                  
      }
      else
      {
        rollback.exec();
        systemError(this, tr("Error importing %1 %2\n\n").arg(pFileName).arg(tmpfileName) + q.lastError().databaseText(), __FILE__, __LINE__);
        return false;
      }
    }
    else if (ignoreErr)
      q.exec("RELEASE SAVEPOINT " + savepointName + ";");
  }

  q.exec("COMMIT;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  QFile file(pFileName);
  if (_metrics->value("XMLSuccessTreatment") == "Delete")
  {
    if (! file.remove())
    {
      systemError(this, tr("Could not remove %1 after successful processing (%2).")
                        .arg(pFileName).arg(file.error()));
      return false;
    }
  }
  else if (_metrics->value("XMLSuccessTreatment") == "Rename")
  {
    QString suffix = _metrics->value("XMLSuccessSuffix");
    if (suffix.isEmpty())
      suffix = ".done";

    QString newname = pFileName + suffix;
    for (int i = 0; QFile::exists(newname) ; i++)
      newname = pFileName + suffix + "." + QString::number(i);

    if (! file.rename(newname))
    {
      systemError(this, tr("Could not rename %1 to %2 after successful processing (%3).")
                        .arg(pFileName).arg(file.error()));
      return false;
    }
  }
  else if (_metrics->value("XMLSuccessTreatment") == "Move")
  {
    QString donedirName = _metrics->value("XMLSuccessDir");
    if (donedirName.isEmpty())
      donedirName = "done";
    if (QDir::isRelativePath(donedirName))
      donedirName = _defaultXMLDir + QDir::separator() + donedirName;

    QDir donedir(donedirName);
    if (! donedir.exists())
      donedir.mkpath(donedirName);

    QString newname = donedirName + QDir::separator() + QFileInfo(file).fileName(); 
    if (QFile::exists(newname))
      newname = newname + QDate::currentDate().toString(".yyyy.MM.dd");
    if (QFile::exists(newname))
      newname = newname + QDateTime::currentDateTime().toString(".hh.mm");
    if (QFile::exists(newname))
      newname = newname + QDateTime::currentDateTime().toString(".ss");

    if (! file.rename(newname))
    {
      systemError(this, tr("<p>Could not move %1 to %2 after successful processing (%3).")
                        .arg(pFileName).arg(newname).arg(file.error()));
      return false;
    }
  }

  // else if (_metrics->value("XMLSuccessTreatment") == "None") {}

  return true;
}

void importXML::sHandleAutoUpdate(const bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}
