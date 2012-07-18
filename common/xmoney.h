/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XMONEY_H
#define XMONEY_H

#include <QVariant>

#include "format.h"

class xmoneyPrivate;

class xmoney {
  public:
    xmoney(float pamt = 0, QString pcurr = QString::null, int pextraprec = 0);
    xmoney(const xmoney  &p);
    xmoney(const QVariant p);
    ~xmoney();

    float   amount()        const;
    QString currency()      const;
    int     decimalPlaces() const;
    int     extraPlaces()   const;
    int     getCurrId()     const;
    QString toDbString()    const;

  protected:
    xmoneyPrivate *_data;
};

#endif
