/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QAbstractItemView>
#include <QDebug>
#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QSqlRelationalDelegate>
#include <QSqlTableModel>
#include <QtScript>

#include <metasql.h>
#include <xsqlquery.h>

#include "xcombobox.h"
#include "xcomboboxprivate.h"
#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"

#define DEBUG false

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QByteArray *UNWRAPSLOT(const char *slotCharStar)
{
  static QRegularExpression stripper("^\\d+(.*)\\(.*\\)$");
  QRegularExpressionMatch match = stripper.match(QString(slotCharStar));
  if (match.hasMatch())
  {
    return new QByteArray(match.captured(1).toUtf8());
  }
  return 0;
}
#else
QByteArray *UNWRAPSLOT(const char *slotCharStar)
{
  static QRegExp stripper("^\\d+(.*)\\(.*\\)$");
  QString theString(slotCharStar);
  if (theString.contains(stripper))
  {
    return new QByteArray(stripper.cap(1).toUtf8());
  }
  return 0;
}
#endif

QHash<XComboBox::XComboBoxTypes, XComboBoxDescrip*> XComboBoxPrivate::typeDescrip;

XComboBoxDescrip::XComboBoxDescrip()
  : type(XComboBox::Adhoc),
    isDirty(true),
    isEditable(false)
{
}

XComboBoxDescrip::XComboBoxDescrip(XComboBox::XComboBoxTypes pType,
                                   const QString &pUi,
                                   const QString &pPriv,
                                   const QString &pQry,
                                   const QString &pNotification,
                                   bool           pEditable,
                                   const QString &pKey,
                                   const QString &pValue)
  : type(pType),
    uiName(pUi),
    privilege(pPriv),
    queryStr(pQry),
    isDirty(true),
    isEditable(pEditable),
    notification(pNotification)
{
  if (! pValue.isNull() && ! pKey.isEmpty())
    params.append(pKey, pValue);
  else if (! pKey.isEmpty())
    params.append(pKey);

  if (QSqlDatabase::database().isOpen())
    query = MetaSQLQuery(queryStr).toQuery(params, QSqlDatabase(), false);
  if (XComboBox::_guiClientInterface)
    connect(XComboBox::_guiClientInterface, SIGNAL(dbConnectionLost()), this, SLOT(sDbConnectionLost()));

  sListen();
}

XComboBoxDescrip::~XComboBoxDescrip()
{
}

void XComboBoxDescrip::sDbConnectionLost()
{
  if (DEBUG) qDebug() << "XComboBoxDescrip::sDbConnectionLost()";
  isDirty = true;
}

void XComboBoxDescrip::sListen()
{
  QSqlDatabase db = QSqlDatabase::database();
  foreach (QString notice,
           notification.split(" ", QString::SkipEmptyParts))
  {
    if (! db.driver()->subscribedToNotifications().contains(notice))
    {
      db.driver()->subscribeToNotification(notice);
      connect(db.driver(), SIGNAL(notification(const QString&)),
              this,        SLOT(sNotified(const QString&)));
    }
  }
}

void XComboBoxDescrip::sNotified(const QString &pNotification)
{
  Q_UNUSED(pNotification);
  isDirty = true;
}

static QString bankaccntMQL("SELECT bankaccnt_id,"
                            "       bankaccnt_name || '-' || bankaccnt_descrip,"
                            "       bankaccnt_name"
                            "  FROM bankaccnt"
                            " <? if exists('isAR') ?>WHERE bankaccnt_ar<? endif ?>"
                            " <? if exists('isAP') ?>WHERE bankaccnt_ap<? endif ?>"
                            " ORDER BY bankaccnt_name;");
static QString cmnttypeMQL("SELECT cmnttype_id, cmnttype_name, cmnttype_name"
                           "  FROM cmnttype"
                           "<? if exists('source_name') ?>"
                           "  JOIN cmnttypesource ON cmnttypesource_cmnttype_id = cmnttype_id"
                           "  JOIN source ON source_id = cmnttypesource_source_id"
                           " WHERE source_name = <? value('source_name') ?>"
                           "<? endif ?>"
                           " ORDER BY cmnttype_order, cmnttype_name;");
static QString countryMQL("SELECT country_id, country_name, country_name"
                          "  FROM country"
                          " <? if exists('Qt') ?>WHERE country_qt_number IS NOT NULL<? endif ?>"
                          " ORDER BY country_name;");
static QString currMQL("SELECT curr_id,"
                       "       currConcat(curr_abbr, curr_symbol), curr_abbr"
                       "  FROM curr_symbol"
                       " <? if exists('notBase') ?>WHERE NOT curr_base<? endif ?>"
                       " ORDER BY curr_base DESC, curr_abbr;");
static QString prjMQL("SELECT prj_id, prj_number || '-' || prj_name, prj_number"
                      "  FROM prj"
                      " WHERE true"
                      " <? if exists('isPO') ?>AND prj_po<? endif ?>"
                      " <? if exists('isSO') ?>AND prj_so<? endif ?>"
                      " <? if exists('isWO') ?>AND prj_wo<? endif ?>"
                      " <? if exists('activeOnly') ?>AND prj_completed_date < current_date<? endif ?>"
                      " ORDER BY desc;" );
static QString rsncodeMQL("SELECT rsncode_id,"
                          "       rsncode_code || '-' || rsncode_descrip,"
                          "       rsncode_code"
                          "  FROM rsncode"
                          "<? if exists('doctype') ?>"
                          " WHERE rsncode_doctype IS NULL"
                          "   OR rsncode_doctype=<? value('doctype') ?>"
                          "<? endif ?>"
                          " ORDER BY rsncode_code;");
static QString salescatMQL("SELECT salescat_id,"
                           "       salescat_name || '-' || salescat_descrip,"
                           "       salescat_name"
                           "  FROM salescat"
                           " <? if exists('activeOnly') ?>WHERE salescat_active<? endif ?>"
                           " ORDER BY salescat_name;");
static QString salesrepMQL("SELECT salesrep_id,"
                           "       salesrep_number || '-' || salesrep_name,"
                           "       salesrep_number"
                           "  FROM salesrep"
                           " <? if exists('activeOnly') ?>WHERE salesrep_active<? endif ?>"
                           " ORDER by salesrep_number;");
static QString termsMQL("SELECT terms_id,"
                        "       terms_code || '-' || terms_descrip, terms_code"
                        "  FROM terms"
                        "  WHERE false"
                        " <? if exists('isAR') ?>OR terms_ar<? endif ?>"
                        " <? if exists('isAP') ?>OR terms_ap<? endif ?>"
                        " ORDER by terms_code;");
static QString usrMQL("SELECT usr_id, usr_username, usr_username"
                      "  FROM usr"
                      " WHERE true"
                      " <? if exists('activeOnly') ?> AND usr_active <? endif ?>"
                      " <? if exists('isAgent') ?>    AND usr_agent  <? endif ?>"
                      " ORDER BY usr_username;");
static QString wrkcntMQL("SELECT wrkcnt_id, (wrkcnt_code || '-' || wrkcnt_descrip), wrkcnt_code"
                         "  FROM xtmfg.wrkcnt"
                         " <? if exists('activeOnly') ?>WHERE wrkcnt_active<? endif ?>"
                         " ORDER BY wrkcnt_code;");

