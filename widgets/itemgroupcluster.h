/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _itemGroupCluster_h
#define _itemGroupCluster_h

#include "virtualCluster.h"

void setupItemGroupClusterLineEdit(QScriptEngine *engine);
void setupItemGroupCluster(QScriptEngine *engine);

class ItemGroupInfo : public VirtualInfo
{
    Q_OBJECT

    public:
      ItemGroupInfo(QWidget*, Qt::WindowFlags = 0);
};

class ItemGroupList : public VirtualList
{
    Q_OBJECT

    public:
      ItemGroupList(QWidget*, Qt::WindowFlags = 0);
};

class ItemGroupSearch : public VirtualSearch
{
    Q_OBJECT

    public:
      ItemGroupSearch(QWidget*, Qt::WindowFlags = 0);
};


class XTUPLEWIDGETS_EXPORT ItemGroupClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    public:
        ItemGroupClusterLineEdit(QWidget*, const char* = 0);

    protected:
        virtual VirtualInfo   *infoFactory();
        virtual VirtualList   *listFactory();
        virtual VirtualSearch *searchFactory();
};

class XTUPLEWIDGETS_EXPORT ItemGroupCluster : public VirtualCluster
{
    Q_OBJECT

    public:
        ItemGroupCluster(QWidget*, const char* = 0);
};

Q_DECLARE_METATYPE(ItemGroupClusterLineEdit*)
Q_DECLARE_METATYPE(ItemGroupCluster*)

#endif
