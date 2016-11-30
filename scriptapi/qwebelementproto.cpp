/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebelementproto.h"

#if QT_VERSION < 0x050000
void setupQWebElementProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue StyleResolveStrategyToScriptValue(QScriptEngine *engine, const QWebElement::StyleResolveStrategy &item)
{
  return engine->newVariant(item);
}
void StyleResolveStrategyFromScriptValue(const QScriptValue &obj, QWebElement::StyleResolveStrategy &item)
{
  item = (QWebElement::StyleResolveStrategy)obj.toInt32();
}

void setupQWebElementProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebElementProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebElement*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QWebElement>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebElement, proto);
  engine->globalObject().setProperty("QWebElement", constructor);

  qScriptRegisterMetaType(engine, StyleResolveStrategyToScriptValue, StyleResolveStrategyFromScriptValue);
  constructor.setProperty("InlineStyle", QScriptValue(engine, QWebElement::InlineStyle), permanent);
  constructor.setProperty("CascadedStyle", QScriptValue(engine, QWebElement::CascadedStyle), permanent);
  constructor.setProperty("ComputedStyle", QScriptValue(engine, QWebElement::ComputedStyle), permanent);
}

QScriptValue constructQWebElement(QScriptContext *context, QScriptEngine  *engine)
{
  QWebElement *obj = 0;
  if (context->argumentCount() == 1) {
    QWebElement other = qscriptvalue_cast<QWebElement>(context->argument(0));
    obj = new QWebElement(other);
  } else {
    obj = new QWebElement();
  }

  return engine->toScriptValue(obj);
}

QWebElementProto::QWebElementProto(QObject *parent) : QObject(parent)
{
}
QWebElementProto::~QWebElementProto()
{
}

void QWebElementProto::addClass(const QString & name)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->addClass(name);
}

void QWebElementProto::appendInside(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->appendInside(markup);
}

void QWebElementProto::appendInside(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->appendInside(element);
}

void QWebElementProto::appendOutside(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->appendOutside(markup);
}

void QWebElementProto::appendOutside(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->appendOutside(element);
}

QString QWebElementProto::attribute(const QString & name, const QString & defaultValue) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->attribute(name, defaultValue);
  return QString();
}

QString QWebElementProto::attributeNS(const QString & namespaceUri, const QString & name, const QString & defaultValue) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->attributeNS(namespaceUri, name, defaultValue);
  return QString();
}

QStringList QWebElementProto::attributeNames(const QString & namespaceUri) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->attributeNames(namespaceUri);
  return QStringList();
}

QStringList QWebElementProto::classes() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->classes();
  return QStringList();
}

QWebElement QWebElementProto::clone() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->clone();
  return QWebElement();
}

QWebElement QWebElementProto::document() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->document();
  return QWebElement();
}

void QWebElementProto::encloseContentsWith(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->encloseContentsWith(element);
}

void QWebElementProto::encloseContentsWith(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->encloseContentsWith(markup);
}

void QWebElementProto::encloseWith(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->encloseWith(markup);
}

void QWebElementProto::encloseWith(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->encloseWith(element);
}

QVariant QWebElementProto::evaluateJavaScript(const QString & scriptSource)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->evaluateJavaScript(scriptSource);
  return QVariant();
}

QWebElementCollection QWebElementProto::findAll(const QString & selectorQuery) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->findAll(selectorQuery);
  return QWebElementCollection();
}

QWebElement QWebElementProto::findFirst(const QString & selectorQuery) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->findFirst(selectorQuery);
  return QWebElement();
}

QWebElement QWebElementProto::firstChild() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->firstChild();
  return QWebElement();
}

QRect QWebElementProto::geometry() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->geometry();
  return QRect();
}

bool QWebElementProto::hasAttribute(const QString & name) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->hasAttribute(name);
  return false;
}

bool QWebElementProto::hasAttributeNS(const QString & namespaceUri, const QString & name) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->hasAttributeNS(namespaceUri, name);
  return false;
}