XComboBoxPrivate::XComboBoxPrivate(XComboBox *pParent)
  : QObject(pParent),
    _default(XComboBox::First),
    _descrip(0),
    _editButton(0),
    _label(0),
    _parent(pParent),
    _popupCounter(0),
    _type(XComboBox::Adhoc),
    _editor(0),
    _slot(0),
    _mapper(0)
{
  setObjectName((_parent ? _parent->objectName() : "XComboBox") + "Private");

  _mapper = new XDataWidgetMapper(pParent);

  if (typeDescrip.isEmpty()) {
    typeDescrip.insert(XComboBox::AddressCommentTypes,
                       new XComboBoxDescrip(XComboBox::AddressCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "ADDR"));
    typeDescrip.insert(XComboBox::APBankAccounts,
                       new XComboBoxDescrip(XComboBox::APBankAccounts,
                       "bankAccounts", "MaintainBankAccounts",
                       bankaccntMQL, "bankaccnt", true, "isAP"));
    typeDescrip.insert(XComboBox::APTerms,
                       new XComboBoxDescrip(XComboBox::APTerms,
                       "termses", "MaintainTerms",
                       termsMQL, "terms", true, "isAP"));
    typeDescrip.insert(XComboBox::ARBankAccounts,
                       new XComboBoxDescrip(XComboBox::ARBankAccounts,
                       "bankAccounts", "MaintainBankAccounts",
                       bankaccntMQL, "bankaccnt", true, "isAR"));
    typeDescrip.insert(XComboBox::ARCMReasonCodes,
                       new XComboBoxDescrip(XComboBox::ARCMReasonCodes,
                       "reasonCodes", "MaintainReasonCodes",
                       rsncodeMQL, "rsncode", true, "ARCM"));
    typeDescrip.insert(XComboBox::ARDMReasonCodes,
                       new XComboBoxDescrip(XComboBox::ARDMReasonCodes,
                       "reasonCodes", "MaintainReasonCodes",
                       rsncodeMQL, "rsncode", true, "ARDM"));
    typeDescrip.insert(XComboBox::ARTerms,
                       new XComboBoxDescrip(XComboBox::ARTerms,
                       "termses", "MaintainTerms",
                       termsMQL, "terms", true, "isAR"));
    typeDescrip.insert(XComboBox::AccountingPeriods,
                       new XComboBoxDescrip(XComboBox::AccountingPeriods,
                       "accountingPeriods", "MaintainAccountingPeriods",
                       "SELECT period_id,"
                       "       formatDate(period_start) || '-' || formatDate(period_end),"
                       "       formatDate(period_start) || '-' || formatDate(period_end)"
                       "  FROM period"
                       " <? if exists('activeOnly') ?>WHERE not period_closed<? endif ?>"
                       " ORDER BY period_start DESC;", "period"));
    typeDescrip.insert(XComboBox::Agent,
                       new XComboBoxDescrip(XComboBox::Agent,
                       "users", "MaintainUsers",
                       usrMQL, QString(), true, "isAgent"));
    typeDescrip.insert(XComboBox::AllCommentTypes,
                       new XComboBoxDescrip(XComboBox::AllCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype"));
    typeDescrip.insert(XComboBox::AllProjects,
                       new XComboBoxDescrip(XComboBox::AllProjects,
                       "projects", "MaintainAllProjects",
                       prjMQL, "prj"));
    typeDescrip.insert(XComboBox::BBOMHeadCommentTypes,
                       new XComboBoxDescrip(XComboBox::BBOMHeadCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BBH"));
    typeDescrip.insert(XComboBox::BBOMItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::BBOMItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BBI"));
    typeDescrip.insert(XComboBox::BOMHeadCommentTypes,
                       new XComboBoxDescrip(XComboBox::BOMHeadCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BMH"));
    typeDescrip.insert(XComboBox::BOMItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::BOMItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BMI"));
    typeDescrip.insert(XComboBox::BOOHeadCommentTypes,
                       new XComboBoxDescrip(XComboBox::BOOHeadCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BOH"));
    typeDescrip.insert(XComboBox::BOOItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::BOOItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "BOI"));
    typeDescrip.insert(XComboBox::CRMAccounts,
                       new XComboBoxDescrip(XComboBox::CRMAccounts,
                       "crmaccounts", "MaintainAllCRMAccounts",
                       "SELECT crmacct_id,"
                       "       crmacct_number || '-' || crmacct_name,"
                       "       crmacct_number"
                       "  FROM crmacct"
                       " <? if exists('activeOnly') ?>WHERE crmacct_active<? endif ?>"
                       " ORDER BY crmacct_number;", "crmacct"));
    typeDescrip.insert(XComboBox::CRMAccountCommentTypes,
                       new XComboBoxDescrip(XComboBox::CRMAccountCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "CRMA"));
    typeDescrip.insert(XComboBox::ClassCodes,
                       new XComboBoxDescrip(XComboBox::ClassCodes,
                       "classCodes", "MaintainClassCodes",
                       "SELECT classcode_id,"
                       "       classcode_code || '-' || classcode_descrip,"
                       "       classcode_code"
                       "  FROM classcode"
                       " ORDER BY classcode_code;", "classcode"));
    typeDescrip.insert(XComboBox::Companies,
                       new XComboBoxDescrip(XComboBox::Companies,
                       "companies", "MaintainChartOfAccounts",
                       "SELECT company_id, company_number, company_number"
                       "  FROM company"
                       " ORDER BY company_number;", "company"));
    typeDescrip.insert(XComboBox::ContactCommentTypes,
                       new XComboBoxDescrip(XComboBox::ContactCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "T"));
    typeDescrip.insert(XComboBox::Contracts,
                       new XComboBoxDescrip(XComboBox::Contracts,
                       "", "",
                       "SELECT contrct_id,"
                       "       vend_number || '-' || contrct_number,"
                       "       contrct_number"
                       "  FROM contrct"
                       "  JOIN vendinfo ON vend_id = contrct_vend_id"
                       " <? if exists('activeOnly') ?>WHERE contrct_expires<? endif ?>"
                       " ORDER BY vend_number, contrct_number;", "contract vendinfo"));
    typeDescrip.insert(XComboBox::CostCategories,
                       new XComboBoxDescrip(XComboBox::CostCategories,
                       "costCategories", "MaintainCostCategories",
                       "SELECT costcat_id,"
                       "       costcat_code || '-' || costcat_descrip,"
                       "       costcat_code"
                       "  FROM costcat"
                       " ORDER BY costcat_code;", "costcat"));
    typeDescrip.insert(XComboBox::Countries,
                       new XComboBoxDescrip(XComboBox::Countries,
                       "countries", "MaintainCountries",
                       countryMQL, "country"));
    typeDescrip.insert(XComboBox::Currencies,
                       new XComboBoxDescrip(XComboBox::Currencies,
                       "currencies", "MaintainCurrencies",
                       currMQL, "curr_symbol"));
    typeDescrip.insert(XComboBox::CurrenciesNotBase,
                       new XComboBoxDescrip(XComboBox::CurrenciesNotBase,
                       "currencies", "MaintainCurrencies",
                       currMQL, "curr_symbol", true, "notBase"));
    typeDescrip.insert(XComboBox::CustomerCommentTypes,
                       new XComboBoxDescrip(XComboBox::CustomerCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "C"));
    typeDescrip.insert(XComboBox::CustomerGroups,
                       new XComboBoxDescrip(XComboBox::CustomerGroups,
                       "customerGroups", "MaintainCustomerGroups",
                       "SELECT custgrp_id, custgrp_name, custgrp_name"
                       "  FROM custgrp"
                       " ORDER BY custgrp_name;", "custgrp"));
    typeDescrip.insert(XComboBox::CustomerTypes,
                       new XComboBoxDescrip(XComboBox::CustomerTypes,
                       "customerTypes", "MaintainCustomerTypes",
                       "SELECT custtype_id,"
                       "       custtype_code || '-' || custtype_descrip"
                       "       custtype_code"
                       "  FROM custtype"
                       " ORDER BY custtype_code;", "custtype"));
    typeDescrip.insert(XComboBox::EmployeeCommentTypes,
                       new XComboBoxDescrip(XComboBox::EmployeeCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "EMP"));
    typeDescrip.insert(XComboBox::ExchangeRateCommentTypes,
                       new XComboBoxDescrip(XComboBox::ExchangeRateCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "FX"));
    typeDescrip.insert(XComboBox::ExpenseCategories,
                       new XComboBoxDescrip(XComboBox::ExpenseCategories,
                       "expenseCategories", "MaintainCustomerTypes",
                       "SELECT expcat_id,"
                       "       expcat_code || '-' || expcat_descrip,"
                       "       expcat_code"
                       "  FROM expcat"
                       " <? if exists('activeOnly') ?>WHERE expcat_active<? endif ?>"
                       " ORDER BY expcat_code;", "expcat"));
    typeDescrip.insert(XComboBox::FinancialLayouts,
                       new XComboBoxDescrip(XComboBox::FinancialLayouts,
                       "financialLayouts", "MaintainFinancialLayouts",
                       "SELECT flhead_id, flhead_name, flhead_name"
                       "  FROM flhead"
                       " <? if exists('activeOnly') ?>WHERE flhead_active<? endif ?>"
                       " ORDER BY flhead_name;", "flhead"));
    typeDescrip.insert(XComboBox::FiscalYears,
                       new XComboBoxDescrip(XComboBox::FiscalYears,
                       "accountingYearPeriods", "MaintainAccountingPeriods",
                       "SELECT yearperiod_id,"
                       "       formatdate(yearperiod_start) || '-' || formatdate(yearperiod_end),"
                       "       formatdate(yearperiod_start) || '-' || formatdate(yearperiod_end)"
                       "  FROM yearperiod"
                       " <? if exists('activeOnly') ?>WHERE not yearperiod_closed<? endif ?>"
                       " ORDER BY yearperiod_start DESC;", "yearperiod"));
    typeDescrip.insert(XComboBox::FreightClasses,
                       new XComboBoxDescrip(XComboBox::FreightClasses,
                       "freightClasses", "MaintainFreightClasses",
                       "SELECT freightclass_id,"
                       "       freightclass_code || '-' || freightclass_descrip,"
                       "       freightclass_code"
                       "  FROM freightclass"
                       " ORDER BY freightclass_code;", "freightclass"));
    typeDescrip.insert(XComboBox::Honorifics,
                       new XComboBoxDescrip(XComboBox::Honorifics,
                       "honorifics", "MaintainTitles",
                       "SELECT hnfc_id, hnfc_code, hnfc_code"
                       "  FROM hnfc"
                       " ORDER BY hnfc_code;", "hnfc"));
    typeDescrip.insert(XComboBox::IncidentCategory,
                       new XComboBoxDescrip(XComboBox::IncidentCategory,
                       "incidentCategories", "MaintainIncidentCategories",
                       "SELECT incdtcat_id, incdtcat_name, incdtcat_name"
                       "  FROM incdtcat"
                       " ORDER BY incdtcat_order, incdtcat_name;", "incdtcat"));
    typeDescrip.insert(XComboBox::IncidentCommentTypes,
                       new XComboBoxDescrip(XComboBox::IncidentCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "INCDT"));
    typeDescrip.insert(XComboBox::IncidentPriority,
                       new XComboBoxDescrip(XComboBox::IncidentPriority,
                       "incidentPriorities", "MaintainIncidentPriorities",
                       "SELECT incdtpriority_id,"
                       "       incdtpriority_name, incdtpriority_name"
                       "  FROM incdtpriority"
                       " ORDER BY incdtpriority_order, incdtpriority_name;",
                       "incdtpriority"));
    typeDescrip.insert(XComboBox::IncidentResolution,
                       new XComboBoxDescrip(XComboBox::IncidentResolution,
                       "incidentResolutions", "MaintainIncidentResolutions",
                       "SELECT incdtresolution_id,"
                       "       incdtresolution_name, incdtresolution_name"
                       "  FROM incdtresolution"
                       " ORDER BY incdtresolution_order, incdtresolution_name;",
                       "incdtresolution"));
    typeDescrip.insert(XComboBox::IncidentSeverity,
                       new XComboBoxDescrip(XComboBox::IncidentSeverity,
                       "incidentSeverities", "MaintainIncidentSeverities",
                       "SELECT incdtseverity_id,"
                       "       incdtseverity_name, incdtseverity_name"
                       "  FROM incdtseverity"
                       " ORDER BY incdtseverity_order, incdtseverity_name;",
                       "incdtseverity"));
    typeDescrip.insert(XComboBox::ItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::ItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "I"));
    typeDescrip.insert(XComboBox::ItemGroups,
                       new XComboBoxDescrip(XComboBox::ItemGroups,
                       "itemGroups", "MaintainItemGroups",
                       "SELECT itemgrp_id, itemgrp_name, itemgrp_name"
                       "  FROM itemgrp"
                       " ORDER BY itemgrp_name;", "itemgrp"));
    typeDescrip.insert(XComboBox::ItemSiteCommentTypes,
                       new XComboBoxDescrip(XComboBox::ItemSiteCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "IS"));
    typeDescrip.insert(XComboBox::ItemSourceCommentTypes,
                       new XComboBoxDescrip(XComboBox::ItemSourceCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "IR"));
    typeDescrip.insert(XComboBox::Locales,
                       new XComboBoxDescrip(XComboBox::Locales,
                       "locales", "MaintainLocales",
                       "SELECT locale_id, locale_code, locale_code"
                       "  FROM locale"
                       " ORDER BY locale_code;", "locale"));
    typeDescrip.insert(XComboBox::LocaleCountries,
                       new XComboBoxDescrip(XComboBox::LocaleCountries,
                       "", "",
                       countryMQL, "country", false, "Qt")); // Qt countries can not be edited
    typeDescrip.insert(XComboBox::LocaleLanguages,
                       new XComboBoxDescrip(XComboBox::LocaleLanguages,
                       "", "",
                       "SELECT lang_id, lang_name, lang_name"
                       "  FROM lang"
                       " WHERE lang_qt_number IS NOT NULL"
                       " ORDER BY lang_name;", "lang",
                       false)); // This is based on Qt languages, no editor
    typeDescrip.insert(XComboBox::LocationCommentTypes,
                       new XComboBoxDescrip(XComboBox::LocationCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "L"));
    typeDescrip.insert(XComboBox::LotSerialCommentTypes,
                       new XComboBoxDescrip(XComboBox::LotSerialCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "LS"));
    typeDescrip.insert(XComboBox::OpportunityCommentTypes,
                       new XComboBoxDescrip(XComboBox::OpportunityCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "OPP"));
    typeDescrip.insert(XComboBox::OpportunityStages,
                       new XComboBoxDescrip(XComboBox::OpportunityStages,
                       "opportunityStages", "MaintainOpportunityStages",
                       "SELECT opstage_id, opstage_name, opstage_name"
                       "  FROM opstage"
                       " ORDER BY opstage_order;", "opstage"));
    typeDescrip.insert(XComboBox::OpportunitySources,
                       new XComboBoxDescrip(XComboBox::OpportunitySources,
                       "opportunitySources", "MaintainOpportunitySources",
                       "SELECT opsource_id, opsource_name, opsource_name"
                       "  FROM opsource"
                       " ORDER BY opsource_name;", "opsource"));
    typeDescrip.insert(XComboBox::OpportunityTypes,
                       new XComboBoxDescrip(XComboBox::OpportunityTypes,
                       "opportunityTypes", "MaintainOpportunityTypes",
                       "SELECT optype_id, optype_name, optype_name"
                       "  FROM optype"
                       " ORDER BY optype_name;", "optype"));
    typeDescrip.insert(XComboBox::PlannerCodes,
                       new XComboBoxDescrip(XComboBox::PlannerCodes,
                       "plannerCodes", "MaintainPlannerCodes",
                       "SELECT plancode_id,"
                       "       plancode_code || '-' || plancode_name,"
                       "       plancode_code"
                       "  FROM plancode"
                       " ORDER BY plancode_code;", "plancode"));
    typeDescrip.insert(XComboBox::PoProjects,
                       new XComboBoxDescrip(XComboBox::PoProjects,
                       "projects", "MaintainAllProjects",
                       prjMQL, "prj", true, "isPO"));
    typeDescrip.insert(XComboBox::ProductCategories,
                       new XComboBoxDescrip(XComboBox::ProductCategories,
                       "productCategories", "MaintainProductCategories",
                       "SELECT prodcat_id,"
                       "       prodcat_code || ' - ' || prodcat_descrip,"
                       "       prodcat_code"
                       "  FROM prodcat"
                       " ORDER BY prodcat_code;", "prodcat"));
    typeDescrip.insert(XComboBox::ProfitCenters,
                       new XComboBoxDescrip(XComboBox::ProfitCenters,
                       "profitCenters", "MaintainChartOfAccounts",
                       "SELECT prftcntr_id, prftcntr_number, prftcntr_number"
                       "  FROM prftcntr"
                       " ORDER BY prftcntr_number;", "prftcntr",
                       _x_metrics && _x_metrics->boolean("GLFFProfitCenters")));
    typeDescrip.insert(XComboBox::ProjectCommentTypes,
                       new XComboBoxDescrip(XComboBox::ProjectCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "J"));
    typeDescrip.insert(XComboBox::PurchaseOrderCommentTypes,
                       new XComboBoxDescrip(XComboBox::PurchaseOrderCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "P"));
    typeDescrip.insert(XComboBox::PurchaseOrderItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::PurchaseOrderItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "PI"));
    typeDescrip.insert(XComboBox::ReasonCodes,
                       new XComboBoxDescrip(XComboBox::ReasonCodes,
                       "reasonCodes", "MaintainReasonCodes",
                       rsncodeMQL, "rsncode"));
    typeDescrip.insert(XComboBox::RegistrationTypes,
                       new XComboBoxDescrip(XComboBox::RegistrationTypes,
                       "", "",
                       "SELECT regtype_id, regtype_code, regtype_code"
                       "  FROM regtype"
                       " ORDER BY regtype_code;", "regtype", false));
    typeDescrip.insert(XComboBox::Reports,
                       new XComboBoxDescrip(XComboBox::Reports,
                       "reports", "MaintainReports",
                       "SELECT a.report_id, a.report_name, a.report_name"
                       "  FROM report a"
                       "  JOIN (SELECT MIN(report_grade) AS report_grade, report_name"
                       "          FROM report"
                       "         GROUP BY report_name) b ON a.report_name=b.report_name"
                       "                                AND a.report_grade=b.report_grade"
                       " ORDER BY report_name;", "report", false));
    typeDescrip.insert(XComboBox::ReturnReasonCodes,
                       new XComboBoxDescrip(XComboBox::ReturnReasonCodes,
                       "reasonCodes", "MaintainReasonCodes",
                       rsncodeMQL, "rsncode", true, "RA"));
    typeDescrip.insert(XComboBox::ReturnAuthCommentTypes,
                       new XComboBoxDescrip(XComboBox::ReturnAuthCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "RA"));
    typeDescrip.insert(XComboBox::ReturnAuthItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::ReturnAuthItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "RI"));
    typeDescrip.insert(XComboBox::QuoteCommentTypes,
                       new XComboBoxDescrip(XComboBox::QuoteCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "Q"));
    typeDescrip.insert(XComboBox::QuoteItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::QuoteItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "QI"));
    typeDescrip.insert(XComboBox::SalesOrderCommentTypes,
                       new XComboBoxDescrip(XComboBox::SalesOrderCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "S"));
    typeDescrip.insert(XComboBox::SalesOrderItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::SalesOrderItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "SI"));
    typeDescrip.insert(XComboBox::SalesCategories,
                       new XComboBoxDescrip(XComboBox::SalesCategories,
                       "salesCategories", "MaintainSalesCategories",
                       salescatMQL, "salescat"));
    typeDescrip.insert(XComboBox::SalesCategoriesActive,
                       new XComboBoxDescrip(XComboBox::SalesCategoriesActive,
                       "salesCategories", "MaintainSalesCategories",
                       salescatMQL, "salescat", true, "activeOnly"));
    typeDescrip.insert(XComboBox::SalesReps,
                       new XComboBoxDescrip(XComboBox::SalesReps,
                       "salesReps", "MaintainSalesReps",
                       salesrepMQL, "salesrep"));
    typeDescrip.insert(XComboBox::SalesRepsActive,
                       new XComboBoxDescrip(XComboBox::SalesRepsActive,
                       "salesReps", "MaintainSalesReps",
                       salesrepMQL, "salesrep", true, "activeOnly"));
    typeDescrip.insert(XComboBox::SaleTypes,
                       new XComboBoxDescrip(XComboBox::SaleTypes,
                       "saleTypes", "MaintainSaleTypes",
                       "SELECT saletype_id,"
                       "       saletype_code || '-' || saletype_descr,"
                       "       saletype_code"
                       "  FROM saletype"
                       " <? if exists('activeOnly') ?>WHERE saletype_active<? endif ?>"
                       " ORDER BY saletype_default DESC, saletype_code;",
                       "saletype"));
    typeDescrip.insert(XComboBox::ShipVias,
                       new XComboBoxDescrip(XComboBox::ShipVias,
                       "shipVias", "MaintainShipVias",
                       "SELECT shipvia_id,"
                       "       shipvia_code || '-' || shipvia_descrip,"
                       "       shipvia_code"
                       "  FROM shipvia"
                       " ORDER BY shipvia_code;", "shipvia"));
    typeDescrip.insert(XComboBox::ShippingCharges,
                       new XComboBoxDescrip(XComboBox::ShippingCharges,
                       "shippingChargeTypes", "MaintainShippingChargeTypes",
                       "SELECT shipchrg_id,"
                       "       shipchrg_name || '-' || shipchrg_descrip,"
                       "       shipchrg_name"
                       "  FROM shipchrg"
                       " ORDER by shipchrg_name;", "shipchrg"));
    typeDescrip.insert(XComboBox::ShippingForms,
                       new XComboBoxDescrip(XComboBox::ShippingForms,
                       "shippingForms", "MaintainShippingForms",
                       "SELECT shipform_id, shipform_name, shipform_name"
                       "  FROM shipform"
                       " ORDER BY shipform_name;", "shipform"));
    typeDescrip.insert(XComboBox::ShippingZones,
                       new XComboBoxDescrip(XComboBox::ShippingZones,
                       "shippingZones", "MaintainShippingZones",
                       "SELECT shipzone_id, shipzone_name, shipzone_name"
                       "  FROM shipzone"
                       " ORDER BY shipzone_name;", "shipzone"));
    typeDescrip.insert(XComboBox::SiteTypes,
                       new XComboBoxDescrip(XComboBox::SiteTypes,
                       "siteTypes", "MaintainSiteTypes",
                       "SELECT sitetype_id, sitetype_name, sitetype_name"
                       "  FROM sitetype"
                       " ORDER BY sitetype_name;", "sitetype"));
    typeDescrip.insert(XComboBox::SoProjects,
                       new XComboBoxDescrip(XComboBox::SoProjects,
                       "projects", "MaintainAllProjects",
                       prjMQL, "prj", true, "isSO"));
    typeDescrip.insert(XComboBox::Subaccounts,
                       new XComboBoxDescrip(XComboBox::Subaccounts,
                       "subaccounts", "MaintainChartOfAccounts",
                       "SELECT subaccnt_id, subaccnt_number, subaccnt_number"
                       "  FROM subaccnt"
                       " ORDER BY subaccnt_number;", "subaccnt",
                       _x_metrics && _x_metrics->boolean("GLFFSubaccounts")));
    typeDescrip.insert(XComboBox::TaxAuths,
                       new XComboBoxDescrip(XComboBox::TaxAuths,
                       "taxAuthorities", "MaintainTaxAuthorities",
                       "SELECT taxauth_id, taxauth_code, taxauth_code"
                       "  FROM taxauth"
                       " ORDER BY taxauth_code;", "taxauth"));
    typeDescrip.insert(XComboBox::TaxClasses,
                       new XComboBoxDescrip(XComboBox::TaxClasses,
                       "taxClasses", "MaintainTaxClasses",
                       "SELECT taxclass_id,"
                       "       taxclass_code || '-' || taxclass_descrip,"
                       "       taxclass_code"
                       "  FROM taxclass"
                       " ORDER BY taxclass_code;", "taxclass"));
    typeDescrip.insert(XComboBox::TaxCodes,
                       new XComboBoxDescrip(XComboBox::TaxCodes,
                       "taxCodes", "MaintainTaxCodes",
                       "SELECT tax_id,"
                       "       tax_code || '-' || tax_descrip,"
                       "       tax_code"
                       "  FROM tax"
                       " ORDER BY tax_code;", "tax"));
    typeDescrip.insert(XComboBox::TaxZones,
                       new XComboBoxDescrip(XComboBox::TaxZones,
                       "taxZones", "MaintainTaxZones",
                       "SELECT taxzone_id,"
                       "       taxzone_code || '-' || taxzone_descrip,"
                       "       taxzone_code"
                       "  FROM taxzone"
                       " ORDER BY taxzone_code;", "taxzone"));
    typeDescrip.insert(XComboBox::TaxTypes,
                       new XComboBoxDescrip(XComboBox::TaxTypes,
                       "taxTypes", "MaintainTaxTypes",
                       "SELECT taxtype_id, taxtype_name, taxtype_name"
                       "  FROM taxtype"
                       " ORDER BY taxtype_name;", "taxtype"));
    typeDescrip.insert(XComboBox::Terms,
                       new XComboBoxDescrip(XComboBox::Terms,
                       "terms", "MaintainTerms",
                       termsMQL, "terms"));
    typeDescrip.insert(XComboBox::TaskCommentTypes,
                       new XComboBoxDescrip(XComboBox::TaskCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "TA"));
    typeDescrip.insert(XComboBox::TimeAttendanceCommentTypes,
                       new XComboBoxDescrip(XComboBox::TimeAttendanceCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "TATC"));
    typeDescrip.insert(XComboBox::TodoItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::TodoItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "TD"));
    typeDescrip.insert(XComboBox::TransferOrderCommentTypes,
                       new XComboBoxDescrip(XComboBox::TransferOrderCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "TO"));
    typeDescrip.insert(XComboBox::TransferOrderItemCommentTypes,
                       new XComboBoxDescrip(XComboBox::TransferOrderItemCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "TI"));
    typeDescrip.insert(XComboBox::UOMs,
                       new XComboBoxDescrip(XComboBox::UOMs,
                       "uoms", "MaintainUOMs",
                       "SELECT uom_id, uom_name, uom_name"
                       "  FROM uom"
                       " ORDER BY uom_name;", "uom"));
    typeDescrip.insert(XComboBox::Users,
                       new XComboBoxDescrip(XComboBox::Users,
                       "users", "MaintainUsers",
                       usrMQL, QString()));
    typeDescrip.insert(XComboBox::ActiveUsers,
                       new XComboBoxDescrip(XComboBox::ActiveUsers,
                       "users", "MaintainUsers",
                       usrMQL, QString(), true, "activeOnly"));
    typeDescrip.insert(XComboBox::VendorCommentTypes,
                       new XComboBoxDescrip(XComboBox::VendorCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "V"));
    typeDescrip.insert(XComboBox::VendorGroups,
                       new XComboBoxDescrip(XComboBox::VendorGroups,
                       "", "",
                       "SELECT vendgrp_id, vendgrp_name, vendgrp_name"
                       "  FROM vendgrp"
                       " ORDER BY vendgrp_name;", "vendgrp", false));
    typeDescrip.insert(XComboBox::VendorTypes,
                       new XComboBoxDescrip(XComboBox::VendorTypes,
                       "vendorTypes", "MaintainVendorTypes",
                       "SELECT vendtype_id,"
                       "       vendtype_code || '-' || vendtype_descrip,"
                       "       vendtype_code"
                       "  FROM vendtype"
                       " ORDER BY vendtype_code;", "venttype"));
    typeDescrip.insert(XComboBox::WarehouseCommentTypes,
                       new XComboBoxDescrip(XComboBox::WarehouseCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "WH"));
    typeDescrip.insert(XComboBox::WoProjects,
                       new XComboBoxDescrip(XComboBox::WoProjects,
                       "projects", "MaintainAllProjects",
                       prjMQL, "prj", true, "isWO"));
    typeDescrip.insert(XComboBox::WorkCenters,
                       new XComboBoxDescrip(XComboBox::WorkCenters,
                       "workCenters", "MaintainWorkCenters",
                       wrkcntMQL, "wrkcnt"));
    typeDescrip.insert(XComboBox::WorkCentersActive,
                       new XComboBoxDescrip(XComboBox::WorkCentersActive,
                       "workCenters", "MaintainWorkCenters",
                       wrkcntMQL, "wrkcnt", true, "activeOnly"));
    typeDescrip.insert(XComboBox::WorkOrderCommentTypes,
                       new XComboBoxDescrip(XComboBox::WorkOrderCommentTypes,
                       "commentTypes", "MaintainCommentTypes",
                       cmnttypeMQL, "cmnttype", true, "source_name", "W"));
    if (DEBUG) qDebug () << "populated" << typeDescrip.size();
  }
}

XComboBoxPrivate::~XComboBoxPrivate()
{
  if (_slot)
  {
    delete _slot;
    _slot = 0;
  }
}

void XComboBoxPrivate::sEdit()
{
  if (_descrip)
    _descrip->isDirty = true;
  if (_editor && ! _slot->isEmpty())
  {
    QMetaObject::invokeMethod(_editor, _slot->data(), Qt::DirectConnection);
  }
  else if (_parent->_guiClientInterface && _parent->parentWidget()->window())
  {
    ParameterList params;
    params.append("mode", "edit");

    QWidget *w = _parent->_guiClientInterface
                        ->openWindow(_uiName.isEmpty() ? _descrip->uiName : _uiName,
                                     params,
                                     _parent->parentWidget()->window(),
                                     Qt::ApplicationModal,
                                     Qt::Dialog);
    if (qobject_cast<QDialog*>(w))
    {
      connect(w, SIGNAL(accepted()), _parent, SLOT(populate()));
      (qobject_cast<QDialog*>(w))->exec();
    }
    else
    {
      w->setAttribute(Qt::WA_DeleteOnClose);
      connect(w, SIGNAL(destroyed()), _parent, SLOT(populate()));
    }
  }
}

bool XComboBoxPrivate::addEditButton()
{
  QString privs = (_descrip ? _descrip->privilege : "") + " " + _privilege;
  QString ui    = (_descrip ? _descrip->uiName : _uiName);

  if (DEBUG)
    qDebug("%s::addEditButton() entered with _editButton %p, privs %s",
           qPrintable(objectName()), _editButton, qPrintable(privs));

  if (! ui.isEmpty() && (privs.trimmed().isEmpty() ||
                         (_x_privileges && _x_privileges->check(privs))))
  {
    QAbstractItemView *view = _parent->view();

    if (view && ! _editButton)
    {
      QWidget *vp = qobject_cast<QWidget*>(view->parent());
      if (vp && vp->layout())
      {
        _editButton = new QPushButton(tr("Edit List"), _parent);
        vp->layout()->addWidget(_editButton);
        connect(_editButton, SIGNAL(clicked()), this, SLOT(sEdit()));
      }
    }
  }
  else if (_editButton)
  {
    disconnect(_editButton, SIGNAL(clicked()), this, SLOT(sEdit()));
    delete _editButton;       // TODO: deletelater?
    _editButton = 0;
  }

  if (DEBUG)
    qDebug("%s::addEditButton() returning with _editButton %p, privs %s",
           qPrintable(objectName()), _editButton, qPrintable(privs));
  return (_editButton != 0);
}

void XComboBoxPrivate::setType(XComboBox::XComboBoxTypes ptype)
{
  if (DEBUG)
    qDebug("%s::setType(%d) entered with (%p %s)",
           qPrintable(objectName()), ptype, _x_privileges,
           qPrintable(_privilege));

  _type    = ptype;
  _descrip = typeDescrip.value(_type);
  if (_descrip && _descrip->isDirty)
  {
    _descrip->sListen();
    _descrip->query.exec();
    _descrip->isDirty = _descrip->notification.isEmpty(); // empty => there's no notification to listen for
  }

  addEditButton();
}

GuiClientInterface* XComboBox::_guiClientInterface = 0;

XComboBox::XComboBox(QWidget *pParent, const char *pName) :
  QComboBox(pParent),
  _data(0)
{
  if(pName)
    setObjectName(pName);
  init();
}

XComboBox::XComboBox(bool pEditable, QWidget *pParent, const char *pName) :
  QComboBox(pParent),
  _data(0)
{
  if(pName)
    setObjectName(pName);
  init();
  setEditable(pEditable);
}

XComboBox::~XComboBox()
{
}

void XComboBox::init()
{
  _data = new XComboBoxPrivate(this);

  setAllowNull(false);
  setSizeAdjustPolicy(AdjustToContents);
  connect(this, SIGNAL(activated(int)), this, SLOT(sHandleNewIndex(int)));
}

bool XComboBox::allowNull() const
{
  return _allowNull;
}

XComboBox::Defaults XComboBox::defaultCode() const
{
  return _data->_default;
}

QString XComboBox::fieldName() const
{
  return _data->_fieldName;
}

QLabel* XComboBox::label() const
{
  return _data->_label;
}

QString XComboBox::listDisplayFieldName() const
{
  return _data->_listDisplayFieldName;
}

QString XComboBox::listIdFieldName() const
{
  return _data->_listIdFieldName;
}

QString XComboBox::listSchemaName() const
{
  return _data->_listSchemaName;
}

QString XComboBox::listTableName() const
{
  return _data->_listTableName;
}

QString XComboBox::nullStr() const
{
  return _data->_nullStr;
}

void XComboBox::setDefaultCode(Defaults p)
{
  _data->_default = p;
}

void XComboBox::setFieldName(QString p)
{
  _data->_fieldName = p;
}

void XComboBox::setListDisplayFieldName(QString p)
{
  _data->_listDisplayFieldName = p;
}

void XComboBox::setListIdFieldName(QString p)
{
  _data->_listIdFieldName = p;
}

// exists only for script exposure
void XComboBox::removeItem(int idx)
{
  QComboBox::removeItem(idx);
}

enum XComboBox::XComboBoxTypes XComboBox::type()
{
  return _data->_type;
}

QString XComboBox::currentDefault()
{
  if (_data->_codes.count())
  {
    if (_data->_default == First)
      return _data->_codes.first();
    else
      return code();
  }
  else
    return QString("");
}

void XComboBox::setDataWidgetMap(XDataWidgetMapper* m)
{
  disconnect(this, SIGNAL(editTextChanged(QString)), this, SLOT(updateMapperData()));

  if (!_data->_listTableName.isEmpty())
  {
    QString tableName="";
    if (_data->_listSchemaName.length())
      tableName = _data->_listSchemaName + ".";
    tableName+= _data->_listTableName;
    static_cast<XSqlTableModel*>(m->model())->setRelation(static_cast<XSqlTableModel*>(m->model())->fieldIndex(_data->_fieldName),
                                 QSqlRelation(tableName, _data->_listIdFieldName, _data->_listDisplayFieldName));

    QSqlTableModel *rel =static_cast<XSqlTableModel*>(m->model())->relationModel(static_cast<XSqlTableModel*>(m->model())->fieldIndex(_data->_fieldName));
    setModel(rel);
    setModelColumn(rel->fieldIndex(_data->_listDisplayFieldName));

    m->setItemDelegate(new QSqlRelationalDelegate(this));
    m->addMapping(this, _data->_fieldName);
    return;
  }
  else if (_data->_codes.count())
    m->addMapping(this, _data->_fieldName, "code", "currentDefault");
  else
    m->addMapping(this, _data->_fieldName, "text", "text");

  _data->_mapper=m;
  connect(this, SIGNAL(editTextChanged(QString)), this, SLOT(updateMapperData()));
}

void XComboBox::setListSchemaName(QString p)
{
  if (_data->_listSchemaName == p)
    return;

  if (!p.isEmpty())
    setType(Adhoc);
  _data->_listSchemaName = p;
}

void XComboBox::setListTableName(QString p)
{
  if (_data->_listTableName == p)
    return;

  _data->_listTableName = p;
  if (!p.isEmpty())
    setType(Adhoc);
}

void XComboBox::setType(XComboBoxTypes pType)
{
  if (DEBUG)
    qDebug("%s::setType() entered with %d", qPrintable(objectName()), pType);
  if (pType != Adhoc)
  {
    setListSchemaName("");
    setListTableName("");
  }
  _data->setType(pType);

  if (_x_metrics == 0)
    return;

  // If we're in Designer, don't populate
  for (QObject *ancestor = this; ancestor; ancestor = ancestor->parent())
  {
    if (ancestor->inherits("xTupleDesigner"))
      return;
  }

  switch (pType)
  {
    case CRMAccounts:
    case Honorifics:
    case PoProjects:
    case ShipVias:
    case SoProjects:
    case UOMs:
    case WoProjects:
      setAllowNull(true);
      break;

    default:
      break;
  }

  if (_data->typeDescrip.contains(pType)) {     // allow for Adhoc
    populate(_data->_descrip->query);
  }

  switch (pType)
  {
    case PoProjects:
    case SoProjects:
    case WoProjects:
      setEnabled(count() > 1);
      break;

    case Currencies:
    case CurrenciesNotBase:
      if (count() <= 1)
      {
        hide();
        if (_data->_label)
          _data->_label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setLabel(QLabel* pLab)
{
  _data->_label = pLab;

  switch (_data->_type)
  {
    case Currencies:
    case CurrenciesNotBase:
      if (count() <= 1)
      {
        hide();
        if (_data->_label)
          _data->_label->hide();
      }
      break;

    default:
      break;
  }
}

void XComboBox::setCode(const QString &pString)
{
  if (DEBUG)
    qDebug("%s::setCode(%s) with _codes.count %d and _ids.count %d",
           qPrintable(objectName()), qPrintable(pString),
           _data->_codes.count(), _data->_ids.count());

  if (pString.isEmpty())
  {
    setId(-1);
    setItemText(0, pString);
  }
  else if (_data->_codes.count())
  {
    for (int counter = 0; counter < _data->_codes.count(); counter++)
    {
      if (_data->_codes.at(counter) == pString)
      {
        if (DEBUG)
          qDebug("%s::setCode(%s) found at %d with _ids.count %d & id %d",
                 qPrintable(objectName()), qPrintable(pString),
                 counter, _data->_ids.count(), id());

        if (_data->_ids.count() && id()!=_data->_ids.at(counter))
          setId(_data->_ids.at(counter));

        return;
      }
      else if (DEBUG)
        qDebug("%s::setCode(%s) not found (%s)",
               qPrintable(objectName()), qPrintable(pString),
               qPrintable(_data->_codes.at(counter)));
    }
  }
  else  // this is an ad-hoc combobox without a query behind it?
  {
    setCurrentIndex(findText(pString));
    if (DEBUG)
      qDebug("%s::setCode(%s) set current item to %d using findData()",
             qPrintable(objectName()), qPrintable(pString), currentIndex());
    if (_data->_ids.count() > currentIndex())
      setId(_data->_ids.at(currentIndex()));
    if (DEBUG)
      qDebug("%s::setCode(%s) current item is %d after setId",
             qPrintable(objectName()), qPrintable(pString), currentIndex());
  }

  if (isEditable())
  {
    setId(-1);
    setItemText(0, pString);
  }
}

void XComboBox::setId(int pTarget)
{
  // reports are a special case: they should really be stored by name, not id
  if (_data->_type == Reports)
  {
    XSqlQuery query;
    query.prepare("SELECT report_id"
                  "  FROM report"
                  " WHERE report_name IN (SELECT report_name"
                  "                         FROM report"
                  "                        WHERE report_id = :report_id);");
    query.bindValue(":report_id", pTarget);
    query.exec();
    while (query.next())
    {
      int id = query.value("report_id").toInt();
      for (int counter = 0; counter < count(); counter++)
      {
        if (_data->_ids.at(counter) == id)
        {
          if(this->id()!=id)
          {
            setCurrentIndex(counter);
            updateMapperData();
            emit newID(pTarget);
            emit valid(true);

            if (allowNull())
              emit notNull(true);
          }

          return;
        }
      }
    }
  }
  else
  {
    for (int counter = 0; counter < _data->_ids.count(); counter++)
    {
      if (_data->_ids.at(counter) == pTarget)
      {
        if(id()!=pTarget)
        {
          setCurrentIndex(counter);
          updateMapperData();
          emit newID(pTarget);
          emit valid(true);

          if (allowNull())
            emit notNull(true);
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
  if (pString == currentText())
    return;

  if (count())
  {
    for (int counter = ((allowNull()) ? 1 : 0); counter < count(); counter++)
    {
      if (itemText(counter) == pString)
      {
        setCurrentIndex(counter);
        updateMapperData();
        emit newID(id());
        return;
      }
    }
  }

  if (isEditable())
  {
    setId(-1);
    setItemText(0, pString);
  }
}

void XComboBox::setAllowNull(bool pAllowNull)
{
  if (DEBUG)
    qDebug("%s::setAllowNull(%d)", qPrintable(objectName()), pAllowNull);
  _allowNull = pAllowNull;
  if (pAllowNull)
  {
    append(-1, _data->_nullStr);
    setItemText(0, _data->_nullStr);
  }
}

void XComboBox::setNull()
{
  if (allowNull())
  {
    setCurrentIndex(0);
    updateMapperData();
    emit newID(-1);
    emit valid(false);
    emit notNull(false);
  }
}

void XComboBox::setNullStr(const QString& pNullStr)
{
  if (DEBUG)
    qDebug("%s::setNullStr(%s)", qPrintable(objectName()), qPrintable(pNullStr));
  _data->_nullStr = pNullStr;
  if (allowNull())
  {
    append(-1, _data->_nullStr);
    setItemText(0, pNullStr);
  }
}

void XComboBox::setText(const QVariant &pVariant)
{
  setText(pVariant.toString());
}

void XComboBox::clear()
{
  QComboBox::clear();

  if (_data->_ids.count())
    _data->_ids.clear();

  if (_data->_codes.count())
    _data->_codes.clear();

  if (allowNull())
    append(-1, _data->_nullStr);
}

// allow repopulating after the underlying contents have changed (e.g. #3698)
void XComboBox::populate()
{
  setType(_data->_type);
}

void XComboBox::populate(XSqlQuery pQuery, int pSelected)
{
  if (DEBUG)
    qDebug("%s::populate(%s, %d) entered",
           qPrintable(objectName()), qPrintable(pQuery.lastQuery()), pSelected);

  int selected = (pSelected >= 0) ? pSelected : id();
  clear();

  if (! pQuery.isActive())
    pQuery.exec();

  // strange if/loop construct lets multiple comboboxes share a query instance
  if (pQuery.first())
    do
    {
      if (pQuery.record().count() < 3)
        append(pQuery.value(0).toInt(), pQuery.value(1).toString());
      else
        append(pQuery.value(0).toInt(), pQuery.value(1).toString(), pQuery.value(2).toString());
    } while (pQuery.next());

  setId(selected);
}

void XComboBox::populate(const QString & pSql, int pSelected)
{
  XSqlQuery query(pSql);
  populate(query, pSelected);
}

void XComboBox::append(int pId, const QString &pText)
{
  append(pId,pText,pText);
}

void XComboBox::append(int pId, const QString &pText, const QString &pCode)
{
  if (DEBUG)
      qDebug("%s::append(%d, %s, %s)",
             qPrintable(objectName()), pId, qPrintable(pText), qPrintable(pCode));

  if (! _data->_ids.contains(pId))
  {
    addItem(pText);
    _data->_ids.append(pId);
    _data->_codes.append(pCode);
  }
}

int XComboBox::id(int pIndex) const
{
  if ((pIndex >= 0) && (pIndex < count()))
  {
    if ( (allowNull()) && (currentIndex() <= 0) )
      return -1;
    else if(pIndex < _data->_ids.count())
      return _data->_ids.at(pIndex);
  }
  return -1;
}

int XComboBox::id() const
{
  if (_data->_ids.count() && currentIndex() != -1)
  {
    if ( (allowNull()) && (currentIndex() <= 0) )
      return -1;
    else
      return _data->_ids.at(currentIndex());
  }
  else
    return -1;
}

QString XComboBox::code() const
{
  if (DEBUG)
    qDebug("%s::code() with currentIndex %d, allowNull %d, _codes.count %d",
           qPrintable(objectName()), currentIndex(), allowNull(),
           _data->_codes.count());

  QString returnValue;

  if ( allowNull() && (currentIndex() <= 0) )
    returnValue = QString::Null();
  else if (currentIndex() >= 0 && _data->_codes.count() > currentIndex())
    returnValue = _data->_codes.at(currentIndex());
  else if (currentIndex() >= 0)
    returnValue = currentText();
  else
    returnValue = QString::Null();

  if (DEBUG)
    qDebug("%s::code() returning %s",
           qPrintable(objectName()), qPrintable(returnValue));
  return returnValue;
}

bool XComboBox::isValid() const
{
  if ((allowNull()) && (id() == -1))
    return false;
  else
    return true;
}

void XComboBox::sHandleNewIndex(int pIndex)
{
  if (DEBUG)
    qDebug("%s::sHandleNewIndex(%d) entered", qPrintable(objectName()), pIndex);

  if (pIndex >= 0 && pIndex < _data->_ids.count())
  {
    updateMapperData();
    emit newID(_data->_ids.at(pIndex));

    if (DEBUG)
      qDebug("%s::shandleNewIndex() emitted newID(%d)",
             qPrintable(objectName()), _data->_ids.at(pIndex));

    if (allowNull())
    {
      emit valid((pIndex != 0));
      emit notNull((pIndex != 0));
    }
  }

  if (DEBUG)
    qDebug("%s::sHandleNewIndex() returning", qPrintable(objectName()));
}

void XComboBox::mousePressEvent(QMouseEvent *event)
{
  emit clicked();

  QComboBox::mousePressEvent(event);
}

void XComboBox::wheelEvent(QWheelEvent *event)
{
  if (_x_preferences)
    if (_x_preferences->boolean("DisableXComboBoxWheelEvent"))
      return;

  QComboBox::wheelEvent(event);
}

void XComboBox::showPopup()
{
  QComboBox::showPopup();
  QAbstractItemView *itemView = view();
  if (_data->_editButton && _data->_popupCounter == 0)
  {
    _data->_popupCounter++;
    itemView->setFixedHeight(itemView->height() + _data->_editButton->height() + 5);
  }
}

QSize XComboBox::sizeHint() const
{
  QSize s = QComboBox::sizeHint();
#ifdef Q_OS_MAC
  s.setWidth(s.width() + 12);
#endif
  return s;
}

void XComboBox::updateMapperData()
{
  QString val;
  if (_data->_codes.count())
    val = code();
  else
    val = currentText();

  if (_data->_mapper->model() &&
    _data->_mapper->model()->data(_data->_mapper->model()->index(_data->_mapper->currentIndex(),_data->_mapper->mappedSection(this))).toString() != val)
  _data->_mapper->model()->setData(_data->_mapper->model()->index(_data->_mapper->currentIndex(),_data->_mapper->mappedSection(this)), val);
}

void XComboBox::insertEditor(XComboBoxTypes type, const QString &uiName,
                             const QString &privilege)
{
  Q_UNUSED(type);
  _data->_uiName    = uiName;
  _data->_privilege = privilege;
  _data->addEditButton();
}

void XComboBox::insertEditor(XComboBoxTypes type, QObject *obj,
                             const char *slot, const QString &privilege)
{
  Q_UNUSED(type);
  _data->_uiName    = QString();
  _data->_privilege = privilege;
  _data->_editor    = qobject_cast<QWidget*>(obj);
  _data->_slot      = UNWRAPSLOT(slot);

  _data->addEditButton();
}

// scripting exposure /////////////////////////////////////////////////////////

QScriptValue XComboBoxDefaultsToScriptValue(QScriptEngine *engine, XComboBox::Defaults const &item)
{
  return QScriptValue(engine, (int)item);
}

void XComboBoxDefaultsFromScriptValue(const QScriptValue &obj, XComboBox::Defaults &item)
{
  item = (XComboBox::Defaults)(obj.toInt32());
}

QScriptValue XComboBoxTypesToScriptValue(QScriptEngine *engine, XComboBox::XComboBoxTypes const &item)
{
  return QScriptValue(engine, (int)item);
}

void XComboBoxTypesFromScriptValue(const QScriptValue &obj, XComboBox::XComboBoxTypes &item)
{
  item = (XComboBox::XComboBoxTypes)(obj.toInt32());
}

QScriptValue constructXComboBox(QScriptContext *context,
                                QScriptEngine  *engine)
{
#if QT_VERSION >= 0x050000
  XComboBox *cbox = 0;

  if (context->argumentCount() == 0)
    cbox = new XComboBox();

  else if (context->argumentCount() == 1 &&
           context->argument(0).isBool())
    cbox = new XComboBox(context->argument(0).toBool());
  else if (context->argumentCount() == 1 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)))
    cbox = new XComboBox(qscriptvalue_cast<QWidget*>(context->argument(0)));

  else if (context->argumentCount() == 2 &&
           context->argument(0).isBool() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)))
    cbox = new XComboBox(context->argument(0).toBool(),
                         qscriptvalue_cast<QWidget*>(context->argument(1)));
  else if (context->argumentCount() == 2 &&
           qscriptvalue_cast<QWidget*>(context->argument(0)) &&
           context->argument(1).isString())
    cbox = new XComboBox(qscriptvalue_cast<QWidget*>(context->argument(0)),
                         context->argument(1).toString().toLatin1().data());

  else if (context->argumentCount() >= 3  &&
           context->argument(0).isBool() &&
           qscriptvalue_cast<QWidget*>(context->argument(1)))
  {
    cbox = new XComboBox(context->argument(0).toBool(),
                         qscriptvalue_cast<QWidget*>(context->argument(1)),
                         context->argument(2).toString().toLatin1().data());
  }

  else
    context->throwError(QScriptContext::UnknownError,
                        QString("Could not find an appropriate XComboBox constructor"));

  return engine->toScriptValue(cbox);
#else
  Q_UNUSED(context); Q_UNUSED(engine); return QScriptValue();
#endif
}

void setupXComboBox(QScriptEngine *engine)
{
  if (! engine->globalObject().property("XComboBox").isFunction())
  {
#if QT_VERSION >= 0x050500
    qScriptRegisterMetaType(engine, XComboBoxTypesToScriptValue,    XComboBoxTypesFromScriptValue);
    qScriptRegisterMetaType(engine, XComboBoxDefaultsToScriptValue, XComboBoxDefaultsFromScriptValue);
#endif

    QScriptValue ctor = engine->newFunction(constructXComboBox);
    QScriptValue meta = engine->newQMetaObject(&XComboBox::staticMetaObject, ctor);

    engine->globalObject().setProperty("XComboBox", meta,
                                       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  }
}
