/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "image.h"

#include <QVariant>
#include <QImageWriter>
#include <QImageReader>
#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <quuencode.h>

/*
 *  Constructs a image as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
image::image(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_fileList, SIGNAL(clicked()), this, SLOT(sFileList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

#ifndef Q_WS_MAC
  _fileList->setMaximumWidth(25);
#endif

  _image = new QLabel();
  _image->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QScrollArea * scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  scrollArea->setWidget(_image);
  QHBoxLayout *layout = new QHBoxLayout;
  layout->setMargin(0);
  layout->addWidget(scrollArea);
  _imageFrame->setLayout(layout);
}

/*
 *  Destroys the object and frees any allocated resources
 */
image::~image()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void image::languageChange()
{
  retranslateUi(this);
}

enum SetResponse image::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("image_id", &valid);
  if (valid)
  {
    _imageid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _fileName->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _filenameLit->hide();
      _fileName->hide();
      _fileList->hide();

      _name->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _filenameLit->hide();
      _fileName->hide();
      _fileList->hide();
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void image::populate()
{
  XSqlQuery image;
  image.prepare( "SELECT image_name, image_descrip, image_data "
                 "FROM image "
                 "WHERE (image_id=:image_id);" );
  image.bindValue(":image_id", _imageid);
  image.exec();
  if (image.first())
  {
    _name->setText(image.value("image_name").toString());
    _descrip->setText(image.value("image_descrip").toString());

    __image.loadFromData(QUUDecode(image.value("image_data").toString()));
    _image->setPixmap(QPixmap::fromImage(__image));
  }
}

void image::sSave()
{
  XSqlQuery newImage;

  if (_mode == cNew)
  {
    if (!__image.isNull())
    {
      XSqlQuery imageid("SELECT NEXTVAL('image_image_id_seq') AS _image_id");
      if (imageid.first())
        _imageid = imageid.value("_image_id").toInt();
//  ToDo
 
      QImageWriter imageIo;
      QBuffer  imageBuffer;
      QString  imageString;

      imageBuffer.open(QIODevice::ReadWrite);
      imageIo.setDevice(&imageBuffer);
      imageIo.setFormat("PNG");

      if (!imageIo.write(__image))
      {
//  ToDo - should issue an error here
        reject();
        return;
      }

      imageBuffer.close();
      imageString = QUUEncode(imageBuffer);

      newImage.prepare( "INSERT INTO image "
                        "(image_id, image_name, image_descrip, image_data) "
                        "VALUES "
                        "(:image_id, :image_name, :image_descrip, :image_data);" );
      newImage.bindValue(":image_id", _imageid);
      newImage.bindValue(":image_name", _name->text());
      newImage.bindValue(":image_descrip", _descrip->text());
      newImage.bindValue(":image_data", imageString);
    }
  }
  else if (_mode == cEdit)
  {
    newImage.prepare( "UPDATE image "
                      "SET image_name=:image_name, image_descrip=:image_descrip "
                      "WHERE (image_id=:image_id);" );
    newImage.bindValue(":image_id", _imageid);
    newImage.bindValue(":image_name", _name->text());
    newImage.bindValue(":image_descrip", _descrip->text());
  }

  newImage.exec();

  done(_imageid);
}

void image::sFileList()
{
  bool first = TRUE;
  bool havejpg = FALSE;
  QString frmtList = QString(tr("Images ("));
  QString ext = QString::null;
  QList<QByteArray> list = QImageReader::supportedImageFormats();
  for (int i = 0; i < list.size(); ++i)
  {
    if (!first)
      frmtList += QString(tr(" "));
    ext = (list.at(i)).lower();

    if (ext == "jpeg")
      ext = "jpg";

    if (ext != "jpg" || !havejpg)
      frmtList += QString(tr("*.")) + ext;

    if (ext == "jpg")
      havejpg = TRUE;

    first = FALSE;
  }

  frmtList += QString(tr(")"));
  if (first)
    frmtList = QString(tr("Images (*.png *.xpm *.jpg *.gif)")); // should I do this?

  _fileName->setText(QFileDialog::getOpenFileName( this, tr("Select Image File"), QString::null, frmtList));

  if (_fileName->text().length())
  {
    if(!__image.load(_fileName->text()))
      QMessageBox::warning(this, tr("Could not load file"),
                            tr( "Could not load the selected file.\n"
                                "The file is not an image, an unknown image format or is corrupt" ) );
    _image->setPixmap(QPixmap::fromImage(__image));
  }
}
