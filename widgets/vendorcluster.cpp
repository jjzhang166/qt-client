/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSqlError>
#include <QVBoxLayout>

#include <metasql.h>
#include <parameter.h>
#include <xsqlquery.h>

#include "vendorcluster.h"
#include "format.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

VendorLineEdit::VendorLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "vendinfo", "vend_id", "vend_number", "vend_name", 0, 0, pName, "vend_active")
{
  _crmacctId = -1;
  _type     = AllVendors;
  _subtype  = CRMAcctLineEdit::Vend;
  _canEdit  = false;
  _editMode = false;

  setTitles(tr("Vendor"), tr("Vendors"));
  setUiName("vendor");
  setEditPriv("MaintainVendors");
  setViewPriv("ViewVendors");
  setNewPriv("MaintainVendors");

  _query = "SELECT vend_id AS id, vend_number AS number, vend_name AS name,"
           "       vend_active AS active, vendtype_code AS type,"
           "       cntct_id, cntct_honorific, cntct_first_name,"
           "       cntct_middle, cntct_last_name, cntct_suffix,"
           "       cntct_phone, cntct_title, cntct_fax, cntct_email,"
           "       addr_id, addr_line1, addr_line2, addr_line3,"
           "       addr_city, addr_state, addr_postalcode, addr_country,"
           "       formatAddr(addr_line1, addr_line2, addr_line3, '', '') AS street"
           "  FROM vendinfo JOIN vendtype ON (vend_vendtype_id=vendtype_id)"
           "                LEFT OUTER JOIN cntct ON (vend_cntct1_id=cntct_id)"
           "                LEFT OUTER JOIN addr ON (vend_addr_id=addr_id)"
           " WHERE (true) ";

  _modeSep = 0;
  _modeAct = new QAction(tr("Edit Number"), this);
  _modeAct->setToolTip(tr("Sets number for editing"));
  _modeAct->setCheckable(true);
  connect(_modeAct, SIGNAL(triggered(bool)), this, SLOT(setEditMode(bool)));
}

void VendorLineEdit::sNew()
{
  QString uiName="vendor";
  ParameterList params;
  params.append("mode", "new");
  sOpenWindow(uiName, params);
}

void VendorLineEdit::setId(int pId)
{
  VirtualClusterLineEdit::setId(pId);
  if (model() && _id != -1)
  {
    setUiName("vendor");
    setEditPriv("MaintainVendors");
    setViewPriv("ViewVendors");
    setNewPriv("MaintainVendors");
    _idColName="vend_id";
    sUpdateMenu();

    _crmacctId = model()->data(model()->index(0,CRMACCT_ID)).toInt();

    emit newCrmacctId(_crmacctId);

  }

  if (_editMode)
    _menuLabel->setPixmap(QPixmap(":/widgets/images/edit.png"));
  else
    _menuLabel->setPixmap(QPixmap(":/widgets/images/magnifier.png"));
}

void VendorLineEdit::setType(VendorLineEditTypes pType)
{
  _type = pType;
  QStringList list;
  switch (_type)
  {
  case ActiveVendors:
    list.append("active");
    // fall-through
  case AllVendors:
    list.append("isvendor");
    _subtype = CRMAcctLineEdit::Vend;
    break;
  }
  list.removeDuplicates();
  setExtraClause(list.join(" AND "));
}

VirtualList* VendorLineEdit::listFactory()
{
  CRMAcctList* list = new CRMAcctList(this);
  list->setSubtype(_subtype);
  return list;
}

VirtualSearch* VendorLineEdit::searchFactory()
{
  CRMAcctSearch* search = new CRMAcctSearch(this);
  search->setSubtype(_subtype);
  return search;
}

bool VendorLineEdit::canEdit()
{
  return _canEdit;
}

