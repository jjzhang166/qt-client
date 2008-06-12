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

#include "standardOperation.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a standardOperation as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
standardOperation::standardOperation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_stdTimes, SIGNAL(toggled(bool)), _setupTime, SLOT(setEnabled(bool)));
    connect(_stdTimes, SIGNAL(toggled(bool)), _runTime, SLOT(setEnabled(bool)));
    connect(_stdTimes, SIGNAL(toggled(bool)), _runQtyPer, SLOT(setEnabled(bool)));
    connect(_stdTimes, SIGNAL(toggled(bool)), _setupReport, SLOT(setEnabled(bool)));
    connect(_stdTimes, SIGNAL(toggled(bool)), _runReport, SLOT(setEnabled(bool)));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sHandleWorkCenter()));
    connect(_stdTimes, SIGNAL(toggled(bool)), _reportSetup, SLOT(setEnabled(bool)));
    connect(_stdTimes, SIGNAL(toggled(bool)), _reportRun, SLOT(setEnabled(bool)));
    connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardOperation::~standardOperation()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardOperation::languageChange()
{
    retranslateUi(this);
}


void standardOperation::init()
{
  _prodUOM->setType(XComboBox::UOMs);

  _wrkcnt->populate( "SELECT -1 AS wrkcnt_id, 'Any' AS wrkcnt_code, 1 AS orderby "
                     "UNION SELECT wrkcnt_id, wrkcnt_code, 2 AS orderby "
                     "FROM wrkcnt "
                     "ORDER BY orderby, wrkcnt_code;" );

  _setupReport->insertItem(tr("Direct Labor"));
  _setupReport->insertItem(tr("Overhead"));
  _setupReport->insertItem(tr("None"));
  _setupReport->setCurrentItem(-1);

  _runReport->insertItem(tr("Direct Labor"));
  _runReport->insertItem(tr("Overhead"));
  _runReport->insertItem(tr("None"));
  _runReport->setCurrentItem(-1);
}

enum SetResponse standardOperation::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdopn_id", &valid);
  if (valid)
  {
    _stdopnid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _toolReference->setEnabled(FALSE);
      _stdTimes->setEnabled(FALSE);
      _setupTime->setEnabled(FALSE);
      _setupReport->setEnabled(FALSE);
      _reportSetup->setEnabled(FALSE);
      _runTime->setEnabled(FALSE);
      _runReport->setEnabled(FALSE);
      _reportRun->setEnabled(FALSE);
      _runQtyPer->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardOperation::sCheck()
{
  if ((_mode == cNew) && (_number->text().length() != 0))
  {
    q.prepare( "SELECT stdopn_id "
               "FROM stdopn "
               "WHERE (UPPER(stdopn_number)=UPPER(:stdopn_number));" );
    q.bindValue(":stdopn_number", _number->text().stripWhiteSpace());
    q.exec();
    if (q.first())
    {
      _stdopnid = q.value("stdopn_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(FALSE);
    }
  }
}

void standardOperation::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().stripWhiteSpace().isEmpty(),
      tr("You must supply a Std. Oper. #."), _number },
    { _stdTimes->isChecked() &&
      _setupTime->toDouble() <= 0 && _runTime->toDouble() <= 0,
      tr("If you select Use Standard Times then please enter at least a Setup "
	 "or a Run Time."), _setupTime },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Standard Operation"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }


  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('stdopn_stdopn_id_seq') AS _stdopn_id;");
    if (q.first())
      _stdopnid = q.value("_stdopn_id").toInt();
    else
    {
      systemError( this, tr("A System Error occurred at standardOperation::%1.")
                         .arg(__LINE__) );
      reject();
      return;
    }
    
    q.prepare( "INSERT INTO stdopn "
               "( stdopn_id, stdopn_number,"
               "  stdopn_descrip1, stdopn_descrip2,"
               "  stdopn_wrkcnt_id, stdopn_toolref, stdopn_stdtimes,"
               "  stdopn_produom, stdopn_invproduomratio,"
               "  stdopn_sutime, stdopn_sucosttype, stdopn_reportsetup,"
               "  stdopn_rntime, stdopn_rncosttype, stdopn_reportrun,"
               "  stdopn_rnqtyper, stdopn_instructions ) "
               "VALUES "
               "( :stdopn_id, :stdopn_number,"
               "  :stdopn_descrip1, :stdopn_descrip2,"
               "  :stdopn_wrkcnt_id, :stdopn_toolref, :stdopn_stdtimes,"
               "  :stdopn_produom, :stdopn_invproduomratio,"
               "  :stdopn_sutime, :stdopn_sucosttype, :stdopn_reportsetup,"
               "  :stdopn_rntime, :stdopn_rncosttype, :stdopn_reportrun,"
               "  :stdopn_rnqtyper, :stdopn_instructions );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdopn "
               "SET stdopn_number=:stdopn_number, stdopn_wrkcnt_id=:stdopn_wrkcnt_id,"
               "    stdopn_descrip1=:stdopn_descrip1, stdopn_descrip2=:stdopn_descrip2,"
               "    stdopn_toolref=:stdopn_toolref, stdopn_stdtimes=:stdopn_stdtimes,"
               "    stdopn_produom=:stdopn_produom, stdopn_invproduomratio=:stdopn_invproduomratio,"
               "    stdopn_sutime=:stdopn_sutime, stdopn_sucosttype=:stdopn_sucosttype, stdopn_reportsetup=:stdopn_reportsetup,"
               "    stdopn_rntime=:stdopn_rntime, stdopn_rncosttype=:stdopn_rncosttype, stdopn_reportrun=:stdopn_reportrun,"
               "    stdopn_rnqtyper=:stdopn_rnqtyper, stdopn_instructions=:stdopn_instructions "
               "WHERE (stdopn_id=:stdopn_id);" );

  q.bindValue(":stdopn_id", _stdopnid);
  q.bindValue(":stdopn_wrkcnt_id", _wrkcnt->id());
  q.bindValue(":stdopn_number", _number->text());
  q.bindValue(":stdopn_descrip1", _description1->text());
  q.bindValue(":stdopn_descrip2", _description2->text());
  q.bindValue(":stdopn_toolref", _toolReference->text());
  q.bindValue(":stdopn_instructions", _instructions->text());
  q.bindValue(":stdopn_produom", _prodUOM->currentText().stripWhiteSpace().upper());
  q.bindValue(":stdopn_invproduomratio", _invProdUOMRatio->toDouble());
  q.bindValue(":stdopn_stdtimes", QVariant(_stdTimes->isChecked(), 0));
  q.bindValue(":stdopn_sutime", _setupTime->toDouble());
  q.bindValue(":stdopn_reportsetup", QVariant(_reportSetup->isChecked(), 0));

  if (_setupReport->currentItem() == 0)
    q.bindValue(":stdopn_sucosttype", "D");
  else if (_setupReport->currentItem() == 1)
    q.bindValue(":stdopn_sucosttype", "O");
  else if (_setupReport->currentItem() == 2)
    q.bindValue(":stdopn_sucosttype", "N");

  q.bindValue(":stdopn_rntime", _runTime->toDouble());
  q.bindValue(":stdopn_reportrun", QVariant(_reportRun->isChecked(), 0));

  if (_runReport->currentItem() == 0)
    q.bindValue(":stdopn_rncosttype", "D");
  else if (_runReport->currentItem() == 1)
    q.bindValue(":stdopn_rncosttype", "O");
  else if (_runReport->currentItem() == 2)
    q.bindValue(":stdopn_rncosttype", "N");

  q.bindValue(":stdopn_rnqtyper", _runQtyPer->toDouble());
  q.exec();

  done(_stdopnid);
}

