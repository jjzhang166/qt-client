/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>
#include "mqlutil.h"

#include "documents.h"
#include "errorReporter.h"
#include "imageview.h"
#include "imageAssignment.h"
#include "docAttach.h"

QMap<QString, struct DocumentMap*> Documents::_strMap;
QMap<int,     struct DocumentMap*> Documents::_intMap;

/** Add another document type to the map by both key and int.

    Actually there are two parallel maps, one by DocumentSources/integer
    and the other by string key. This simplifies lookups and provides
    backward compatibility with existing UI file definitions.

    @param id    A DocumentSources enum value or a unique extension-supplied int
    @param key   A human-readable abbreviation for the document type
    @param trans A human-readable, translatable term for the document type
    @param param The name of the parameter a window checks for the record id
    @param ui    The application or extension window to create a new record
    @param priv  The privileges required to create a new record

    @return true on success, false if this entry would create a duplicate
            on either id or key.
 */
bool Documents::addToMap(int id,        QString key, QString trans,
                         QString param, QString ui,  QString priv)
{
  if (_strMap.contains(key) || _intMap.contains(id)) {
    qDebug() << "Documents::addToMap(" << id << ", " << key << ") duplicate!";
    return false;
  }

  DocumentMap *entry = new DocumentMap(id, key, trans, param, ui, priv);
  _strMap.insert(key, entry);
  _intMap.insert(id,  entry);
  return true;
}

// Inconsistencies between here and the rest of the app: S? Q?
QMap<QString, struct DocumentMap *> &Documents::documentMap() {
  if (! _x_privileges)
    return _strMap;

  if (_strMap.isEmpty()) {
    XSqlQuery q("SELECT * FROM source;");
    addToMap(Uninitialized,     "",      tr("[Pick a Document Type]")                           );
    while (q.next()) {
      addToMap(q.value("source_docass_num").toInt(),
               q.value("source_docass").toString(),
               tr(q.value("source_descrip").toString().toLatin1().data()),
               q.value("source_key_param").toString(),
               q.value("source_uiform_name").toString(),
               q.value("source_create_priv").toString());
    }
    ErrorReporter::error(QtCriticalMsg, 0, tr("Error Getting Document Types"),
                         q, __FILE__, __LINE__);
  }

  return _strMap;
}

GuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);

  _sourceid = -1;
  _readOnly = false;
  if (_strMap.isEmpty()) {
    (void)documentMap();
  }

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "target_type" );
  _doc->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "target_number" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  _itemColumn, Qt::AlignLeft,true, "purpose");
  _doc->addColumn(tr("Can View"), _ynColumn, Qt::AlignCenter, false, "canview");
  _doc->addColumn(tr("Can Edit"), _ynColumn, Qt::AlignCenter, false, "canedit");
  
  connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));
  connect(_doc, SIGNAL(valid(bool)), this, SLOT(handleSelection()));
  connect(_doc, SIGNAL(itemSelected(int)), this, SLOT(handleItemSelected()));
  handleSelection();

  if (_x_privileges)
  {
    QMenu * newDocMenu = new QMenu;

    QAction* imgAct = new QAction(tr("Image"), this);
    imgAct->setEnabled(_x_privileges->check("MaintainImages"));
    connect(imgAct, SIGNAL(triggered()), this, SLOT(sNewImage()));
    newDocMenu->addAction(imgAct);

    foreach (DocumentMap *value, _strMap) {
      if (! value->newPriv.isEmpty()) {
        QAction* act = new QAction(value->translation, this);
        QStringList data;
        data << value->doctypeStr << value->uiname;
        act->setEnabled(_x_privileges->check(value->newPriv));
        act->setData(data);
        connect(act, SIGNAL(triggered()), this, SLOT(sNewDoc()));
        newDocMenu->addAction(act);
      }
    }

    _newDoc->setMenu(newDocMenu);
  }
}

