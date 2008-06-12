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

#include <QMessageBox>
#include <QSqlError>
#include "revisioncluster.h"

QString RevisionLineEdit::typeText()
{
  return _typeText;
}

RevisionCluster::RevisionCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  setLabel("Revision: ");
  addNumberWidget(new RevisionLineEdit(this, pName));
  _info->hide();
  if (_x_metrics)
    if (!_x_metrics->boolean("RevControl"))
      _list->hide();

  connect(_number, SIGNAL(modeChanged()), this, SLOT(sModeChanged()));
  connect(_number, SIGNAL(canActivate(bool)), this, SLOT(sCanActivate(bool)));
}
RevisionLineEdit::RevisionLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "rev", "rev_id", "rev_number", 0, "CASE WHEN rev_status='A' THEN 'Active' WHEN rev_status='P' THEN 'Pending' ELSE 'Inactive' END", 0, pName)
{
  setTitles(tr("Revision"), tr("Revisions"));
  _type=All;
  _allowNew=FALSE;
  if (_x_metrics)
    _isRevControl=(_x_metrics->boolean("RevControl"));
}

RevisionLineEdit::RevisionTypes RevisionCluster::type()
{
  return (static_cast<RevisionLineEdit*>(_number))->type();
}

RevisionLineEdit::RevisionTypes RevisionLineEdit::type()
{
  return _type;
}

RevisionLineEdit::Modes RevisionCluster::mode()
{
  return (static_cast<RevisionLineEdit*>(_number))->mode();
}

RevisionLineEdit::Modes RevisionLineEdit::mode()
{
  return _mode;
}
void RevisionCluster::activate()
{
  XSqlQuery activate;
  activate.prepare("SELECT activateRev(:rev_id) AS result;");
  activate.bindValue(":rev_id", _number->id());
  activate.exec();
  if (activate.first())
    setDescription("Active");
  else  if (activate.lastError().type() != QSqlError::None)
  {
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                                              .arg(__FILE__)
                                              .arg(__LINE__),
    activate.lastError().databaseText());
        return; 
  }
}

void RevisionCluster::sCanActivate(bool p)
{
  emit canActivate(p);
}
void RevisionCluster::setActive()
{
  return (static_cast<RevisionLineEdit*>(_number))->setActive();
}

void RevisionCluster::setMode(QString pmode)
{
  return (static_cast<RevisionLineEdit*>(_number))->setMode(pmode);
}

void RevisionCluster::setMode(RevisionLineEdit::Modes pmode)
{
  return (static_cast<RevisionLineEdit*>(_number))->setMode(pmode);
}

void RevisionCluster::setTargetId(int ptargetid)
{
  return (static_cast<RevisionLineEdit*>(_number))->setTargetId(ptargetid);
}

void RevisionCluster::setType(QString ptype)
{
  return (static_cast<RevisionLineEdit*>(_number))->setType(ptype);
}

void RevisionCluster::setType(RevisionLineEdit::RevisionTypes ptype)
{
  return (static_cast<RevisionLineEdit*>(_number))->setType(ptype);
}

void RevisionCluster::sModeChanged()
{
  if  (_x_privileges)
    if (_x_metrics->boolean("RevControl"))
    {
          _list->setVisible(((RevisionLineEdit::View==(static_cast<RevisionLineEdit*>(_number))->mode()) && (_x_privileges->check("ViewInactiveRevisions") || _x_privileges->check("MaintainRevisions"))) ||
                                                          ((RevisionLineEdit::Use==(static_cast<RevisionLineEdit*>(_number))->mode()) && _x_privileges->check("UseInactiveRevisions")) ||
                                                              ((RevisionLineEdit::Maintain==(static_cast<RevisionLineEdit*>(_number))->mode()) && (_x_privileges->check("MaintainRevisions") || _x_privileges->check("ViewInactiveRevisions"))));
          (static_cast<RevisionLineEdit*>(_number))->setDisabled(((RevisionLineEdit::Maintain==(static_cast<RevisionLineEdit*>(_number))->mode()) && !_x_privileges->check("MaintainRevisions")) ||
                                                                    ((RevisionLineEdit::Use==(static_cast<RevisionLineEdit*>(_number))->mode()) ||
                                                                        (RevisionLineEdit::View==(static_cast<RevisionLineEdit*>(_number))->mode())));  
        }
        else
    {
      _list->hide();
      (static_cast<RevisionLineEdit*>(_number))->setEnabled(TRUE);
    }
}

void RevisionLineEdit::setMode(Modes pMode)
{
  if (_mode!=pMode)
  {
    _mode = pMode;
    if  (_x_privileges)
      _allowNew=((pMode=Maintain) && (_x_privileges->check("MaintainRevisions")));
    emit modeChanged();
  }
}

