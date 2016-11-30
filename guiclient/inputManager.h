/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef inputManager_h
#define inputManager_h

#include <QObject>
#include <QEvent>

class InputManagerPrivate;
class QScriptEngine;

#define	cBCWorkOrder              0x00000010
#define	cBCWorkOrderMaterial      0x00000020
#define	cBCWorkOrderOperation     0x00000040
#define	cBCSalesOrder             0x00000100
#define cBCSalesOrderLineItem     0x00000200
#define cBCItemSite               0x00000400
#define cBCItem                   0x00000800
#define cBCUPCCode                0x00001000
#define cBCEANCode                0x00002000
#define cBCCountTag               0x00004000
#define cBCLocation               0x00010000
#define cBCLocationIssue          0x00020000
#define cBCLocationContents       0x00040000
#define cBCUser                   0x00100000
#define	cBCTransferOrder          0x00200000
#define cBCTransferOrderLineItem  0x00400000
#define cBCLotSerialNumber	      0x00800000
#define	cBCPurchaseOrder          0x01000000
#define cBCPurchaseOrderLineItem  0x02000000

class InputManager : public QObject
{
  Q_OBJECT

  public:
    InputManager();

    Q_INVOKABLE void notify(int, QObject *, QObject *, const QString &);
    Q_INVOKABLE QString slotName(const QString &);

    void scriptAPI(QScriptEngine *engine, QString globalName);

  public slots:
    void sRemove(QObject *);

  signals:
    void gotBarCode(int type, int id);

  protected:
    bool eventFilter(QObject *, QEvent *);

  private:
    friend class InputManagerPrivate;
    InputManagerPrivate *_private;

};

#endif