bool QWebElementProto::hasAttributes() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->hasAttributes();
  return false;
}

bool QWebElementProto::hasClass(const QString & name) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->hasClass(name);
  return false;
}

bool QWebElementProto::hasFocus() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->hasFocus();
  return false;
}

bool QWebElementProto::isNull() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

QWebElement QWebElementProto::lastChild() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->lastChild();
  return QWebElement();
}

QString QWebElementProto::localName() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->localName();
  return QString();
}

QString QWebElementProto::namespaceUri() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->namespaceUri();
  return QString();
}

QWebElement QWebElementProto::nextSibling() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->nextSibling();
  return QWebElement();
}

QWebElement QWebElementProto::parent() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->parent();
  return QWebElement();
}

QString QWebElementProto::prefix() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->prefix();
  return QString();
}

void QWebElementProto::prependInside(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->prependInside(markup);
}

void QWebElementProto::prependInside(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->prependInside(element);
}

void QWebElementProto::prependOutside(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->prependOutside(markup);
}

void QWebElementProto::prependOutside(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->prependOutside(element);
}

QWebElement QWebElementProto::previousSibling() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->previousSibling();
  return QWebElement();
}

void QWebElementProto::removeAllChildren()
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->removeAllChildren();
}

void QWebElementProto::removeAttribute(const QString & name)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->removeAttribute(name);
}

void QWebElementProto::removeAttributeNS(const QString & namespaceUri, const QString & name)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->removeAttributeNS(namespaceUri, name);
}

void QWebElementProto::removeClass(const QString & name)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->removeClass(name);
}

void QWebElementProto::removeFromDocument()
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->removeFromDocument();
}

void QWebElementProto::render(QPainter * painter)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->render(painter);
}

void QWebElementProto::render(QPainter * painter, const QRect & clip)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->render(painter, clip);
}

void QWebElementProto::replace(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->replace(markup);
}

void QWebElementProto::replace(const QWebElement & element)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->replace(element);
}

void QWebElementProto::setAttribute(const QString & name, const QString & value)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setAttribute(name, value);
}

void QWebElementProto::setAttributeNS(const QString & namespaceUri, const QString & name, const QString & value)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setAttributeNS(namespaceUri, name, value);
}

void QWebElementProto::setFocus()
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setFocus();
}

void QWebElementProto::setInnerXml(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setInnerXml(markup);
}

void QWebElementProto::setOuterXml(const QString & markup)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setOuterXml(markup);
}

void QWebElementProto::setPlainText(const QString & text)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setPlainText(text);
}

void QWebElementProto::setStyleProperty(const QString & name, const QString & value)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->setStyleProperty(name, value);
}

QString QWebElementProto::styleProperty(const QString & name, QWebElement::StyleResolveStrategy strategy) const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->styleProperty(name, strategy);
  return QString();
}

QString QWebElementProto::tagName() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->tagName();
  return QString();
}

QWebElement & QWebElementProto::takeFromDocument()
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->takeFromDocument();
  return *(new QWebElement());
}

QString QWebElementProto::toInnerXml() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->toInnerXml();
  return QString();
}

QString QWebElementProto::toOuterXml() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->toOuterXml();
  return QString();
}

QString QWebElementProto::toPlainText() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->toPlainText();
  return QString();
}

void QWebElementProto::toggleClass(const QString & name)
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    item->toggleClass(name);
}

QWebFrame * QWebElementProto::webFrame() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item) {
    return item->webFrame();
  }

  // TODO: You cannot `ruturn new QWebFrame();` directly, so we make a new QWebPage first.
  QWebPage *page = new QWebPage();
  return page->currentFrame();
}

QString QWebElementProto::toString() const
{
  QWebElement *item = qscriptvalue_cast<QWebElement*>(thisObject());
  if (item)
    return item->toPlainText();
  return QString("QWebElement(unknown)");
}

#endif
