/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __INPUTMANAGERPRIVATE_H__
#define __INPUTMANAGERPRIVATE_H__

#include <QHash>
#include <QList>
#include <QObject>

class InputManager;
class ScanEvent;

class ReceiverItem
{
  public:
    ReceiverItem();

    ReceiverItem(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot);

    inline int type()        { return _type;   };
    inline QObject *parent() { return _parent; };
    inline QObject *target() { return _target; };
    inline QString  slot()   { return _slot;   };
    inline bool isNull()     { return _null;   };
    bool operator==(const ReceiverItem &value) const;

    int     _type;
    QObject *_parent;
    QObject *_target;
    QString _slot;
    bool    _null;
};

class InputManagerPrivate : public QObject
{
  Q_OBJECT

  public:
    InputManagerPrivate(InputManager *parent);

    static QHash<QString, ScanEvent*> eventList;

    InputManager       *_parent;
    QList<ReceiverItem> _receivers;
    int                 _state;
    int                 _cursor;
    ScanEvent          *_event;
    int                 _length1;
    int                 _length2;
    int                 _length3;
    QString             _buffer;

    void dispatchScan(int type);

    void         addToEventList(QString prefix, int type, int length1, int length2, int length3, QString descrip, QString query);
    ReceiverItem findReceiver(int pMask);
    QString      queryFieldName(int barcodeType, int receiverType);

  signals:
    void gotBarCode(int type, int id);
};

#endif
