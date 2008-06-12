/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

//  inputManager.cpp
//  Created 05/13/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#include <QObject>
#include <QList>
#include <QKeyEvent>
#include <QEvent>

#include <xsqlquery.h>

#include "guiclient.h"

#include "inputManager.h"

typedef struct
{
  int  event;
  char string[5];
  int  length1;
  int  length2;
  int  length3;
} InputEvent;

#define cBCCProlog    "\x0b\x38"
#define cBCPrologSize 2
#define cBCTypeSize   4

#define cIdle         0x01
#define cProlog       0x02
#define cType         0x04
#define cHeader       0x08
#define cData         0x10

#ifdef Q_WS_MACX
#define cPrologCtrl   0x80    /* Macintosh-only */
#endif


static InputEvent _eventList[] =
{
  { cBCWorkOrder,          "WOXX", 1, 1, 0 },
  { cBCWorkOrderMaterial,  "WOMR", 1, 1, 1 },
  { cBCWorkOrderOperation, "WOOP", 1, 1, 1 },
  { cBCSalesOrder,         "SOXX", 1, 0, 0 },
  { cBCSalesOrderLineItem, "SOLI", 1, 1, 0 },
  { cBCTransferOrder,         "TOXX", 1, 0, 0 },
  { cBCTransferOrderLineItem, "TOLI", 1, 1, 0 },
  { cBCItemSite,           "ISXX", 2, 1, 0 },
  { cBCItem,               "ITXX", 2, 0, 0 },
  { cBCUPCCode,            "ITUP", 0, 0, 0 },
  { cBCEANCode,            "ITEA", 0, 0, 0 },
  { cBCCountTag,           "CTXX", 2, 0, 0 },
  { cBCLocation,           "LOXX", 1, 2, 0 },
  { cBCLocationIssue,      "LOIS", 1, 2, 0 },
  { cBCLocationContents,   "LOCN", 1, 2, 0 },
  { cBCUser,               "USER", 1, 0, 0 },
  { cBCLotSerialNumber,    "LSNX", 1, 0, 0 }, // TODO: itemloc? lsdetail?
  { -1,                    "",     0, 0, 0 }
};


class ReceiverItem
{
  public:
    ReceiverItem()
    {
      _null = TRUE;
    };

    ReceiverItem(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
    {
      _type   = pType;
      _parent = pParent;
      _target = pTarget;
      _slot   = pSlot;
      _null   = FALSE;
    };

    inline int type()        { return _type;   };
    inline QObject *parent() { return _parent; };
    inline QObject *target() { return _target; };
    inline QString slot()    { return _slot;   };
    inline bool isNull()     { return _null;   };
    bool operator==(const ReceiverItem &value) const
    {
      return (_null && value._null) || (_target == value._target) ;
    };

  private:
    int     _type;
    QObject *_parent;
    QObject *_target;
    QString _slot;
    bool    _null;
};


class InputManagerPrivate
{
  public:
    InputManagerPrivate()
    {
      _state = cIdle;
    };

    QList<ReceiverItem> _receivers;
    int                      _state;
    int                      _cursor;
    int                      _eventCursor;
    int                      _length1;
    int                      _length2;
    int                      _length3;
    int                      _type;
    QString                  _buffer;

