/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2013 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 *
 * Originally contributed by Specter Business Solutions - specter.ca/business
 */

#ifndef LOTSERIALUTILS_H
#define LOTSERIALUTILS_H

#include <QtCore>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#endif
#include "parameter.h"

class LotSerialUtils
{

public:
    LotSerialUtils();
    ~LotSerialUtils();

    QList<int> getLotCharIds() const;
    QList<int> getLotCharTypes() const;
    QStringList getLotCharNames() const;
    int numLotChars() const;

    void updateLotCharacteristics(int ls_id, const QList<QWidget *> &widgets) const;
    void setParams(ParameterList &params);

    static int getNextLotId();
    static QList<int> getLotSerialIds();
    static QList<QWidget *> addLotCharsToGridLayout(QWidget *parent, QGridLayout *layout,
                                                    const LotSerialUtils &lsChars);

    static void addCharsToTreeWidget(QTreeWidget *tree, int ls_numberColumn, int numInitialColums, bool repopulate);

private:
    QList<int> _charIds;
    QList<int> _charTypes;
    QStringList _charNames;
};

#endif // LOTSERIALUTILS_H
