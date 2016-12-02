/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "apopencluster.h"

#define DEBUG false

#define EXTRACLAUSE " (apopen_open) "	                        \
          " AND ((apopen_amount - apopen_paid) > 0 ) "             

ApopenCluster::ApopenCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new ApopenLineEdit(this, pName));
}

ApopenLineEdit::DocTypes ApopenCluster::allowedDocTypes()  const
{
  return (qobject_cast<ApopenLineEdit*>(_number))->allowedDocTypes();
}

void ApopenLineEdit::setExtraClause(const QString &clause)
{
  _userExtraClause = QString(clause);
}

void ApopenCluster::setExtraClause(const QString &clause, const QString&)
{
  (qobject_cast<ApopenLineEdit*>(_number))->setExtraClause(clause);
}

ApopenLineEdit::DocType  ApopenCluster::type()             const
{
  return (qobject_cast<ApopenLineEdit*>(_number))->type();
}

QString  ApopenCluster::typeString()       const
{
  return (qobject_cast<ApopenLineEdit*>(_number))->typeString();
}

void ApopenCluster::setVendId(int pvendid)
{
  return (qobject_cast<ApopenLineEdit*>(_number))->setVendId(pvendid);
}

void ApopenCluster::setAllowedDocTypes(const ApopenLineEdit::DocTypes ptypes)
{
  return (qobject_cast<ApopenLineEdit*>(_number))->setAllowedDocTypes(ptypes);
}

ApopenLineEdit::ApopenLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "apopen", "apopen_id", "apopen_docnumber", 
          "formatmoney(apopen_amount-apopen_paid) "          
          , "apopen_doctype", 
          EXTRACLAUSE, pName)
{
  setAllowedDocTypes(AnyType);
  _vendId       = -1;
  _standardExtraClause = EXTRACLAUSE;

  if (DEBUG)
    qDebug("%s::ApopenLineEdit() _standardExtraClause = %s",
           qPrintable(objectName()), qPrintable(_standardExtraClause));

}

void ApopenLineEdit::setVendId(int pVend)
{
  _vendId = pVend;
  _vendClause = QString("apopen_vend_id=%1").arg(_vendId);
  (void)buildExtraClause();
}

ApopenLineEdit::DocTypes ApopenLineEdit::allowedDocTypes()  const
{
  return _allowedTypes;
}

void ApopenLineEdit::setAllowedDocTypes(DocTypes ptypes)
{
  QStringList typelist;

  _allowedTypes = ptypes;

  if (ptypes == AnyType)
    _typeClause = "";
  else
  {
    if (_allowedTypes & CreditMemo)     typelist << "'C'";
    if (_allowedTypes & DebitMemo)      typelist << "'D'";
    if (_allowedTypes & Voucher)        typelist << "'V'";

    _typeClause = "(apopen_doctype IN (" + typelist.join(", ") + "))" ;
  }

  switch (ptypes) 
  {
    case CreditMemo:  setTitles(tr("Credit Memo"),  tr("Credit Memos"));  break;
    case DebitMemo:   setTitles(tr("Debit Memo"),   tr("Debit Memos"));   break;
    case Voucher:     setTitles(tr("Voucher"),      tr("Voucher"));       break;

    default:          setTitles(tr("A/P Open Item"),tr("A/P Open Items"));break;
  }

  (void)buildExtraClause();
}

ApopenLineEdit::DocType ApopenLineEdit::type() const
{
  if ("C" == _description) return CreditMemo;
  if ("D" == _description) return DebitMemo;
  if ("V" == _description) return Voucher;

  return AnyType;
}

QString ApopenLineEdit::typeString() const
{
  if ("C" == _description) return tr("Credit Memo");
  if ("D" == _description) return tr("Debit Memo");
  if ("V" == _description) return tr("Voucher");

  return QString("A/P Open Item");
}

QString ApopenLineEdit::buildExtraClause()
{
  QStringList clauses;
  if (! _vendClause.isEmpty())          clauses << _vendClause;
  if (! _typeClause.isEmpty())          clauses << _typeClause;
  if (! _standardExtraClause.isEmpty()) clauses << _standardExtraClause;
  if (! _userExtraClause.isEmpty())     clauses << _userExtraClause;

  VirtualClusterLineEdit::setExtraClause(clauses.join(" AND "));

  if (DEBUG)
    qDebug("%s::buildExtraClause() returning %s",
           qPrintable(objectName()), qPrintable(_extraClause));

  return _extraClause;
}
