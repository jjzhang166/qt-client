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

#include "glSeries.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "glSeriesItem.h"
#include "storedProcErrorLookup.h"
#include "submitAction.h"

#define cPostStandardJournal 0x10

glSeries::glSeries(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,	SIGNAL(clicked()),	this, SLOT(sClose()));
  connect(_delete,	SIGNAL(clicked()),	this, SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_new,		SIGNAL(clicked()),	this, SLOT(sNew()));
  connect(_post,	SIGNAL(clicked()),	this, SLOT(sPost()));
  connect(_save,	SIGNAL(clicked()),	this, SLOT(sSave()));

  _glseries->addColumn(tr("Account"), -1,           Qt::AlignLeft  );
  _glseries->addColumn(tr("Debit"),   _moneyColumn, Qt::AlignRight );
  _glseries->addColumn(tr("Credit"),  _moneyColumn, Qt::AlignRight );

  _source->setText("G/L");
  _source->setEnabled(false);

  _doctype->addItem("AD");
  _doctype->addItem("CD");
  _doctype->addItem("CK");
  _doctype->addItem("CM");
  _doctype->addItem("CR");
  _doctype->addItem("CT");
  _doctype->addItem("DM");
  _doctype->addItem("DS");
  _doctype->addItem("IN");
  _doctype->addItem("JE");
  _doctype->addItem("MM");
  _doctype->addItem("PO");
  _doctype->addItem("SO");
  _doctype->addItem("ST");
  _doctype->addItem("VO");
  _doctype->addItem("WO");
  _doctype->setCurrentIndex(_doctype->findText("JE"));

  _submit = false;
}

glSeries::~glSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void glSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse glSeries::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  _submit = pParams.inList("submit");
  if((_submit)  && (_metrics->boolean("EnableBatchManager")))
    _post->setText(tr("Submit"));
  else
    _submit = false;

  param = pParams.value("glSequence", &valid);
  if (valid)
  {
    _glsequence = param.toInt();
    q.prepare("SELECT DISTINCT glseries_distdate, glseries_source,"
	      "                glseries_doctype,  glseries_docnumber,"
	      "                glseries_notes"
              "  FROM glseries"
              " WHERE (glseries_sequence=:glseries_sequence);" );
    q.bindValue(":glseries_sequence", _glsequence);
    q.exec();
    if(q.first())
    {
      _date->setDate(q.value("glseries_distdate").toDate());
      _source->setText(q.value("glseries_source").toString());
      int idx = _doctype->findText(q.value("glseries_doctype").toString());
      if(idx < 0)
        _doctype->addItem(q.value("glseries_doctype").toString());
      _doctype->setCurrentIndex(_doctype->findText(q.value("glseries_doctype").toString()));
      _docnumber->setText(q.value("glseries_docnumber").toString());
      _notes->setText(q.value("glseries_notes").toString());
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
    sFillList();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT fetchGLSequence() AS glsequence;");
      if (q.first())
        _glsequence = q.value("glsequence").toInt();
      else if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _new->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _post->setFocus();
    }
    else if (param.toString() == "postStandardJournal")
    {
      _mode = cPostStandardJournal;
      
      _doctype->setCurrentIndex(_doctype->findText("ST"));
      _doctype->setEnabled(false);
      _date->setEnabled(FALSE);
      _notes->setEnabled(FALSE);

      q.prepare( "SELECT DISTINCT glseries_docnumber, stdjrnl_notes "
                 "  FROM glseries, stdjrnl "
                 " WHERE ( (stdjrnl_name=glseries_docnumber) "
                 "   AND   (stdjrnl_notes IS NOT NULL) "
                 "   AND   (stdjrnl_notes != '') "
                 "   AND   (glseries_sequence=:glsequence) ); ");
      q.bindValue(":glsequence", _glsequence);
      q.exec();
      if (q.lastError().type() != QSqlError::None)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
      while(q.next())
        _notes->append(q.value("glseries_docnumber").toString() + ": " + q.value("stdjrnl_notes").toString() + "\n\n");

      _post->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _close->setText("&Close");
      _post->setEnabled(false);
      _save->setEnabled(false);
      _new->setEnabled(false);
      _edit->setEnabled(false);
      _delete->setEnabled(false);
      _source->setEnabled(false);
      _doctype->setEnabled(false);
      _docnumber->setEnabled(false);
      _date->setEnabled(false);
      _notes->setEnabled(false);

      _close->setFocus();
    }
  }

  return NoError;
}