int Documents::type() const
{
  DocumentMap *elem = _strMap.value(_sourcetype);
  return elem ? elem->doctypeId : Uninitialized;
}

void Documents::setType(int sourceType)
{
  DocumentMap *elem = _intMap.value(sourceType);
  setType(elem ? elem->doctypeStr : "");
}

void Documents::setType(QString sourceType)
{
  _sourcetype = sourceType;
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{
  _readOnly = pReadOnly;

  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  handleSelection();
}

void Documents::sNewDoc(QString ptype, QString pui)
{
  QString  type = ptype;
  QString  ui   = pui;
  QAction *act = qobject_cast<QAction*>(sender());
  if (act) {
    QStringList elem = act->data().toStringList();
    type = elem.at(0);
    ui   = elem.at(1);
  }

  ParameterList params;
  params.append("mode", "new");
  int target_id;
  QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openWindow(ui, params, parentWidget(),Qt::WindowModal, Qt::Dialog));
  target_id = newdlg->exec();
  if (target_id != QDialog::Rejected) {
    sInsertDocass(type, target_id);
  }
  refresh();
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sInsertDocass(QString target_type, int target_id)
{
  XSqlQuery ins;
  ins.prepare("INSERT INTO docass ( docass_source_id, docass_source_type, docass_target_id, docass_target_type )"
            "  VALUES ( :sourceid, :sourcetype::text, :targetid, :targettype); ");
  ins.bindValue(":sourceid", _sourceid);
  ins.bindValue(":sourcetype", _sourcetype);
  ins.bindValue(":targetid", target_id);
  ins.bindValue(":targettype", target_type);
  ins.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Attachment Error"),
                       ins, __FILE__, __LINE__);
}

void Documents::sEditDoc()
{
  sOpenDoc("edit");
}

