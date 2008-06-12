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

#include "itemImages.h"

#include <qvariant.h>
#include <quuencode.h>
#include <qstatusbar.h>
#include <qimage.h>

/*
 *  Constructs a itemImages as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemImages::itemImages(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_prev, SIGNAL(clicked()), this, SLOT(sPrevious()));
    connect(_next, SIGNAL(clicked()), this, SLOT(sNext()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemImages::~itemImages()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemImages::languageChange()
{
    retranslateUi(this);
}


void itemImages::init()
{
  statusBar()->hide();

#ifndef Q_WS_MAC
  _prev->setMaximumWidth(25);
  _next->setMaximumWidth(25);
#endif
}

enum SetResponse itemImages::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  return NoError;
}

void itemImages::sPrevious()
{
  _images.prev();
  loadImage();
}

void itemImages::sNext()
{
  _images.next();
  loadImage();
}

void itemImages::sFillList()
{
  _images.prepare( "SELECT itemimage_id, image_data, image_descrip,"
                   "       CASE WHEN (itemimage_purpose='I') THEN :inventoryDescription"
                   "            WHEN (itemimage_purpose='P') THEN :productDescription"
                   "            WHEN (itemimage_purpose='E') THEN :engineeringReference"
                   "            WHEN (itemimage_purpose='M') THEN :miscellaneous"
                   "            ELSE :other"
                   "       END AS purpose "
                   "FROM itemimage, image "
                   "WHERE ( (itemimage_image_id=image_id)"
                   " AND (itemimage_item_id=:item_id) ) "
                   "ORDER BY image_name;" );
  _images.bindValue(":item_id", _item->id());
  _images.bindValue(":inventoryDescription", tr("Inventory Description"));
  _images.bindValue(":productDescription", tr("Product Description"));
  _images.bindValue(":engineeringReference", tr("Engineering Reference"));
  _images.bindValue(":miscellaneous", tr("Miscellaneous"));
  _images.bindValue(":other", tr("Other"));
  _images.exec();
  if (_images.first())
    loadImage();
  else
  {
    _prev->setEnabled(FALSE);
    _next->setEnabled(FALSE);
    _image->clear();
  }
}

void itemImages::loadImage()
{
  _prev->setEnabled(_images.at() != 0);
  _next->setEnabled(_images.at() < (_images.size() - 1));

  _description->setText(_images.value("purpose").toString() + " - " + _images.value("image_descrip").toString());

  QImage image;
  image.loadFromData(QUUDecode(_images.value("image_data").toString()));
  _image->setPixmap(QPixmap::fromImage(image));
}