void RevisionLineEdit::setMode(QString pmode)
{
  if (pmode == "View")
    setMode(View);
  else if (pmode == "Use")
    setMode(Use);
  else if (pmode == "Maintain")
    setMode(Maintain);
  else
  {
    QMessageBox::critical(this, tr("Invalid Mode"),
                          tr("RevisionLineEdit::setMode received "
                             "an invalid Mode %1").arg(pmode));
    setMode(View);
  }
}

void RevisionLineEdit::setActive()
{
  if (_isRevControl)
  {
    XSqlQuery revision;
    revision.prepare( "SELECT getActiveRevId(:target_type,:target_id) AS rev_id;" );
    revision.bindValue(":target_type", typeText());
    revision.bindValue(":target_id", _targetId);
    revision.exec();
    if (revision.first())
    {
      setId(revision.value("rev_id").toInt());
    }
    else
    {
      setId(-1);
    }
  }
}

void RevisionLineEdit::setId(const int pId)
{
  VirtualClusterLineEdit::setId(pId);
  _cachenum = text();
  if  (_x_privileges)
    emit canActivate((_mode==Maintain) && 
                        (_x_privileges->check("MaintainRevisions")) &&
                                        (_description=="Pending"));
}

void RevisionLineEdit::setTargetId(int pItem)
{
  if (_isRevControl)
  {
    _targetId = pItem;
    setExtraClause(QString(" ((rev_target_type='%1') AND (rev_target_id=%2)) ").arg(typeText()).arg(pItem));
    setActive();
  }
}

void RevisionLineEdit::setType(QString ptype)
{
  if (ptype == "BOM")
    setType(BOM);
  else if (ptype == "BOO")
    setType(BOO);
  else
  {
    QMessageBox::critical(this, tr("Invalid Revision Type"),
                          tr("RevisionLineEdit::setType received "
                             "an invalid RevisionType %1").arg(ptype));
    setType(All);
  }
}

void RevisionLineEdit::setType(RevisionTypes pType)
{
  _type = pType;
  switch (pType)
  {
    case BOM:
          _typeText="BOM";
          break;

    case BOO:
          _typeText="BOO";
          break;

        default:
          _typeText="All";
        break;
  }
}

void RevisionLineEdit::sParse()
{
  if ((_isRevControl) && (!_parsed))
  {
    QString stripped = text().stripWhiteSpace().upper();
    if (stripped.length() == 0)
    {
      setId(-1);
          _parsed = TRUE;
    }
    else
    {
        
      XSqlQuery numQ;
          numQ.prepare("SELECT rev_id, "
                       "rev_number, "
                       "CASE WHEN rev_status='A' THEN "
                       "  'Active' "
                       "WHEN rev_status='P' THEN "
                       "  'Pending' "
                       "ELSE 'Inactive' "
                       "END AS status "
                       "FROM rev "
                       "WHERE ((rev_number=UPPER(:number))"
                       " AND (rev_target_id=:target_id)"
                       " AND (rev_target_type=:target_type));");
          numQ.bindValue(":number", stripped);
          numQ.bindValue(":target_id",_targetId);
          numQ.bindValue(":target_type",typeText());
          numQ.exec();
          if (numQ.first())
          {
            _valid = true;
            setId(numQ.value("rev_id").toInt());
                setText(numQ.value("rev_number").toString());
          }
      else
      {
            if (_allowNew)
            {
          if (QMessageBox::question(this, tr("Create New Revision?"),
                  tr("Revision does not exist.  Would you like to create a new one from the current active revision?"),
                             QMessageBox::Yes | QMessageBox::Default,
                             QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
                  {
                    XSqlQuery newrev;
                        if (_type==BOM)
                          newrev.prepare("SELECT createBomRev(:target_id,:revision) AS result;");
                        else if (_type==BOO)
                      newrev.prepare("SELECT createBooRev(:target_id,:revision) AS result;");
                        newrev.bindValue(":target_id", _targetId);
                        newrev.bindValue(":revision", text());
                    newrev.exec();
                        if (newrev.first())
                          setId(newrev.value("result").toInt());
                        else
                        {
                  setText(_cachenum);
                  if (newrev.lastError().type() != QSqlError::None)
                  QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
                      newrev.lastError().databaseText());
                        }
                  }
                  else
                  {
                    setText(_cachenum);
                  }
                }
                else 
            {
                  setText(_cachenum);
              if (numQ.lastError().type() != QSqlError::None)
              QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                              .arg(__FILE__)
                              .arg(__LINE__),
          numQ.lastError().databaseText());
        }
          }
        }
  }
  _parsed = TRUE;
  emit valid(_valid);
  emit parsed();
}

