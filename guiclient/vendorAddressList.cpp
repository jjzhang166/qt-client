/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendorAddressList.h"

#include <QVariant>

vendorAddressList::vendorAddressList(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_vendaddr, SIGNAL(itemSelected(int)), _select, SLOT(animateClick()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));

  _vendaddr->addColumn(tr("Code"),    _orderColumn, Qt::AlignLeft,   true,  "code" );
  _vendaddr->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true,  "name" );
  _vendaddr->addColumn(tr("Address"), 100,          Qt::AlignLeft,   true,  "address" );
}

vendorAddressList::~vendorAddressList()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorAddressList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorAddressList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  XSqlQuery setVendor;
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vendid = param.toInt();
    setVendor.prepare("SELECT (vend_number||' - '||vend_name) AS f_name,"
              "       addr_line1"
              "  FROM vendinfo"
              "  LEFT OUTER JOIN addr ON (vend_addr_id=addr_id)"
              " WHERE(vend_id=:vend_id);");
    setVendor.bindValue(":vend_id", _vendid);
    setVendor.exec();
    if(setVendor.first())
    {
      _vendName->setText(setVendor.value("f_name").toString());
      _vendAddr1->setText(setVendor.value("addr_line1").toString());
    }
  }

  sFillList();

  return NoError;
}

void vendorAddressList::sSelect()
{
  done(_vendaddr->id());
}

void vendorAddressList::sClose()
{
  done(_vendaddrid);
}

void vendorAddressList::sFillList()
{
  XSqlQuery vendorFillList;
  vendorFillList.prepare( "SELECT -1 AS id, 'Main' AS code, vend_name AS name, addr_line1 AS address,"
             "       0 AS orderby "
             "FROM vendinfo "
             "LEFT OUTER JOIN addr ON (vend_addr_id=addr_id) "
             "WHERE (vend_id=:vend_id) "
             "UNION "
             "SELECT vendaddr_id AS id, vendaddr_code AS code, vendaddr_name AS name, addr_line1 AS address,"
             "       1 AS orderby "
             "FROM vendaddrinfo "
             "LEFT OUTER JOIN addr ON (vendaddr_addr_id=addr_id) "
             "WHERE (vendaddr_vend_id=:vend_id) "
             "ORDER BY orderby, code;" );
  vendorFillList.bindValue(":vend_id", _vendid);
  vendorFillList.exec();
  _vendaddr->populate(vendorFillList, _vendid);
}

