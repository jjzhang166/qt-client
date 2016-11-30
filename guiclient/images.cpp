/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "images.h"

#include <QMenu>
#include <QSqlError>

#include <parameter.h>
#include <metasql.h>
#include "mqlutil.h"

#include "image.h"
#include "guiclient.h"
#include "errorReporter.h"

images::images(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_image, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_image, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_image, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_showSize, SIGNAL(clicked()), this, SLOT(sFillList()));

  _image->addColumn(tr("Name"),  _itemColumn, Qt::AlignLeft, true, "image_name");
  _image->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "image_descrip");
  _image->addColumn(tr("Size"),   _qtyColumn, Qt::AlignRight,true, "image_size");
  _image->addColumn(tr("Package"),_qtyColumn, Qt::AlignRight,false,"nspname");

  connect(_image, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

  sFillList();
}

images::~images()
{
  // no need to delete child widgets, Qt does it all for us
}

void images::languageChange()
{
  retranslateUi(this);
}

void images::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  image newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void images::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("image_id", _image->id());

  image newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void images::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("image_id", _image->id());

  image newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void images::sDelete()
{
  XSqlQuery imagesDelete;
  imagesDelete.prepare( "DELETE FROM image "
             "WHERE (image_id=:image_id);" );
  imagesDelete.bindValue(":image_id", _image->id());
  imagesDelete.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Selected Image"),
                                imagesDelete, __FILE__, __LINE__))
  {
    return;
  }

  sFillList();
}

void images::sFillList()
{
  _image->setColumnHidden(_image->column("image_size"), !_showSize->isChecked());
  XSqlQuery imagesFillList;
  MetaSQLQuery mql = mqlLoad("images", "list");
  ParameterList params;
  if (_showSize->isChecked())
    params.append("displayImageSize", true);

  imagesFillList = mql.toQuery(params);
  _image->populate(imagesFillList);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Image Information"),
                                imagesFillList, __FILE__, __LINE__))
  {
    return;
  }
}
