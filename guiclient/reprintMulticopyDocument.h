/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef REPRINTMULTICOPYDOCUMENT_H
#define REPRINTMULTICOPYDOCUMENT_H

#include "ui_reprintMulticopyDocument.h"

#include <parameter.h>

#include "distributeInventory.h"
#include "guiclient.h"
#include "xdialog.h"

class XDocCopySetter;
class reprintMulticopyDocumentPrivate;

class reprintMulticopyDocument : public XDialog
{
  Q_OBJECT

  friend class reprintMulticopyDocumentPrivate;
  Q_PROPERTY(QString doctype   READ doctype   WRITE setDoctype)
  Q_PROPERTY(QString reportKey READ reportKey WRITE setReportKey)

  public:
    reprintMulticopyDocument(QWidget    *parent = 0,
                             const char *name   = 0,
                             bool        modal  = false,
                             Qt::WindowFlags  fl     = 0);
    reprintMulticopyDocument(QString numCopiesMetric,
                             QString watermarkMetric,
                             QString showPriceMetric,
                             QWidget    *parent = 0,
                             const char *name   = 0,
                             bool        modal  = false,
                             Qt::WindowFlags  fl     = 0);
    ~reprintMulticopyDocument();

    Q_INVOKABLE virtual XDocCopySetter *copies();
                virtual QString         doctype();
    Q_INVOKABLE virtual ParameterList   getParamsDocList();
    Q_INVOKABLE virtual ParameterList   getParamsOneCopy(int row, XTreeWidgetItem *item);
    Q_INVOKABLE virtual bool            isOnPrintedList(const int docid);
    Q_INVOKABLE virtual XTreeWidget    *list();
    Q_INVOKABLE virtual QWidget        *optionsWidget();
                virtual QString         reportKey();
    Q_INVOKABLE virtual void            setNumCopiesMetric(QString metric);
                virtual void            setReportKey(QString key);
    Q_INVOKABLE virtual void            setShowPriceMetric(QString metric);
    Q_INVOKABLE virtual void            setWatermarkMetric(QString metric);

  public slots:
    virtual enum SetResponse set(const ParameterList & pParams);
    virtual void sAddToPrintedList(XTreeWidgetItem *item);
    virtual void sPopulate();
    virtual void sPrint();
    virtual bool sPrintOneDoc(XTreeWidgetItem *item);
    virtual void setDoctype(QString doctype);

  signals:
    void docUpdated(int);
    void aboutToStart(XTreeWidgetItem*);
    void finishedPrinting(int);
    void finishedWithAll();
    void timeToPrintOneDoc(XTreeWidgetItem*);

  protected slots:
    virtual void languageChange();

  protected:
    reprintMulticopyDocumentPrivate *_data;

    QString _docListQueryString;

};

#endif // REPRINTMULTICOPYDOCUMENT_H
