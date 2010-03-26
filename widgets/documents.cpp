/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDesktopServices>
#include <QMessageBox>
#include <QDebug>
#include <QDialog>
#include <QUrl>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>
#include "mqlutil.h"

#include "documents.h"
#include "imageAssignment.h"
#include "docAttach.h"

// CAUTION: This will break if the order of this list does not match
//          the order of the enumerated values as defined.
const Documents::DocumentMap Documents::_documentMap[] =
{
  DocumentMap( Uninitialized,     " "   ),
  DocumentMap( Address,           "ADDR"),
  DocumentMap( BBOMHead,          "BBH" ),
  DocumentMap( BBOMItem,          "BBI" ),
  DocumentMap( BOMHead,           "BMH" ),
  DocumentMap( BOMItem,           "BMI" ),
  DocumentMap( BOOHead,           "BOH" ),
  DocumentMap( BOOItem,           "BOI" ),
  DocumentMap( CRMAccount,        "CRMA"),
  DocumentMap( Contact,           "T"   ),
  DocumentMap( Customer,          "C"   ),
  DocumentMap( Employee,          "EMP" ),
  DocumentMap( Incident,          "INCDT"),
  DocumentMap( Item,              "I"   ),
  DocumentMap( ItemSite,          "IS"  ),
  DocumentMap( ItemSource,        "IR"  ),
  DocumentMap( Location,          "L"   ),
  DocumentMap( LotSerial,         "LS"   ),
  DocumentMap( Opportunity,       "OPP" ),
  DocumentMap( Project,           "J"   ),
  DocumentMap( PurchaseOrder,     "P"   ),
  DocumentMap( PurchaseOrderItem, "PI"  ),
  DocumentMap( ReturnAuth,        "RA"  ),
  DocumentMap( ReturnAuthItem,    "RI"  ),
  DocumentMap( Quote,             "Q"   ),
  DocumentMap( QuoteItem,         "QI"  ),
  DocumentMap( SalesOrder,        "S"   ),
  DocumentMap( SalesOrderItem,    "SI"  ),
  DocumentMap( Todo,              "TODO"),
  DocumentMap( TransferOrder,     "TO"  ),
  DocumentMap( TransferOrderItem, "TI"  ),
  DocumentMap( Vendor,            "V"   ),
  DocumentMap( Warehouse,         "WH"  ),
  DocumentMap( WorkOrder,         "W"   ),
};

GuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "target_type" );
  _doc->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "target_number" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  _itemColumn, Qt::AlignLeft,true, "purpose");
  
  connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));
  connect(_doc, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(sEditDoc()));

  if (_x_privileges)
  {
    QMenu * newDocMenu = new QMenu;

    QAction* imgAct = new QAction(tr("Image"), this);
    imgAct->setEnabled(_x_privileges->check("MaintainImages"));
    connect(imgAct, SIGNAL(triggered()), this, SLOT(sNewImage()));
    newDocMenu->addAction(imgAct);

    QAction* incdtAct = new QAction(tr("Incident"), this);
    incdtAct->setEnabled(_x_privileges->check("AddIncidents"));
    connect(incdtAct, SIGNAL(triggered()), this, SLOT(sNewIncdt()));
    newDocMenu->addAction(incdtAct);

    QAction* todoAct = new QAction(tr("To Do"), this);
    todoAct->setEnabled(_x_privileges->check("MaintainPersonalTodoList") ||
                        _x_privileges->check("MaintainOtherTodoLists"));
    connect(todoAct, SIGNAL(triggered()), this, SLOT(sNewToDo()));
    newDocMenu->addAction(todoAct);

    QAction* oppAct = new QAction(tr("Opportunity"), this);
    oppAct->setEnabled(_x_privileges->check("MaintainOpportunities"));
    connect(oppAct, SIGNAL(triggered()), this, SLOT(sNewOpp()));
    newDocMenu->addAction(oppAct);

    QAction* projAct = new QAction(tr("Project"), this);
    projAct->setEnabled(_x_privileges->check("MaintainProjects"));
    connect(projAct, SIGNAL(triggered()), this, SLOT(sNewProj()));
    newDocMenu->addAction(projAct);

    _newDoc->setMenu(newDocMenu);
  }
}

