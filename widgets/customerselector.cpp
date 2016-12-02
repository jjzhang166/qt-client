/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <xsqlquery.h>
#include <parameter.h>
#include <metasql.h>
#include <errorReporter.h>

#include "customerselector.h"

CustomerSelector::CustomerSelector(QWidget *pParent, const char *pName) : QWidget(pParent)
{
  setupUi(this);

  if (pName)
    setObjectName(pName);

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  _allowedStates = 0;
  populate(AllCust | SelectedCust | SelectedGroup | SelectedType | TypePattern);

  _select->setCurrentIndex(AllCust);
  _cust->setType(CLineEdit::AllCustomers);
  _cust->setId(-1);
  _customerTypes->setId(-1);
  _customerGroup->setId(-1);
  _customerType->setText("");



  connect(_select,         SIGNAL(currentIndexChanged(int)), this, SIGNAL(updated()));
  connect(_select,         SIGNAL(currentIndexChanged(int)), this, SIGNAL(newState(int)));
  connect(_select,         SIGNAL(currentIndexChanged(int)), this, SLOT(setStackElement()));
  connect(_cust,           SIGNAL(newId(int)), this, SIGNAL(updated()));
  connect(_cust,           SIGNAL(newId(int)), this, SIGNAL(newCustId(int)));
  connect(_cust,           SIGNAL(valid(bool)), this, SIGNAL(validCust(bool)));
  connect(_customerTypes,  SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_customerTypes,  SIGNAL(newID(int)), this, SIGNAL(newCustTypeId(int)));
  connect(_customerType,   SIGNAL(editingFinished()), this, SIGNAL(updated()));
  connect(_customerType,   SIGNAL(editingFinished()), this, SLOT(sTypePatternFinished()));
  connect(_customerType,   SIGNAL(textChanged(QString)), this, SIGNAL(newTypePattern(QString)));
  connect(_customerGroup,  SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_customerGroup,  SIGNAL(newID(int)), this, SIGNAL(newCustGroupId(int)));
  connect(_customerGroup,  SIGNAL(valid(bool)), this, SIGNAL(validCustGroup(bool)));


  setFocusProxy(_select);
}

CustomerSelector::~CustomerSelector()
{
}

void CustomerSelector::languageChange()
{
  retranslateUi(this);
}

void CustomerSelector::appendValue(ParameterList &pParams)
{
  if (isSelectedCust())
    pParams.append("cust_id", _cust->id());
  if (isSelectedGroup())
    pParams.append("custgrp_id", _customerGroup->id());
  if (isSelectedType())
    pParams.append("custtype_id", _customerTypes->id());
  if (isTypePattern())
    pParams.append("custtype_pattern", _customerType->text());
}

void CustomerSelector::bindValue(XSqlQuery &pQuery)
{
  if (isSelectedCust())
    pQuery.bindValue(":cust_id", _cust->id());
  if (isSelectedGroup())
    pQuery.bindValue(":custgrp_id", _customerGroup->id());
  if (isSelectedType())
    pQuery.bindValue(":custtype_id", _customerTypes->id());
  if (isTypePattern())
    pQuery.bindValue(":custtype_pattern", _customerType->text());
}

bool CustomerSelector::isValid()
{
  if (isSelectedCust())
    return _cust->isValid();
  else if (isSelectedGroup())
    return _customerGroup->isValid();
  else if (isSelectedType())
    return _customerTypes->isValid();
  else if (isTypePattern())
    return ! _customerType->text().trimmed().isEmpty();
  else if (isAll())
    return true;

  return false;
}

void CustomerSelector::setCustId(int p)
{
  _cust->setId(p);
  setState(SelectedCust);
}

void CustomerSelector::setCustTypeId(int p)
{
  _customerTypes->setId(p);
  setState(SelectedType);
}

void CustomerSelector::setTypePattern(const QString &p)
{
  _customerType->setText(p);
  sTypePatternFinished();
  setState(TypePattern);
}

