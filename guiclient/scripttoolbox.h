/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __SCRIPTTOOLBOX_H__
#define __SCRIPTTOOLBOX_H__

#include <QObject>
#include <QVariant>
#include <QtScript>

#include <parametergroup.h>

#include "parameter.h"
#include "guiclient.h"

#include "addresscluster.h"     // for AddressCluster::SaveFlags

class QWidget;
class QLayout;
class QGridLayout;
class QBoxLayout;
class QStackedLayout;
class QScriptEngine;

/* TODO: remove this enum and use AddressCluster::SaveFlags directly
   for some reason working with AddressCluster::SaveFlags failed but this works.
 */
enum SaveFlags
{ CHECK = AddressCluster::CHECK,
  CHANGEONE = AddressCluster::CHANGEONE,
  CHANGEALL = AddressCluster::CHANGEALL
};

Q_DECLARE_METATYPE(ParameterList)
Q_DECLARE_METATYPE(XSqlQuery);
Q_DECLARE_METATYPE(enum SetResponse)
Q_DECLARE_METATYPE(enum ParameterGroup::ParameterGroupStates);
Q_DECLARE_METATYPE(enum ParameterGroup::ParameterGroupTypes);
Q_DECLARE_METATYPE(enum Qt::WindowModality);
Q_DECLARE_METATYPE(enum SaveFlags);

QScriptValue ParameterListtoScriptValue(QScriptEngine *engine, const ParameterList &params);
void ParameterListfromScriptValue(const QScriptValue &obj, ParameterList &params);

QScriptValue XSqlQuerytoScriptValue(QScriptEngine *engine, const XSqlQuery &qry);
void XSqlQueryfromScriptValue(const QScriptValue &obj, XSqlQuery &qry);

QScriptValue SetResponsetoScriptValue(QScriptEngine *engine, const enum SetResponse &sr);
void SetResponsefromScriptValue(const QScriptValue &obj, enum SetResponse &sr);

QScriptValue ParameterGroupStatestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupStates &en);
void ParameterGroupStatesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupStates &en);

QScriptValue ParameterGroupTypestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupTypes &en);
void ParameterGroupTypesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupTypes &en);

QScriptValue QtWindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &en);
void QtWindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &en);

QScriptValue SaveFlagstoScriptValue(QScriptEngine *engine, const enum SaveFlags &en);
void SaveFlagsfromScriptValue(const QScriptValue &obj, enum SaveFlags &en);

class ScriptToolbox : public QObject
{
  Q_OBJECT

  public:
    ScriptToolbox(QScriptEngine * engine);
    virtual ~ScriptToolbox();

    static QScriptValue variantToScriptValue(QScriptEngine *, QVariant);
    static void setLastWindow(QWidget * lw);

  public slots:

    QObject * executeQuery(const QString & query);
    QObject * executeQuery(const QString & query, const ParameterList & params);
    QObject * executeDbQuery(const QString & group, const QString & name);
    QObject * executeDbQuery(const QString & group, const QString & name, const ParameterList & params);
    QObject * executeBegin();
    QObject * executeCommit();
    QObject * executeRollback();
    
    QObject * qtyVal();
    QObject * TransQtyVal();
    QObject * qtyPerVal();
    QObject * percentVal();
    QObject * moneyVal();
    QObject * negMoneyVal();
    QObject * priceVal();
    QObject * costVal();
    QObject * ratioVal();
    QObject * weightVal();
    QObject * runTimeVal();
    QObject * orderVal();
    QObject * dayVal();

    QObject * widgetGetLayout(QWidget * w);

    QObject * createGridLayout();

    void layoutGridAddLayout(QObject *, QObject *, int row, int column, int alignment = 0);

    void layoutBoxInsertWidget(QObject *, int index, QWidget *, int stretch = 0, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int row, int column, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int fromRow, int fromColumn, int rowSpan, int columnSpan, int alignment = 0);
    void layoutStackedInsertWidget(QObject *, int index, QWidget *);

    QObject * menuAddAction(QObject * menu, const QString & text, const bool enabled = true);
    QObject * menuAddMenu(QObject * menu, const QString & text, const QString & name = QString());
    QObject * menuAddSeparator(QObject * menu);
    QObject * menuInsertAction(QObject * menu, QObject * before, const QString & name = QString(), const bool enabled = true);
    QObject * menuInsertMenu(QObject * menu, QObject * before, const QString & name);
    QObject * menuInsertSeparator(QObject * menu, QObject * before);
    void      menuRemove(QObject * menu, QObject * action);
    int       menuActionCount(QObject * menu);

    int       tabCount(QWidget * tab);
    QWidget * tabWidget(QWidget * tab, int idx);
    int       tabInsertTab(QWidget * tab, int idx, QWidget * page, const QString & text);
    int       tabTabIndex(QWidget * tab, QWidget * page);
    void      tabRemoveTab(QWidget * tab, int idx);
    void      tabSetTabEnabled(QWidget * tab, int idx, bool enable);
    void      tabSetTabText(QWidget * tab, int idx, const QString & text);
    QString   tabtabText(QWidget * tab, int idx);

    QWidget * createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString());
    QObject * createLayout(const QString & className, QWidget * parent, const QString & name = QString());
    QWidget * loadUi(const QString & screenName, QWidget * parent = 0);

    QWidget * lastWindow() const;
    QWidget * openWindow(const QString pname, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0);

    void addColumnXTreeWidget(QWidget * tree, const QString &, int, int, bool = true, const QString = QString(), const QString = QString());
    void populateXTreeWidget(QWidget * tree, QObject * pSql, bool = FALSE);
    
    void loadQWebView(QWidget * webView, const QString & url);

    bool printReport(const QString & name, const ParameterList & params, const QString & pdfFilename = QString::null);

    bool coreDisconnect(QObject * sender, const QString & signal, QObject * receiver, const QString & method);

    QString fileDialog(QWidget * parent, const QString & caption, const QString & dir, const QString & filter, int fileModeSel, int acceptModeSel);
    void openUrl(const QString & fileUrl);
    bool copyFile(const QString & oldName, const QString & newName);
    QString getFileName(const QString & path);
    bool renameFile(const QString & oldName, const QString & newName);
    bool removeFile(const QString & name);
    bool fileExists(const QString & name);
    QString getCurrentDir();
    QString getHomeDir();
    void    listProperties(const QScriptValue &obj) const;
    bool    makePath(const QString & mkPath, const QString & rootPath);
    int     messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons = 0x00000400, int defaultButton = 0x00000000);
    bool    removePath(const QString & rmPath, const QString & rootPath);
    QString rootPath();
    QString textStreamRead(const QString & name);
    bool    textStreamWrite(const QString & name, const QString & WriteText);

    int     saveCreditCard(QWidget *parent,
                              int custId,
                              QString ccName, 
                              QString ccAddress1, 
                              QString ccAddress2,
                              QString ccCity, 
                              QString ccState, 
                              QString ccZip, 
                              QString ccCountry,
                              QString ccNumber,
                              QString ccType,
                              QString ccExpireMonth,
                              QString ccExpireYear,
                              int ccId = 0,
                              bool ccActive = true );
    QObject *getCreditCardProcessor();
    

  private:
    QScriptEngine * _engine;
    static QWidget * _lastWindow;
};

#endif // __SCRIPTTOOLBOX_H__