void Documents::setType(enum DocumentSources pSource)
{
  _source = pSource;
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{

  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _viewDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
  disconnect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  if(!pReadOnly)
  {
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
    connect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  }
}

void Documents::sNewDoc(QString type, QString ui)
{
  ParameterList params;
  params.append("mode", "new");
  int target_id;
  QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openWindow(ui, params, parentWidget(),Qt::WindowModal, Qt::Dialog));
  target_id = newdlg->exec();
  if (target_id != QDialog::Rejected)
    sInsertDocass(type, target_id);
  refresh();
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sNewIncdt()
{
  sNewDoc("INCDT","incident");
}

void Documents::sNewToDo()
{
  sNewDoc("TODO", "todoItem");
}

void Documents::sNewProj()
{
  sNewDoc("J", "project");
}

void Documents::sNewOpp()
{
  sNewDoc("OPP","opportunity");
}

void Documents::sInsertDocass(QString target_type, int target_id)
{
  XSqlQuery q;
  q.prepare("INSERT INTO docass ( docass_source_id, docass_source_type, docass_target_id, docass_target_type )"
            "  VALUES ( :sourceid, :sourcetype::text, :targetid, :targettype); ");
  q.bindValue(":sourceid", _sourceid);
  q.bindValue(":sourcetype", _documentMap[_source].ident);
  q.bindValue(":targetid", target_id);
  q.bindValue(":targettype", target_type);
  q.exec();
}

void Documents::sEditDoc()
{
  sOpenDoc("edit");
}

void Documents::sOpenDoc(QString mode)
{
  QString ui;
  QString docType = _doc->currentItem()->rawValue("target_type").toString();
  int targetid = _doc->currentItem()->id("target_number");
  ParameterList params;
  params.append("mode", mode);

  //image -- In the future this needs to be changed to use docass instead of imageass
  if (docType == "IMG")
  {
    params.append("imageass_id", _doc->id());
    imageAssignment newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
    refresh();
    return;
  }
  //url -- In the future this needs to be changed to use docass instead of url
  else if (docType == "URL")
  {
   //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(_doc->currentItem()->rawValue("description").toString());
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
    return;
  }
  else if (docType == "INCDT")
  {
    params.append("incdt_id", targetid);
    ui = "incident";
  }
  else if (docType == "TODO")
  {
    params.append("todoitem_id", targetid);
    ui = "todoItem";
  }
  else if (docType == "OPP")
  {
    params.append("ophead_id", targetid);
    ui = "opportunity";
  }
  else if (docType == "J")
  {
    params.append("prj_id", targetid);
    ui = "project";
  }
  else if (docType == "CRMA")
  {
    params.append("crmacct_id", targetid);
    ui = "crmaccount";
  }
  else if (docType == "T")
  {
    params.append("cntct_id", targetid);
  }
  else if (docType == "V")
  {
    params.append("vend_id", targetid);
    ui = "vendor";
  }
  else if (docType == "I")
  {
    params.append("item_id", targetid);
    ui = "item";
  }
  else if (docType == "S")
  {
    params.append("sohead_id", targetid);
    ui = "salesOrder";
  }
  else if (docType == "P")
  {
    params.append("pohead_id", targetid);
    ui = "purchaseOrder";
  }
  else if (docType == "W")
  {
    params.append("wo_id", targetid);
    ui = "workOrder";
  }
  else if (docType == "EMP")
  {
    params.append("emp_id", targetid);
    ui = "employee";
  }
  else if (docType == "C")
  {
    params.append("cust_id", targetid);
    ui = "customer";
  }
  else
  {
    QMessageBox::critical(this, tr("Error"),
                          tr("Unknown File Type."));
    return;
  }

  QWidget* w = 0;
  if (parentWidget()->window())
  {
    if (parentWidget()->window()->isModal())
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);
    else
      w = _guiClientInterface->openWindow(ui, params, parentWidget()->window() , Qt::NonModal, Qt::Window);
  }

  if (w->inherits("QDialog"))
  {
    QDialog* newdlg = qobject_cast<QDialog*>(w);
    newdlg->exec();
  }

  refresh();
}

