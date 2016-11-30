/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "atlasMap.h"

#include <QAbstractMessageHandler>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QXmlQuery>

#include "storedProcErrorLookup.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

#define DEBUG false

// TODO: replace with XMessageBoxMessageHandler
class AtlasMapMessageHandler : public QAbstractMessageHandler
{
  public:
    AtlasMapMessageHandler(QObject *p = 0) : QAbstractMessageHandler(p)
    {
    }

  protected:
    virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
    {
      QString msg = atlasMap::tr("<p>There was a problem reading the Atlas file, "
                       "line %1 column %2: %3<br>(%4)")
                        .arg(sourceLocation.line())
                        .arg(sourceLocation.column())
                        .arg(description)
                        .arg(identifier.toString());
      switch (type)
      {
        case QtDebugMsg:
          QMessageBox::information(0, atlasMap::tr("XML Error"), msg);
          break;
        case QtWarningMsg:
          QMessageBox::warning(0, atlasMap::tr("XML Error"), msg);
          break;
        case QtCriticalMsg:
        case QtFatalMsg:
        default:
          QMessageBox::critical(0, atlasMap::tr("XML Error"), msg);
          break;
      }
    }
};

bool atlasMap::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

int atlasMap::exec()
{
  if (userHasPriv())
    return XDialog::exec();
  else
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Occurred"),
                         tr("%1: Insufficient privileges to view this window")
                         .arg(windowTitle()),__FILE__,__LINE__);
    return XDialog::Rejected;
  }
}

atlasMap::atlasMap(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_atlas, SIGNAL(editingFinished()), this, SLOT(sHandleAtlas()));
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  QString filter = tr("Atlas Files (*.xml)");
  _atlas->setFilter(filter);

  _filtertype->append(0, tr("File name"),          "filename");
  _filtertype->append(1, tr("First line of file"), "firstline");

  _mode         = cNew;
  _atlasmapId   = -1;
  _msghandler   = new AtlasMapMessageHandler(this);
#if defined Q_OS_MAC
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirMac");
#elif defined Q_OS_WIN
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirWindows");
#elif defined Q_OS_LINUX
  _defaultDir   = _metrics->value("CSVAtlasDefaultDirLinux");
#endif
}

atlasMap::~atlasMap()
{
  // no need to delete child widgets, Qt does it all for us
}

void atlasMap::languageChange()
{
  retranslateUi(this);
}

enum SetResponse atlasMap::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("defaultDir", &valid);
  if (valid)
    _defaultDir = param.toString();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(false);
      _filter->setEnabled(false);
      _filtertype->setEnabled(false);
      _atlas->setEnabled(false);
      _map->setEnabled(false);
      _firstLine->setEnabled(false);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }
  }

  param = pParams.value("atlasmap_id", &valid);
  if (valid)
  {
    _atlasmapId = param.toInt();
    sPopulate();
  }

  return NoError;
}

void atlasMap::sSave()
{

  QList<GuiErrorCheck>errors;
  errors<<GuiErrorCheck(_name->text().trimmed().isEmpty(), _name,
                        tr("<p>Please enter a name for this Atlas Map before saving it."))
        <<GuiErrorCheck(_filter->text().trimmed().isEmpty(), _filter,
                         tr("<p>Please enter a filter before saving this Atlas Map."))
        <<GuiErrorCheck(_filtertype->currentText().trimmed().isEmpty(), _filtertype,
                        tr("<p>Please select a filter type before saving this Atlas Map."))
        <<GuiErrorCheck(_atlas->text().trimmed().isEmpty(), _atlas,
                        tr("<p>Please enter an Atlas File Name before saving this Atlas Map."))
        <<GuiErrorCheck(_map->id() <= -1, _map,
                       tr("Please select a Map Name before saving this Atlas Map."));

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Save Atlas Map"),errors))
      return;

  XSqlQuery dupq;
  dupq.prepare( "SELECT atlasmap_name "
             "FROM atlasmap "
             "WHERE ((atlasmap_name=:name)"
             " AND   (atlasmap_id<>:atlasmap_id) );" );

  dupq.bindValue(":atlasmap_id",   _atlasmapId);
  dupq.bindValue(":name",          _name->text());
  dupq.exec();
  if (dupq.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Atlas Map"),
                          tr("<p>This Name is already in use by another "
                             "Atlas Map."));
    _name->setFocus();
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Atlas Map Information"),
                                dupq, __FILE__, __LINE__))
  {
    return;
  }

  XSqlQuery upsq;
  if (cNew == _mode)
    upsq.prepare("INSERT INTO atlasmap ("
                 "    atlasmap_name,  atlasmap_filter, atlasmap_filtertype,"
                 "    atlasmap_atlas, atlasmap_map,    atlasmap_headerline"
                 ") VALUES ("
                 "    :name,  :filter, :filtertype,"
                 "    :atlas, :map,    :headerline"
                 ") RETURNING atlasmap_id;");
  else if (cEdit == _mode)
  {
    upsq.prepare("UPDATE atlasmap SET"
                 "    atlasmap_name=:name,"
                 "    atlasmap_filter=:filter,"
                 "    atlasmap_filtertype=:filtertype,"
                 "    atlasmap_atlas=:atlas,"
                 "    atlasmap_map=:map,"
                 "    atlasmap_headerline=:headerline"
                 " WHERE (atlasmap_id=:id)"
                 " RETURNING atlasmap_id;");
    upsq.bindValue(":id", _atlasmapId);
  }

  upsq.bindValue(":name",       _name->text());
  upsq.bindValue(":filter",     _filter->text());
  upsq.bindValue(":filtertype", _filtertype->code());
  upsq.bindValue(":atlas",      _atlas->text());
  upsq.bindValue(":map",        _map->currentText());
  upsq.bindValue(":headerline", _firstLine->isChecked());
  upsq.exec();
  if (upsq.first())
    _atlasmapId = upsq.value("atlasmap_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Atlas Map Information"),
                                upsq, __FILE__, __LINE__))
  {
    return;
  }

  accept();
}

