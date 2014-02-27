/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reschedulePoitem.h"

#include <QMessageBox>
#include <QVariant>

reschedulePoitem::reschedulePoitem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));
  connect(_po, SIGNAL(valid(bool)), _poitem, SLOT(setEnabled(bool)));
  connect(_po, SIGNAL(newId(int, QString)), this, SLOT(sPopulatePoitem(int)));
  connect(_poitem, SIGNAL(newID(int)), this, SLOT(sPopulate(int)));
}

reschedulePoitem::~reschedulePoitem()
{
  // no need to delete child widgets, Qt does it all for us
}

void reschedulePoitem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse reschedulePoitem::set(const ParameterList &pParams)
{
  XSqlQuery rescheduleet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _po->setReadOnly(true);
  }

  param = pParams.value("poitem_id", &valid);
  if (valid)
  {
    rescheduleet.prepare( "SELECT pohead_id "
               "FROM pohead, poitem "
               "WHERE ((poitem_pohead_id=pohead_id)"
               "  AND (pohead_status='O')"
               "  AND (poitem_id=:poitem_id));" );
    rescheduleet.bindValue(":poitem_id", param.toInt());
    rescheduleet.exec();
    if (rescheduleet.first())
    {
      _po->setId(rescheduleet.value("pohead_id").toInt());
      _po->setReadOnly(true);
      _poitem->setId(param.toInt());
      _poitem->setEnabled(false);
    }
    else
    {
      QMessageBox::critical(this, tr("P/O Not Open"),
        tr("The P/O line item you are trying to reschedule does\n"
           "not belong to an Open P/O.") );
      return UndefinedError;
    }
  }

  return NoError;
}

void reschedulePoitem::sPopulatePoitem(int pPoheadid)
{
  XSqlQuery reschedulePopulatePoitem;
  reschedulePopulatePoitem.prepare( "SELECT poitem_id,"
             "       ( poitem_linenumber || '-' ||"
             "         COALESCE(item_number, poitem_vend_item_number) || ' (' ||"
             "         COALESCE(item_descrip1, firstLine(poitem_vend_item_descrip)) || ')' ) "
             "FROM poitem LEFT OUTER JOIN "
             "     ( itemsite JOIN item "
             "       ON (itemsite_item_id=item_id)"
             "     ) ON (poitem_itemsite_id=itemsite_id) "
             "WHERE ( (poitem_status <> 'C')"
             " AND (poitem_pohead_id=:pohead_id) ) "
             "ORDER BY poitem_linenumber;" );
  reschedulePopulatePoitem.bindValue(":pohead_id", pPoheadid);
  reschedulePopulatePoitem.exec();
  _poitem->populate(reschedulePopulatePoitem);
  sPopulate(_poitem->id());
}

void reschedulePoitem::sPopulate(int pPoitemid)
{
  XSqlQuery reschedulePopulate;
  if (pPoitemid == -1)
  {
    _current->clear();
    _new->clear();
    _reschedule->setEnabled(false);
  }
  else
  {
    reschedulePopulate.prepare( "SELECT poitem_duedate "
               "FROM poitem "
               "WHERE (poitem_id=:poitem_id);" );
    reschedulePopulate.bindValue(":poitem_id", pPoitemid);
    reschedulePopulate.exec();
    if (reschedulePopulate.first())
    {
      _current->setDate(reschedulePopulate.value("poitem_duedate").toDate());
      _reschedule->setEnabled(true);
    }
  }
}

void reschedulePoitem::sReschedule()
{
  XSqlQuery rescheduleReschedule;
  if (!_new->isValid())
  {
    QMessageBox::critical( this, tr("Invalid Reschedule Date"),
                           tr("<p>You must enter a reschedule due date before "
                              "you may save this Purchase Order Item.") );
    _new->setFocus();
    return;
  }

  rescheduleReschedule.prepare("SELECT changePoitemDueDate(:poitem_id, :dueDate);");
  rescheduleReschedule.bindValue(":poitem_id", _poitem->id());
  rescheduleReschedule.bindValue(":dueDate", _new->date());
  rescheduleReschedule.exec();

  omfgThis->sPurchaseOrdersUpdated(_po->id(), true);

  accept();
}