void VendorLineEdit::setCanEdit(bool p)
{
  if (p == _canEdit || !_x_metrics)
    return;

  if (p)
  {
    if (_x_privileges && _subtype == CRMAcctLineEdit::Vend)
      _canEdit = _x_privileges->check("MaintainVendors");
  }
  else
    _canEdit=p;

  if (!_canEdit)
    setEditMode(false);

  sUpdateMenu();
}

bool VendorLineEdit::editMode()
{
  return _editMode;
}

bool VendorLineEdit::setEditMode(bool p)
{
  if (p == _editMode)
    return p;

  if (!_canEdit)
    return false;

  _editMode=p;
  _modeAct->setChecked(p);

  if (_x_preferences)
  {
    if (!_x_preferences->boolean("ClusterButtons"))
    {
      if (_editMode)
        _menuLabel->setPixmap(QPixmap(":/widgets/images/edit.png"));
      else
        _menuLabel->setPixmap(QPixmap(":/widgets/images/magnifier.png"));
    }

    if (!_x_metrics->boolean("DisableAutoComplete") && _editMode)
      disconnect(this, SIGNAL(textEdited(QString)), this, SLOT(sHandleCompleter()));
    else if (!_x_metrics->boolean("DisableAutoComplete"))
      connect(this, SIGNAL(textEdited(QString)), this, SLOT(sHandleCompleter()));
  }
  sUpdateMenu();

  setDisabled(_editMode &&
              _x_metrics->value("CRMAccountNumberGeneration") == "A");

 if (!_editMode)
   selectAll();

  emit editable(p);
  return p;
}

void VendorLineEdit::sParse()
{
  if (_editMode)
    return;
  
  VirtualClusterLineEdit::sParse();
}

void VendorLineEdit::sUpdateMenu()
{
  VirtualClusterLineEdit::sUpdateMenu();
  if (_x_preferences)
  {
    if (_x_preferences->boolean("ClusterButtons"))
      return;
  }
  else
    return;

  if (_canEdit)
  {
    if (!menu()->actions().contains(_modeAct))
    {
      _infoAct->setVisible(false);
      menu()->addAction(_modeAct);
    }

    _listAct->setDisabled(_editMode);
    _searchAct->setDisabled(_editMode);
    _listAct->setVisible(!_editMode);
    _searchAct->setVisible(!_editMode);
  }
  else
  {
    if (menu()->actions().contains(_modeAct))
    {
      _infoAct->setVisible(true);
      menu()->removeAction(_modeAct);
    }
  }

  // Handle New
  bool canNew = false;

  if (_subtype == CRMAcctLineEdit::Vend)
    canNew = (_x_privileges->check("MaintainVendors"));

  _newAct->setEnabled(canNew && isEnabled());
}

bool VendorLineEdit::canOpen()
{
  return VirtualClusterLineEdit::canOpen() && !canEdit();
}


//////////////////////////////////////////////////////////////

VendorCluster::VendorCluster(QWidget *pParent, const char *pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new VendorLineEdit(this, pName));

  VendorLineEdit* number = static_cast<VendorLineEdit*>(_number);
  connect(number, SIGNAL(newCrmacctId(int)), this, SIGNAL(newCrmacctId(int)));
  connect(number, SIGNAL(editable(bool)), this, SIGNAL(editable(bool)));
  connect(number, SIGNAL(editable(bool)), this, SLOT(sHandleEditMode(bool)));

  setLabel(tr("Vendor #:"));
  setNameVisible(true);
  setDescriptionVisible(true);
}

void VendorCluster::setType(VendorLineEdit::VendorLineEditTypes pType)
{
  static_cast<VendorLineEdit*>(_number)->setType(pType);
}

bool VendorCluster::setEditMode(bool p) const
{
  return static_cast<VendorLineEdit*>(_number)->setEditMode(p);
}

void VendorCluster::sHandleEditMode(bool p)
{
  VendorLineEdit* number = static_cast<VendorLineEdit*>(_number);

  if (p)
    connect(number, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
  else
    disconnect(number, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}