void atlasMap::sPopulate()
{
  XSqlQuery atlasPopulate;
  if (_atlasmapId <= 0)
  {
    _name->clear();
    _filter->clear();
    _filtertype->setId(-1);
    _atlas->clear();
    _map->setId(-1);
    _firstLine->setChecked(false);
  }
  else
  {
    atlasPopulate.prepare("SELECT * FROM atlasmap WHERE (atlasmap_id=:id);");
    atlasPopulate.bindValue(":id", _atlasmapId);
    atlasPopulate.exec();
    if (atlasPopulate.first())
    {
      _name->setText(atlasPopulate.value("atlasmap_name").toString());
      _filter->setText(atlasPopulate.value("atlasmap_filter").toString());
      _filtertype->setCode(atlasPopulate.value("atlasmap_filtertype").toString());
      _atlas->setText(atlasPopulate.value("atlasmap_atlas").toString());
      sHandleAtlas();
      if (! atlasPopulate.value("atlasmap_map").toString().isEmpty())
        _map->setCode(atlasPopulate.value("atlasmap_map").toString());
      _firstLine->setChecked(atlasPopulate.value("atlasmap_headerline").toBool());
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Atlas Map Information"),
                                  atlasPopulate, __FILE__, __LINE__))
    {
      return;
    }
  }
}

void atlasMap::sHandleAtlas()
{
  _map->clear();

  if (_atlas->text().isEmpty())
    return;

  if (DEBUG)
    qDebug("atlasMap::sHandleAtlas() entered with %s and %s",
           qPrintable(_atlas->text()), qPrintable(_defaultDir));

  if (! _defaultDir.isEmpty() && _atlas->text().startsWith(_defaultDir))
    _atlas->setText(_atlas->text().remove(0, _defaultDir.length() + 1));

  QFile atlasfile;
  if (QFile::exists(_atlas->text()))
    atlasfile.setFileName(_atlas->text());
  else if (QFile::exists(_defaultDir + QDir::separator() + _atlas->text()))
    atlasfile.setFileName(_defaultDir + QDir::separator() + _atlas->text());
  else
  {
    QMessageBox::warning(this, tr("Could not find Atlas"),
                         tr("<p>Could not find the Atlas file to open to look "
                            "for CSV import Maps."));
    return;
  }

  if (! atlasfile.open(QIODevice::ReadOnly))
  {
    QMessageBox::critical(this, tr("Could not open Atlas"),
                          tr("<p>Could not open the Atlas file %1 (error %2).")
                          .arg(atlasfile.fileName(), atlasfile.errorString()));
    return;
  }

  QXmlQuery mapq;
  mapq.setMessageHandler(_msghandler);

  if (! mapq.setFocus(&atlasfile))
  {
    QMessageBox::critical(this, tr("No Focus"),
                          tr("<p>Could not set focus on the Atlas %1")
                          .arg(atlasfile.fileName()));
    return;
  }

  // string() at the end tells the query to generate a sequence of values
  mapq.setQuery("/CSVAtlas/CSVMap/Name/text()/string()");
  if (! mapq.isValid())
  {
    QMessageBox::critical(this, tr("Invalid Query"),
                          tr("<p>The query is not valid for some reason"));
    return;
  }

  QStringList maplist;
  if (! mapq.evaluateTo(&maplist))
  {
    QMessageBox::warning(this, tr("No Maps"),
                         tr("<p>Could not find any Maps in the Atlas %1")
                         .arg(atlasfile.fileName()));
    return;
  }
  else
    for (int i = 0; i < maplist.size(); i++)
      _map->append(i, maplist.at(i));
}
