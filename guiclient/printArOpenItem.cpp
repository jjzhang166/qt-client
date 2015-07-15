/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printArOpenItem.h"

#include <QMessageBox>
#include <QVariant>

printArOpenItem::printArOpenItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  _docinfoQueryString = "SELECT aropen_id        AS docid, aropen_id,"
                        "       aropen_docnumber AS docnumber,"
                        "       false            AS printed,"
                        "       'AROpenItem'     AS reportname,"
                        "       aropen_doctype"
                        "  FROM aropen"
                        " WHERE (aropen_id=<? value('docid') ?>);" ;
  setReportKey("aropen_id");

  connect(_aropen,        SIGNAL(valid(bool)), this, SLOT(setPrintEnabled(bool)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printArOpenItem::~printArOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void printArOpenItem::languageChange()
{
  retranslateUi(this);
}

// standard document type abbreviation, not the aropen-specific abbreviation
QString printArOpenItem::doctype()
{
  switch (_aropen->type())
  {
    case AropenLineEdit::Invoice:       return "IN";
    case AropenLineEdit::DebitMemo:     return "DM";
    case AropenLineEdit::CreditMemo:    return "CM";
    case AropenLineEdit::CashDeposit:   return "CR";
    default:
      qWarning("printArOpenItem::doctype() unknown for %d", _aropen->type());
      return "";
  }
}

ParameterList printArOpenItem::getParams(XSqlQuery *docq)
{
  ParameterList params;
  params.append("aropen_id", docq->value("aropen_id").toInt());
  QString _type = docq->value("aropen_doctype").toString();
  if (_type == "I" || _type == "D")
  {
    params.append("docTypeID", 1);
    params.append("creditMemo",       tr("Credit Memo"));
    params.append("cashdeposit",      tr("Cash Deposit"));
    params.append("check",            tr("Check"));
    params.append("certifiedCheck",   tr("Certified Check"));
    params.append("masterCard",       tr("Master Card"));
    params.append("visa",             tr("Visa"));
    params.append("americanExpress",  tr("American Express"));
    params.append("discoverCard",     tr("Discover Card"));
    params.append("otherCreditCard",  tr("Other Credit Card"));
    params.append("cash",             tr("Cash"));
    params.append("wireTransfer",     tr("Wire Transfer"));
    params.append("other",            tr("Other"));
  }  
  else if (_type == "C" || _type == "R")
  {
    params.append("docTypeRC",   1);
    params.append("invoice",     tr("Invoice"));
    params.append("debitMemo",   tr("Debit Memo"));
    params.append("apcheck",     tr("A/P Check"));
    params.append("cashreceipt", tr("Cash Receipt"));
  }
  else
  {
    params.append("checkParamsReturn", false);
  }

  return params;
}

void printArOpenItem::sPopulate(XSqlQuery *qry)
{
  _aropen->setId(qry->value("docid").toInt());
}
