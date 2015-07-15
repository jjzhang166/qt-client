/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef PRINTSINGLECOPYDOCUMENT_H
#define PRINTSINGLECOPYDOCUMENT_H

#include "ui_printSinglecopyDocument.h"

#include <parameter.h>

#include "guiclient.h"
#include "xdialog.h"

class XDocCopySetter;
class printSinglecopyDocumentPrivate;

class printSinglecopyDocument : public XDialog
{
  Q_OBJECT

  friend class printSinglecopyDocumentPrivate;
  Q_PROPERTY(QString doctype   READ doctype   WRITE setDoctype)
  Q_PROPERTY(QString reportKey READ reportKey WRITE setReportKey)

  public:
    printSinglecopyDocument(QWidget    *parent = 0,
                            const char *name   = 0,
                            bool        modal  = false,
                            Qt::WindowFlags  fl     = 0);
    ~printSinglecopyDocument();

    Q_INVOKABLE virtual void            clear();
                virtual QString         doctype();
    Q_INVOKABLE virtual ParameterList   getParams(XSqlQuery *docq);
    Q_INVOKABLE virtual ParameterList   getParamsDocList();
    Q_INVOKABLE virtual int             id();
    Q_INVOKABLE virtual bool            isOkToPrint();
    Q_INVOKABLE virtual bool            isOnPrintedList(const int docid);
    Q_INVOKABLE virtual bool            isSetup();
    Q_INVOKABLE virtual QWidget        *optionsWidget();
                virtual QString         reportKey();
                virtual void            setReportKey(QString key);
    Q_INVOKABLE virtual void            setSetup(bool setup);

  public slots:
    virtual enum SetResponse set(const ParameterList &pParams);
    virtual void    sAddToPrintedList(XSqlQuery *docq);
    virtual void    sClearPrintedList();
    virtual bool    sMarkOnePrinted(XSqlQuery *docq);
    virtual void    sPopulate();
    virtual void    sPrint();
    virtual bool    sPrintOneDoc(XSqlQuery *docq);
    virtual void    setDoctype(QString doctype);
    virtual void    setId(int docid);
    virtual void    setPrintEnabled(bool enabled);

  signals:
    void docUpdated(int);
    void aboutToStart(XSqlQuery *qry);
    void finishedPrinting(int);
    void finishedWithAll();
    void newId(int id);
    void populated(XSqlQuery            *docq);
    void timeToMarkOnePrinted(XSqlQuery *docq);
    void timeToPrintOneDoc(XSqlQuery    *docq);

  protected slots:
    virtual void languageChange();

  protected:
    printSinglecopyDocumentPrivate *_data;

    QString _docinfoQueryString;
    QString _markAllPrintedQry;
    QString _markOnePrintedQry;
};

#endif // PRINTSINGLECOPYDOCUMENT_H
