/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "docAttach.h"

#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QMessageBox>
#include <QString>
#include <QUiLoader>
#include <QUrl>
#include <QVariant>

#include "documents.h"
#include "errorReporter.h"
#include "../common/shortcuts.h"
#include "imageview.h"

#define DEBUG false

class StackDescriptor
{
  public:
    StackDescriptor(QWidget *page, QWidget *doc)
      : stackPage(page),
        docWidget(doc)
    {
    }

    QWidget *stackPage;
    QWidget *docWidget;
};

class docAttachPrivate {
  public:
    docAttachPrivate(docAttach *parent)
      : p(parent)
    {
      // url and file match _docType->populate below, at least for now
      map.insert(-3,                           new StackDescriptor(p->_urlPage,     p->_url));
      map.insert(-2,                           new StackDescriptor(p->_filePage,    p->_file));
      map.insert(Documents::Contact,           new StackDescriptor(p->_cntctPage,   p->_cntct));
      map.insert(Documents::CRMAccount,        new StackDescriptor(p->_crmacctPage, p->_crmacct));
      map.insert(Documents::Customer,          new StackDescriptor(p->_custPage,    p->_cust));
      map.insert(Documents::Employee,          new StackDescriptor(p->_empPage,     p->_emp));
      map.insert(Documents::Uninitialized,     new StackDescriptor(p->_filePage,    p->_file));
      map.insert(Documents::Uninitialized,     new StackDescriptor(p->_imagePage,   p->_img));
      map.insert(Documents::Incident,          new StackDescriptor(p->_incdtPage,   p->_incdt));
      map.insert(Documents::Invoice,           new StackDescriptor(p->_invoicePage, p->_invoice));
      map.insert(Documents::Item,              new StackDescriptor(p->_itemPage,    p->_item));
      map.insert(Documents::Opportunity,       new StackDescriptor(p->_oppPage,     p->_opp));
      map.insert(Documents::Project,           new StackDescriptor(p->_projPage,    p->_proj));
      map.insert(Documents::PurchaseOrder,     new StackDescriptor(p->_poPage,      p->_po));
      map.insert(Documents::SalesOrder,        new StackDescriptor(p->_soPage,      p->_so));
      map.insert(Documents::Vendor,            new StackDescriptor(p->_vendPage,    p->_vend));
      map.insert(Documents::Uninitialized,     new StackDescriptor(p->_urlPage,     p->_url));
      map.insert(Documents::WorkOrder,         new StackDescriptor(p->_woPage,      p->_wo));

      if (! _x_privileges) return; // Qt Designer doesn't connect to the database

      XSqlQuery q("SELECT * FROM source"
                  " WHERE source_widget NOT IN ('', 'core');");
      QUiLoader uil(p);
      while (q.next())
      {
        QWidget *w = 0;
        QString  description = q.value("source_widget").toString();
        if (DEBUG)
          qDebug() << "checking" << q.value("source_name") << description;
        if (description.startsWith("SELECT", Qt::CaseInsensitive))
        {
          XComboBox *c = new XComboBox();
          c->populate(description);
          w = c;
        }
        else if (description.contains("Cluster"))
        {
          w = uil.createWidget(description, p,
                               "_" + q.value("source_name").toString());
        }
        if (w) {
          QString litValue = q.value("source_descrip").toString();
          QWidget     *page = new QWidget();
          QFormLayout *lyt  = new QFormLayout(p);
          QLabel      *lit  = new QLabel(QT_TRANSLATE_NOOP("docAttach", litValue));
          page->setLayout(lyt);
          lyt->addRow(lit, w);
          p->_documentsStack->addWidget(page);
          map.insert(q.value("source_docass_num").toInt(), new StackDescriptor(page, w));
          if (DEBUG) qDebug() << "created a widget for" << description;
        }
        else
        {
          qDebug() << "Could not create a widget for" << description;
        }
      }
      ErrorReporter::error(QtCriticalMsg, 0, "Error Getting Document Types",
                           q, __FILE__, __LINE__);
    }

    docAttach *p;
    QMap<int, StackDescriptor*> map;
};

