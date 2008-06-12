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

//  xcombobox.cpp
//  Created 03/16/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QLabel>
#include <QMouseEvent>

#include <xsqlquery.h>
#include "xcombobox.h"

#define DEBUG false

XComboBox::XComboBox(QWidget *pParent, const char *pName) :
  QComboBox(pParent, pName)
{
  _type = Adhoc;
  _lastId = -1;
  setAllowNull(false);
  _nullStr = "";
  _label = 0;

  connect(this, SIGNAL(activated(int)), this, SLOT(sHandleNewIndex(int)));

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

XComboBox::XComboBox(bool pEditable, QWidget *pParent, const char *pName) :
  QComboBox(pEditable, pParent, pName)
{
  _type = Adhoc;
  _lastId = -1;
  setAllowNull(false);
  _label = 0;

  connect(this, SIGNAL(activated(int)), this, SLOT(sHandleNewIndex(int)));

#ifdef Q_WS_MAC
  QFont f = font();
  f.setPointSize(f.pointSize() - 2);
  setFont(f);
#endif
}

enum XComboBox::XComboBoxTypes XComboBox::type()
{
  return _type;
}

void XComboBox::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, QByteArray("code"));
}

void XComboBox::setType(XComboBoxTypes pType)
{
  if (_type == pType)
    return;

  _type = pType;

  if (_x_metrics == 0)
    return;

  XSqlQuery query;

  switch (pType)
  {
    case Adhoc:
      break;

    case UOMs:
      setAllowNull(TRUE);
      query.exec( "SELECT uom_id, uom_name, uom_name "
                  "FROM uom "
                  "ORDER BY uom_name;" );
    break;

    case ClassCodes:
      query.exec( "SELECT classcode_id, (classcode_code || '-' || classcode_descrip), classcode_code  "
                  "FROM classcode "
                  "ORDER BY classcode_code;" );
      break;

    case ItemGroups:
      query.exec( "SELECT itemgrp_id, itemgrp_name, itemgrp_name "
                  "FROM itemgrp "
                  "ORDER BY itemgrp_name;" );
      break;

    case CostCategories:
      query.exec( "SELECT costcat_id,  (costcat_code || '-' || costcat_descrip), costcat_code "
                  "FROM costcat "
                  "ORDER BY costcat_code;" );
      break;

    case ProductCategories:
      query.exec( "SELECT prodcat_id, (prodcat_code || ' - ' || prodcat_descrip), prodcat_code "
                  "FROM prodcat "
                  "ORDER BY prodcat_code;" );
      break;

    case PlannerCodes:
      query.exec( "SELECT plancode_id, (plancode_code || '-' || plancode_name), plancode_code "
                  "FROM plancode "
                  "ORDER BY plancode_code;" );
      break;

    case CustomerTypes:
      query.exec( "SELECT custtype_id, (custtype_code || '-' || custtype_descrip), custtype_code "
                  "FROM custtype "
                  "ORDER BY custtype_code;" );
      break;

    case CustomerGroups:
      query.exec( "SELECT custgrp_id, custgrp_name, custgrp_name "
                  "FROM custgrp "
                  "ORDER BY custgrp_name;" );
      break;

    case VendorTypes:
      query.exec( "SELECT vendtype_id, (vendtype_code || '-' || vendtype_descrip), vendtype_code "
                  "FROM vendtype "
                  "ORDER BY vendtype_code;" );
      break;

    case VendorGroups:
      query.exec( "SELECT vendgrp_id, vendgrp_name, vendgrp_name "
                  "FROM vendgrp "
                  "ORDER BY vendgrp_name;" );
      break;

    case SalesRepsActive:
      query.exec( "SELECT salesrep_id, (salesrep_number || '-' || salesrep_name), salesrep_number "
                  "FROM salesrep "
                  "WHERE (salesrep_active) "
                  "ORDER by salesrep_number;" );
      break;

    case ShipVias:
      setAllowNull(TRUE);
      setEditable(TRUE);
      query.exec( "SELECT shipvia_id, (shipvia_code || '-' || shipvia_descrip), shipvia_code "
                  "FROM shipvia "
                  "ORDER BY shipvia_code;" );
      break;

    case SalesReps:
      query.exec( "SELECT salesrep_id, (salesrep_number || '-' || salesrep_name), salesrep_number "
                  "FROM salesrep "
                  "ORDER by salesrep_number;" );
      break;

    case ShippingCharges:
      query.exec( "SELECT shipchrg_id, (shipchrg_name || '-' || shipchrg_descrip), shipchrg_name "
                  "FROM shipchrg "
                  "ORDER by shipchrg_name;" );
      break;

    case ShippingForms:
      query.exec( "SELECT shipform_id, shipform_name, shipform_name "
                  "FROM shipform "
                  "ORDER BY shipform_name;" );
      break;

    case Terms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip), terms_code "
                  "FROM terms "
                  "ORDER by terms_code;" );
      break;

    case ARTerms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip), terms_code "
                  "FROM terms "
                  "WHERE (terms_ar) "
                  "ORDER by terms_code;" );
      break;

    case APTerms:
      query.exec( "SELECT terms_id, (terms_code || '-' || terms_descrip), terms_code "
                  "FROM terms "
                  "WHERE (terms_ap) "
                  "ORDER by terms_code;" );
      break;

    case ARBankAccounts:
      query.exec( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip), bankaccnt_name "
                  "FROM bankaccnt "
                  "WHERE (bankaccnt_ar) "
                  "ORDER BY bankaccnt_name;" );
      break;

    case APBankAccounts:
      query.exec( "SELECT bankaccnt_id, (bankaccnt_name || '-' || bankaccnt_descrip), bankaccnt_name "
                  "FROM bankaccnt "
                  "WHERE (bankaccnt_ap) "
                  "ORDER BY bankaccnt_name;" );
      break;

    case AccountingPeriods:
      query.exec( "SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end), (formatDate(period_start) || '-' || formatDate(period_end)) "
                  "FROM period "
                  "ORDER BY period_start DESC;" );
      break;

    case FinancialLayouts:
      query.exec( "SELECT flhead_id, flhead_name, flhead_name "
                  "FROM flhead "
                  "WHERE (flhead_active) "
                  "ORDER BY flhead_name;" );
      break;

    case FiscalYears:
      query.exec( "SELECT yearperiod_id, formatdate(yearperiod_start) || '-' || formatdate(yearperiod_end), formatdate(yearperiod_start) || '-' || formatdate(yearperiod_end)"
                  "  FROM yearperiod"
                  " ORDER BY yearperiod_start DESC;" );
      break;

    case SoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name), prj_number "
                  "FROM prj "
                  "WHERE (prj_so) "
                  "ORDER BY prj_name;" );
      break;

    case WoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name), prj_number "
                  "FROM prj "
                  "WHERE (prj_wo) "
                  "ORDER BY prj_name;" );
      break;

    case PoProjects:
      setAllowNull(TRUE);
      query.exec( "SELECT prj_id, (prj_number || '-' || prj_name), prj_number "
                  "FROM prj "
                  "WHERE (prj_po) "
                  "ORDER BY prj_name;" );
      break;

    case Currencies:
      query.exec( "SELECT curr_id, currConcat(curr_abbr, curr_symbol), curr_abbr"
                  " FROM curr_symbol "
                  "ORDER BY curr_base DESC, curr_abbr;" );
      break;

    case CurrenciesNotBase:
      query.exec( "SELECT curr_id, currConcat(curr_abbr, curr_symbol), curr_abbr"
                  " FROM curr_symbol "
                  " WHERE curr_base = FALSE "
                  "ORDER BY curr_abbr;" );
      break;

    case Companies:
      query.exec( "SELECT company_id, company_number, company_number "
                  "FROM company "
                  "ORDER BY company_number;" );
      break;

    case ProfitCenters:
      setEditable(_x_metrics->boolean("GLFFProfitCenters"));
      query.exec( "SELECT prftcntr_id, prftcntr_number, prftcntr_number "
                  "FROM prftcntr "
                  "ORDER BY prftcntr_number;" );
      break;

    case Subaccounts:
      setEditable(_x_metrics->boolean("GLFFSubaccounts"));
      query.exec( "SELECT subaccnt_id, subaccnt_number, subaccnt_number "
                  "FROM subaccnt "
                  "ORDER BY subaccnt_number;" );
      break;

    case CustomerCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'C')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case VendorCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'V')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case ItemCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'I')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case ProjectCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'J')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case LotSerialCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "WHERE (strpos(cmnttype_usedin, 'L')>0)"
                  "ORDER BY cmnttype_name;" );
      break;

    case AllCommentTypes:
      query.exec( "SELECT cmnttype_id, cmnttype_name, cmnttype_name "
                  "FROM cmnttype "
                  "ORDER BY cmnttype_name;" );
      break;

    case AllProjects:
      query.exec( "SELECT prj_id, prj_name, prj_name "
                  "FROM prj "
                  "ORDER BY prj_name;" );
      break;

    case Users:
      query.exec( "SELECT usr_id, usr_username, usr_username "
                  "FROM usr "
                  "ORDER BY usr_username;" );
      break;

    case SalesCategories:
      query.exec( "SELECT salescat_id, (salescat_name || '-' || salescat_descrip), salescat_name "
                  "FROM salescat "
                  "ORDER BY salescat_name;" );
      break;

    case ExpenseCategories:
      query.exec( "SELECT expcat_id, (expcat_code || '-' || expcat_descrip), expcat_code "
                  "FROM expcat "
                  "ORDER BY expcat_code;" );
      break;

    case ReasonCodes:
      query.exec( "SELECT rsncode_id, (rsncode_code || '-' || rsncode_descrip), rsncode_code"
                  "  FROM rsncode "
                  "ORDER BY rsncode_code;" );
      break;

    case TaxCodes:
      query.exec( "SELECT tax_id, (tax_code || '-' || tax_descrip), tax_code"
                  "  FROM tax "
                  "ORDER BY tax_code;" );
      break;

    case WorkCenters:
      query.exec( "SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip), wrkcnt_code"
                  "  FROM wrkcnt "
                  "ORDER BY wrkcnt_code;" );
      break;

    case CRMAccounts:
      setAllowNull(TRUE);
      query.exec( "SELECT crmacct_id, (crmacct_number || '-' || crmacct_name), crmacct_number"
                  "  FROM crmacct "
                  "ORDER BY crmacct_number;" );
      break;

    case Honorifics:
      setAllowNull(TRUE);
      query.exec( "SELECT hnfc_id, hnfc_code, hnfc_code"
                  "  FROM hnfc "
                  "ORDER BY hnfc_code;" );
      break;

    case IncidentSeverity:
      query.exec( "SELECT incdtseverity_id, incdtseverity_name, incdtseverity_name"
                  "  FROM incdtseverity"
                  " ORDER BY incdtseverity_order, incdtseverity_name;" );
      break;

    case IncidentPriority:
      query.exec( "SELECT incdtpriority_id, incdtpriority_name, incdtpriority_name"
                  "  FROM incdtpriority"
                  " ORDER BY incdtpriority_order, incdtpriority_name;" );
      break;

    case IncidentResolution:
      query.exec( "SELECT incdtresolution_id, incdtresolution_name, incdtresolution_name"
                  "  FROM incdtresolution"
                  " ORDER BY incdtresolution_order, incdtresolution_name;" );
      break;

    case IncidentCategory:
      query.exec( "SELECT incdtcat_id, incdtcat_name, incdtcat_name"
                  "  FROM incdtcat"
                  " ORDER BY incdtcat_order, incdtcat_name;" );
      break;

    case TaxAuths:
      query.exec( "SELECT taxauth_id, taxauth_code, taxauth_code"
                  "  FROM taxauth"
                  " ORDER BY taxauth_code;" );
      break;

    case TaxTypes:
      query.exec( "SELECT taxtype_id, taxtype_name, taxtype_name"
                  "  FROM taxtype"
                  " ORDER BY taxtype_name;" );
      break;

    case Agent:
      query.exec( "SELECT usr_id, usr_username, usr_username "
                  "  FROM usr"
                  " WHERE (usr_agent) "
                  " ORDER BY usr_username;" );
      break;

    case Reports:
      query.exec( "SELECT a.report_id, a.report_name, a.report_name "
                  "FROM report a, "
                  "    (SELECT MIN(report_grade) AS report_grade, report_name "
                  "     FROM report "
                  "     GROUP BY report_name) b "
                  "WHERE ((a.report_name=b.report_name)"
                  "  AND  (a.report_grade=b.report_grade)) "
                  "ORDER BY report_name;" );
      break;

    case OpportunityStages:
      query.exec("SELECT opstage_id, opstage_name, opstage_name "
                 "  FROM opstage"
                 " ORDER BY opstage_order;");
      break;

    case OpportunitySources:
      query.exec("SELECT opsource_id, opsource_name, opsource_name "
                 "  FROM opsource;");
      break;

    case OpportunityTypes:
      query.exec("SELECT optype_id, optype_name, optype_name "
                 "  FROM optype;");
      break;

    case Locales:
      query.exec("SELECT locale_id, locale_code, locale_code "
                 "  FROM locale"
                 " ORDER BY locale_code;");
      break;

    case LocaleLanguages:
      query.exec("SELECT lang_id, lang_name, lang_name "
                 "  FROM lang"
                 " WHERE lang_qt_number IS NOT NULL"
                 " ORDER BY lang_name;");
      break;

    case Countries:
      query.exec("SELECT country_id, country_name, country_name "
                 "  FROM country"
                 " ORDER BY country_name;");
      break;

    case LocaleCountries:
      query.exec("SELECT country_id, country_name, country_name "
                 "  FROM country"
                 " WHERE country_qt_number IS NOT NULL"
                 " ORDER BY country_name;");
      break;
      
    case RegistrationTypes:
      query.exec("SELECT regtype_id, regtype_code, regtype_code "
                 "  FROM regtype"
                 " ORDER BY regtype_code;");
      break;

  }

  populate(query);

  switch (pType)
  {
    case SoProjects:
    case WoProjects:
    case PoProjects:
      setEnabled(count() > 1);
      break;

    case Currencies:
      if (count() <= 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    case CurrenciesNotBase:
      if (count() < 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setLabel(QLabel* pLab)
{
  _label = pLab;

  switch (_type)
  {
    case Currencies:
      if (count() <= 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    case CurrenciesNotBase:
      if (count() < 1)
      {
        hide();
        if (_label)
          _label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setCode(QString pString)
{
  if (DEBUG)
    qDebug("%s::setCode(%d %d %s) with _codes.count %d and _ids.count %d",
           objectName().toAscii().data(), pString.isNull(), pString.isEmpty(),
           pString.toAscii().data(), _codes.count(), _ids.count());

  if (pString.isEmpty())
  {
    setId(-1);
    setCurrentText(pString);
  }
  else if (count() == _codes.count())
  {
    for (int counter = ((allowNull()) ? 1 : 0); counter < count(); counter++)
    {
      if (_codes.at(counter) == pString)
      {
        if (DEBUG)
          qDebug("%s::setCode(%s) found at %d with _ids.count %d & _lastId %d",
                 objectName().toAscii().data(), pString.toAscii().data(),
                 counter, _ids.count(), _lastId);
        setCurrentItem(counter);
        
        if (_ids.count() && _lastId!=_ids.at(counter))
          setId(_ids.at(counter));
        
        return;
      }
    }
  }
  else  // this is an ad-hoc combobox without a query behind it?
  {
    setCurrentItem(findText(pString));
    if (DEBUG)
      qDebug("%s::setCode(%s) set current item to %d using findData()",
             objectName().toAscii().data(), pString.toAscii().data(),
             currentItem());
    if (_ids.count() > currentItem())
      setId(_ids.at(currentItem()));
    if (DEBUG)
      qDebug("%s::setCode(%s) current item is %d after setId",
             objectName().toAscii().data(), pString.toAscii().data(),
             currentItem());
  }
  
  if (editable())
  {
    setId(-1);
    setCurrentText(pString);
  }
}

void XComboBox::setId(int pTarget)
{
  // reports are a special case: they should really be stored by name, not id
  if (_type == Reports)
  {
    XSqlQuery query;
    query.prepare("SELECT report_id "
                  "FROM report "
                  "WHERE (report_name IN (SELECT report_name "
                  "                       FROM report "
                  "                       WHERE (report_id=:report_id)));");
    query.bindValue(":report_id", pTarget);
    query.exec();
    while (query.next())
    {
      int id = query.value("report_id").toInt();
      for (int counter = 0; counter < count(); counter++)
      {
        if (_ids.at(counter) == id)
        {
          setCurrentItem(counter);

          if(_lastId!=id)
          {
            _lastId = id;
            emit newID(pTarget);
            emit valid(TRUE);

            if (allowNull())
              emit notNull(TRUE);
          }

          return;
        }
      }
    }
  }
  else
  {
    for (int counter = 0; counter < count(); counter++)
    {
      if (_ids.at(counter) == pTarget)
      {
        setCurrentItem(counter);

        if(_lastId!=pTarget)
        {
          _lastId = pTarget;
          emit newID(pTarget);
          emit valid(TRUE);

          if (allowNull())
            emit notNull(TRUE);
        }

        return;
      }
    }
  }

  setNull();
}

void XComboBox::setText(QVariant &pVariant)
{
  XComboBox::setText(pVariant.toString());
}

void XComboBox::setText(const QString &pString)
{
  if (count())
  {
    for (int counter = ((allowNull()) ? 1 : 0); counter < count(); counter++)
  {
      if (text(counter) == pString)
      {
        setCurrentItem(counter);
        return;
      }
    }
  }

  if (editable())
  {
    setId(-1);
    setCurrentText(pString);
  }
}

void XComboBox::setAllowNull(bool pAllowNull)
{
  _allowNull = pAllowNull;
}

void XComboBox::setNull()
{
  if (allowNull())
  {
    _lastId = -1;

    setCurrentItem(0);
    emit newID(-1);
    emit valid(FALSE);
    emit notNull(FALSE);
  }
}

void XComboBox::setNullStr(const QString& pNullStr)
{
  _nullStr = pNullStr;
  if (allowNull())
    setItemText(0, pNullStr);
}

void XComboBox::setText(const QVariant &pVariant)
{
  setText(pVariant.toString());
}

void XComboBox::setEditable(bool pEditable)
{
/*
  QWidget    *before = NULL;
  QWidget    *after  = NULL;

// Find the focus proxy before and after this
  QFocusData *fd = focusData();
  for (QWidget *cursor = fd->first();; cursor=fd->next())
  {
    if (cursor == this)
    {
      before = fd->prev();
      fd->next();
      after = fd->next();
      break;
    }
    else if (cursor == fd->last())
      break;
  }
*/

//  Set the editable state
  QComboBox::setEditable(pEditable);

/*
//  If possible, reset the tab order
  if (before)
  {
    setTabOrder(before, this);
    setTabOrder(this, after);
  }
*/
}

bool XComboBox::editable() const
{
  return QComboBox::editable();
}

void XComboBox::clear()
{
  QComboBox::clear();

  if (_ids.count())
    _ids.clear();
    
  if (_codes.count())
    _codes.clear();

  if (allowNull())
    append(-1, _nullStr);
}

// a hack to repopulate the combobox to fix MC bug 3698
void XComboBox::populate()
{
    enum XComboBoxTypes tmpType = _type;
    setType(Adhoc);
    setType(tmpType);
}

void XComboBox::populate(XSqlQuery &pQuery, int pSelected)
{
  int selected = 0;
  int counter  = 0;

//  Clear any old data
  clear();

  if (allowNull())
    counter++;

//  Load the combobox with the contents of the passed query, if any
  if (pQuery.first())
  {
    do
    {
      append(pQuery.value(0).toInt(), pQuery.value(1).toString(), pQuery.value(2).toString());

      if (pQuery.value(0).toInt() == pSelected)
        selected = counter;

      counter++;
    }
    while(pQuery.next());
  }

  setCurrentItem(selected);
  if (_ids.count())
  {
    _lastId = _ids.at(selected);

    if (allowNull())
      emit notNull(TRUE);
  }
  else
  {
    _lastId = -1;

    if (allowNull())
      emit notNull(FALSE);
  }

  emit newID(_lastId);
  emit valid((_lastId != -1));
}

void XComboBox::populate(const QString & pSql, int pSelected)
{
  qApp->setOverrideCursor(Qt::waitCursor);
  XSqlQuery query(pSql);
  populate(query, pSelected);
  qApp->restoreOverrideCursor();
}

void XComboBox::append(int pId, const QString &pText)
{
  append(pId,pText,pText);
}

void XComboBox::append(int pId, const QString &pText, const QString &pCode)
{
  _ids.append(pId);
  insertItem(pText);
  _codes.append(pCode);
}

int XComboBox::id(int pIndex) const
{
  if ((pIndex >= 0) && (pIndex < count()))
  {
    if ( (allowNull()) && (currentItem() <= 0) )
      return -1;
    else
      return _ids.at(pIndex);
  }
  else
    return -1;
}

int XComboBox::id() const
{
  if (_ids.count())
  {
    if ( (allowNull()) && (currentItem() <= 0) )
      return -1;
    else
      return _ids.at(currentItem());
  }
  else
    return -1;
}

QString XComboBox::code() const
{
  if (DEBUG)
    qDebug("%s::code() with currentItem %d, allowNull %d, and _codes.count %d",
           objectName().toAscii().data(), currentItem(), allowNull(),
           _codes.count());

  QString returnValue;

  if ( allowNull() && (currentItem() <= 0) )
    returnValue = QString::Null();
  else if (currentItem() >= 0 && _codes.count() > currentItem())
    returnValue = _codes.at(currentItem());
  else if (currentItem() >= 0)
    returnValue = currentText();
  else
    returnValue = QString::Null();

  if (DEBUG)
    qDebug("%s::code() returning %s",
           objectName().toAscii().data(), returnValue.toAscii().data());
  return returnValue;
}

bool XComboBox::isValid() const
{
  if ((allowNull()) && (id() == -1))
    return FALSE;
  else
    return TRUE;
}

void XComboBox::sHandleNewIndex(int pIndex)
{
  if (DEBUG)
    qDebug("%s::sHandleNewIndex(%d)",objectName().toAscii().data(), pIndex);

  if ((pIndex >= 0) && (pIndex < _ids.count()) && (_ids.at(pIndex) != _lastId))
  {
    _lastId = _ids.at(pIndex);
    emit newID(_lastId);

    if (DEBUG)
      qDebug("%s::sHandleNewIndex() emitted %d",
             objectName().toAscii().data(), _lastId);
    
    if (allowNull())
    {
      emit valid((pIndex != 0));
      emit notNull((pIndex != 0));
    }
  }

  if (DEBUG)
    qDebug("%s::sHandleNewIndex() returning",
           objectName().toAscii().data());
}

void XComboBox::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QComboBox::mousePressEvent(event);
}

QSize XComboBox::sizeHint() const
{
  QSize s = QComboBox::sizeHint();
#ifdef Q_WS_MAC
  s.setWidth(s.width() + 12);
#endif
  return s;
}

