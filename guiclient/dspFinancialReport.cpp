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

#include "dspFinancialReport.h"

#include <QCloseEvent>
#include <QInputDialog>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QStack>
#include <QVariant>

#include <openreports.h>
#include "dspFinancialReport.h"

#define cFlRoot  0
#define cFlItem  1
#define cFlGroup 2
#define cFlSpec  3

#define cBegining 0
#define cEnding   1
#define cDebits   2
#define cCredits  3
#define cBudget   4
#define cDiff     5

dspFinancialReport::dspFinancialReport(QWidget* parent, const char* name, Qt::WFlags fl)
    : QWidget(parent, name, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_layout, SIGNAL(itemCollapsed(QTreeWidgetItem*)), this, SLOT(sCollapsed(QTreeWidgetItem*)));
  connect(_layout, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(sExpanded(QTreeWidgetItem*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_periods, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_periods, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditPeriodLabel()));
  connect(_flhead, SIGNAL(newID(int)), this, SLOT(sReportChanged(int)));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sToggleTrend()));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sTogglePeriod()));
  connect(_trend, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_month, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_quarter, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  connect(_year, SIGNAL(toggled(bool)), this, SLOT(sFillPeriods()));
  
  _flhead->setType(XComboBox::FinancialLayouts);

  // populate _periods
  _periods->addColumn(tr("Period"), _itemColumn, Qt::AlignLeft );
  _periods->addColumn(tr("Alternate Label"), -1, Qt::AlignLeft );
  sFillPeriods();

  _layout->addColumn( tr("Group/Account Name"), -1,              Qt::AlignLeft  );

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _shownumbers->setChecked(true);
    _showzeros->setChecked(true);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspFinancialReport::~dspFinancialReport()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspFinancialReport::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspFinancialReport::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("flhead_id", &valid);
  if (valid)
    _flhead->setId(param.toInt());

  return NoError;
}

bool dspFinancialReport::sCheck()
{
  bool _result = true;
  //Make sure user has upgraded period settings
  q.exec("SELECT period_id FROM period WHERE period_quarter IS NULL;");
  if (q.first())
  {
    QMessageBox::warning( this, tr("Setup Incomplete"),
                         tr("Please make sure all accounting periods\n"
                            "are associated with a quarter and fiscal year\n"
                            "before using this application.") );
    _result = false;
  }
  
  return _result;
}

void dspFinancialReport::sFillList()
{  
  if (!sCheck())
    return;
  if (_trend->isChecked())
    sFillListTrend();
  else 
    sFillListStatement();
}

