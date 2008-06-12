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

#include "booitemImage.h"

#include <qvariant.h>
#include "image.h"

/*
 *  Constructs a booitemImage as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
booitemImage::booitemImage(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_image, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
booitemImage::~booitemImage()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void booitemImage::languageChange()
{
    retranslateUi(this);
}


void booitemImage::init()
{
  _image->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft );
  _image->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  sFillList();
}

enum SetResponse booitemImage::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("booitem_id", &valid);
  if (valid)
    _booitemid = param.toInt();

  param = pParams.value("booimage_id", &valid);
  if (valid)
  {
    _booimageid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _purpose->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
      _purpose->setEnabled(FALSE);
    }
  }

  return NoError;
}

void booitemImage::sSave()
{
  QString purpose;
  if (_purpose->currentItem() == 0)  
    purpose = "I";
  if (_purpose->currentItem() == 1)  
    purpose = "P";
  if (_purpose->currentItem() == 2)  
    purpose = "E";
  if (_purpose->currentItem() == 3)  
    purpose = "M";

  XSqlQuery newImage;

  if (_mode == cNew)
  {
    _booimageid = -1;

    if (purpose != "M")
    {
      XSqlQuery booimageid;
      booimageid.prepare( "SELECT booimage_id "
                           "FROM booimage "
                           "WHERE ( (booimage_booitem_id=:booitem_id)"
                           " AND (booimage_purpose=:booimage_purpose) );" );
      booimageid.bindValue(":booitem_id", _booitemid);
      booimageid.bindValue(":booimage_purpose", purpose);
      booimageid.exec();
      if (booimageid.first())
      {
        _booimageid = booimageid.value("booimage_id").toInt();
        _mode = cEdit;
      }
    }
      
    if (_booimageid == -1)
    {
      XSqlQuery booimageid;
      booimageid.prepare("SELECT NEXTVAL('booimage_booimage_id_seq') AS _booimage_id;");
      booimageid.exec();
      if (booimageid.first())
        _booimageid = booimageid.value("_booimage_id").toInt();
//  ToDo
    }

    newImage.prepare( "INSERT INTO booimage "
                      "( booimage_id, booimage_booitem_id, booimage_purpose, booimage_image_id ) "
                      "VALUES "
                      "( :booimage_id, :booimage_booitem_id, :booimage_purpose, :booimage_image_id );" );
  }

  if (_mode == cEdit)
    newImage.prepare( "UPDATE booimage "
                      "SET booimage_purpose=:booimage_purpose, booimage_image_id=:booimage_image_id "
                      "WHERE (booimage_id=:booimage_id);" );

  newImage.bindValue(":booimage_id", _booimageid);
  newImage.bindValue(":booimage_booitem_id", _booitemid);
  newImage.bindValue(":booimage_image_id", _image->id());
  newImage.bindValue(":booimage_purpose", purpose);

  newImage.exec();

  done(_booimageid);
}

void booitemImage::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  image newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void booitemImage::sFillList()
{
  _image->populate( "SELECT image_id, image_name, firstLine(image_descrip) "
                    "FROM image "
                    "ORDER BY image_name;" );
}

void booitemImage::populate()
{
  XSqlQuery booimage;
  booimage.prepare( "SELECT booimage_purpose, booimage_image_id "
                     "FROM booimage "
                     "WHERE (booimage_id=:booimage_id);" );
  booimage.bindValue(":booimage_id", _booimageid);
  booimage.exec();
  if (booimage.first())
  {
    QString purpose = booimage.value("booimage_purpose").toString();

    if (purpose == "I")
      _purpose->setCurrentItem(0);
    else if (purpose == "P")
      _purpose->setCurrentItem(1);
    else if (purpose == "E")
      _purpose->setCurrentItem(2);
    else if (purpose == "M")
      _purpose->setCurrentItem(3);

    _image->setId(booimage.value("booimage_image_id").toInt());
  }
}