void standardOperation::populate()
{
  XSqlQuery stdopn;
  stdopn.prepare( "SELECT stdopn_number, stdopn_descrip1, stdopn_instructions,"
                  "       stdopn_descrip2, stdopn_toolref,"
                  "       stdopn_wrkcnt_id, stdopn_stdtimes,"
                  "       stdopn_produom, stdopn_sucosttype, stdopn_rncosttype,"
                  "       formatQty(stdopn_sutime) AS sutime, stdopn_reportsetup,"
                  "       formatQty(stdopn_rntime) AS rntime, stdopn_reportrun,"
                  "       formatQty(stdopn_rnqtyper) AS rnqtyper,"
                  "       formatUOMRatio(stdopn_invproduomratio) AS invproduomratio "
                  "FROM stdopn "
                  "WHERE (stdopn_id=:stdopn_id);" );
  stdopn.bindValue(":stdopn_id", _stdopnid);
  stdopn.exec();
  if (stdopn.first())
  {
    _number->setText(stdopn.value("stdopn_number"));
    _description1->setText(stdopn.value("stdopn_descrip1"));
    _description2->setText(stdopn.value("stdopn_descrip2"));
    _toolReference->setText(stdopn.value("stdopn_toolref"));
    _wrkcnt->setId(stdopn.value("stdopn_wrkcnt_id").toInt());
    _prodUOM->setText(stdopn.value("stdopn_produom"));
    _setupTime->setText(stdopn.value("sutime"));
    _reportSetup->setChecked(stdopn.value("stdopn_reportsetup").toBool());
    _runTime->setText(stdopn.value("rntime"));
    _reportRun->setChecked(stdopn.value("stdopn_reportrun").toBool());
    _runQtyPer->setText(stdopn.value("rnqtyper"));
    _invProdUOMRatio->setText(stdopn.value("invproduomratio"));
    _instructions->setText(stdopn.value("stdopn_instructions").toString());

    bool field = stdopn.value("stdopn_stdtimes").toBool();
    _stdTimes->setChecked(field);
    _setupTime->setEnabled(field);
    _setupReport->setEnabled(field);
    _runTime->setEnabled(field);
    _runReport->setEnabled(field);
    _runQtyPer->setEnabled(field);

    if (stdopn.value("stdopn_sucosttype").toString() == "D")
      _setupReport->setCurrentItem(0);
    else if (stdopn.value("stdopn_sucosttype").toString() == "O")
      _setupReport->setCurrentItem(1);
    else if (stdopn.value("stdopn_sucosttype").toString() == "N")
      _setupReport->setCurrentItem(2);

    if (stdopn.value("stdopn_rncosttype").toString() == "D")
      _runReport->setCurrentItem(0);
    else if (stdopn.value("stdopn_rncosttype").toString() == "O")
      _runReport->setCurrentItem(1);
    else if (stdopn.value("stdopn_rncosttype").toString() == "N")
      _runReport->setCurrentItem(2);
  }
}

void standardOperation::sHandleWorkCenter()
{
  if (_wrkcnt->id() != -1)
  {
    q.prepare( "SELECT formatTime(wrkcnt_avgsutime) AS setup "
               "FROM wrkcnt "
               "WHERE (wrkcnt_id=:wrkcnt_id);" );
    q.bindValue(":wrkcnt_id", _wrkcnt->id());
    q.exec();
    if (q.first())
      _setupTime->setText(q.value("setup").toString());
  }
}