void dspFinancialReport::sFillListStatement()
{
  unsigned int c = 0;
  unsigned int colCount = 0;
  XSqlQuery label;
  QList<int> periodsRef;
  QString qc;
  QString qw;

  q.prepare("SELECT * FROM flcol WHERE flcol_id=:flcolid");
  q.bindValue(":flcolid", _flcol->id());
  q.exec();
  if (q.first())
  {
    //Find which period selected
    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    for (int i = 0; i < selected.size(); i++)
      periodsRef.prepend(((XTreeWidgetItem*)(selected[i]))->id());
    if(periodsRef.count() < 1)
      return;

    //Get date labels for period
    label.prepare( "SELECT * FROM getflstmthead(:flcolid,:periodid)");
    label.bindValue(":flcolid", _flcol->id());
    label.bindValue(":periodid", periodsRef.at(0));
    label.exec();

    if (label.first())
    {
      //Clear columns
      _layout->clear();
      _layout->setColumnCount(1);

      //Build report query
      qc = ("SELECT flstmtitem_order AS orderby, flstmtitem_level AS level,"
          " CASE  "
		  "   WHEN flstmtitem_type = 'G' THEN 2 "
		  "   WHEN flstmtitem_type = 'I' THEN 1 "
		  "   ELSE -1 "
		  " END AS type, flstmtitem_type_id AS id,"
          " flstmtitem_name AS name");

      if (q.value("flcol_month").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
            colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cDebits)), _bigMoneyColumn, Qt::AlignRight );
               colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cCredits)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_monthdb,flstmtitem_monthcr";
           if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_monthdb <> formatMoney(0)) OR (flstmtitem_monthcr <> formatMoney(0))";
        }
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
          if (qw != "")
           qw += " OR ";
         qc += ",flstmtitem_month";
            qw = "(flstmtitem_month <> formatMoney(0))";
        if (q.value("flcol_prcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_month").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_monthprcnt";
        }
        if (q.value("flcol_budget").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_monthbudget";
           if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_monthbudget <> formatMoney(0))";
           if (q.value("flcol_budgetprcnt").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_month").toString()), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_monthbudgetprcnt";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_monthbudgetdiff";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_month").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_monthbudgetdiffprcnt";
          }        
        }
      }
      if (q.value("flcol_quarter").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
            colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cDebits)), _bigMoneyColumn, Qt::AlignRight );
               colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cCredits)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_qtrdb,flstmtitem_qtrcr";
           if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_qtrdb <> formatMoney(0)) OR (flstmtitem_qtrcr <> formatMoney(0))";
        }
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
         qc += ",flstmtitem_qtr";
         if (qw != "")
             qw += " OR ";
         qw += "(flstmtitem_qtr <> formatMoney(0))";
        if (q.value("flcol_prcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_qtr").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_qtrprcnt";
        }
        if (q.value("flcol_budget").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_qtrbudget";
           if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_qtrbudget <> formatMoney(0))";
           if (q.value("flcol_budgetprcnt").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_qtr").toString()), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_qtrbudgetprcnt";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_qtrbudgetdiff";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
              colCount++;
            _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_qtr").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_qtrbudgetdiffprcnt";
          }        
        }
      }
      if (q.value("flcol_year").toBool())
      {
        if (q.value("flcol_showdb").toBool())
        {
            colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cDebits)), _bigMoneyColumn, Qt::AlignRight );
          colCount++;
               _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cCredits)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_yeardb,flstmtitem_yearcr";
            if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_yeardb <> formatMoney(0)) OR (flstmtitem_yearcr <> formatMoney(0))";
        }
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
         qc += ",flstmtitem_year";
          if (qw != "")
           qw += " OR ";
         qw += "(flstmtitem_year <> formatMoney(0))";
        if (q.value("flcol_prcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_year").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_yearprcnt";
        }
        if (q.value("flcol_budget").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_yearbudget";
            if (qw != "")
             qw += " OR ";
           qw += "(flstmtitem_yearbudget <> formatMoney(0))";
           if (q.value("flcol_budgetprcnt").toBool())
          {
            colCount++;
            _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_year").toString()), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_yearbudgetprcnt";
          }
          if (q.value("flcol_budgetdiff").toBool())
          {
            colCount++;
            _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_yearbudgetdiff";
          }
          if (q.value("flcol_budgetdiffprcnt").toBool())
          {
            colCount++;
            _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_year").toString()).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
             qc += ",flstmtitem_yearbudgetdiffprcnt";
          }        
        }
      }
      if (q.value("flcol_priormonth").toBool())
      {
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
         qc += ",flstmtitem_prmonth";
           if (qw != "")
           qw += " OR ";
         qw += "(flstmtitem_prmonth <> formatMoney(0))";
        if (q.value("flcol_priorprcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_prmonth").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prmonthprcnt";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prmonthdiff";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_prmonth").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prmonthdiffprcnt";
        }    
      }
      if (q.value("flcol_priorquarter").toBool())
      {
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
         qc += ",flstmtitem_prqtr";
            if (qw != "")
           qw += " OR ";
         qw += "(flstmtitem_prqtr <> formatMoney(0))";
        if (q.value("flcol_priorprcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_prqtr").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prqtrprcnt";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prqtrdiff";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_prqtr").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_prqtrdiffprcnt";
        }    
      }
      if (q.value("flcol_prioryear").toString() != "N")
      {
          colCount++;
        _layout->addColumn( tr("%1\n%2").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
         qc += ",flstmtitem_pryear";
            if (qw != "")
           qw += " OR ";
         qw += "(flstmtitem_pryear <> formatMoney(0))";
        if (q.value("flcol_priorprcnt").toBool())
        {
            colCount++;
          _layout->addColumn( tr("%1\n% of Group").arg(label.value("flstmthead_pryear").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_pryearprcnt";
        }
        if (q.value("flcol_priordiff").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 Diff.").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_pryeardiff";
        }
        if (q.value("flcol_priordiffprcnt").toBool())
        {
          colCount++;
          _layout->addColumn( tr("%1\n%2 % Diff.").arg(label.value("flstmthead_pryear").toString()).arg(label.value("flstmthead_typedescrip2").toString()), _bigMoneyColumn, Qt::AlignRight );
           qc += ",flstmtitem_pryeardiffprcnt";
        }    
      }
      qc += " FROM financialreport(:flcolid,:periodid,:shownumbers,false)";
      if (!_showzeros->isChecked())
        qc += " WHERE (" + qw + "  OR (flstmtitem_type <> 'I'))";
      q.prepare(qc);
      q.bindValue(":flcolid", _flcol->id());
      q.bindValue(":periodid", periodsRef.at(0));
      q.bindValue(":shownumbers", _shownumbers->isChecked());
      q.exec();
      
      QStack<XTreeWidgetItem*> parent;
      XTreeWidgetItem *last = 0;
      int level = 0;
      while(q.next())
      {
        // If the level this item is on is lower than the last level we just did then we need
        // to pop the stack a number of times till we are equal.
        while(q.value("level").toInt() < level)
        {
          level--;
          last = parent.pop();
        }

        // If the level this item is on is higher than the last level we need to push the last
        // item onto the stack a number of times till we are equal. (Should only ever be 1.)
        while(q.value("level").toInt() > level)
        {
          level++;
          parent.push(last);
          last = 0;
        }

        // If there is an item in the stack use that as the parameter to the new xlistviewitem
        // otherwise we'll just use the xlistview _layout
        if(!parent.isEmpty() && parent.top())
          last = new XTreeWidgetItem(parent.top(), last, q.value("id").toInt(), q.value("type").toInt(), q.value("name"));
        else
          last = new XTreeWidgetItem(_layout, last, q.value("id").toInt(), q.value("type").toInt(), q.value("name"));
        for(c = 0; c < colCount; c++)
          last->setText(1+c, q.value(5+c).toString());
      }

      _layout->expandAll();
    }
  }
}