void Documents::sOpenDoc(QString mode)
{
  QString ui;
  QString docType = _doc->currentItem()->rawValue("target_type").toString();
  int targetid = _doc->currentItem()->id("target_number");
  ParameterList params;
  if (docType == "Q" && mode == "view")
    params.append("mode", "viewQuote");
  else if (docType == "Q" && mode == "edit")
    params.append("mode", "editQuote");
  else
    params.append("mode", mode);

  // TODO: image -- change to use docass instead of imageass
  if (docType == "IMG")
  {
    XSqlQuery img;
    img.prepare("SELECT imageass_image_id "
                "FROM imageass "
                "WHERE (imageass_id=:imageass_id); ");
    img.bindValue(":imageass_id", _doc->id());
    img.exec();
    img.first();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Image Info"),
                             img, __FILE__, __LINE__))
      return;

    params.append("image_id", img.value("imageass_image_id").toInt());
    imageview newdlg(this, "", true);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
      refresh();
    return;
  }
  // TODO: url -- change to use docass instead of url
  // TODO: separate URL from FILE handling and replace use of url view
  else if (docType == "URL" || docType == "FILE")
  {
    if (mode == "edit")
    {
      ParameterList params;
      params.append("url_id", targetid);

      docAttach newdlg(this, "", true);
      newdlg.set(params);
      newdlg.exec();

      refresh();
      return;
    }

    XSqlQuery qfile;
    qfile.prepare("SELECT url_id, url_source_id, url_source, url_title, url_url, url_stream"
                  " FROM url"
                  " WHERE (url_id=:url_id);");

    qfile.bindValue(":url_id", _doc->id());
    qfile.exec();

    // If file is in the database, copy to a temp. directory in the file system and open it.
    if (qfile.first() && (docType == "FILE"))
    {
      QFileInfo fi( qfile.value("url_url").toString() );
      QDir tdir;
      // TODO: QDesktopServices::openUrl(urldb) on windows does not open files
      // containing spaces. why not?
#ifdef Q_OS_WIN
      QString fileName = fi.fileName().remove(" ");
#else
      QString fileName = fi.fileName();
#endif
      QString filePath = tdir.tempPath() + "/xtTempDoc/" +
	                 qfile.value("url_id").toString() + "/";
      QFile tfile(filePath + fileName);

      // Remove any previous watches
      if (_guiClientInterface)
        _guiClientInterface->removeDocumentWatch(tfile.fileName());

      if (! tdir.exists(filePath))
        tdir.mkpath(filePath);

      if (!tfile.open(QIODevice::WriteOnly))
      {
        QMessageBox::warning( this, tr("File Open Error"),
                             tr("Could Not Create File %1.").arg(tfile.fileName()) );
        return;
      }
      tfile.write(qfile.value("url_stream").toByteArray());
      QUrl urldb;
      urldb.setUrl(tfile.fileName());
#ifndef Q_OS_WIN
      urldb.setScheme("file");
#endif
      tfile.close();
      if (! QDesktopServices::openUrl(urldb))
      {
        QMessageBox::warning(this, tr("File Open Error"),
			     tr("Could not open %1.").arg(urldb.toString()));
	return;
      }

      // Add a watch to the file that will save any changes made to the file back to the database.
      if (_guiClientInterface && !_readOnly) // TODO: only if NOT read-only
        _guiClientInterface->addDocumentWatch(tfile.fileName(),qfile.value("url_id").toInt());
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this,
                                  tr("Error Getting Assignment"),
                                  qfile, __FILE__, __LINE__))
      return;
    else
    {
      QUrl url(_doc->currentItem()->rawValue("description").toString());
      if (url.scheme().isEmpty())
        url.setScheme("file");
      QDesktopServices::openUrl(url);
      return;
    }
  }
  else
  {
    struct DocumentMap *elem = _strMap.value(docType);
    if (! elem) {
      QMessageBox::critical(this, tr("Error"),
                            tr("Unknown document type %1").arg(docType));
      return;
    }

    params.append(elem->idParam, targetid);
    ui = elem->uiname;
  }

  QWidget* w = 0;
  if (parentWidget()->window())
  {
    if (parentWidget()->window()->isModal())
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window(),
                                          Qt::WindowModal, Qt::Dialog);
    else
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window(),
                                          Qt::NonModal, Qt::Window);
  }

  if (w && w->inherits("QDialog"))
  {
    QDialog* newdlg = qobject_cast<QDialog*>(w);
    newdlg->exec();
  }

  refresh();
}

void Documents::sViewDoc()
{
  sOpenDoc("view");
}

void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("sourceType", _sourcetype);
  params.append("source_id", _sourceid);

  docAttach newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();

  refresh();
}

