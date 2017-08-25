/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QList>
#include <QObject>
#include <QSqlError>
#include <QScriptEngine>
#include <QScriptValue>

#include <xsqlquery.h>

#include "guiclient.h"

#include "inputManager.h"
#include "inputManagerPrivate.h"

#define DEBUG false

class ScanEvent
{
  public:
    QString              descrip;
    int                  length1;
    int                  length2;
    int                  length3;
    QString              prefix;
    QString              query;
    int                  type;

    ScanEvent(int type, QString prefix, int length1, int length2, int length3, QString descrip, QString query)
      :
        descrip(descrip),
        length1(length1),
        length2(length2),
        length3(length3),
        prefix(prefix),
        query(query),
        type(type)
    {
    }
};

#define cBCCProlog    "\x0b\x38"
#define cBCPrologSize 2
#define cBCTypeSize   4

#define cIdle         0x01
#define cProlog       0x02
#define cType         0x04
#define cHeader       0x08
#define cData         0x10

#ifdef Q_OS_MAC
#define cPrologCtrl   0x80    /* Macintosh-only */
#endif

ReceiverItem::ReceiverItem()
  : _type(0),
    _parent(0),
    _target(0),
    _null(true)
{
}

ReceiverItem::ReceiverItem(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
  : _type(pType),
    _parent(pParent),
    _target(pTarget),
    _slot(pSlot),
    _null(false)
{
}

bool ReceiverItem::operator==(const ReceiverItem &value) const
{
  return (_null && value._null) || (_target == value._target) ;
}

QHash<QString, ScanEvent*> InputManagerPrivate::eventList;

InputManagerPrivate::InputManagerPrivate(InputManager *parent)
  : QObject(parent),
    _parent(parent),
    _state(cIdle),
    _event(0)
{
  if (eventList.isEmpty())
  {
    // MUST BE CAREFUL HERE. e.g. POXX and POLI must take pohead_number = :f1 and return pohead_id as id
    // TODO:                                                  tr() for vvvvvvvvvvvvvvvvvvvvv
    addToEventList("WOXX", cBCWorkOrder,             1, 1, 0, "Work Order %1-%2",               "SELECT wo_id AS id FROM wo WHERE wo_number = :f1 and wo_subnumber = :f2;" );
//  addToEventList("WOMR", cBCWorkOrderMaterial,     1, 1, 1, );
    addToEventList("WOOP", cBCWorkOrderOperation,    1, 1, 1, "Work Order %1-%2, Operation %3", "SELECT wo_id AS id, wooper_id AS altid FROM wo JOIN wooper ON wooper_wo_id = wo_id AND wo_number = :f1 and wo_subnumber = :f2 and wooper_seqnumber = :f3;" );
    addToEventList("POXX", cBCPurchaseOrder,         1, 0, 0, "Purchase Order %1",              "SELECT pohead_id AS id FROM pohead WHERE pohead_number = :f1;" );
    addToEventList("POLI", cBCPurchaseOrderLineItem, 1, 1, 0, "Purchase Order Line %1-%2",      "SELECT pohead_id AS id, poitem_id AS altid, itemsite_id, itemsite_item_id FROM pohead JOIN poitem ON poitem_pohead_id=pohead_id JOIN itemsite ON poitem_itemsite_id = itemsite_id WHERE pohead_number = :f1 AND poitem_linenumber = :f2;" );
    addToEventList("SOXX", cBCSalesOrder,            1, 0, 0, "Sales Order %1",                 "SELECT cohead_id AS id FROM cohead WHERE cohead_number = :f1;" );
    addToEventList("SOLI", cBCSalesOrderLineItem,    1, 1, 0, "Sales Order Line %1-%2",         "SELECT cohead_id AS id, coitem_id AS altid, itemsite_id, itemsite_item_id FROM cohead JOIN coitem ON coitem_cohead_id=cohead_id JOIN itemsite ON coitem_itemsite_id = itemsite_id WHERE cohead_number=:f1 AND coitem_linenumber = :f2 AND coitem_subnumber = :f3;" );
    addToEventList("TOXX", cBCTransferOrder,         1, 0, 0, "Transfer Order %1",              "SELECT tohead_id AS id FROM tohead WHERE tohead_number = :f1;" );
    addToEventList("TOLI", cBCTransferOrderLineItem, 1, 1, 0, "Transfer Order Line %1-%2",      "SELECT tohead_id AS id, toitem_id AS altid, toitem_item_id AS seq FROM tohead JOIN toitem ON toitem_tohead_id=tohead_id WHERE tohead_number = :f1 AND toitem_linenumber = :f2;" );
    addToEventList("ISXX", cBCItemSite,              2, 1, 0, "Item %1, Site %2",               "SELECT itemsite_id AS id, itemsite_item_id AS altid FROM itemsite JOIN item ON itemsite_item_id=item_id JOIN whsinfo ON itemsite_warehous_id = warehous_id WHERE item_number = :f1 AND warehous_code = :f2;" );
    addToEventList("ITXX", cBCItem,                  2, 0, 0, "Item %1",                        "SELECT item_id AS id FROM item WHERE item_number = :f1;" );
    addToEventList("ITUP", cBCUPCCode,               0, 0, 0, "UPC %1 for Item %2",             "SELECT item_id, item_number FROM item WHERE item_upccode = :f1 AND item_active);" );
//  addToEventList("ITEA", cBCEANCode,               0, 0, 0 );
    addToEventList("CTXX", cBCCountTag,              2, 0, 0, "Count Tag %1",                   "SELECT invcnt_id AS id FROM invcnt WHERE invcnt_tagnumber = :f1;" );
    addToEventList("LOXX", cBCLocation,              1, 2, 0, "Site %1, Location %2",           "SELECT location_id AS id FROM location JOIN whsinfo ON location_warehous_id = warehous_id WHERE warehous_code = :f1 AND location_name = :f2;" );
    addToEventList("LOIS", cBCLocationIssue,         1, 2, 0, "Site %1, Location %2",           "SELECT location_id AS id FROM location JOIN whsinfo ON location_warehous_id = warehous_id WHERE warehous_code = :f1 AND location_name = :f2;" );
    addToEventList("LOCN", cBCLocationContents,      1, 2, 0, "Site %1, Location %2",           "SELECT location_id AS id FROM location JOIN whsinfo ON location_warehous_id = warehous_id WHERE warehous_code = :f1 AND location_name = :f2;" );
    addToEventList("USER", cBCUser,                  1, 0, 0, "User %1",                        "SELECT usr_id AS id FROM usr WHERE usr_username = :f1;" );
    addToEventList("LSNX", cBCLotSerialNumber,       1, 0, 0, "Lot/Serial %1",                  "SELECT lsdetail_id AS id FROM lsdetail WHERE formatlotserialnumber(lsdetail_ls_id) = :f1;" );
    // TODO: itemloc? lsdetail?
  }
}

ReceiverItem InputManagerPrivate::findReceiver(int pMask)
{
  for (int counter = 0; counter < _receivers.count(); counter++)
    if (_receivers[counter].type() & pMask)
      return _receivers[counter];
  return ReceiverItem();
}

void InputManagerPrivate::addToEventList(QString prefix, int type, int length1, int length2, int length3, QString descrip, QString query) {
  if (! eventList.contains(prefix))
    eventList.insert(prefix, new ScanEvent(type, prefix, length1, length2, length3, descrip, query));
};

InputManager::InputManager()
{
  _private = new InputManagerPrivate(this);
  connect(_private, SIGNAL(gotBarCode(int, int)), this, SIGNAL(gotBarCode(int, int)));
}

// TODO: treat _receivers as a stack, not a queue
void InputManager::notify(int pType, QObject *pParent, QObject *pTarget, const QString &pSlot)
{
  if (DEBUG)
    qDebug() << "InputManager::notify() entered" << pType << pTarget << pSlot;
  _private->_receivers.prepend(ReceiverItem(pType, pParent, pTarget, pSlot));
  connect(pTarget, SIGNAL(destroyed(QObject *)), this, SLOT(sRemove(QObject *)));
}

/* Application scripts need to pass strings as the last argument to
   InputManager::notify() but don't have access to the C++ SLOT() macro
   to generate it. This function mimics the SLOT() macro and will need
   to be maintained as Qt changes.
 */
QString InputManager::slotName(const QString &slotname)
{
  if (DEBUG)
    qDebug("slotName(%s) entered", qPrintable(slotname));
  return QString("1") + slotname;
}

void InputManager::sRemove(QObject *pTarget)
{
  if (DEBUG) qDebug() << "InputManager::sRemove() entered" << pTarget;
  for (int counter = 0; counter < _private->_receivers.count(); counter++)
    if (_private->_receivers[counter].target() == pTarget)
      _private->_receivers.removeAt(counter);
}

bool InputManager::eventFilter(QObject *, QEvent *pEvent)
{
  if (pEvent->type() == QEvent::KeyPress)
  {
//  Swallow the Shift key as the Symbol Bar Code readers express a Shift key
//  for each upper case alpha character
    if (((QKeyEvent *)pEvent)->key() == Qt::Key_Shift)
      return false;

    int character = ((QKeyEvent *)pEvent)->text().data()->toLatin1();
    if (DEBUG)
      qDebug("Scanned %d (key %d) at _private->_state=%d",
           character, ((QKeyEvent *)pEvent)->key(), _private->_state);

    /* The Macintosh seems to handle control characters differently than
       Linux and Windows.  Apparently we need an extra state to look for
       the CTRL key (which Qt treats as Qt::Key_Meta, using Qt::Key_Control
       for the Apple/Command key).
     */
    switch (_private->_state)
    {
      case cIdle:
#ifdef Q_OS_MAC
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
          return false;

        break;

#ifdef Q_OS_MAC
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
        if (character == cBCCProlog[_private->_cursor])
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
          QString prefix(_private->_buffer);
          _private->_event = InputManagerPrivate::eventList.value(prefix, 0);
          if (_private->_event)
          {
            _private->_state = cHeader;
            _private->_cursor = 0;
            _private->_buffer = "";
          }
          else
            _private->_state = cIdle;
        }
        break;

      case cHeader:
        _private->_buffer += character;
        _private->_cursor++;

        if (_private->_cursor == ( _private->_event->length1 +
                                   _private->_event->length2 +
                                   _private->_event->length3 ) )
        {
          bool check;

          if (_private->_event->length1)
          {
            _private->_length1 = _private->_buffer.left(_private->_event->length1).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length1 = 0;

          if (_private->_event->length2)
          {
            _private->_length2 = _private->_buffer.mid( _private->_event->length1,
                                                        _private->_event->length2 ).toInt(&check);
            if (!check)
            {
              _private->_state = cIdle;
              break;
            }
          }
          else
            _private->_length2 = 0;

          if (_private->_event->length3)
          {
            _private->_length3 = _private->_buffer.right(_private->_event->length3).toInt(&check);
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
          // make sure we got a recognized scan event
          switch (_private->_event->type)
          {
            case cBCCountTag:
            case cBCItem:
            case cBCLocation:
            case cBCLotSerialNumber:
            case cBCPurchaseOrder:
            case cBCSalesOrder:
            case cBCSalesOrderLineItem:
            case cBCTransferOrder:
            case cBCTransferOrderLineItem:
            case cBCUser:
            case cBCWorkOrder:
            case cBCWorkOrderOperation:
            case cBCPurchaseOrderLineItem:
            case cBCItemSite:
            case cBCUPCCode:
            case cBCLocationIssue:
            case cBCLocationContents:
              _private->dispatchScan(_private->_event->type);
              // FALLTHROUGH

            default:
              _private->_state = cIdle;
              break;
          }
        }
        break;
    }

    return true;
  }

  return false;
}

/* return a null QString if we couldn't find the right query field to extract
 */
QString InputManagerPrivate::queryFieldName(int barcodeType, int receiverType)
{
  if (DEBUG)
    qDebug("queryFieldName(%d, %d) entered", barcodeType, receiverType);
  QString result;
  switch (receiverType)
  {
    case cBCItem:
      if (barcodeType == cBCPurchaseOrderLineItem ||
          barcodeType == cBCSalesOrderLineItem    ||
          barcodeType == cBCItemSite)
        result = "itemsite_item_id";
      else if (barcodeType == cBCTransferOrderLineItem)
        result = "toitem_item_id";
      else
        result = "id";
      break;
    case cBCItemSite:
      if (barcodeType == cBCPurchaseOrderLineItem ||
          barcodeType == cBCSalesOrderLineItem)
        result = "itemsite_id";
      else
        result = "id";
      break;
    case cBCCountTag:
    case cBCLocation:
    case cBCLotSerialNumber:
    case cBCPurchaseOrder:
    case cBCPurchaseOrderLineItem:
    case cBCSalesOrder:
    case cBCSalesOrderLineItem:
    case cBCTransferOrder:
    case cBCTransferOrderLineItem:
    case cBCUser:
    case cBCWorkOrder:
      result = "id";
      break;
    case cBCWorkOrderOperation:
      result = "altid";
      break;
    case cBCUPCCode:
    case cBCLocationIssue:
    case cBCLocationContents:
      result = "id";
      break;
  }
  if (DEBUG)
    qDebug("queryFieldName(%d, %d) returning %s", barcodeType, receiverType, qPrintable(result));
  return result;
}

void InputManagerPrivate::dispatchScan(int type)
{
  if (DEBUG)
    qDebug("dispatchScan(%d) entered", type);
  ReceiverItem receiver = findReceiver(type);
  if (! receiver.isNull())
  {
    if (DEBUG)
      qDebug() << "dispatchScan() receiver:"     << receiver.type()
               << receiver.parent()      << "->" << receiver.target()
               << "[" << receiver.slot() << "]"  << receiver.isNull();

    QString number    = _buffer.left(_length1);
    QString subNumber = _buffer.mid(_length1, _length2);
    QString seqNumber = _buffer.right(_length3);
    if (DEBUG)
      qDebug() << "dispatchScan:" << _length1 << _length2 << _length3 << number << subNumber << seqNumber;

    // TODO: can we remove this special-casing for kit sales order items?
    if (type & cBCSalesOrderLineItem) {
      int subsep = subNumber.indexOf(".");
      if (subsep >= 0)
      {
        subNumber = subNumber.left(subsep);
        seqNumber = subNumber.right(subNumber.length() - (subsep + 1));
      }
      if (seqNumber.isEmpty())
        seqNumber = "0";
    }

    QString descrip;
    if (_length3 > 0)
      descrip = _event->descrip.arg(number, subNumber, seqNumber);
    else if (_length2 > 0)
      descrip = _event->descrip.arg(number, subNumber);
    else
      descrip = _event->descrip.arg(number);

    XSqlQuery q;
    q.prepare(_event->query);
    q.bindValue(":f1", number);
    q.bindValue(":f2", subNumber);
    q.bindValue(":f3", seqNumber);
    q.exec();
    if (q.first())
    {
      message(tr("Scanned %1").arg(descrip), 1000);

      QString fieldName = queryFieldName(type, receiver.type());

      if (fieldName.isEmpty())
      {
        message(tr("Don't know how to send %1 (barcode %2, receiver %3)")
                .arg(descrip).arg(type, receiver.type()));
        return;
      }

      int id = q.value(fieldName).toInt();
      QGenericArgument idArg = Q_ARG(int, id);
      // convert "1methodName(args)(stuff)" to just "methodName"
      QString methodName = receiver.slot();
      methodName.replace(QRegExp("^1([a-z][a-z0-9_]*).*", Qt::CaseInsensitive), "\\1");

      if (DEBUG)
        qDebug() << receiver.target() << methodName.toLatin1().data() << id;
      (void)QMetaObject::invokeMethod(receiver.target(),
                                      methodName.toLatin1().data(), idArg);
      emit gotBarCode(type, id);
    }
    else if (q.lastError().type() != QSqlError::NoError)
      message(tr("Error Scanning %1: %2").arg(descrip, q.lastError().text()), 1000);
    else
      message(tr("%1 not found").arg(descrip));
  }
}

void InputManager::scriptAPI(QScriptEngine *engine, QString globalName)
{
  if (engine)
  {
    QScriptValue im = engine->newQObject(this);
    engine->globalObject().setProperty(globalName, im);

    QList< QPair<QString, int> > constants;

    constants << QPair<QString, int>("cBCWorkOrder",             cBCWorkOrder)
//            << QPair<QString, int>("cBCWorkOrderMaterial",     cBCWorkOrderMaterial)
              << QPair<QString, int>("cBCWorkOrderOperation",    cBCWorkOrderOperation)
              << QPair<QString, int>("cBCSalesOrder",            cBCSalesOrder)
              << QPair<QString, int>("cBCSalesOrderLineItem",    cBCSalesOrderLineItem)
              << QPair<QString, int>("cBCItemSite",              cBCItemSite)
              << QPair<QString, int>("cBCItem",                  cBCItem)
              << QPair<QString, int>("cBCUPCCode",               cBCUPCCode)
//            << QPair<QString, int>("cBCEANCode",               cBCEANCode)
              << QPair<QString, int>("cBCCountTag",              cBCCountTag)
              << QPair<QString, int>("cBCLocation",              cBCLocation)
              << QPair<QString, int>("cBCLocationIssue",         cBCLocationIssue)
              << QPair<QString, int>("cBCLocationContents",      cBCLocationContents)
              << QPair<QString, int>("cBCUser",                  cBCUser)
              << QPair<QString, int>("cBCTransferOrder",         cBCTransferOrder)
              << QPair<QString, int>("cBCTransferOrderLineItem", cBCTransferOrderLineItem)
              << QPair<QString, int>("cBCLotSerialNumber",       cBCLotSerialNumber)
        ;

    QPair<QString, int> pair;
    foreach (pair, constants) {
      im.setProperty(pair.first, QScriptValue(engine, pair.second), QScriptValue::ReadOnly | QScriptValue::Undeletable);
    }
  }
}
