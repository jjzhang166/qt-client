/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configurePD.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "storedProcErrorLookup.h"
#include "guiclient.h"
#include "errorReporter.h"

configurePD::configurePD(QWidget* parent, const char* name, bool /*modal*/, Qt::WindowFlags fl)
    : XAbstractConfigure(parent, fl)
{
  XSqlQuery configureconfigurePD;
  setupUi(this);

  if (name)
    setObjectName(name);

  _inactiveBomItems->setChecked(_metrics->boolean("AllowInactiveBomItems"));
  _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));
  _changeLog->setChecked(_metrics->boolean("ItemChangeLog"));
  _allowDelete->setChecked(_metrics->boolean("AllowBOMItemDelete"));
  _autoItemSearch->setChecked(_metrics->boolean("AutoItemSearch"));

  QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
  if (issueMethod == "S")
    _issueMethod->setCurrentIndex(0);
  else if (issueMethod == "L")
    _issueMethod->setCurrentIndex(1);
  else if (issueMethod == "M")
    _issueMethod->setCurrentIndex(2);
    
  if(_metrics->value("Application") == "PostBooks")
  {
    _revControl->hide();
    _transforms->hide();
  }
  else
  {
    configureconfigurePD.exec("SELECT * FROM itemtrans LIMIT 1;");
    if (configureconfigurePD.first())
    {
      _transforms->setChecked(true);
      _transforms->setEnabled(false);
    }
    else 
      _transforms->setChecked(_metrics->boolean("Transforms"));

    configureconfigurePD.exec("SELECT * FROM rev LIMIT 1;");
    if (configureconfigurePD.first())
    {
      _revControl->setChecked(true);
      _revControl->setEnabled(false);
    }
    else 
      _revControl->setChecked(_metrics->boolean("RevControl"));
  }
  
  this->setWindowTitle("Products Configuration");

  //adjustSize();
}

configurePD::~configurePD()
{
  // no need to delete child widgets, Qt does it all for us
}

void configurePD::languageChange()
{
  retranslateUi(this);
}

bool configurePD::sSave()
{
  XSqlQuery configureSave;
  emit saving();

  if (!_metrics->boolean("RevControl") && (_revControl->isChecked()))
  {
    if (QMessageBox::warning(this, tr("Enable Revision Control"),
      tr("Enabling revision control will create control records "
         "for products that contain revision number data.  This "
         "change can not be undone.  Do you wish to proceed?"),
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      _metrics->set("RevControl", true);
      
      QString rsql = "SELECT createbomrev(bomhead_item_id,bomhead_revision) AS result "
                     "  FROM bomhead "
                     " WHERE((COALESCE(bomhead_revision,'') <> '') "
                     "   AND (bomhead_rev_id=-1))";
      if (_metrics->value("Application") != "Standard" && _metrics->value("Application") != "PostBooks")
        rsql += " UNION "
                "SELECT createboorev(boohead_item_id,boohead_revision) "
                "  FROM boohead "
                " WHERE((COALESCE(boohead_revision,'') <> '') "
                "   AND (boohead_rev_id=-1));";
      configureSave.exec(rsql);
      if (configureSave.first() && (configureSave.value("result").toInt() < 0))
      {
        ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Period"),
                                 storedProcErrorLookup("CreateRevision",
                                 configureSave.value("result").toInt()),
                                 __FILE__, __LINE__);
        _metrics->set("RevControl", false);
        return false;
      }
      if (configureSave.lastError().type() != QSqlError::NoError)
      {
        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
          .arg(__FILE__)
          .arg(__LINE__),
          configureSave.lastError().databaseText());
        _metrics->set("RevControl", false);
        return false;
      }
    }
    else
      return false;
  }

  _metrics->set("Transforms", ((_transforms->isChecked()) && (!_transforms->isHidden())));
  _metrics->set("RevControl", ((_revControl->isChecked()) && (!_revControl->isHidden())));
  _metrics->set("AllowInactiveBomItems", _inactiveBomItems->isChecked());
  _metrics->set("DefaultSoldItemsExclusive", _exclusive->isChecked());
  _metrics->set("ItemChangeLog", _changeLog->isChecked());
  _metrics->set("AllowBOMItemDelete", _allowDelete->isChecked());
  _metrics->set("AutoItemSearch", _autoItemSearch->isChecked());
  
  if (_issueMethod->currentIndex() == 0)
    _metrics->set("DefaultWomatlIssueMethod", QString("S"));
  else if (_issueMethod->currentIndex() == 1)
    _metrics->set("DefaultWomatlIssueMethod", QString("L"));
  else if (_issueMethod->currentIndex() == 2)
    _metrics->set("DefaultWomatlIssueMethod", QString("M"));

  return true;
}