    ReceiverItem findReceiver(int pMask)
    {
      for (int counter = 0; counter < _receivers.count(); counter++)
        if (_receivers[counter].type() & pMask)
          return _receivers[counter];

      return ReceiverItem();
    };
};


InputManager::InputManager()
{
  _private = new InputManagerPrivate();
}

void InputManager::notify(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
{
  _private->_receivers.prepend(ReceiverItem(pType, pParent, pTarget, pSlot));
  connect(pTarget, SIGNAL(destroyed(QObject *)), this, SLOT(sRemove(QObject *)));
}

void InputManager::sRemove(QObject *pTarget)
{
  for (int counter = 0; counter < _private->_receivers.count(); counter++)
    if (_private->_receivers[counter].target() == pTarget)
      _private->_receivers.remove(_private->_receivers.at(counter));
}

bool InputManager::eventFilter(QObject *, QEvent *pEvent)
{
  if (pEvent->type() == QEvent::KeyPress)
  {
//  Swallow the Shift key as the Symbol Bar Code readers express a Shift key
//  for each upper case alpha character
    if (((QKeyEvent *)pEvent)->key() == Qt::Key_Shift)
      return FALSE;

    int character = ((QKeyEvent *)pEvent)->ascii();
    /* qDebug("Scanned %d (key %d) at _private->_state=%d",
           character, ((QKeyEvent *)pEvent)->key(), _private->_state);
     */

    /* The Macintosh seems to handle control characters differently than
       Linux and Windows.  Apparently we need an extra state to look for
       the CTRL key (which Qt treats as Qt::Key_Meta, using Qt::Key_Control
       for the Apple/Command key).
     */
    switch (_private->_state)
    {
      case cIdle:
#ifdef Q_WS_MACX
        if (((QKeyEvent *)pEvent)->key() == Qt::Key_Meta)
        {
          _private->_state = cPrologCtrl;
          _private->_cursor = 0;
        }
#else
        if (character == QString(cBCCProlog)[0])
        {
          _private->_state = cProlog;
          _private->_cursor = 0;
        }
#endif
        else
          return FALSE;

        break;

#ifdef Q_WS_MACX
      case cPrologCtrl:
        // why does character come back as 0?
	// on an Intel Mac with Qt 4 the key() came back as Key_PageUp
	// but with PowerPC Mac with Qt 3 the key() came back as 'V'.
	// Accept either for now.
        if (((QKeyEvent *)pEvent)->key() - 64 == QString(cBCCProlog)[0] ||
            ((QKeyEvent *)pEvent)->key()      == Qt::Key_PageUp)
        {
          _private->_state = cProlog;
          _private->_cursor = 0;
        }
        else
          _private->_state = cIdle;

        break;
#endif

      case cProlog:
        _private->_cursor++;
        if (character == *(cBCCProlog + _private->_cursor))
        {
          if (_private->_cursor == (cBCPrologSize - 1))
          {
            _private->_state = cType;
            _private->_cursor = 0;
            _private->_buffer = "";
          }
        }
        else
          _private->_state = cIdle;

        break;

      case cType:
        _private->_buffer += character;
        if (++_private->_cursor == cBCTypeSize)
        {
          int cursor;
          for (cursor = 0, _private->_type = 0; _eventList[cursor].string[0] != '\0'; cursor++)
          {
            if (_private->_buffer == QString(_eventList[cursor].string))
            {
              _private->_eventCursor = cursor;
              _private->_type = _eventList[cursor].event;
	      break;
	    }
	  }

	  if (_private->_type == 0)
            _private->_state = cIdle;
          else
          {
            _private->_state = cHeader;
            _private->_cursor = 0;
            _private->_buffer = "";
	  }
        }

        break;

      case cHeader:
        _private->_buffer += character;
        _private->_cursor++;

        if (_private->_cursor == ( _eventList[_private->_eventCursor].length1 +
                         _eventList[_private->_eventCursor].length2 +
                         _eventList[_private->_eventCursor].length3 ) )
        {
          bool check;

          if (_eventList[_private->_eventCursor].length1)
          {
            _private->_length1 = _private->_buffer.left(_eventList[_private->_eventCursor].length1).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length1 = 0;

          if (_eventList[_private->_eventCursor].length2)
          {
            _private->_length2 = _private->_buffer.mid( _eventList[_private->_eventCursor].length1,
                                                        _eventList[_private->_eventCursor].length2 ).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length2 = 0;

          if (_eventList[_private->_eventCursor].length3)
          {
            _private->_length3 = _private->_buffer.right(_eventList[_private->_eventCursor].length3).toInt(&check);	    
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length3 = 0;

          _private->_cursor = 0;
          _private->_buffer = "";
          _private->_state = cData;
	}

	break;

      case cData:
        _private->_buffer += character;
        _private->_cursor++;

        if (_private->_cursor == (_private->_length1 + _private->_length2 + _private->_length3))
        {
          _private->_state = cIdle;

          switch (_private->_type)
          {
            case cBCWorkOrderOperation:
              dispatchWorkOrderOperation();
              break;

            case cBCSalesOrder:
              dispatchSalesOrder();
              break;

            case cBCTransferOrder:
              dispatchTransferOrder();
              break;

            case cBCCountTag:
              dispatchCountTag();
              break;

            case cBCWorkOrder:
              dispatchWorkOrder();
              break;

            case cBCSalesOrderLineItem:
              dispatchSalesOrderLineItem();
              break;

            case cBCTransferOrderLineItem:
              dispatchTransferOrderLineItem();
              break;

            case cBCItemSite:
              dispatchItemSite();
              break;

            case cBCItem:
              dispatchItem();
              break;

            case cBCUPCCode:
              dispatchUPCCode();
              break;

            case cBCLocation:
              dispatchLocation();
              break;

            case cBCLocationIssue:
              dispatchLocationIssue();
              break;

            case cBCLocationContents:
              dispatchLocationContents();
              break;

	    case cBCLotSerialNumber:
	      dispatchLotSerialNumber();
	      break;

            case cBCUser:
              dispatchUser();
              break;

            default:
              _private->_state = cIdle;
              break;
          }
	}

        break;
    }

    return TRUE;
  }
  else
    return FALSE;
}

void InputManager::dispatchWorkOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCWorkOrder);
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    if (receiver.type() == cBCWorkOrder)
    {
      XSqlQuery woid;
      woid.prepare( "SELECT wo_id "
                    "FROM wo "
                    "WHERE ( (wo_number=:wo_number)"
                    " AND (wo_subnumber=:wo_subnumber) );" );
      woid.bindValue(":wo_number", number);
      woid.bindValue(":wo_subnumber", subNumber);
      woid.exec();
      if (woid.first())
      {
        message( tr("Scanned Work Order #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (connect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrder(woid.value("wo_id").toInt());
          disconnect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot());
        }
      }
      else
        message( tr("Work Order #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchWorkOrderOperation()
{
  ReceiverItem receiver = _private->findReceiver((cBCWorkOrderOperation | cBCWorkOrder));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.mid(_private->_length1, _private->_length2);
    QString seqNumber = _private->_buffer.right(_private->_length3);

    XSqlQuery wooperid;
    wooperid.prepare( "SELECT wo_id, wooper_id "
                      "FROM wo, wooper "
                      "WHERE ( (wooper_wo_id=wo_id)"
                      " AND (wo_number=:wo_number)"
                      " AND (wo_subnumber=:wo_subnumber)"
                      " AND (wooper_seqnumber=:wooper_seqnumber) );" );
    wooperid.bindValue(":wo_number", number);
    wooperid.bindValue(":wo_subnumber", subNumber);
    wooperid.bindValue(":wooper_seqnumber", seqNumber);
    wooperid.exec();
    if (wooperid.first())
    {
      message( tr("Scanned Work Order #%1-%2, Operation %3.")
               .arg(number)
               .arg(subNumber)
               .arg(seqNumber), 1000 );

      if (receiver.type() == cBCWorkOrderOperation)
      {
        if (connect(this, SIGNAL(readWorkOrderOperation(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrderOperation(wooperid.value("wooper_id").toInt());
          disconnect(this, SIGNAL(readWorkOrderOperation(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCWorkOrder)
      {
        if (connect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot()))
        {
          emit readWorkOrder(wooperid.value("wo_id").toInt());
          disconnect(this, SIGNAL(readWorkOrder(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Work Order #%1-%2, Operation %3 does not exist in the Database.")
               .arg(number)
               .arg(subNumber)
               .arg(seqNumber), 1000 );
  }
}

void InputManager::dispatchSalesOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCSalesOrder);
  if (!receiver.isNull())
  {
    QString number = _private->_buffer.left(_private->_length1);

    XSqlQuery soheadid;
    soheadid.prepare( "SELECT cohead_id "
                      "FROM cohead "
                      "WHERE (cohead_number=:sohead_number);" );
    soheadid.bindValue(":sohead_number", number);
    soheadid.exec();
    if (soheadid.first())
    {
      message( tr("Scanned Sales Order #%1.")
               .arg(number), 1000 );

      if (connect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot()))
      {
        emit readSalesOrder(soheadid.value("cohead_id").toInt());
        disconnect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Sales Order #%1 does not exist in the Database.")
               .arg(number), 1000 );
  }
}

void InputManager::dispatchTransferOrder()
{
  ReceiverItem receiver = _private->findReceiver(cBCTransferOrder);
  if (!receiver.isNull())
  {
    QString number = _private->_buffer.left(_private->_length1);

    XSqlQuery toheadid;
    toheadid.prepare( "SELECT tohead_id "
                      "FROM tohead "
                      "WHERE (tohead_number=:tohead_number);" );
    toheadid.bindValue(":tohead_number", number);
    toheadid.exec();
    if (toheadid.first())
    {
      message( tr("Scanned Transfer Order #%1.")
               .arg(number), 1000 );

      if (connect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot()))
      {
        emit readTransferOrder(toheadid.value("tohead_id").toInt());
        disconnect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Transfer Order #%1 does not exist in the Database.")
               .arg(number), 1000 );
  }
}

void InputManager::dispatchSalesOrderLineItem()
{
  ReceiverItem receiver = _private->findReceiver((cBCSalesOrderLineItem | cBCSalesOrder | cBCItemSite | cBCItem));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    if ( (receiver.type() == cBCSalesOrderLineItem) ||
         (receiver.type() == cBCSalesOrder) )
    {
      XSqlQuery soitemid;
      soitemid.prepare( "SELECT cohead_id, coitem_id "
                        "FROM cohead, coitem "
                        "WHERE ( (coitem_cohead_id=cohead_id)"
                        " AND (cohead_number=:sohead_number)"
                        " AND (coitem_linenumber=:soitem_linenumber) );" );
      soitemid.bindValue(":sohead_number", number);
      soitemid.bindValue(":soitem_linenumber", subNumber);
      soitemid.exec();
      if (soitemid.first())
      {
        message( tr("Scanned Sales Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCSalesOrderLineItem)
        {
          if (connect(this, SIGNAL(readSalesOrderLineItem(int)), receiver.target(), receiver.slot()))
          {
            emit readSalesOrderLineItem(soitemid.value("coitem_id").toInt());
            disconnect(this, SIGNAL(readSalesOrderLineItem(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCSalesOrder)
       {
          if (connect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot()))
          {
            emit readSalesOrder(soitemid.value("cohead_id").toInt());
            disconnect(this, SIGNAL(readSalesOrder(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Sales Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
    else if ( (receiver.type() == cBCItemSite) ||
              (receiver.type() == cBCItem) )
    {
      XSqlQuery itemsiteid;
      itemsiteid.prepare( "SELECT itemsite_id, itemsite_item_id "
                          "FROM cohead, coitem, itemsite "
                          "WHERE ( (coitem_cohead_id=cohead_id)"
                          " AND (coitem_itemsite_id=itemsite_id)"
                          " AND (cohead_number=:sohead_number)"
                          " AND (coitem_linenumber=:soitem_linenumber) );" );
      itemsiteid.bindValue(":sohead_number", number);
      itemsiteid.bindValue(":soitem_linenumber", subNumber);
      itemsiteid.exec();
      if (itemsiteid.first())
      {
        message( tr("Scanned Sales Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCItemSite)
        {
          if (connect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot()))
          {
            emit readItemSite(itemsiteid.value("itemsite_id").toInt());
            disconnect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCItem)
        {
          if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
          {
            emit readItem(itemsiteid.value("itemsite_item_id").toInt());
            disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Sales Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchTransferOrderLineItem()
{
  ReceiverItem receiver = _private->findReceiver((cBCTransferOrderLineItem | cBCTransferOrder | cBCItem));
  if (!receiver.isNull())
  {
    QString number    = _private->_buffer.left(_private->_length1);
    QString subNumber = _private->_buffer.right(_private->_length2);

    if ( (receiver.type() == cBCTransferOrderLineItem)	||
         (receiver.type() == cBCTransferOrder)		||
	 (receiver.type() == cBCItem) )
    {
      XSqlQuery toitemid;
      toitemid.prepare( "SELECT tohead_id, toitem_id, toitem_item_id "
                        "FROM tohead, toitem "
                        "WHERE ( (toitem_tohead_id=tohead_id)"
                        " AND (tohead_number=:tohead_number)"
                        " AND (toitem_linenumber=:toitem_linenumber) );" );
      toitemid.bindValue(":tohead_number", number);
      toitemid.bindValue(":toitem_linenumber", subNumber);
      toitemid.exec();
      if (toitemid.first())
      {
        message( tr("Scanned Transfer Order Line #%1-%2.")
                 .arg(number)
                 .arg(subNumber), 1000 );

        if (receiver.type() == cBCTransferOrderLineItem)
        {
          if (connect(this, SIGNAL(readTransferOrderLineItem(int)), receiver.target(), receiver.slot()))
          {
            emit readTransferOrderLineItem(toitemid.value("toitem_id").toInt());
            disconnect(this, SIGNAL(readTransferOrderLineItem(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCTransferOrder)
        {
          if (connect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot()))
          {
            emit readTransferOrder(toitemid.value("tohead_id").toInt());
            disconnect(this, SIGNAL(readTransferOrder(int)), receiver.target(), receiver.slot());
          }
        }
        else if (receiver.type() == cBCItem)
        {
          if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
          {
            emit readItem(toitemid.value("toitem_item_id").toInt());
            disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
          }
        }
      }
      else
        message( tr("Transfer Order Line #%1-%2 does not exist in the Database.")
                 .arg(number)
                 .arg(subNumber), 1000 );
    }
  }
}

void InputManager::dispatchItemSite()
{
  ReceiverItem receiver = _private->findReceiver((cBCItemSite | cBCItem));
  if (!receiver.isNull())
  {
    QString itemNumber    = _private->_buffer.left(_private->_length1);
    QString warehouseCode = _private->_buffer.right(_private->_length2);

    XSqlQuery itemsiteid;
    itemsiteid.prepare( "SELECT itemsite_id, itemsite_item_id "
                        "FROM itemsite, item, warehous "
                        "WHERE ( (itemsite_warehous_id=warehous_id)"
                        " AND (itemsite_item_id=item_id)"
                        " AND (item_number=:item_number)"
                        " AND (warehous_code=:warehous_code) );" );
    itemsiteid.bindValue(":item_number", itemNumber);
    itemsiteid.bindValue(":warehous_code", warehouseCode);
    itemsiteid.exec();
    if (itemsiteid.first())
    {
      message( tr("Scanned Item %1, Warehouse %2.")
               .arg(itemNumber)
               .arg(warehouseCode), 1000 );

      if (receiver.type() == cBCItemSite)
      {
        if (connect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot()))
        {
          emit readItemSite(itemsiteid.value("itemsite_id").toInt());
          disconnect(this, SIGNAL(readItemSite(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCItem)
      {
        if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
        {
          emit readItem(itemsiteid.value("itemsite_item_id").toInt());
          disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Item %1, Warehouse %2 does not exist in the Database.")
               .arg(itemNumber)
               .arg(warehouseCode), 1000 );
  }
}

void InputManager::dispatchItem()
{
  ReceiverItem receiver = _private->findReceiver(cBCItem);
  if (!receiver.isNull())
  {
    QString itemNumber    = _private->_buffer.left(_private->_length1);

    XSqlQuery itemid;
    itemid.prepare( "SELECT item_id "
                    "FROM item "
                    "WHERE (item_number=:item_number);" );
    itemid.bindValue(":item_number", itemNumber);
    itemid.exec();
    if (itemid.first())
    {
      message( tr("Scanned Item %1.")
               .arg(itemNumber), 1000 );

      if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
      {
        emit readItem(itemid.value("item_id").toInt());
        disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Item %1 does not exist in the Database.")
               .arg(itemNumber), 1000 );
  }
}

void InputManager::dispatchUPCCode()
{
  ReceiverItem receiver = _private->findReceiver(cBCItem);
  if (!receiver.isNull())
  {
    QString upcCode = _private->_buffer.left(_private->_length1);

    XSqlQuery itemid;
    itemid.prepare( "SELECT item_id, item_number "
                    "FROM item "
                    "WHERE (item_upccode=:item_upccode);" );
    itemid.bindValue(":item_upccode", upcCode);
    itemid.exec();
    if (itemid.first())
    {
      message( tr("Scanned UPC %1 for Item %2.")
               .arg(upcCode)
               .arg(itemid.value("item_number").toString()), 1000 );

      if (connect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot()))
      {
        emit readItem(itemid.value("item_id").toInt());
        disconnect(this, SIGNAL(readItem(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("UPC Code %1 does not exist in the Database.")
               .arg(upcCode), 1000 );
  }
}

void InputManager::dispatchCountTag()
{
  ReceiverItem receiver = _private->findReceiver(cBCCountTag);
  if (!receiver.isNull())
  {
    QString tagNumber = _private->_buffer.left(_private->_length1);

    XSqlQuery cnttagid;
    cnttagid.prepare( "SELECT invcnt_id "
                      "FROM invcnt "
                      "WHERE (invcnt_tagnumber=:tagnumber);" );
    cnttagid.bindValue(":tagnumber", tagNumber);
    cnttagid.exec();
    if (cnttagid.first())
    {
      message( tr("Scanned Count Tag %1.")
               .arg(tagNumber), 1000 );

      if (connect(this, SIGNAL(readCountTag(int)), receiver.target(), receiver.slot()))
      {
        emit readCountTag(cnttagid.value("invcnt_id").toInt());
        disconnect(this, SIGNAL(readCountTag(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Item %1 does not exist in the Database.")
               .arg(tagNumber), 1000 );
  }
}

void InputManager::dispatchLocation()
{
  ReceiverItem receiver = _private->findReceiver(cBCLocation);
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Warehouse %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
      {
        emit readLocation(locationid.value("location_id").toInt());
        disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Warehouse %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchLocationIssue()
{
  ReceiverItem receiver = _private->findReceiver((cBCLocation | cBCLocationIssue));
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Warehouse %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (receiver.type() == cBCLocationIssue)
      {
        if (connect(this, SIGNAL(readLocationIssue(int)), receiver.target(), receiver.slot()))
        {
          emit readLocationIssue(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocationIssue(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCLocation)
      {
        if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
        {
          emit readLocation(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Warehouse %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchLocationContents()
{
  ReceiverItem receiver = _private->findReceiver((cBCLocation | cBCLocationContents));
  if (!receiver.isNull())
  {
    QString warehouseCode = _private->_buffer.left(_private->_length1);
    QString locationCode  = _private->_buffer.right(_private->_length2);

    XSqlQuery locationid;
    locationid.prepare( "SELECT location_id "
                        "FROM location, warehous "
                        "WHERE ( (location_warehous_id=warehous_id)"
                        " AND (warehous_code=:warehous_code)"
                        " AND (location_name=:location_name) );" );
    locationid.bindValue(":warehous_code", warehouseCode);
    locationid.bindValue(":location_name", locationCode);
    locationid.exec();
    if (locationid.first())
    {
      message( tr("Scanned Warehouse %1, Location %2.")
               .arg(warehouseCode) 
               .arg(locationCode), 1000 );

      if (receiver.type() == cBCLocationContents)
      {
        if (connect(this, SIGNAL(readLocationContents(int)), receiver.target(), receiver.slot()))
        {
          emit readLocationContents(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocationContents(int)), receiver.target(), receiver.slot());
        }
      }
      else if (receiver.type() == cBCLocation)
      {
        if (connect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot()))
        {
          emit readLocation(locationid.value("location_id").toInt());
          disconnect(this, SIGNAL(readLocation(int)), receiver.target(), receiver.slot());
        }
      }
    }
    else
      message( tr("Warehouse %1, Location %2 does not exist in the Database.")
               .arg(warehouseCode)
               .arg(locationCode), 1000 );
  }
}

void InputManager::dispatchUser()
{
  ReceiverItem receiver = _private->findReceiver((cBCUser));
  if (!receiver.isNull())
  {
    QString username = _private->_buffer.left(_private->_length1);

    XSqlQuery userid;
    userid.prepare( "SELECT usr_id "
                        "FROM usr "
                        "WHERE (usr_username=:username);" );
    userid.bindValue(":username", username);
    userid.exec();
    if (userid.first())
    {
      message( tr("Scanned User %1.")
               .arg(username), 1000 );

      if (connect(this, SIGNAL(readUser(int)), receiver.target(), receiver.slot()))
      {
	emit readUser(userid.value("usr_id").toInt());
	disconnect(this, SIGNAL(readUser(int)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("User %1 not exist in the Database.")
               .arg(username), 1000 );
  }
}

void InputManager::dispatchLotSerialNumber()
{
  // qDebug("dispatchLotSerialNumber");
  ReceiverItem receiver = _private->findReceiver(cBCLotSerialNumber);
  if (!receiver.isNull())
  {
    QString lotserial = _private->_buffer.left(_private->_length1);

    XSqlQuery lsdetail;
    lsdetail.prepare( "SELECT lsdetail_id "
		      "FROM lsdetail "
		      "WHERE (formatlotserialnumber(lsdetail_ls_id)=:lotserial);" );
    lsdetail.bindValue(":lotserial", lotserial);
    lsdetail.exec();
    if (lsdetail.first())
    {
      message( tr("Scanned Lot/Serial # %1.").arg(lotserial), 1000);

      if (connect(this, SIGNAL(readLotSerialNumber(QString)), receiver.target(), receiver.slot()))
      {
        emit readLotSerialNumber(lotserial);
        disconnect(this, SIGNAL(readLotSerialNumber(QString)), receiver.target(), receiver.slot());
      }
    }
    else
      message( tr("Lot/Serial # %1 does not exist in the Database.")
               .arg(lotserial), 1000 );
  }
}
