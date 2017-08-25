/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef XDOCCOPYSETTER_H
#define XDOCCOPYSETTER_H

#include "ui_xdoccopysetter.h"

class QScriptEngine;

void setupXDocCopySetter(QScriptEngine *engine);

class XDocCopySetter : public QWidget, public Ui::XDocCopySetter
{
  Q_OBJECT

  Q_PROPERTY(QString labelText       READ labelText       WRITE setLabelText)
  Q_PROPERTY(QString numCopiesMetric READ numCopiesMetric WRITE setNumCopiesMetric)
  Q_PROPERTY(QString showPriceMetric READ showPriceMetric WRITE setShowPriceMetric)
  Q_PROPERTY(QString watermarkMetric READ watermarkMetric WRITE setWatermarkMetric)

  public:
    XDocCopySetter(QWidget* parent = 0, const char* name = 0);
    ~XDocCopySetter();

                virtual QString labelText()       const;
    Q_INVOKABLE virtual int     numCopies()       const;
                virtual QString numCopiesMetric() const;
    Q_INVOKABLE virtual bool    showCosts(const int row) const;
                virtual QString showPriceMetric() const;
                virtual QString watermarkMetric() const;
    Q_INVOKABLE virtual QString watermark(const int row) const;

  public slots:
    virtual bool save();
    virtual void setLabelText(const QString text);
    virtual void setNumCopies(const int     numCopies);
    virtual void setNumCopiesMetric(const QString metric);
    virtual void setShowPriceMetric(const QString metric);
    virtual void setWatermarkMetric(const QString metric);

  protected slots:
    virtual void languageChange();
    virtual void sEditWatermark();

  protected:
    QString _numCopiesMetric;
    QString _showPriceMetric;
    QString _watermarkMetric;

  private:

};

#endif // XDOCCOPYSETTER_H