/**
 *  Constructs a docAttach as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 *
 *  This screen will receive the source type and id from the calling screen.
 *  Then the user will select a target type and id on this screen.
 *  When the user clicks Save, a new row will be entered into docass and
 *  the screen will return a docass_id to the calling screen.
 */

docAttach::docAttach(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
  setObjectName(name ? name : "docAttach");
  setModal(modal);

  _p = new docAttachPrivate(this);

  _save = _buttonBox->button(QDialogButtonBox::Save);
  _save->setEnabled(false);
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_docType,   SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(_filetitle,   SIGNAL(textChanged(QString)), this, SLOT(sHandleButtons()));
  connect(_fileList,  SIGNAL(clicked()),  this, SLOT(sFileList()));
  connect(_save,      SIGNAL(clicked()),  this, SLOT(sSave()));

  _sourcetype = "";
  _sourceid = -1;
  _targetid = -1;
  _urlid = -1;
  _mode = "new";

  _po->setAllowedTypes(OrderLineEdit::Purchase);
  _so->setAllowedTypes(OrderLineEdit::Sales);

  _docType->populate("SELECT * FROM"
                     "(SELECT source_docass_num, source_descrip, source_name"
                     "  FROM source"
                     " WHERE source_widget != ''"
                     " UNION SELECT -2, 'File',     'FILE'"
                     " UNION SELECT -3, 'Web Site', 'URL') data"
                     " ORDER BY source_descrip;");

#ifndef Q_OS_MAC
    _fileList->setMaximumWidth(25);
#else
    _fileList->setMinimumWidth(60);
    _fileList->setMinimumHeight(32);
#endif

    shortcuts::setStandardKeys(this);
    adjustSize();
}

docAttach::~docAttach()
{
  // no need to delete child widgets, Qt does it all for us
}

void docAttach::languageChange()
{
  retranslateUi(this);
}

void docAttach::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  //source type from document widget
  param = pParams.value("sourceType", &valid);
  if (valid)
    _sourcetype = param.toString();

  //source id from document widget
  param = pParams.value("source_id", &valid);
  if (valid)
    _sourceid = param.toInt();

  // Only urls are editable
  param = pParams.value("url_id", &valid);
  if(valid)
  {
    if (DEBUG) qDebug() << "got url_id" << param;
    XSqlQuery qry;
    _urlid = param.toInt();
    qry.prepare("SELECT url_source, url_source_id, url_title, url_url, url_stream "
                "  FROM url"
                " WHERE (url_id=:url_id);" );
    qry.bindValue(":url_id", _urlid);
    qry.exec();
    if(qry.first())
    {
      setWindowTitle(tr("Edit Attachment Link"));
      QUrl url(qry.value("url_url").toString());
      if (url.scheme().isEmpty())
        url.setScheme("file");

      if (DEBUG) qDebug() << qry.value("url_url") << "converted to" << url;

      _url->setText(url.toString());
      if (url.scheme() == "file")
      {
        if (DEBUG)
          qDebug() << "file title:"    << qry.value("url_title").toString()
                   << " text:"         << url.toString()
                   << "stream length:" << qry.value("url_stream").toString().length();
        _docType->setId(-2);
        _filetitle->setText(qry.value("url_title").toString());
        _file->setText(url.toString());
        if (qry.value("url_stream").toString().length())
        {
          _fileList->setEnabled(false);
          _file->setEnabled(false);
          _saveDbCheck->setEnabled(false);
        }
      }
      else
      {
        if (DEBUG)
          qDebug() << "! file title:" << qry.value("url_title").toString()
                   << " text:" << url.toString();
        _docType->setId(-3);
        _urltitle->setText(qry.value("url_title").toString());
        _url->setText(url.toString());
      }
      _mode = "edit";
      _docType->setEnabled(false);
    }
    ErrorReporter::error(QtCriticalMsg, 0, tr("Error URL"),
                         qry, __FILE__, __LINE__);
  }
}

void docAttach::sHandleNewId(int id)
{
  _save->setEnabled(id != -1);
}

