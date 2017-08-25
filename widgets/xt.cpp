/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xt.h"

void setupXt(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;
  QScriptValue widget = engine->globalObject().property("Xt");
  if (! widget.isObject()) {
    widget = engine->newObject();
    engine->globalObject().setProperty("Xt", widget, ro);
  }

  widget.setProperty("RawRole",         QScriptValue(engine, Xt::RawRole),         ro);
  widget.setProperty("ScaleRole",       QScriptValue(engine, Xt::ScaleRole),       ro);
  widget.setProperty("IdRole",          QScriptValue(engine, Xt::IdRole),          ro);
  widget.setProperty("RunningSetRole",  QScriptValue(engine, Xt::RunningSetRole),  ro);
  widget.setProperty("RunningInitRole", QScriptValue(engine, Xt::RunningInitRole), ro);
  widget.setProperty("TotalSetRole",    QScriptValue(engine, Xt::TotalSetRole),    ro);
  widget.setProperty("TotalInitRole",   QScriptValue(engine, Xt::TotalInitRole),   ro);
  widget.setProperty("IndentRole",      QScriptValue(engine, Xt::IndentRole),      ro);
  widget.setProperty("DeletedRole",     QScriptValue(engine, Xt::DeletedRole),     ro);

  widget.setProperty("AllModules",         QScriptValue(engine, Xt::AllModules),      ro);
  widget.setProperty("AccountingModule",   QScriptValue(engine, Xt::AccountingModule),ro);
  widget.setProperty("SalesModule",        QScriptValue(engine, Xt::SalesModule),           ro);
  widget.setProperty("CRMModule",          QScriptValue(engine, Xt::CRMModule),             ro);
  widget.setProperty("ManufactureModule",  QScriptValue(engine, Xt::ManufactureModule),     ro);
  widget.setProperty("PurchaseModule",     QScriptValue(engine, Xt::PurchaseModule),        ro);
  widget.setProperty("ScheduleModule",     QScriptValue(engine, Xt::ScheduleModule),        ro);
  widget.setProperty("InventoryModule",    QScriptValue(engine, Xt::InventoryModule), ro);
  widget.setProperty("ProductsModule",     QScriptValue(engine, Xt::ProductsModule),  ro);
  widget.setProperty("SystemModule",       QScriptValue(engine, Xt::SystemModule),    ro);
}

Q_DECLARE_METATYPE(enum Xt::ItemDataRole);
Q_DECLARE_METATYPE(enum Xt::StandardModules);
