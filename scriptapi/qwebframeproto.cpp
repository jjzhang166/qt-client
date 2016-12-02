/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebframeproto.h"

#include <QIcon>
#include <QList>
#include <QMultiMap>
#include <QPrinter>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebHitTestResult>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSecurityOrigin>

QScriptValue RenderLayerToScriptValue(QScriptEngine *engine, const QWebFrame::RenderLayer &item)
{
  return engine->newVariant(item);
}
void RenderLayerFromScriptValue(const QScriptValue &obj, QWebFrame::RenderLayer &item)
{
  item = (QWebFrame::RenderLayer)obj.toInt32();
}

QScriptValue QWebFrametoScriptValue(QScriptEngine *engine, QWebFrame* const &item)
{
  return engine->newQObject(item);
}
void QWebFramefromScriptValue(const QScriptValue &obj, QWebFrame* &item)
{
  item = qobject_cast<QWebFrame*>(obj.toQObject());
}

void setupQWebFrameProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebFrametoScriptValue, QWebFramefromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebFrameProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebFrame*>(), proto);
  // Not allowed. Is private in in qwebframe.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebFrame>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebFrame, proto);
  engine->globalObject().setProperty("QWebFrame",  constructor);

  qScriptRegisterMetaType(engine, RenderLayerToScriptValue, RenderLayerFromScriptValue);
  constructor.setProperty("ContentsLayer", QScriptValue(engine, QWebFrame::ContentsLayer), permanent);
  constructor.setProperty("ScrollBarLayer", QScriptValue(engine, QWebFrame::ScrollBarLayer), permanent);
  constructor.setProperty("PanIconLayer", QScriptValue(engine, QWebFrame::PanIconLayer), permanent);
  constructor.setProperty("AllLayers", QScriptValue(engine, QWebFrame::AllLayers), permanent);
}

QScriptValue constructQWebFrame(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  Q_UNUSED(context);
  QWebFrame *obj = 0;

  return engine->toScriptValue(obj);
}

QWebFrameProto::QWebFrameProto(QObject *parent)
    : QObject(parent)
{
}

#if QT_VERSION >= 0x050000
void QWebFrameProto::addToJavaScriptWindowObject(const QString & name, QObject * object, QWebFrame::ValueOwnership own)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->addToJavaScriptWindowObject(name, object, own);
}
#endif

QUrl QWebFrameProto::baseUrl() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->baseUrl();
  return QUrl();
}

QList<QWebFrame *> QWebFrameProto::childFrames() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->childFrames();
  return QList<QWebFrame *>();
}

QSize QWebFrameProto::contentsSize() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->contentsSize();
  return QSize();
}

QWebElement QWebFrameProto::documentElement() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->documentElement();
  return QWebElement();
}

QWebElementCollection QWebFrameProto::findAllElements(const QString & selectorQuery) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->findAllElements(selectorQuery);
  return QWebElementCollection();
}

QWebElement QWebFrameProto::findFirstElement(const QString & selectorQuery) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->findFirstElement(selectorQuery);
  return QWebElement();
}

QString QWebFrameProto::frameName() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->frameName();
  return QString();
}

QRect QWebFrameProto::geometry() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->geometry();
  return QRect();
}

bool QWebFrameProto::hasFocus() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->hasFocus();
  return false;
}

QWebHitTestResult QWebFrameProto::hitTestContent(const QPoint & pos) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->hitTestContent(pos);
  return QWebHitTestResult();
}

QIcon QWebFrameProto::icon() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->icon();
  return QIcon();
}

void QWebFrameProto::load(const QUrl & url)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->load(url);
}

void QWebFrameProto::load(const QNetworkRequest & req, QNetworkAccessManager::Operation operation, const QByteArray & body)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->load(req, operation, body);
}

QMultiMap<QString, QString> QWebFrameProto::metaData() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->metaData();
  return QMultiMap<QString, QString>();
}

QWebPage* QWebFrameProto::page() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->page();
  return 0;
}

