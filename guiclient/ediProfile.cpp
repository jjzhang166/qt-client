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

#include "ediProfile.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "ediForm.h"

/*
 *  Constructs a ediProfile as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ediProfile::ediProfile(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_type, SIGNAL(activated(int)), _stack, SLOT(raiseWidget(int)));
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_forms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_forms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_forms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediProfile::~ediProfile()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediProfile::languageChange()
{
    retranslateUi(this);
}


void ediProfile::init()
{
  _mode = cNew;
  _ediprofileid = -1;

  _forms->addColumn(tr("Type"),           -1, Qt::AlignLeft );
  _forms->addColumn(tr("Output"), _itemColumn, Qt::AlignLeft );
}

enum SetResponse ediProfile::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("ediprofile_id", &valid);
  if (valid)
  {
    _ediprofileid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _mode = cNew;
    else if (param.toString() == "edit")
      _mode = cEdit;
  }

  return NoError;
}

bool ediProfile::save()
{
  if(_name->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("You must enter in a valid name for this profile.") );
    _name->setFocus();
    return false;
  }

  if(cNew == _mode)
    q.prepare("SELECT ediprofile_id"
              "  FROM ediprofile"
              " WHERE (ediprofile_name=:ediprofile_name); ");
  else if(cEdit == _mode)
    q.prepare("SELECT ediprofile_id"
              "  FROM ediprofile"
              " WHERE ((ediprofile_name=:ediprofile_name)"
              "   AND  (ediprofile_id!=:ediprofile_id)); ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.bindValue(":ediprofile_name", _name->text().stripWhiteSpace());
  q.exec();
  if(q.first())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("The name you have entered is already in the system. Please choose a different name.") );
    _name->setFocus();
    return false;
  }

  // TODO: Add specific checking based on type

  if( (cNew == _mode) && (-1 == _ediprofileid) )
  {
    q.prepare("SELECT nextval('ediprofile_ediprofile_id_seq') AS result; ");
    q.exec();
    if(q.first())
      _ediprofileid = q.value("result").toInt();
    else
    {
      QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
        tr("There was a database error preventing this record from being saved.") );
      return false;
    }
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO ediprofile "
              "(ediprofile_id, ediprofile_name, ediprofile_notes,"
              " ediprofile_type, ediprofile_option1, ediprofile_option2,"
              " ediprofile_option3, ediprofile_option4, ediprofile_option5) "
              "VALUES(:ediprofile_id, :ediprofile_name, :ediprofile_notes,"
              " :ediprofile_type, :ediprofile_option1, :ediprofile_option2,"
              " :ediprofile_option3, :ediprofile_option4, :ediprofile_option5);" );
  else
    q.prepare("UPDATE ediprofile"
              "   SET ediprofile_name=:ediprofile_name,"
              "       ediprofile_notes=:ediprofile_notes,"
              "       ediprofile_type=:ediprofile_type,"
              "       ediprofile_option1=:ediprofile_option1,"
              "       ediprofile_option2=:ediprofile_option2,"
              "       ediprofile_option3=:ediprofile_option3,"
              "       ediprofile_option4=:ediprofile_option4,"
              "       ediprofile_option5=:ediprofile_option5"
              " WHERE (ediprofile_id=:ediprofile_id); " );

  q.bindValue(":ediprofile_id", _ediprofileid);
  q.bindValue(":ediprofile_name", _name->text().stripWhiteSpace());
  q.bindValue(":ediprofile_notes", _notes->text());

  switch(_type->currentItem())
  {
    case 0: // Ftp
      q.bindValue(":ediprofile_type", "ftp");
      q.bindValue(":ediprofile_option1", _ftpServer->text());
      q.bindValue(":ediprofile_option2", _ftpLogin->text());
      q.bindValue(":ediprofile_option3", _ftpPassword->text());
      q.bindValue(":ediprofile_option4", _ftpDirectory->text());
      break;
    case 1: // Email
      q.bindValue(":ediprofile_type", "email");
      q.bindValue(":ediprofile_option1", _emailTo->text());
      q.bindValue(":ediprofile_option2", _emailSubject->text());
      q.bindValue(":ediprofile_option3", _emailBody->text());
      q.bindValue(":ediprofile_option4", _emailCC->text());
      break;
    default:
      QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
        tr("The selected type for this profile is not recognized.") );
      _type->setFocus();
      return false;
  }

  if(!q.exec())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Profile"),
      tr("There was a database error preventing this record from being saved.") );
    return false;
  }

  if(cNew == _mode)
    _mode = cEdit;

  return true;
}

void ediProfile::sSave()
{
  if(save())
    accept();
}

void ediProfile::sNew()
{
  if(cNew == _mode)
  {
    if(QMessageBox::information( this, tr("Would you like to Save?"),
         tr("The Profile has not yet been saved to the database. Before continuing\n"
            "you must save the Profile so it exists on the database.\n\n"
            "Would you like to save before continuing?"),
         (QMessageBox::Cancel | QMessageBox::Escape),
         (QMessageBox::Ok | QMessageBox::Default) ) != QMessageBox::Ok)
      return;

    if(!save())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("ediprofile_id", _ediprofileid);

  ediForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfile::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ediprofile_id", _ediprofileid);
  params.append("ediform_id", _forms->id());

  ediForm newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfile::sDelete()
{
  q.prepare("SELECT deleteEDIForm(:ediform_id) AS RESULT;");
  q.bindValue(":ediform_id", _forms->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete EDI Form"),
          tr("The select EDI Form cannot be deleted because it is currently\n"
             "being referenced by one or more records.") );
        return;
      default:
        sFillList();
    }
  }
  else
    systemError( this, tr("A System Error occurred at ediProfile::%1.")
                       .arg(__LINE__) );
}

void ediProfile::populate()
{
  q.prepare("SELECT ediprofile_name, ediprofile_notes,"
            "       ediprofile_type, ediprofile_option1,"
            "       ediprofile_option2, ediprofile_option3,"
            "       ediprofile_option4, ediprofile_option5"
            "  FROM ediprofile"
            " WHERE (ediprofile_id=:ediprofile_id); ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("ediprofile_name").toString());
    _notes->setText(q.value("ediprofile_notes").toString());
    
    if("ftp" == q.value("ediprofile_type").toString())
    {
      _type->setCurrentItem(0);
      _stack->raiseWidget(0);
      _ftpServer->setText(q.value("ediprofile_option1").toString());
      _ftpLogin->setText(q.value("ediprofile_option2").toString());
      _ftpPassword->setText(q.value("ediprofile_option3").toString());
      _ftpDirectory->setText(q.value("ediprofile_option4").toString());
    }
    else if("email" == q.value("ediprofile_type").toString())
    {
      _type->setCurrentItem(1);
      _stack->raiseWidget(1);
      _emailTo->setText(q.value("ediprofile_option1").toString());
      _emailSubject->setText(q.value("ediprofile_option2").toString());
      _emailBody->setText(q.value("ediprofile_option3").toString());
      _emailCC->setText(q.value("ediprofile_option4").toString());
    }
    else
    {
      QMessageBox::critical( this, tr("Unknown Type Encountered"),
        tr("An unknown type of %1 was encountered on the record you were trying to open.").arg(q.value("ediprofile_type").toString()) );
      return;
    }

    sFillList();
  }
}

void ediProfile::sFillList()
{
  q.prepare("SELECT ediform_id, ediform_type, ediform_output"
            "  FROM ediform"
            " WHERE (ediform_ediprofile_id=:ediprofile_id) "
            "ORDER BY ediform_type; ");
  q.bindValue(":ediprofile_id", _ediprofileid);
  q.exec();
  _forms->populate(q);
}
