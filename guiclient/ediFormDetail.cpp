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

#include "ediFormDetail.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a ediFormDetail as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ediFormDetail::ediFormDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediFormDetail::~ediFormDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediFormDetail::languageChange()
{
    retranslateUi(this);
}


void ediFormDetail::init()
{
  _mode = cNew;
  _ediformid = -1;
  _ediformdetailid = -1;
}

enum SetResponse ediFormDetail::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("ediform_id", &valid);
  if (valid)
    _ediformid = param.toInt();

  param = pParams.value("ediformdetail_id", &valid);
  if (valid)
  {
    _ediformdetailid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if("new" == param.toString())
      _mode = cNew;
    else if("edit" == param.toString())
      _mode = cEdit;
  }

  return NoError;
}

void ediFormDetail::sSave()
{
  if(_name->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("You must enter in a valid name for this form detail.") );
    _name->setFocus();
    return;
  }

  if(_query->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("You must enter in a valid query for this form detail.") );
    _query->setFocus();
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO ediformdetail "
              "(ediformdetail_ediform_id,"
              " ediformdetail_name, ediformdetail_query,"
              " ediformdetail_order, ediformdetail_notes) "
              "VALUES(:ediform_id,"
              " :ediformdetail_name, :ediformdetail_query,"
              " :ediformdetail_order, :ediformdetail_notes);");
  else
    q.prepare("UPDATE ediformdetail"
              "   SET ediformdetail_name=:ediformdetail_name,"
              "       ediformdetail_query=:ediformdetail_query,"
              "       ediformdetail_order=:ediformdetail_order,"
              "       ediformdetail_notes=:ediformdetail_notes "
              " WHERE (ediformdetail_id=:ediformdetail_id); ");

  q.bindValue(":ediform_id", _ediformid);
  q.bindValue(":ediformdetail_id", _ediformdetailid);
  q.bindValue(":ediformdetail_name", _name->text().stripWhiteSpace());
  q.bindValue(":ediformdetail_query", _query->text().stripWhiteSpace());
  q.bindValue(":ediformdetail_notes", _notes->text());
  q.bindValue(":ediformdetail_order", _order->value());

  if(!q.exec())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form Detail"),
      tr("There was a database error preventing this record from being saved.") );
    return;
  }

  accept();
}

void ediFormDetail::populate()
{
  q.prepare("SELECT ediformdetail_name, ediformdetail_order,"
            "       ediformdetail_query, ediformdetail_notes"
            "  FROM ediformdetail"
            " WHERE (ediformdetail_id=:ediformdetail_id); ");
  q.bindValue(":ediformdetail_id", _ediformdetailid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ediformdetail_name").toString());
    _order->setValue(q.value("ediformdetail_order").toInt());
    _query->setText(q.value("ediformdetail_query").toString());
    _notes->setText(q.value("ediformdetail_notes").toString());
  }
}