void Documents::sDetachDoc()
{
  XSqlQuery detach;
  if (_doc->id() < 0)
    return;

  if(QMessageBox::question( this, tr("Confirm Detach"),
       tr("<p>You have requested to detach the selected document."
          " In some cases this may permanently remove the document from the system.</p>"
          "<p>Are you sure you want to continue?</p>"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
    return; // user doesn't want to continue so get out of here

  if ( _doc->currentItem()->rawValue("target_type") == "IMG" )
  {
    detach.prepare( "DELETE FROM imageass "
                    "WHERE (imageass_id = :docid );" );
  }
  else if ( _doc->currentItem()->rawValue("target_type") == "URL" ||
            _doc->currentItem()->rawValue("target_type") == "FILE")
  {
    detach.prepare( "DELETE FROM url "
                    "WHERE (url_id = :docid );" );
  }
  else
  {
    detach.prepare( "DELETE FROM docass "
                    "WHERE (docass_id = :docid );" );
  }
  detach.bindValue(":docid", _doc->id());
  detach.exec();
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Detaching"),
                       detach, __FILE__, __LINE__);
  refresh();
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _doc->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate doc list
  QString sql("SELECT id, target_number, target_type, "
              " target_id AS target_number_xtidrole, source_type, source_id, purpose, "
              " name, description, "
              " hasPrivOnObject('view', target_type, target_id) AS canview,"
              " hasPrivOnObject('edit', target_type, target_id) AS canedit,"
              " CASE WHEN (purpose='I') THEN :inventory"
              " WHEN (purpose='P') THEN :product"
              " WHEN (purpose='E') THEN :engineering"
              " WHEN (purpose='M') THEN :misc"
              " WHEN (purpose='A') THEN :child"
              " WHEN (purpose='C') THEN :parent"
              " WHEN (purpose='S') THEN :sibling"
              " WHEN (purpose='D') THEN :dupe"
              " ELSE :other"
              " END AS purpose_qtdisplayrole, "
              " CASE "
              " WHEN (target_type='ADDR') THEN :address "
              " WHEN (target_type='BBH') THEN :bbomhead "
              " WHEN (target_type='BBI') THEN :bbomitem "
              " WHEN (target_type='BMH') THEN :bomhead "
              " WHEN (target_type='BMI') THEN :bomitem "
              " WHEN (target_type='BOH') THEN :boohead "
              " WHEN (target_type='BOI') THEN :booitem "
              " WHEN (target_type='CRMA') THEN :crma "
              " WHEN (target_type='T') THEN :contact "
              " WHEN (target_type='CNTR') THEN :contract "
              " WHEN (target_type='CM') THEN :creditmemo "
              " WHEN (target_type='CMI') THEN :creditmemoitem "
              " WHEN (target_type='C') THEN :cust "
              " WHEN (target_type='EMP') THEN :emp "
              " WHEN (target_type='INCDT') THEN :incident "
              " WHEN (target_type='INV') THEN :invoice "
              " WHEN (target_type='INVI') THEN :invoiceitem "
              " WHEN (target_type='I') THEN :item "
              " WHEN (target_type='IS') THEN :itemsite "
              " WHEN (target_type='IR') THEN :itemsrc "
              " WHEN (target_type='L') THEN :location "
              " WHEN (target_type='LS') THEN :lotserial "
              " WHEN (target_type='OPP') THEN :opp "
              " WHEN (target_type='J') THEN :project "
              " WHEN (target_type='P') THEN :po "
              " WHEN (target_type='PI') THEN :poitem "
              " WHEN (target_type='RA') THEN :ra "
              " WHEN (target_type='RI') THEN :raitem "
              " WHEN (target_type='Q') THEN :quote "
              " WHEN (target_type='QI') THEN :quoteitem "
              " WHEN (target_type='S') THEN :so "
              " WHEN (target_type='SI') THEN :soitem "
              " WHEN (target_type='SHP') THEN :shipto "
              " WHEN (target_type='TE') THEN :timeexpense "
              " WHEN (target_type='TODO') THEN :todo "
              " WHEN (target_type='TO') THEN :to "
              " WHEN (target_type='TI') THEN :toitem "
              " WHEN (target_type='V') THEN :vendor "
              " WHEN (target_type='VCH') THEN :voucher "
              " WHEN (target_type='WH') THEN :whse "
              " WHEN (target_type='W') THEN :wo "
              " WHEN (target_type='TASK') THEN :projecttask "
              " WHEN (target_type='URL') THEN :url "
              " WHEN (target_type='FILE') THEN :file "
              " WHEN (target_type='IMG') THEN :image "
              " ELSE NULL "
              " END AS target_type_qtdisplayrole "
              "FROM docinfo "
              "WHERE ((source_type=:source) "
              " AND (source_id=:sourceid)) "
              "ORDER by target_type_qtdisplayrole, target_number; ");
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));

  query.bindValue(":parent", tr("Parent"));
  query.bindValue(":child", tr("Child"));
  query.bindValue(":sibling", tr("Related to"));
  query.bindValue(":dupe", tr("Duplicate of"));
  query.bindValue(":other", tr("Other"));

  query.bindValue(":address",        _strMap.value("ADDR")->translation);
  query.bindValue(":bbomhead",       _strMap.value("BBH")->translation);
  query.bindValue(":bbomitem",       _strMap.value("BBI")->translation);
  query.bindValue(":bomhead",        _strMap.value("BMH")->translation);
  query.bindValue(":bomitem",        _strMap.value("BMI")->translation);
  query.bindValue(":boohead",        _strMap.value("BOH")->translation);
  query.bindValue(":booitem",        _strMap.value("BOI")->translation);
  query.bindValue(":crma",           _strMap.value("CRMA")->translation);
  query.bindValue(":contact",        _strMap.value("T")->translation);
  query.bindValue(":contract",       _strMap.value("CNTR")->translation);
  query.bindValue(":creditmemo",     _strMap.value("CM")->translation);
  query.bindValue(":creditmemoitem", _strMap.value("CMI")->translation);
  query.bindValue(":cust",           _strMap.value("C")->translation);
  query.bindValue(":emp",            _strMap.value("EMP")->translation);
  query.bindValue(":incident",       _strMap.value("INCDT")->translation);
  query.bindValue(":invoice",        _strMap.value("INV")->translation);
  query.bindValue(":invoiceitem",    _strMap.value("INVI")->translation);
  query.bindValue(":item",           _strMap.value("I")->translation);
  query.bindValue(":itemsite",       _strMap.value("IS")->translation);
  query.bindValue(":itemsrc",        _strMap.value("IR")->translation);
  query.bindValue(":location",       _strMap.value("L")->translation);
  query.bindValue(":lotserial",      _strMap.value("LS")->translation);
  query.bindValue(":opp",            _strMap.value("OPP")->translation);
  query.bindValue(":project",        _strMap.value("J")->translation);
  query.bindValue(":po",             _strMap.value("P")->translation);
  query.bindValue(":poitem",         _strMap.value("PI")->translation);
  query.bindValue(":ra",             _strMap.value("RA")->translation);
  query.bindValue(":raitem",         _strMap.value("RI")->translation);
  query.bindValue(":quote",          _strMap.value("Q")->translation);
  query.bindValue(":quoteitem",      _strMap.value("QI")->translation);
  query.bindValue(":so",             _strMap.value("S")->translation);
  query.bindValue(":soitem",         _strMap.value("SI")->translation);
  query.bindValue(":shipto",         _strMap.value("SHP")->translation);
  query.bindValue(":timeexpense",    _strMap.value("TE")->translation);
  query.bindValue(":todo",           _strMap.value("TODO")->translation);
  query.bindValue(":to",             _strMap.value("TO")->translation);
  query.bindValue(":toitem",         _strMap.value("TI")->translation);
  query.bindValue(":vendor",         _strMap.value("V")->translation);
  query.bindValue(":voucher",        _strMap.value("VCH")->translation);
  query.bindValue(":whse",           _strMap.value("WH")->translation);
  query.bindValue(":wo",             _strMap.value("W")->translation);
  query.bindValue(":projecttask",    _strMap.value("TASK")->translation);

  query.bindValue(":image", tr("Image"));
  query.bindValue(":url", tr("URL"));
  query.bindValue(":file", tr("File"));

  query.bindValue(":source",   _sourcetype);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _doc->populate(query,true);
  ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Documents"),
                       query, __FILE__, __LINE__);
}

void Documents::handleSelection(bool /*pReadOnly*/)
{
  if (_doc->selectedItems().count() &&
      (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
       _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->setText(tr("Open"));
  }
  else
  {
    _viewDoc->setText(tr("View"));
  }

  bool valid = (_doc->selectedItems().count() > 0);
  _editDoc->setEnabled(valid && !_readOnly && _doc->currentItem()->rawValue("canedit").toBool());
  _viewDoc->setEnabled(valid && _doc->currentItem()->rawValue("canview").toBool());
}

void Documents::handleItemSelected()
{
  if(_readOnly ||
     (_doc->currentItem()->rawValue("target_type").toString() == "URL" ||
      _doc->currentItem()->rawValue("target_type").toString() == "FILE" ))
  {
    _viewDoc->animateClick();
  }
  else
  {
    _editDoc->animateClick();
  }
}
