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

#include "woOperation.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "inputManager.h"
/*
 *  Constructs a woOperation as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
woOperation::woOperation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_stdopn, SIGNAL(newID(int)), this, SLOT(sHandleStdopn(int)));
    connect(_fixedFont, SIGNAL(toggled(bool)), this, SLOT(sHandleFont(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_wo, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(_runTime, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
    connect(_invProdUOMRatio, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
    connect(_wo, SIGNAL(qtyOrderedChanged(const QString&)), _qtyOrdered, SLOT(setText(const QString&)));
    connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateWoInfo(int)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
woOperation::~woOperation()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void woOperation::languageChange()
{
    retranslateUi(this);
}


void woOperation::init()
{
  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);
  _fixedFont->setChecked(_preferences->boolean("UsedFixedWidthFonts"));

  _prodUOM->setType(XComboBox::UOMs);

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code" );

  _stdopn->populate( "SELECT -1, TEXT('None') AS stdopn_number "
                     "UNION SELECT stdopn_id, stdopn_number "
                     "FROM stdopn "
                     "ORDER BY stdopn_number" );
}

enum SetResponse woOperation::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
  }

  param = pParams.value("wooper_id", &valid);
  if (valid)
  {
    _wooperid = param.toInt();
    _wo->setReadOnly(TRUE);

    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _setupTimeConsumedLit->setEnabled(FALSE);
      _setupTimeRemainingLit->setEnabled(FALSE);
      _setupComplete->setEnabled(FALSE);
      _runTimeConsumedLit->setEnabled(FALSE);
      _runTimeRemainingLit->setEnabled(FALSE);
      _runComplete->setEnabled(FALSE);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _wo->setReadOnly(TRUE);
      _executionDay->setEnabled(FALSE);
      _stdopn->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _toolingReference->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _setupTime->setEnabled(FALSE);
      _runTime->setEnabled(FALSE);
      _reportSetup->setEnabled(FALSE);
      _reportRun->setEnabled(FALSE);
      _receiveStock->setEnabled(FALSE);
      _issueComp->setEnabled(FALSE);
      _setupComplete->setEnabled(FALSE);
      _runComplete->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
      _fixedFont->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void woOperation::sSave()
{
  if (_receiveStock->isChecked())
  {
    q.prepare( "UPDATE wooper "
               "SET wooper_rcvinv=FALSE "
               "WHERE (wooper_wo_id=:wo_id);" );
    q.bindValue(":wo_id", _wo->id());
    q.exec();
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('wooper_wooper_id_seq') AS wooper_id;");
    if (q.first())
      _wooperid = q.value("wooper_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }

    q.prepare( "INSERT INTO wooper "
               "( wooper_id, wooper_wo_id,"
               "  wooper_seqnumber,"
               "  wooper_wrkcnt_id, wooper_stdopn_id,"
               "  wooper_descrip1, wooper_descrip2, wooper_toolref,"
               "  wooper_produom, wooper_invproduomratio,"
               "  wooper_sutime, wooper_surpt, wooper_sucomplete, wooper_suconsumed,"
               "  wooper_rntime, wooper_rnqtyper,"
               "  wooper_rnrpt, wooper_rncomplete, wooper_rnconsumed,"
               "  wooper_rcvinv, wooper_issuecomp,"
               "  wooper_scheduled,"
               "  wooper_qtyrcv,"
               "  wooper_instruc )"
               "SELECT :wooper_id, :wo_id,"
               "       (COALESCE(MAX(wooper_seqnumber), 0) + 10),"
               "       :wooper_wrkcnt_id, :wooper_stdopn_id,"
               "       :wooper_descrip1, :wooper_descrip2, :wooper_toolref,"
               "       :wooper_produom, :wooper_invproduomratio,"
               "       :wooper_sutime, :wooper_surpt, :wooper_sucomplete, 0,"
               "       :wooper_rntime, :wooper_rnqtyper,"
               "       :wooper_rnrpt, :wooper_rncomplete, 0,"
               "       :wooper_rcvinv, :wooper_issuecomp,"
               "       (wo_startdate + :executionDay),"
               "       0.0,"
               "       :wooper_instruc "
               "FROM wo LEFT OUTER JOIN wooper ON (wooper_wo_id=wo_id) "
               "WHERE (wo_id=:wo_id) "
               "GROUP BY wo_startdate;" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE wooper "
               "SET wooper_wrkcnt_id=:wooper_wrkcnt_id, wooper_stdopn_id=:wooper_stdopn_id,"
               "    wooper_descrip1=:wooper_descrip1, wooper_descrip2=:wooper_descrip2, wooper_toolref=:wooper_toolref,"
               "    wooper_produom=:wooper_produom, wooper_invproduomratio=:wooper_invproduomratio,"
               "    wooper_sutime=:wooper_sutime, wooper_surpt=:wooper_surpt, wooper_sucomplete=:wooper_sucomplete,"
               "    wooper_rntime=:wooper_rntime, wooper_rnqtyper=:wooper_rnqtyper,"
               "    wooper_rnrpt=:wooper_rnrpt, wooper_rncomplete=:wooper_rncomplete,"
               "    wooper_rcvinv=:wooper_rcvinv, wooper_issuecomp=:wooper_issuecomp,"
               "    wooper_scheduled=(wo_startdate + :executionDay - 1),"
               "    wooper_instruc=:wooper_instruc "
               "FROM wo "
               "WHERE ( (wooper_wo_id=wo_id)"
               " AND (wooper_id=:wooper_id) );" );

  q.bindValue(":wooper_id", _wooperid);
  q.bindValue(":wo_id", _wo->id());
  q.bindValue(":wooper_wrkcnt_id", _wrkcnt->id());
  q.bindValue(":wooper_stdopn_id", _stdopn->id());
  q.bindValue(":wooper_descrip1", _description1->text());
  q.bindValue(":wooper_descrip2", _description2->text());
  q.bindValue(":wooper_toolref", _toolingReference->text());
  q.bindValue(":wooper_produom", _prodUOM->currentText());
  q.bindValue(":wooper_invproduomratio", _invProdUOMRatio->toDouble());
  q.bindValue(":wooper_sutime", _setupTime->toDouble());
  q.bindValue(":wooper_rntime", _runTime->toDouble());
  q.bindValue(":wooper_rnqtyper", (_runTime->toDouble() / _cachedQtyOrdered));
  q.bindValue(":executionDay", _executionDay->value());
  q.bindValue(":wooper_instruc", _instructions->text());
  q.bindValue(":wooper_surpt", QVariant(_reportSetup->isChecked(), 0));
  q.bindValue(":wooper_sucomplete", QVariant(_setupComplete->isChecked(), 0));
  q.bindValue(":wooper_rnrpt", QVariant(_reportRun->isChecked(), 0));
  q.bindValue(":wooper_rncomplete", QVariant(_runComplete->isChecked(), 0));
  q.bindValue(":wooper_rcvinv", QVariant(_receiveStock->isChecked(), 0));
  q.bindValue(":wooper_issuecomp", QVariant(_issueComp->isChecked(), 0));
  q.exec();

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooperid, TRUE);

  done(_wooperid);
}

void woOperation::sHandleFont(bool pFixed)
{
  if (pFixed)
    _instructions->setFont(omfgThis->fixedFont());
  else
    _instructions->setFont(omfgThis->systemFont());
}

void woOperation::sHandleStdopn(int pStdopnid)
{
  if (_stdopn->id() != -1)
  {
    q.prepare( "SELECT stdopn_descrip1, stdopn_descrip2, stdopn_toolref,"
               "       stdopn_wrkcnt_id, stdopn_stdtimes,"
               "       stdopn_produom,"
               "       formatQty(stdopn_sutime) AS sutime,"
               "       formatQty(stdopn_rntime) AS rntime,"
               "       formatUOMRatio(stdopn_invproduomratio) AS invproduomratio "
               "FROM stdopn "
               "WHERE (stdopn_id=:stdopn_id);" );
    q.bindValue(":stdopn_id", pStdopnid);
    q.exec();
    if (q.first())
    {
      _description1->setText(q.value("stdopn_descrip1"));
      _description2->setText(q.value("stdopn_descrip2"));
      _toolingReference->setText(q.value("stdopn_toolref"));

      _wrkcnt->setId(q.value("stdopn_wrkcnt_id").toInt());

      if (q.value("stdopn_stdtimes").toBool())
      {
        _setupTime->setText(q.value("sutime"));
        _prodUOM->setText(q.value("stdopn_produom"));
        _invProdUOMRatio->setText(q.value("invproduomratio"));
      }
    }
  }
}

void woOperation::sCalculateInvRunTime()
{
  if ((_runTime->toDouble() != 0.0) && (_invProdUOMRatio->toDouble() != 0.0))
  {
    _invRunTime->setText(formatCost(_runTime->toDouble() / _cachedQtyOrdered / _invProdUOMRatio->toDouble()));
    _invPerMinute->setText(formatCost((1 / (_runTime->toDouble() / _cachedQtyOrdered / _invProdUOMRatio->toDouble()))));
  }
  else
  {
    _invRunTime->setText(formatCost(0.0));
    _invPerMinute->setText(formatCost(0.0));
  }
}

void woOperation::sPopulateWoInfo(int pWoid)
{
  if(pWoid == -1)
    return;

  q.prepare( "SELECT wo_qtyord, uom_name "
             "FROM wo, itemsite, item, uom "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (item_inv_uom_id=uom_id)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", pWoid);
  q.exec();
  if (q.first())
  {
    _cachedQtyOrdered = q.value("wo_qtyord").toDouble();

    _invUOM1->setText(q.value("uom_name").toString());
    _invUOM2->setText(q.value("uom_name").toString());
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}


void woOperation::populate()
{
  XSqlQuery wooper;
  wooper.prepare( "SELECT wooper_wo_id, wooper_seqnumber, wooper_wrkcnt_id, wooper_stdopn_id,"
                  "       wooper_descrip1, wooper_descrip2, wooper_toolref,"
                  "       wooper_produom, formatUOMRatio(wooper_invproduomratio) AS invproduomratio,"
                  "       formatTime(wooper_sutime) AS sutime, wooper_surpt,"
                  "       formatTime(wooper_rntime) AS rntime, wooper_rnrpt,"
                  "       formatTime(wooper_suconsumed) AS suconsumed, wooper_suconsumed, wooper_sucomplete,"
                  "       formatTime(wooper_rnconsumed) AS rnconsumed, wooper_rnconsumed, wooper_rncomplete,"
                  "       formatTime(noNeg(wooper_sutime - wooper_suconsumed)) AS suremaining,"
                  "       formatTime(noNeg(wooper_rntime - wooper_rnconsumed)) AS rnremaining,"
                  "       wooper_rcvinv, wooper_issuecomp,"
                  "       (DATE(wooper_scheduled) - wo_startdate + 1) AS executionday,"
                  "       wooper_instruc "
                  "FROM wooper, wo "
                  "WHERE ( (wooper_wo_id=wo_id)"
                  " AND (wooper_id=:wooper_id) );" );
  wooper.bindValue(":wooper_id", _wooperid);
  wooper.exec();
  if (wooper.first())
  {
    _wo->setId(wooper.value("wooper_wo_id").toInt());
    _stdopn->setId(wooper.value("wooper_stdopn_id").toInt());
    _operSeqNum->setText(wooper.value("wooper_seqnumber").toString());
    _description1->setText(wooper.value("wooper_descrip1"));
    _description2->setText(wooper.value("wooper_descrip2"));
    _prodUOM->setText(wooper.value("wooper_produom"));
    _invProdUOMRatio->setText(wooper.value("invproduomratio"));
    _toolingReference->setText(wooper.value("wooper_toolref"));
    _wrkcnt->setId(wooper.value("wooper_wrkcnt_id").toInt());

    _setupTime->setText(wooper.value("sutime"));
    _setupTimeConsumed->setText(wooper.value("suconsumed").toString());
    _setupComplete->setChecked(wooper.value("wooper_sucomplete").toBool());
    _setupTimeRemaining->setText(wooper.value("suremaining").toString());
    _reportSetup->setChecked(wooper.value("wooper_surpt").toBool());

    _runTime->setText(wooper.value("rntime"));
    _runTimeConsumed->setText(wooper.value("rnconsumed").toString());
    _runComplete->setChecked(wooper.value("wooper_rncomplete").toBool());
    _runTimeRemaining->setText(wooper.value("rnremaining").toString());
    _reportRun->setChecked(wooper.value("wooper_rnrpt").toBool());

    _receiveStock->setChecked(wooper.value("wooper_rcvinv").toBool());
    _issueComp->setChecked(wooper.value("wooper_issuecomp").toBool());
    _executionDay->setValue(wooper.value("executionday").toInt());
    _instructions->setText(wooper.value("wooper_instruc").toString());

    if ((wooper.value("wooper_suconsumed").toDouble() > 0) || (wooper.value("wooper_rnconsumed").toDouble() > 0))
    {
      _stdopn->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _toolingReference->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
    }
  }
}