QWebFrame* QWebFrameProto::parentFrame() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->parentFrame();
  return 0;
}

QPoint QWebFrameProto::pos() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->pos();
  return QPoint();
}

void QWebFrameProto::render(QPainter * painter, const QRegion & clip)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->render(painter, clip);
}

void QWebFrameProto::render(QPainter * painter, int layer, const QRegion & clip)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->render(painter, QWebFrame::RenderLayer(layer), clip);
}

QUrl QWebFrameProto::requestedUrl() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->requestedUrl();
  return QUrl();
}

void QWebFrameProto::scroll(int dx, int dy)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scroll(dx, dy);
}

QRect QWebFrameProto::scrollBarGeometry(Qt::Orientation orientation) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollBarGeometry(orientation);
  return QRect();
}

int QWebFrameProto::scrollBarMaximum(Qt::Orientation orientation) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollBarMaximum(orientation);
  return 0;
}

int QWebFrameProto::scrollBarMinimum(Qt::Orientation orientation) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollBarMinimum(orientation);
  return 0;
}

Qt::ScrollBarPolicy QWebFrameProto::scrollBarPolicy(Qt::Orientation orientation) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollBarPolicy(orientation);
  return Qt::ScrollBarAsNeeded;
}

int QWebFrameProto::scrollBarValue(Qt::Orientation orientation) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollBarValue(orientation);
  return 0;
}

QPoint QWebFrameProto::scrollPosition() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->scrollPosition();
  return QPoint();
}

void QWebFrameProto::scrollToAnchor(const QString & anchor)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->scrollToAnchor(anchor);
}

QWebSecurityOrigin QWebFrameProto::securityOrigin() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->securityOrigin();
#if QT_VERSION >= 0x050000
  return QWebSecurityOrigin(QUrl());
#else
  return QWebSecurityOrigin(QWebSecurityOrigin::allOrigins().first()); // TODO: what's better?
#endif
}

void QWebFrameProto::setContent(const QByteArray & data, const QString & mimeType, const QUrl & baseUrl)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setContent(data, mimeType, baseUrl);
}

void QWebFrameProto::setFocus()
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setFocus();
}

void QWebFrameProto::setHtml(const QString & html, const QUrl & baseUrl)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setHtml(html, baseUrl);
}

void QWebFrameProto::setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setScrollBarPolicy(orientation, policy);
}

void QWebFrameProto::setScrollBarValue(Qt::Orientation orientation, int value)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setScrollBarValue(orientation, value);
}

void QWebFrameProto::setScrollPosition(const QPoint & pos)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setScrollPosition(pos);
}

void QWebFrameProto::setTextSizeMultiplier(qreal factor)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setTextSizeMultiplier(factor);
}

void QWebFrameProto::setUrl(const QUrl & url)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setUrl(url);
}

void QWebFrameProto::setZoomFactor(qreal factor)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->setZoomFactor(factor);
}

qreal QWebFrameProto::textSizeMultiplier() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->textSizeMultiplier();
  return qreal();
}

QString QWebFrameProto::title() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->title();
  return QString();
}

QString QWebFrameProto::toHtml() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->toHtml();
  return QString();
}

QString QWebFrameProto::toPlainText() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->toPlainText();
  return QString();
}

QUrl QWebFrameProto::url() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->url();
  return QUrl();
}

qreal QWebFrameProto::zoomFactor() const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->zoomFactor();
  return qreal();
}

// Reimplemented Public Functions.
bool QWebFrameProto::event(QEvent * e)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->event(e);
  return false;
}

// Public slots.
QVariant QWebFrameProto::evaluateJavaScript(const QString& scriptSource)
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    return item->evaluateJavaScript(scriptSource);
  return QVariant();
}

void QWebFrameProto::print(QPrinter * printer) const
{
  QWebFrame *item = qscriptvalue_cast<QWebFrame*>(thisObject());
  if (item)
    item->print(printer);
}