void docAttach::sHandleButtons()
{
  if (_docType->id() == -1) {
    return;
  }
  _docAttachPurpose->setEnabled(true);

  StackDescriptor*pageDesc   = _p->map.value(_docType->id());
  QWidget        *pageWidget = qobject_cast<QWidget *>(pageDesc->stackPage);
  VirtualCluster *docCluster = qobject_cast<VirtualCluster *>(pageDesc->docWidget);
  XComboBox      *docCombo   = qobject_cast<XComboBox *>(pageDesc->docWidget);
  WoCluster      *wocluster  = qobject_cast<WoCluster *>(pageDesc->docWidget);

  if (! pageWidget)
  {
    _save->setEnabled(false);
    return;
  }
  _documentsStack->setCurrentWidget(pageWidget);

  if (docCluster)
  {
    connect(docCluster, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(docCluster, SIGNAL(newId(int)),   this, SLOT(sHandleNewId(int)));
    _save->setEnabled(docCluster->isValid());
  }
  else if (docCombo)
  {
    connect(docCombo, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    _save->setEnabled(docCombo->isValid());
  }
  else if (wocluster)
  {
    connect(wocluster, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(wocluster, SIGNAL(newId(int)),   this, SLOT(sHandleNewId(int)));
    _save->setEnabled(wocluster->isValid());
  }
  else
  {
    if (DEBUG)
      qDebug() << pageDesc->docWidget->objectName() << "is a"
               << pageDesc->docWidget->metaObject()->className();
    _docAttachPurpose->setEnabled(false);
    _docAttachPurpose->setCurrentIndex(0);
    _save->setEnabled(true); // presumably we're on the file or url stack page
  }
}

void docAttach::sSave()
{  
  if (_docType->id() == -1) {
    return;
  }

  _docAttachPurpose->setEnabled(true);
  XSqlQuery newDocass;
  QString title;
  QUrl url;

  //set the purpose
  if (_docAttachPurpose->currentIndex() == 0)
    _purpose = "S";
  else if (_docAttachPurpose->currentIndex() == 1)
    _purpose = "A";
  else if (_docAttachPurpose->currentIndex() == 2)
    _purpose = "C";
  else if (_docAttachPurpose->currentIndex() == 3)
    _purpose = "D";

  StackDescriptor*pageDesc   = _p->map.value(_docType->id());
  VirtualCluster *docCluster = qobject_cast<VirtualCluster *>(pageDesc->docWidget);
  XComboBox      *docCombo   = qobject_cast<XComboBox *>(pageDesc->docWidget);
  WoCluster      *wocluster  = qobject_cast<WoCluster *>(pageDesc->docWidget);

  _targettype = _docType->code();
  if (docCluster)
  {
    _targetid = docCluster->id();
  }
  else if (docCombo)
  {
    _targetid = docCombo->id();
  }
  else if (wocluster)
  {
    _targetid = wocluster->id();
  }
  else if (_documentsStack->currentWidget() == _filePage)
  {
    if(_file->text().trimmed().isEmpty())
    {
      QMessageBox::warning( this, tr("Must Specify file"),
                            tr("You must specify a file before you may save.") );
      return;
    }

     _targettype = "URL";
     title = _filetitle->text();
     url = QUrl(_file->text());
     if (url.scheme().isEmpty())
       url.setScheme("file");
  }
  else if (_documentsStack->currentWidget() == _urlPage)
  {
    if(_url->text().trimmed().isEmpty())
    {
      QMessageBox::warning( this, tr("Must Specify file"),
                            tr("You must specify a file before you may save.") );
      return;
    }

    _targettype = "URL";
    title = _urltitle->text();
    url = QUrl(_url->text());
    if (url.scheme().isEmpty())
      url.setScheme("http");
  }

  if (_targettype == "IMG")
  {
    // First determine if the id is in the image table, and not one of it's inherited versions
    // if it is not then we will create a copy in the image table to keep the FK's working
    XSqlQuery qq;
    qq.prepare("SELECT image_id FROM ONLY image WHERE image_id=:image_id");
    qq.bindValue(":image_id", _targetid);
    if(qq.exec() && !qq.first())
    {
      qq.exec("SELECT nextval(('\"image_image_id_seq\"'::text)::regclass) AS newid;");
      if(qq.first())
      {
        int newid = qq.value("newid").toInt();
        qq.prepare("INSERT INTO image (image_id, image_name, image_descrip, image_data) "
                   "SELECT :newid, image_name, image_descrip, image_data"
                   "  FROM image WHERE image_id=:image_id;");
        qq.bindValue(":newid", newid);
        qq.bindValue(":image_id", _targetid);
        if(qq.exec())
          _targetid = newid;
      }
    }
     // For now images are handled differently because of legacy structures...
    newDocass.prepare( "INSERT INTO imageass "
                       "( imageass_source, imageass_source_id, imageass_image_id, imageass_purpose ) "
                       "VALUES "
                       "( :docass_source_type, :docass_source_id, :docass_target_id, :docass_purpose );" );
  }
  else if (_targettype == "URL")
  {
    if(!url.isValid())
    {
      QMessageBox::warning( this, tr("Must Specify valid path"),
                            tr("You must specify a path before you may save.") );
      return;
    }

    QByteArray  bytarr;
    QFileInfo fi(url.toLocalFile());

    if(_saveDbCheck->isChecked() &&
       (url.scheme()=="file") &&
       (_mode == "new"))
    {
      if (!fi.exists())
      {
        QMessageBox::warning( this, tr("File Error"),
                             tr("File %1 was not found and will not be saved.").arg(url.toLocalFile()));
        return;
      }
      QFile sourceFile(url.toLocalFile());
      if (!sourceFile.open(QIODevice::ReadOnly))
      {
        QMessageBox::warning( this, tr("File Open Error"),
                             tr("Could not open source file %1 for read.")
                                .arg(url.toLocalFile()));
        return;
      }
      bytarr = sourceFile.readAll();
      url.setPath(fi.fileName().remove(" "));
      url.setScheme("");
    }

    // TODO: replace use of URL view
    if (_mode == "new" && bytarr.isNull())
      newDocass.prepare( "INSERT INTO url "
                         "( url_source, url_source_id, url_title, url_url, url_stream ) "
                         "VALUES "
                         "( :docass_source_type, :docass_source_id, :title, :url, :stream );" );
    else if (_mode == "new")
      newDocass.prepare( "INSERT INTO url "
                         "( url_source, url_source_id, url_title, url_url, url_stream ) "
                         "VALUES "
                         "( :docass_source_type, :docass_source_id, :title, :url, :stream );" );
    else
      newDocass.prepare( "UPDATE url SET "
                         "  url_title = :title, "
                         "  url_url = :url "
                         "WHERE (url_id=:url_id);" );
    newDocass.bindValue(":url_id", _urlid);
    newDocass.bindValue(":title", title);
    newDocass.bindValue(":url", url.toString());
    newDocass.bindValue(":stream", bytarr);
  }
  else
  {
    newDocass.prepare( "INSERT INTO docass "
                       "( docass_source_type, docass_source_id, docass_target_type, docass_target_id, docass_purpose ) "
                       "VALUES "
                       "( :docass_source_type, :docass_source_id, :docass_target_type, :docass_target_id, :docass_purpose );" );
    newDocass.bindValue(":docass_target_type", _targettype);
  }

  if (_targettype == _sourcetype &&
      _targetid == _sourceid)
  {
    QMessageBox::critical(this,tr("Invalid Selection"),
                          tr("You may not attach a document to itself."));
    return;
  }

  newDocass.bindValue(":docass_source_type", _sourcetype);
  newDocass.bindValue(":docass_source_id", _sourceid);
  newDocass.bindValue(":docass_target_id", _targetid);
  newDocass.bindValue(":docass_purpose", _purpose);

  newDocass.exec();

  accept();
  return;
}

void docAttach::sFileList()
{
  _file->setText(QString("file:%1").arg(QFileDialog::getOpenFileName( this, tr("Select File"), QString::null)));
  if (!_filetitle->text().length())
  {
    QFileInfo fi = QFileInfo(_file->text());
    _filetitle->setText(fi.fileName());
  }
}
