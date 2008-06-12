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

#include "imagecluster.h"

#include <QPixmap>
#include <QScrollArea>

#include <quuencode.h>

#define DEBUG   false

ImageCluster::ImageCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new ImageClusterLineEdit(this, pName));

  _image = new QLabel("picture here");
  _image->setPixmap(_nullPixmap);
  _image->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  QScrollArea * scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  scrollArea->setWidget(_image);
  _grid->addWidget(scrollArea, 1, 0, -1, -1);
  _description->hide();
  _name->hide();

  _nullPixmap = QPixmap();
}

void ImageCluster::clear()
{
  if (DEBUG)
    qDebug("%s::clear()", qPrintable(objectName()));
  VirtualCluster::clear();
  _image->setPixmap(_nullPixmap);
}

void ImageCluster::sRefresh()
{
  if (DEBUG)
    qDebug("%s::sRefresh()", qPrintable(objectName()));

  VirtualCluster::sRefresh();
  if (_description->text().isEmpty())
  {
    if (DEBUG)
      qDebug("ImageCluster::sRefresh() without a picture");
    _image->setText(tr("picture here"));
    _image->setPixmap(_nullPixmap);
  }
  else
  {
    if (DEBUG)
      qDebug("ImageCluster::sRefresh() has picture %s, %d",
             qPrintable(_description->text().right(128)),
             _description->text().length());
    QImage tmpImage;
    tmpImage.loadFromData(QUUDecode(_description->text()));
    _image->setPixmap(QPixmap::fromImage(tmpImage));
  }

  if (DEBUG)
    qDebug("ImageCluster::sRefresh() returning");
}

ImageClusterLineEdit::ImageClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "image", "image_id", "image_name", "image_descrip", "image_data", 0, pName)
{
  setTitles(tr("Image"), tr("Images"));
  _numClause = QString(" AND (UPPER(image_name)=UPPER(:number)) ");
}

VirtualInfo *ImageClusterLineEdit::infoFactory()
{
  return new ImageInfo(this);
}

VirtualList *ImageClusterLineEdit::listFactory()
{
  return new ImageList(this);
}

VirtualSearch *ImageClusterLineEdit::searchFactory()
{
  return new ImageSearch(this);
}

ImageInfo::ImageInfo(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualInfo(pParent, pFlags)
{
  _descripLit->hide();
  _descrip->hide();
}

ImageList::ImageList(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualList(pParent, pFlags)
{
  _listTab->hideColumn(2);
}

ImageSearch::ImageSearch(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualSearch(pParent, pFlags)
{
  _listTab->hideColumn(2);
  _searchDescrip->hide();
}