void glSeries::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("glSequence", _glsequence);

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void glSeries::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("glseries_id", _glseries->id());

  if (_mode == cPostStandardJournal)
    params.append("postStandardJournal");

  glSeriesItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void glSeries::sDelete()
{
  q.prepare( "DELETE FROM glseries "
             "WHERE (glseries_id=:glseries_id);" );
  q.bindValue(":glseries_id", _glseries->id());
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

bool glSeries::update()
{
  if (_mode != cPostStandardJournal)
  {
    if(!_date->isValid())
    {
      QMessageBox::information( this, tr("Cannot Post G/L Series"),
				tr("<p>You must enter a Distribution Date for this Series.") );
      _date->setFocus();
      return false;
    }

    if(_metrics->boolean("MandatoryGLEntryNotes") && _notes->text().stripWhiteSpace().isEmpty())
    {
      QMessageBox::information( this, tr("Cannot Post G/L Series"),
				tr("<p>You must enter some Notes to describe this transaction.") );
      _notes->setFocus();
      return false;
    }

    q.prepare( "UPDATE glseries "
               "SET glseries_notes=:glseries_notes, "
               "    glseries_source=:source,"
	       "    glseries_doctype=:doctype,"
	       "    glseries_docnumber=:docnumber,"
               "    glseries_distdate=:glseries_distdate "
               "WHERE (glseries_sequence=:glseries_sequence);" );
    q.bindValue(":glseries_notes", _notes->text());
    q.bindValue(":source",	_source->text());
    q.bindValue(":doctype",	_doctype->currentText());
    q.bindValue(":docnumber",	_docnumber->text());
    q.bindValue(":glseries_sequence", _glsequence);
    q.bindValue(":glseries_distdate", _date->date());
    q.exec();
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  q.prepare("SELECT SUM(glseries_amount) AS result "
            "  FROM glseries "
            " WHERE (glseries_sequence=:glseries_sequence); ");
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if(q.first())
  {
    double result = q.value("result").toDouble();
    if(result != 0)
    {
      QMessageBox::critical( this, tr("Cannot Post G/L Series"),
			     tr("<p>The G/L Series information is unbalanced and cannot be posted. Please correct this before continuing.") );
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  omfgThis->sGlSeriesUpdated();
  return true;
}

void glSeries::sSave()
{
  if (! update())
    return;
  accept();
}

void glSeries::sPost()
{
  if (! update())
    return;

  if(_submit)
  {
    ParameterList params;

    params.append("action_name", "PostGLSeries");
    params.append("glseries_sequence", _glsequence); 

    submitAction newdlg(this, "", true);
    newdlg.set(params);

    if(newdlg.exec() == XDialog::Accepted)
    {
      // TODO: do something?
    }
  }
  else
  {
    q.prepare("SELECT postGLSeriesNoSumm(:glseries_sequence) AS return;");
    q.bindValue(":glseries_sequence", _glsequence);
    q.exec();
    if (q.first())
    {
      int returnVal = q.value("return").toInt();
      if (returnVal < 0)
      {
        systemError(this, storedProcErrorLookup("postGLSeriesNoSumm", returnVal),
		    __FILE__, __LINE__);
        return;
      }
    }
    else if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if(cPostStandardJournal == _mode)
  {
    accept();
    return;
  }

  _date->clear();
  _notes->clear();
  _debits->clear();
  _credits->clear();
  _glseries->clear();
  
  ParameterList params;
  params.append("mode", "new");
  set(params);
}

void glSeries::sClose()
{
  if (cNew == _mode &&
      (_glseries->topLevelItemCount() <= 0 ||
      QMessageBox::question(this, tr("Delete G/L Series?"),
			    tr("<p>Are you sure you want to delete this G/L "
			       "Series Entry?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes))
  {
    q.prepare("SELECT deleteGLSeries(:glsequence);");
    q.bindValue(":glsequence", _glsequence);
    q.exec();
    if (q.lastError().type() != QSqlError::None)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    omfgThis->sGlSeriesUpdated();
  }

  reject();
}

void glSeries::sFillList()
{
  q.prepare( "SELECT glseries_id, (formatGLAccount(accnt_id) || '-' || accnt_descrip),"
             "       CASE WHEN (glseries_amount < 0) THEN formatMoney(glseries_amount * -1)"
             "            ELSE ''"
             "       END AS debit,"
             "       CASE WHEN (glseries_amount > 0) THEN formatMoney(glseries_amount)"
             "            ELSE ''"
             "       END AS credit "
             "FROM glseries, accnt "
             "WHERE ( (glseries_accnt_id=accnt_id)"
             " AND (glseries_sequence=:glseries_sequence) );" );
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _glseries->clear();
  _glseries->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare( "SELECT formatMoney( SUM( CASE WHEN (glseries_amount < 0) THEN (glseries_amount * -1)"
             "                              ELSE 0"
             "                         END ) ) AS f_debit,"
             "       formatMoney( SUM( CASE WHEN (glseries_amount > 0) THEN glseries_amount"
             "                              ELSE 0"
             "                         END ) ) AS f_credit,"
             "       (SUM(glseries_amount) <> 0) AS oob "
             "FROM glseries "
             "WHERE (glseries_sequence=:glseries_sequence);" );
  q.bindValue(":glseries_sequence", _glsequence);
  q.exec();
  if (q.first())
  {
    _debits->setText(q.value("f_debit").toString());
    _credits->setText(q.value("f_credit").toString());

    if (q.value("oob").toBool())
    {
      _debits->setPaletteForegroundColor(QColor("red"));
      _credits->setPaletteForegroundColor(QColor("red"));
    }
    else
    {
      _debits->setPaletteForegroundColor(QColor("black"));
      _credits->setPaletteForegroundColor(QColor("black"));
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
