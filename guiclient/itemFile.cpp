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

#include "itemFile.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvariant.h>

/*
 *  Constructs a itemFile as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
itemFile::itemFile(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemFile::~itemFile()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemFile::languageChange()
{
    retranslateUi(this);
}


void itemFile::init()
{
  _mode = cNew;
  _itemid = -1;
  _itemfileid = -1;
}

enum SetResponse itemFile::set( const ParameterList & pParams )
{
  QVariant param;
  bool        valid;
  
  param = pParams.value("item_id", &valid);
  if(valid)
    _itemid = param.toInt();

  param = pParams.value("itemfile_id", &valid);
  if(valid)
  {
    _itemfileid = param.toInt();
    q.prepare("SELECT itemfile_item_id, itemfile_title, itemfile_url"
              "  FROM itemfile"
              " WHERE (itemfile_id=:itemfile_id);" );
    q.bindValue(":itemfile_id", _itemfileid);
    q.exec();
    if(q.first())
    {
      _itemid = q.value("itemfile_item_id").toInt();
      _title->setText(q.value("itemfile_title").toString());
      _url->setText(q.value("itemfile_url").toString());
    }
    // TODO: catch any possible errors
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
      _mode = cNew;
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
      _title->setEnabled(false);
      _url->setEnabled(false);
    }
  }
  
  return NoError;
}

void itemFile::sSave()
{
  if(_url->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::warning( this, tr("Must Specify file"),
      tr("You must specify a file before you may save.") );
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO itemfile"
              "       (itemfile_item_id, itemfile_title, itemfile_url) "
              "VALUES (:item_id, :title, :url); ");
  else //if(cEdit == _mode)
    q.prepare("UPDATE itemfile"
              "   SET itemfile_title=:title,"
              "       itemfile_url=:url"
              " WHERE (itemfile_id=:itemfile_id); ");

  q.bindValue(":item_id", _itemid);
  q.bindValue(":itemfile_id", _itemfileid);
  q.bindValue(":title", _title->text().stripWhiteSpace());
  q.bindValue(":url", _url->text().stripWhiteSpace());
  q.exec();

  accept();
}
