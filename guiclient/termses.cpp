/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "termses.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "terms.h"
#include "errorReporter.h"

termses::termses(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_terms, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  
  _terms->addColumn(tr("Code"),_itemColumn, Qt::AlignLeft,  true, "terms_code");
  _terms->addColumn(tr("Description"),  -1, Qt::AlignLeft,  true, "terms_descrip");
  _terms->addColumn(tr("Type"),_dateColumn, Qt::AlignCenter,true, "type");
  _terms->addColumn(tr("A/P"),   _ynColumn, Qt::AlignCenter,true, "terms_ap");
  _terms->addColumn(tr("A/R"),   _ynColumn, Qt::AlignCenter,true, "terms_ar");

  if (_privileges->check("MaintainTerms"))
  {
    connect(_terms, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_terms, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_terms, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(false);
    connect(_terms, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

termses::~termses()
{
    // no need to delete child widgets, Qt does it all for us
}

void termses::languageChange()
{
    retranslateUi(this);
}

void termses::sFillList()
{
  XSqlQuery termsesFillList;
  termsesFillList.prepare( "SELECT terms_id, terms_code, terms_descrip,"
             "       CASE WHEN (terms_type = 'D') THEN :days"
             "            WHEN (terms_type = 'P') THEN :proximo"
             "            ELSE '?'"
             "       END AS type,"
             "       terms_ap, terms_ar "
             "FROM terms "
             "ORDER BY terms_code;" );
  termsesFillList.bindValue(":days", tr("Days"));
  termsesFillList.bindValue(":proximo", tr("Proximo"));
  termsesFillList.exec();
  _terms->populate(termsesFillList);
}

void termses::sDelete()
{
  XSqlQuery termsesDelete;
  termsesDelete.prepare( "SELECT cust_id "
             "FROM custinfo "
             "WHERE (cust_terms_id=:terms_id);" );
  termsesDelete.bindValue(":terms_id", _terms->id());
  termsesDelete.exec();
  if (termsesDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Terms Code"),
                           tr("<p>You may not delete the selected Terms Code "
                              "are there are one or more Customers assigned to "
                              "it. You must reassign these Customers before "
                              "you may delete the selected Terms Code." ) );
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Customer Information"),
                                termsesDelete, __FILE__, __LINE__))
  {
    return;
  }

  termsesDelete.prepare( "SELECT vend_id "
             "FROM vendinfo "
             "WHERE (vend_terms_id=:terms_id);" );
  termsesDelete.bindValue(":terms_id", _terms->id());
  termsesDelete.exec();
  if (termsesDelete.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Terms Code"),
                           tr("<p>You may not delete the selected Terms Code "
                              "as there are one or more Vendors assigned to "
                              "it. You must reassign these Vendors before you "
                              "may delete the selected Terms Code." ) );
    return;
  }
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Vendor Information"),
                                termsesDelete, __FILE__, __LINE__))
  {
    return;
  }

  termsesDelete.prepare( "DELETE FROM terms "
             "WHERE (terms_id=:terms_id);" );
  termsesDelete.bindValue(":terms_id", _terms->id());
  termsesDelete.exec();

  sFillList();
}

void termses::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  terms newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void termses::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("terms_id", _terms->id());

  terms newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void termses::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("terms_id", _terms->id());

  terms newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void termses::sPopulateMenu( QMenu * )
{
}

void termses::sPrint()
{
  orReport report("TermsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
