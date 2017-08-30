/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef metricsenc_h
#define metricsenc_h

#include "metrics.h"

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>

class Parametersenc : public Parameters
{
  Q_OBJECT

  protected:
    QString   _key;

  public:
    Parametersenc(const QString &key, QObject *parent = 0);
    virtual ~Parametersenc() {};

    virtual void load();

  protected:
    virtual void _set(const QString &pName, QVariant pValue);

};

class Metricsenc : public Parametersenc
{
  public:
    Metricsenc(const QString &key, QObject *parent = 0);
};

#endif