void dspFinancialReport::sFillListTrend()
{
  int c = 0;
  unsigned int colCount = 0;
  QList<int> periodsRef;
  QStringList periods;
  QString interval;
  
  if (_month->isChecked())
  interval = "M";
  else if (_quarter->isChecked())
    interval = "Q";
  else
    interval = "Y";

  QString customlabel;
  q.prepare("SELECT flhead_custom_label"
            "  FROM flhead"
            " WHERE (flhead_id=:flhead_id);");
  q.bindValue(":flhead_id", _flhead->id());
  q.exec();
  if(q.first())
    customlabel = q.value("flhead_custom_label").toString();
  if(customlabel.isEmpty())
    customlabel = tr("Custom");

  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  QString label;
  for (int i = 0; i < selected.size(); i++)
  {
    label = selected[i]->text(1).isEmpty() ? selected[i]->text(0) : selected[i]->text(1);
    periodsRef.prepend(((XTreeWidgetItem*)(selected[i]))->id());
    periods.prepend(label);
  }
  if(periodsRef.count() < 1)
    return;

  _layout->clear();
  _layout->setColumnCount(1);

  q.prepare("SELECT financialReport(:flhead_id, :period_id, :interval) AS result;");
  
  QString q1c = QString("SELECT r0.flrpt_order AS orderby, r0.flrpt_level AS level,"
                        "       :group AS type, flgrp_id AS id,"
                        "       flgrp_name AS name");
  QString q1f = QString(" FROM flgrp");
  QString q1w = QString(" WHERE ((true)");

  QString q2c = QString("SELECT r0.flrpt_order AS orderby, r0.flrpt_level AS level,"
                        "       :item AS type, flitem_id AS id,");
  if (_shownumbers->isChecked())
    q2c += " (formatGLAccount(accnt_id) || '-' || accnt_descrip) AS name ";
  else
    q2c += " accnt_descrip AS name ";

  QString q2f = QString(" FROM flitem, accnt ");
  QString q2w = QString(" WHERE ((true) AND accnt_id IN (SELECT * FROM getflitemaccntid(flitem_id)) ");

  QString q3c = QString("SELECT r0.flrpt_order AS orderby, r0.flrpt_level AS level,"
                        "       :spec AS type, flspec_id AS id,"
                        "       flspec_name AS name");
  QString q3f = QString(" FROM flspec");
  QString q3w = QString(" WHERE ((true)");

  QString q4c = QString("SELECT r0.flrpt_order AS orderby, r0.flrpt_level AS level,"
                        "       -1 AS type, r0.flrpt_type_id AS id,"
                        "       CASE WHEN(r0.flrpt_type='T' AND r0.flrpt_level=0) THEN COALESCE(r0.flrpt_altname, 'Total')"
                        "            WHEN(r0.flrpt_type='T') THEN COALESCE(r0.flrpt_altname, 'Subtotal')"
                        "            ELSE ('Type ' || r0.flrpt_type || ' ' || text(r0.flrpt_type_id))"
                        "       END AS name");
  QString q4f = QString(" FROM");
  QString q4w = QString(" WHERE ((true)");
  
  QString qt;
  QString qz;

  for(c = 0; c < periodsRef.count(); c++)
  {
    if(_showBegBal->isChecked())
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cBegining)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showstart) THEN formatMoney(r%1.flrpt_beginning) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_beginning)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_beginning)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_beginning)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_beginning <> 0)").arg(c);
    }
    if(_showBegBalPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cBegining)), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showstartprcnt) THEN formatPrcnt(r%1.flrpt_beginningprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_beginningprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_beginningprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_beginningprcnt)").arg(c);
    }
    if(_showDebits->isChecked())
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cDebits)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdelta) THEN formatMoney(r%1.flrpt_debits) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_debits)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_debits)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_debits)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_debits <> 0)").arg(c);
    }
    if(_showDebitsPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cDebits)), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdeltaprcnt) THEN formatPrcnt(r%1.flrpt_debitsprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_debitsprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_debitsprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_debitsprcnt)").arg(c);
    }
    if(_showCredits->isChecked())
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cCredits)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdelta) THEN formatMoney(r%1.flrpt_credits) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_credits)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_credits)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_credits)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_credits <> 0)").arg(c);
    }
    if(_showCreditsPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cCredits)), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdeltaprcnt) THEN formatPrcnt(r%1.flrpt_creditsprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_creditsprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_creditsprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_creditsprcnt)").arg(c);
    }
    if ((_showEndBal->isChecked()) || (_type->text() == "Balance Sheet"))
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cEnding)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showend) THEN formatMoney(r%1.flrpt_ending) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_ending)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_ending)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_ending)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_ending <> 0)").arg(c);
    }
    if(_showEndBalPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cEnding)), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showendprcnt) THEN formatPrcnt(r%1.flrpt_endingprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_endingprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_endingprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_endingprcnt)").arg(c);
    }
    if(_showBudget->isChecked())
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cBudget)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showbudget) THEN formatMoney(r%1.flrpt_budget) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_budget)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_budget)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_budget)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_budget <> 0)").arg(c);
    }
    if(_showBudgetPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cBudget)),   _ynColumn,       Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showbudgetprcnt) THEN formatPrcnt(r%1.flrpt_budgetprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_budgetprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_budgetprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_budgetprcnt)").arg(c);
    }
    if ((_showDiff->isChecked()) || (_type->text() == "Income Statement") || (_type->text() == "Cash Flow Statement"))
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(_columnLabels.value(cDiff)), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdiff) THEN formatMoney(r%1.flrpt_diff) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_diff)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_diff)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_diff)").arg(c);
      
      if (qt != "")
    qt += " + ";
    qt += QString("r%1.flrpt_diff").arg(c);
      
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_diff <> 0)").arg(c);
    }
    if(_showDiffPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(_columnLabels.value(cDiff)), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showdiffprcnt) THEN formatMoney(r%1.flrpt_diffprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_diffprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_diffprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_diffprcnt)").arg(c);
    }
    if(_showCustom->isChecked())
    {
      _layout->addColumn( tr("%1\n%2").arg(periods.at(c)).arg(customlabel), _bigMoneyColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showcustom) THEN formatMoney(r%1.flrpt_custom) ELSE NULL END").arg(c);
      q2c += QString(",formatMoney(r%1.flrpt_custom)").arg(c);
      q3c += QString(",formatMoney(r%1.flrpt_custom)").arg(c);
      q4c += QString(",formatMoney(r%1.flrpt_custom)").arg(c);
      if (qz != "")
     qz += " OR ";
     qz += QString("(r%1.flrpt_custom <> 0)").arg(c);
    }
    if(_showCustomPrcnt->isChecked())
    {
      _layout->addColumn( tr("%1\n%2 %").arg(periods.at(c)).arg(customlabel), _ynColumn, Qt::AlignRight );
      colCount++;
      q1c += QString(",CASE WHEN(flgrp_summarize AND flgrp_showcustomprcnt) THEN formatMoney(r%1.flrpt_customprcnt) ELSE NULL END").arg(c);
      q2c += QString(",formatPrcnt(r%1.flrpt_customprcnt)").arg(c);
      q3c += QString(",formatPrcnt(r%1.flrpt_customprcnt)").arg(c);
      q4c += QString(",formatPrcnt(r%1.flrpt_customprcnt)").arg(c);
    }
  
    q1f += QString(", flrpt AS r%1").arg(c);

    q1w += QString(" AND (r%1.flrpt_type='G')").arg(c);
    q1w += QString(" AND (r%1.flrpt_type_id=flgrp_id)").arg(c);
    q1w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q1w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q1w += QString(" AND (r%1.flrpt_username=CURRENT_USER)").arg(c);
    q1w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q1w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);


    q2f += QString(", flrpt AS r%1").arg(c);

    q2w += QString(" AND (r%1.flrpt_type='I')").arg(c);
    q2w += QString(" AND (r%1.flrpt_type_id=flitem_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q2w += QString(" AND (r%1.flrpt_username=CURRENT_USER)").arg(c);
    q2w += QString(" AND (r%1.flrpt_accnt_id=accnt_id)").arg(c);
    q2w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q2w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);


    q3f += QString(", flrpt AS r%1").arg(c);

    q3w += QString(" AND (r%1.flrpt_type='S')").arg(c);
    q3w += QString(" AND (r%1.flrpt_type_id=flspec_id)").arg(c);
    q3w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q3w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q3w += QString(" AND (r%1.flrpt_username=CURRENT_USER)").arg(c);
    q3w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q3w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);

    if(c > 0)
      q4f += QString(", flrpt AS r%1").arg(c);
    else
      q4f += QString(" flrpt AS r%1").arg(c);

    q4w += QString(" AND (NOT (r%1.flrpt_type IN ('G','I','S')))").arg(c);
    q4w += QString(" AND (r%1.flrpt_flhead_id=:flhead_id)").arg(c);
    q4w += QString(" AND (r%1.flrpt_period_id=%2)").arg(c).arg(periodsRef.at(c));
    q4w += QString(" AND (r%1.flrpt_username=CURRENT_USER)").arg(c);
    q4w += QString(" AND (r%1.flrpt_interval='%2')").arg(c).arg(interval);
    if(c > 0)
      q4w += QString(" AND (r0.flrpt_order=r%1.flrpt_order)").arg(c);

    q.bindValue(":flhead_id", _flhead->id());
    q.bindValue(":period_id", periodsRef.at(c));
    q.bindValue(":interval", interval);
    q.exec();
  }
  
  //Grand Total for Trend Reports
  if ((_trend->isChecked()) && ((_type->text() == "Income Statement") || (_type->text() == "Cash Flow Statement")))
  {
    _layout->addColumn( tr("Grand\nTotal"), _bigMoneyColumn, Qt::AlignRight );
    colCount++;
    q1c += ",CASE WHEN(flgrp_summarize AND flgrp_showdiff) THEN formatMoney(" + qt + ") ELSE NULL END";
    q2c += ",formatMoney(" + qt + ")";
    q3c += ",formatMoney(" + qt + ")"; 
    q4c += ",formatMoney(" + qt + ")";
  }
  
  if (!_showzeros->isChecked())
  {
    q2w += " AND (" + qz + ")";
    q3w += " AND (" + qz + ")";
  }
  
  q1w += ")";
  q2w += ")";
  q3w += ")";
  q4w += ")";
  
  QString query = q1c + q1f + q1w +
                  QString(" UNION ") +
                  q2c + q2f + q2w +
                  QString(" UNION ") +
                  q3c + q3f + q3w +
                  QString(" UNION ") +
                  q4c + q4f + q4w +
                  QString(" ORDER BY orderby;");
  q.prepare(query);
  q.bindValue(":flhead_id", _flhead->id());
  q.bindValue(":item", cFlItem);
  q.bindValue(":group", cFlGroup);
  q.bindValue(":spec", cFlSpec);
  q.exec();

  QStack<XTreeWidgetItem*> parent;
  XTreeWidgetItem *last = 0;
  int level = 0;
  while(q.next())
  {
    // If the level this item is on is lower than the last level we just did then we need
    // to pop the stack a number of times till we are equal.
    while(q.value("level").toInt() < level)
    {
      level--;
      last = parent.pop();
    }

    // If the level this item is on is higher than the last level we need to push the last
    // item onto the stack a number of times till we are equal. (Should only ever be 1.)
    while(q.value("level").toInt() > level)
    {
      level++;
      parent.push(last);
      last = 0;
    }

    // If there is an item in the stack use that as the parameter to the new xlistviewitem
    // otherwise we'll just use the xlistview _layout
    if(!parent.isEmpty() && parent.top())
      last = new XTreeWidgetItem(parent.top(), last, q.value("id").toInt(), q.value("type").toInt(), q.value("name"));
    else
      last = new XTreeWidgetItem(_layout, last, q.value("id").toInt(), q.value("type").toInt(), q.value("name"));
    for(unsigned int uc = 0; uc < colCount; uc++)
      last->setText(1+uc, q.value(5+uc).toString());
  }

  _layout->expandAll();
}