void Documents::sViewDoc()
{
  sOpenDoc("view");
}

void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  docAttach newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  refresh();
}

void Documents::sDetachDoc()
{
  XSqlQuery q;
	if (_doc->id() < 0)
		return;

	if ( _doc->currentItem()->rawValue("target_type") == "IMG" )
	{
	  q.prepare( "DELETE FROM imageass "
	             "WHERE (imageass_id = :docid );" );
	}
	else if ( _doc->currentItem()->rawValue("target_type") == "URL"  )
	{
	  q.prepare( "DELETE FROM url "
	             "WHERE (url_id = :docid );" );
	}
	else
	{
	  q.prepare( "DELETE FROM docass "
	             "WHERE (docass_id = :docid );" );
	}
	q.bindValue(":docid", _doc->id());
	q.exec();
	refresh();
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _doc->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate doc list
  QString sql("SELECT id, target_number, target_type, "
              " target_id AS target_number_xtidrole, source_type, source_id, purpose, "
              " name, description, "
              " CASE WHEN (purpose='I') THEN :inventory"
              " WHEN (purpose='P') THEN :product"
              " WHEN (purpose='E') THEN :engineering"
              " WHEN (purpose='M') THEN :misc"
              " WHEN (purpose='A') THEN :child"
              " WHEN (purpose='C') THEN :parent"
              " WHEN (purpose='S') THEN :sibling"
              " WHEN (purpose='D') THEN :dupe"
              " ELSE :other"
              " END AS purpose_qtdisplayrole, "
              " CASE WHEN (target_type='T') THEN :contact "
              " WHEN (target_type='CRMA') THEN :crma "
              " WHEN (target_type='C') THEN :cust "
              " WHEN (target_type='EMP') THEN :emp "
              " WHEN (target_type='URL') THEN :url "
              " WHEN (target_type='IMG') THEN :image "
              " WHEN (target_type='INCDT') THEN :incident "
              " WHEN (target_type='I') THEN :item "
              " WHEN (target_type='OPP') THEN :opp "
              " WHEN (target_type='J') THEN :project "
              " WHEN (target_type='P') THEN :po "
              " WHEN (target_type='S') THEN :so "
              " WHEN (target_type='V') THEN :vendor "
              " WHEN (target_type='W') THEN :wo "
              " WHEN (target_type='TODO') THEN :todo "
              " ELSE :error "
              " END AS target_type_qtdisplayrole "
              "FROM docinfo "
              "WHERE ((source_type=:source) "
              " AND (source_id=:sourceid)) "
              "ORDER by target_type_qtdisplayrole, target_number; ");
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));

  query.bindValue(":parent", tr("Parent"));
  query.bindValue(":child", tr("Child"));
  query.bindValue(":sibling", tr("Related to"));
  query.bindValue(":dupe", tr("Duplicate of"));
  query.bindValue(":other", tr("Other"));

  query.bindValue(":po", tr("Purchase Order"));
  query.bindValue(":so", tr("Sales Order"));
  query.bindValue(":wo", tr("Work Order"));
  query.bindValue(":image", tr("Image"));
  query.bindValue(":incident", tr("Incident"));
  query.bindValue(":todo", tr("To-Do"));
  query.bindValue(":task", tr("Task"));
  query.bindValue(":project", tr("Project"));
  query.bindValue(":item", tr("Item"));
  query.bindValue(":crma", tr("CRM Account"));
  query.bindValue(":cust", tr("Customer"));
  query.bindValue(":vendor", tr("Vendor"));
  query.bindValue(":contact", tr("Contact"));
  query.bindValue(":opp", tr("Opportunity"));
  query.bindValue(":url", tr("URL"));
  query.bindValue(":emp", tr("Employee"));

  query.bindValue(":source", _documentMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _doc->populate(query,TRUE);
}



