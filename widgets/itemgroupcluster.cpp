/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "itemgroupcluster.h"

ItemGroupCluster::ItemGroupCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new ItemGroupClusterLineEdit(this, pName));
    setDescriptionVisible(true);
}

ItemGroupClusterLineEdit::ItemGroupClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "itemgrp", "itemgrp_id", "itemgrp_name", 0, "itemgrp_descrip", 0, pName, 0)
{
    setTitles(tr("Item Group"), tr("Item Groups"));
    setUiName("itemGroup");
    setEditPriv("MaintainItemGroups");
    setNewPriv("MaintainItemGroups");
    // no ViewItemGroups priv
}

VirtualInfo *ItemGroupClusterLineEdit::infoFactory()
{
    return new ItemGroupInfo(this);
}

VirtualList *ItemGroupClusterLineEdit::listFactory()
{
    return new ItemGroupList(this);
}

VirtualSearch *ItemGroupClusterLineEdit::searchFactory()
{
    return new ItemGroupSearch(this);
}

ItemGroupInfo::ItemGroupInfo(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualInfo(pParent, pFlags)
{
    _numberLit->setText(tr("Name:"));
}

ItemGroupList::ItemGroupList(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualList(pParent, pFlags)
{
    QTreeWidgetItem *hitem = _listTab->headerItem();
    hitem->setText(0, tr("Name"));
}

ItemGroupSearch::ItemGroupSearch(QWidget *pParent, Qt::WindowFlags pFlags) : VirtualSearch(pParent, pFlags)
{
    _searchNumber->setText(tr("Search through Names"));
    QTreeWidgetItem *hitem = _listTab->headerItem();
    hitem->setText(0, tr("Name"));
}

// script exposure ////////////////////////////////////////////////////////////

QScriptValue ItemGroupClusterLineEdittoScriptValue(QScriptEngine *engine, ItemGroupClusterLineEdit* const &item)
{
    return engine->newQObject(item);
}

void ItemGroupClusterLineEditfromScriptValue(const QScriptValue &obj, ItemGroupClusterLineEdit* &item)
{
    item = qobject_cast<ItemGroupClusterLineEdit*>(obj.toQObject());
}

QScriptValue constructItemGroupClusterLineEdit(QScriptContext *context,
                                         QScriptEngine  *engine)
{
    ItemGroupClusterLineEdit *obj = 0;

    if (context->argumentCount() == 1 &&
        qscriptvalue_cast<QWidget*>(context->argument(0)))
      obj = new ItemGroupClusterLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)));

    else if (context->argumentCount() >= 2 &&
             qscriptvalue_cast<QWidget*>(context->argument(0)))
      obj = new ItemGroupClusterLineEdit(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                       qPrintable(context->argument(1).toString()));

    else
      context->throwError(QScriptContext::UnknownError,
                          "could not find an appropriate ItemGroupClusterLineEdit constructor");

    return engine->toScriptValue(obj);
}

void setupItemGroupClusterLineEdit(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, ItemGroupClusterLineEdittoScriptValue, ItemGroupClusterLineEditfromScriptValue);
    QScriptValue widget = engine->newFunction(constructItemGroupClusterLineEdit);
    engine->globalObject().setProperty("ItemGroupClusterLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

QScriptValue ItemGroupClustertoScriptValue(QScriptEngine *engine, ItemGroupCluster* const &item)
{
    return engine->newQObject(item);
}

void ItemGroupClusterfromScriptValue(const QScriptValue &obj, ItemGroupCluster* &item)
{
    item = qobject_cast<ItemGroupCluster*>(obj.toQObject());
}

QScriptValue constructItemGroupCluster(QScriptContext *context,
                                       QScriptEngine  *engine)
{
    ItemGroupCluster *obj = 0;

    if (context->argumentCount() == 1 &&
        qscriptvalue_cast<QWidget*>(context->argument(0)))
      obj = new ItemGroupCluster(qscriptvalue_cast<QWidget*>(context->argument(0)));

    else if (context->argumentCount() >= 2 &&
             qscriptvalue_cast<QWidget*>(context->argument(0)))
      obj = new ItemGroupCluster(qscriptvalue_cast<QWidget*>(context->argument(0)),
                                 qPrintable(context->argument(1).toString()));

    else
      context->throwError(QScriptContext::UnknownError,
                          "could not find an appropriate ItemGroupCluster constructor");

    return engine->toScriptValue(obj);
}

void setupItemGroupCluster(QScriptEngine *engine)
{
    qScriptRegisterMetaType(engine, ItemGroupClustertoScriptValue, ItemGroupClusterfromScriptValue);
    QScriptValue widget = engine->newFunction(constructItemGroupCluster);
    engine->globalObject().setProperty("ItemGroupCluster", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}
