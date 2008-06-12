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

#include "ediForm.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "ediFormDetail.h"

ediForm::ediForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_type, SIGNAL(activated(int)), this, SLOT(sTypeSelected(int)));
    connect(_accept, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_csvDelete, SIGNAL(clicked()), this, SLOT(sCSVDelete()));
    connect(_csvEdit, SIGNAL(clicked()), this, SLOT(sCSVEdit()));
    connect(_csvNew, SIGNAL(clicked()), this, SLOT(sCSVNew()));

    _mode = cNew;
    _ediprofileid = -1;
    _ediformid = -1;

    _csvDetails->addColumn(tr("Order"), _seqColumn, Qt::AlignLeft );
    _csvDetails->addColumn(tr("Name"),          -1, Qt::AlignLeft );

    sTypeSelected(0);
}

ediForm::~ediForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void ediForm::languageChange()
{
    retranslateUi(this);
}

enum SetResponse ediForm::set( const ParameterList & pParams )
{
  QVariant param;
  bool     valid;

  param = pParams.value("ediprofile_id", &valid);
  if (valid)
    _ediprofileid = param.toInt();

  param = pParams.value("ediform_id", &valid);
  if (valid)
  {
    _ediformid = param.toInt();
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

void ediForm::sTypeSelected( int pType )
{
  bool on = (pType != 0);

  _type->setEnabled(!on);

  _output->setEnabled(on);
  _query->setEnabled(on);
  _file->setEnabled(on);
  _stack->setEnabled(on);

  if( (-1 == _ediformid) && (pType != 0) )
  {
    q.prepare("SELECT ediform_id"
              "  FROM ediform"
              " WHERE ((ediform_ediprofile_id=:ediprofile_id)"
              "   AND  (ediform_type=:ediform_type) ); " );
    q.bindValue(":ediprofile_id", _ediprofileid);
    switch(pType)
    {
      case 1: // Invoice
        q.bindValue(":ediform_type", "invoice");
        break;
    };
    q.exec();
    if(q.first())
    {
      _ediformid = q.value("ediform_id").toInt();
      _mode = cEdit;
      populate();
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

bool ediForm::save()
{
  if(0 == _type->currentItem())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form"),
      tr("<p>You must select a Form Type before you can save this form.") );
    _type->setFocus();
    return false;
  }

  if(_file->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::critical( this, tr("Cannot Save EDI Form"),
			  tr("<p>You must specify a file name format before "
			     "you can save this form.") );
    _file->setFocus();
    return false;
  }

  // TODO: Add specific checking based on Output

  if( (cNew == _mode) && (-1 == _ediformid) )
  {
    q.prepare("SELECT nextval('ediform_ediform_id_seq') AS result; ");
    q.exec();
    if(q.first())
      _ediformid = q.value("result").toInt();
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
    else
    {
      QMessageBox::critical( this, tr("Cannot Save EDI Form"),
        tr("<p>Could not retrieve the next internal EDI Form ID.") );
      return false;
    }
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO ediform "
              "(ediform_id, ediform_ediprofile_id,"
              " ediform_type, ediform_output,"
              " ediform_query, ediform_file,"
              " ediform_option1, ediform_option2) "
              "VALUES(:ediform_id, :ediprofile_id,"
              "       :ediform_type, :ediform_output,"
              "       :ediform_query, :ediform_file,"
              "       :ediform_option1, :ediform_option2);" );
  else
    q.prepare("UPDATE ediform"
              "   SET ediform_type=:ediform_type,"
              "       ediform_output=:ediform_output,"
              "       ediform_query=:ediform_query,"
              "       ediform_file=:ediform_file,"
              "       ediform_option1=:ediform_option1,"
              "       ediform_option2=:ediform_option2"
              " WHERE (ediform_id=:ediform_id); " );

  q.bindValue(":ediprofile_id", _ediprofileid);
  q.bindValue(":ediform_id", _ediformid);
  q.bindValue(":ediform_query", _query->text().stripWhiteSpace());
  q.bindValue(":ediform_file", _file->text().stripWhiteSpace());

  switch(_type->currentItem())
  {
    case 1: // Invoice
      q.bindValue(":ediform_type", "invoice");
      break;
    case 0: // None Selected
    default:
      QMessageBox::critical( this, tr("Cannot Save EDI Form"),
        tr("<p>The selected type for this form is not recognized.") );
      _type->setFocus();
      return false;
  }

  switch(_output->currentItem())
  {
    case 0: // Report
      q.bindValue(":ediform_output", "report");
      q.bindValue(":ediform_option1", _reportReport->currentText());
      break;
    case 1: // CSV
      q.bindValue(":ediform_output", "csv");
      q.bindValue(":ediform_option1", _csvDelimeter->currentText());
      break;
    default:
      QMessageBox::critical( this, tr("Cannot Save EDI Form"),
        tr("<p>The selected output for this form is not recognized.") );
      _output->setFocus();
      return false;
  }

  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if(cNew == _mode)
    _mode = cEdit;

  return true;
}

void ediForm::sSave()
{
  if(save())
    accept();
}

void ediForm::sCSVNew()
{
  if(cNew == _mode)
  {
    if(QMessageBox::information( this, tr("Would you like to Save?"),
				tr("<p>The Form has not yet been saved to the "
				   "database. Before continuing you must save "
				   "the Form so it exists on the database.<p>"
				   "Would you like to save before continuing?"),
         (QMessageBox::Cancel | QMessageBox::Escape),
         (QMessageBox::Ok | QMessageBox::Default) ) != QMessageBox::Ok)
      return;

    if(!save())
      return;
  }

  ParameterList params;
  params.append("mode", "new");
  params.append("ediform_id", _ediformid);

  ediFormDetail newdlg(this, "", TRUE);
  newdlg.set(params);

  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediForm::sCSVEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ediform_id", _ediformid);
  params.append("ediformdetail_id", _csvDetails->id());

  ediFormDetail newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediForm::sCSVDelete()
{
  q.prepare("DELETE FROM ediformdetail"
            " WHERE (ediformdetail_id=:ediformdetail_id); ");
  q.bindValue(":ediformdetail_id", _csvDetails->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void ediForm::populate()
{
  q.prepare("SELECT ediform_type, ediform_output,"
            "       ediform_query, ediform_file,"
            "       ediform_option1, ediform_option2"
            "  FROM ediform"
            " WHERE (ediform_id=:ediform_id); ");
  q.bindValue(":ediform_id", _ediformid);
  q.exec();
  if(q.first())
  {
    if("invoice" == q.value("ediform_type").toString())
    {
      _type->setCurrentItem(1);
      sTypeSelected(1);
    }
    else
    {
      QMessageBox::critical( this, tr("Unknown Type Encountered"),
			    tr("<p>An unknown type of %1 was encountered on "
			       "the record you were trying to open.")
			    .arg(q.value("ediform_type").toString()) );
      return;
    }
    _query->setText(q.value("ediform_query").toString());
    _file->setText(q.value("ediform_file").toString());

    if("report" == q.value("ediform_output").toString())
    {
      _output->setCurrentItem(0);
      _stack->raiseWidget(0);
      _reportReport->setCurrentText(q.value("ediform_option1").toString());
    }
    else if("csv" == q.value("ediform_output").toString())
    {
      _output->setCurrentItem(1);
      _stack->raiseWidget(1);
      _csvDelimeter->setCurrentText(q.value("ediform_option1").toString());
    }
    else
    {
      QMessageBox::critical( this, tr("Unknown Output Encountered"),
			    tr("<p>An unknown output of %1 was encountered on "
			       "the record you were trying to open.")
			    .arg(q.value("ediform_output").toString()) );
      return;
    }

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void ediForm::sFillList()
{
  q.prepare("SELECT ediformdetail_id, ediformdetail_order, ediformdetail_name"
            "  FROM ediformdetail"
            " WHERE (ediformdetail_ediform_id=:ediform_id) "
            "ORDER BY ediformdetail_order; ");
  q.bindValue(":ediform_id", _ediformid);
  q.exec();
  _csvDetails->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
