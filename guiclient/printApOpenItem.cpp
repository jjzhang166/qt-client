/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printApOpenItem.h"

#include <QMessageBox>
#include <QVariant>

printApOpenItem::printApOpenItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());

  _docinfoQueryString = "SELECT apopen_id        AS docid, apopen_id,"
                        "       apopen_docnumber AS docnumber,"
                        "       false            AS printed,"
                        "       'APOpenItem'     AS reportname,"
                        "       apopen_doctype"
                        "  FROM apopen"
                        " WHERE (apopen_id=<? value('docid') ?>);" ;
  setReportKey("apopen_id");

  connect(_apopen,        SIGNAL(valid(bool)), this, SLOT(setPrintEnabled(bool)));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printApOpenItem::~printApOpenItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void printApOpenItem::languageChange()
{
  retranslateUi(this);
}

// standard document type abbreviation, not the apopen-specific abbreviation
QString printApOpenItem::doctype()
{
  switch (_apopen->type())
  {
    case ApopenLineEdit::Voucher:       return "VO";
    case ApopenLineEdit::DebitMemo:     return "DM";
    case ApopenLineEdit::CreditMemo:    return "CM";
    default:
      qWarning("printApOpenItem::doctype() unknown for %d", _apopen->type());
      return "";
  }
}

ParameterList printApOpenItem::getParams(XSqlQuery *docq)
{
  ParameterList params;
  params.append("apopen_id", docq->value("apopen_id").toInt());
  QString _type = docq->value("apopen_doctype").toString();
  if (_type == "V" || _type == "D")
  {
    params.append("docTypeID", 1);
    params.append("creditMemo",       tr("Credit Memo"));
    params.append("other",            tr("Other"));
  }  
  else if (_type == "C")
  {
    params.append("docTypeCM",   1);
    params.append("voucher",     tr("Voucher"));
    params.append("debitMemo",   tr("Debit Memo"));
    params.append("apcheck",     tr("A/P Check"));
  }
  else
  {
    params.append("checkParamsReturn", false);
  }

  return params;
}

void printApOpenItem::sPopulate(XSqlQuery *qry)
{
  _apopen->setId(qry->value("docid").toInt());
}