void dspFinancialReport::sFillPeriods()
{
  _periods->clear();
    
  if ((!_trend->isChecked()) || (_month->isChecked()))
  {
  _periods->populate("SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end)) AS f_name, period_name "
            "  FROM period "
            "ORDER BY period_start DESC;" );
  }
  else if (_quarter->isChecked())
  {
    _periods->populate("SELECT LAST(period_id), ('Q' || period_quarter || '-' || EXTRACT(year from yearperiod_end)) FROM "
            " (SELECT period_id, period_quarter, yearperiod_end "
            " FROM period,yearperiod "
            " WHERE (period_yearperiod_id=yearperiod_id) "
            " ORDER BY period_end) AS data "
            " GROUP by period_quarter,yearperiod_end "
            " ORDER by EXTRACT(year from yearperiod_end) DESC, period_quarter DESC"); 
  }
  else
  {
    _periods->populate("SELECT (SELECT LAST(period_id) FROM "
            "(SELECT period_id,period_start FROM period "
            " WHERE period_yearperiod_id=yearperiod_id "
            " ORDER BY period_start) "
            " as data),EXTRACT(year from yearperiod_end) "
            " FROM yearperiod "
            " ORDER BY EXTRACT(year from yearperiod_end) DESC" ); 
  }
  
}                     