void CustomerSelector::setCustGroupId(int p)
{
  _customerGroup->setId(p);
  setState(SelectedGroup);
}

void CustomerSelector::setState(enum CustomerSelectorState p)
{
  if (p == AllCust)
    _select->setCode("A");
  else if (p == SelectedCust)
    _select->setCode("C");
  else if (p == SelectedGroup)
    _select->setCode("G");
  else if (p == SelectedType)
    _select->setCode("T");
  else if (p == TypePattern)
    _select->setCode("P");
  else
    _select->setCode("N");
}

void CustomerSelector::sTypePatternFinished()
{
  emit newTypePattern(_customerType->text());
  emit updated();
}

/*
   keep the contents of another customer group (p) synchronized with this in a
   manner which allows any changes on this to propagate to p and any objects
   listening for signals from p will respond to changes to this.
   hide p because it's superfluous.
*/
void CustomerSelector::synchronize(CustomerSelector *p)
{
  connect(this, SIGNAL(newTypePattern(QString)), p, SLOT(setTypePattern(QString)));
  connect(this, SIGNAL(newState(int)),           p, SLOT(setState(int)));
  connect(this, SIGNAL(newCustId(int)),          p, SLOT(setCustId(int)));
  connect(this, SIGNAL(newCustTypeId(int)),      p, SLOT(setCustTypeId(int)));
  connect(this, SIGNAL(newCustGroupId(int)),     p, SLOT(setCustGroupId(int)));

  p->hide();
}

void CustomerSelector::populate(int selection)
{
  QString sql = "SELECT -1 AS selkey, '[Please Select]', 'N'"
                "<? if exists('allcust') ?>"
                " UNION "
                "SELECT 1 AS selkey, 'All Customers' AS selname, 'A' AS selcode"
                "<? endif ?>"
                "<? if exists('cust') ?>"
                " UNION "
                "SELECT 2 AS selkey, 'Customer' as selname, 'C' as selcode"
                "<? endif ?>"
                "<? if exists('custgrp') ?>"
                " UNION "
                "SELECT 3 AS selkey, 'Customer Group' as selname, 'G' as selcode"
                "<? endif ?>"
                "<? if exists('custtype') ?>"
                " UNION "
                "SELECT 4 AS selkey, 'Customer Type' AS selname, 'T' as selcode"
                "<? endif ?>"
                "<? if exists('typepattern') ?>"
                " UNION "
                "SELECT 5 AS selkey, 'Customer Type Pattern' AS selname, 'P' as selcode"
                "<? endif ?>"
                " ORDER BY selkey;";
  ParameterList params;
  MetaSQLQuery mql(sql);

  if (isAllowedType(selection))
  {
    if (selection & AllCust) params.append("allcust");
    if (selection & SelectedCust)  params.append("cust");
    if (selection & SelectedGroup) params.append("custgrp");
    if (selection & SelectedType) params.append("custtype");
    if (selection & TypePattern) params.append("typepattern");
    _allowedStates = selection;
  }
  else
    ErrorReporter::error(QtCriticalMsg, this, tr("Invalid type"), QString::number(selection), __FILE__, __LINE__);
  XSqlQuery q = mql.toQuery(params);
  _select->populate(q);
}

void CustomerSelector::setStackElement()
{
  if (isSelectedCust())
    _selectStack->setCurrentIndex(1);
  else if (isSelectedGroup())
    _selectStack->setCurrentIndex(2);
  else if (isSelectedType())
    _selectStack->setCurrentIndex(3);
  else if (isTypePattern())
    _selectStack->setCurrentIndex(4);
  else
    _selectStack->setCurrentIndex(0);
}

QString CustomerSelector::selectCode()
{
  return _select->code();
}

bool CustomerSelector::isAllowedType(const int s)
{
  return ((s & (AllCust | SelectedCust | SelectedGroup | SelectedType | TypePattern)) ||
          (s & (          SelectedCust | SelectedGroup  )));
}