void dspFinancialReport::sCollapsed( QTreeWidgetItem * item )
{
  for (int i = 0; i < item->childCount(); i++)
  {
    XTreeWidgetItem *child = (XTreeWidgetItem*)item->child(i);
    if(child->altId() == -1)
    {
      for (int i = 1; i < _layout->columnCount(); i++)
        item->setText(i, child->text(i));
      return;
    }
  }
}

void dspFinancialReport::sExpanded( QTreeWidgetItem * item )
{
  if(item->childCount() > 0)
    for(int i = 1; i < _layout->columnCount(); i++)
      item->setText(i, "");
}

void dspFinancialReport::sPrint()
{
  ParameterList params;
  QString interval;
  QString reportdef;
  
  if (_month->isChecked())
    interval = "M";
  else if (_quarter->isChecked())
    interval = "Q";
  else
    interval = "Y";
     
  if(_shownumbers->isChecked())
    params.append("shownumbers");
  if(_showzeros->isChecked())
    params.append("showzeros");
  

  QList<QVariant> periodList;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
    periodList.prepend(((XTreeWidgetItem*)(selected[i]))->id());
  
  if(periodList.isEmpty())
  {
    QMessageBox::warning(this, tr("No Period(s) Selected"),
      tr("You must select at least one period to report on.") );
    return;
  }

  if (_trend->isChecked())
  {
    if (_type->text() == "Ad Hoc")
      reportdef = "FinancialReport";
    else
      reportdef = "FinancialTrend";

    params.append("flhead_id", _flhead->id());
    params.append("period_id_list", periodList);
    params.append("interval", interval);  

    orReport report(reportdef, params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else
  {
    q.prepare("SELECT report_name "
        " FROM flcol,report "
        " WHERE ((flcol_report_id=report_id)"
        " AND (flcol_id=:flcol_id))");
    q.bindValue(":flcol_id",_flcol->id());
    q.exec();
    if (q.first())
    {  
      params.append("flcol_id", _flcol->id());
      params.append("period_id", periodList.at(0));
  
      orReport report(q.value("report_name").toString(), params);
      if (report.isValid())
        report.print();
      else
        report.reportError(this);
    }    
  }
}

void dspFinancialReport::sPopulateMenu( QMenu * pMenu )
{
  pMenu->insertItem(tr("Edit Alternate Label..."), this, SLOT(sEditPeriodLabel()));
}

void dspFinancialReport::sEditPeriodLabel()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_periods->currentItem();
  if(!item)
    return;

  bool ok;
  QString text = QInputDialog::getText( tr("Alternate Label"),
        tr("Enter an alternate label for the period %1:").arg(item->text(0)),
        QLineEdit::Normal, item->text(1), &ok, this );
  if(ok)
    item->setText(1, text);
}

void dspFinancialReport::sReportChanged(int flheadid)
{
    //Populate column layouts
  q.prepare( "SELECT flcol_id, flcol_name "
  " FROM flcol "
  " WHERE flcol_flhead_id=:flhead_id "
  " ORDER BY flcol_name; ");
  q.bindValue(":flhead_id", flheadid);
  q.exec();
  _flcol->populate(q);

  q.prepare("SELECT flhead_usealtbegin, flhead_altbegin,"
        "       flhead_usealtend, flhead_altend,"
        "       flhead_usealtdebits, flhead_altdebits,"
        "       flhead_usealtcredits, flhead_altcredits,"
        "       flhead_usealtbudget, flhead_altbudget,"
        "       flhead_usealtdiff, flhead_altdiff, "
        "      flhead_type "
        "  FROM flhead"
        " WHERE (flhead_id=:flhead_id);");
  q.bindValue(":flhead_id", flheadid);
  q.exec();
  if(q.first())
  {
    if(q.value("flhead_usealtbegin").toBool())
      _columnLabels.insert(cBegining, q.value("flhead_altbegin").toString());
    else
      _columnLabels.insert(cBegining, tr("Beg. Bal."));

    if(q.value("flhead_usealtend").toBool())
      _columnLabels.insert(cEnding, q.value("flhead_altend").toString());
    else if (q.value("flhead_type").toString() == "B")
      _columnLabels.insert(cEnding, tr("Balance"));
    else
      _columnLabels.insert(cEnding, tr("End. Bal."));

    if(q.value("flhead_usealtdebits").toBool())
      _columnLabels.insert(cDebits, q.value("flhead_altdebits").toString());
    else
      _columnLabels.insert(cDebits, tr("Debits"));

    if(q.value("flhead_usealtcredits").toBool())
      _columnLabels.insert(cCredits, q.value("flhead_altcredits").toString());
    else
      _columnLabels.insert(cCredits, tr("Credits"));

    if(q.value("flhead_usealtbudget").toBool())
      _columnLabels.insert(cBudget, q.value("flhead_altbudget").toString());
    else
      _columnLabels.insert(cBudget, tr("Budget"));

    if(q.value("flhead_usealtdiff").toBool())
      _columnLabels.insert(cDiff, q.value("flhead_altdiff").toString());
    else if (q.value("flhead_type").toString() == "I")
      _columnLabels.insert(cDiff, tr("Income"));
    else if (q.value("flhead_type").toString() == "C")
      _columnLabels.insert(cDiff, tr("Cash"));
    else
      _columnLabels.insert(cDiff, tr("Difference"));
      
    _showBegBal->setText(_columnLabels.value(cBegining));
    _showBegBalPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cBegining)));
    _showEndBal->setText(_columnLabels.value(cEnding));
    _showEndBalPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cEnding)));
    _showDebits->setText(_columnLabels.value(cDebits));
    _showDebitsPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cDebits)));
    _showCredits->setText(_columnLabels.value(cCredits));
    _showCreditsPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cCredits)));
    _showBudget->setText(_columnLabels.value(cBudget));
    _showBudgetPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cBudget)));
    _showDiff->setText(_columnLabels.value(cDiff));
    _showDiffPrcnt->setText(tr("%1 %").arg(_columnLabels.value(cDiff)));


    if (q.value("flhead_type").toString()== "A")
    {
      _showColumnsGroup->setEnabled(true);
      _trend->setChecked(true);
      _periods->setSelectionMode(QAbstractItemView::ExtendedSelection);
      _flcol->setEnabled(false);
      _type->setText("Ad Hoc");
    }
    else
    {
      if (q.value("flhead_type").toString()== "I")
        _type->setText("Income Statement");
      if (q.value("flhead_type").toString()== "B")
        _type->setText("Balance Sheet");
      if (q.value("flhead_type").toString()== "C")
        _type->setText("Cash Flow Statement");
      _showColumnsGroup->setEnabled(false);  
      _trend->setChecked(false);
      sTogglePeriod();
    }    
  }
}

void dspFinancialReport::sToggleTrend()
{

  if (!_trend->isChecked())
  {
  if (_type->text() == "Ad Hoc")
    _trend->setChecked(true);
  }

}

void dspFinancialReport::sTogglePeriod()
{

  if (_trend->isChecked())
  {
    _periods->setSelectionMode(QAbstractItemView::MultiSelection);
    _flcol->setEnabled(false);
  }
  else
  {
    _periods->setSelectionMode(QAbstractItemView::SingleSelection); 
    _flcol->setEnabled(true); 
  }  

}
