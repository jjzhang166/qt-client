/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBELEMENTPROTO_H__
#define __QWEBELEMENTPROTO_H__

#include <QScriptEngine>

void setupQWebElementProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QWebElement>
#include <QWebPage>

Q_DECLARE_METATYPE(QWebElement*)
//Q_DECLARE_METATYPE(QWebElement) // Already Q_DECLARE_METATYPE() in qwebelement.h.
Q_DECLARE_METATYPE(enum QWebElement::StyleResolveStrategy)

QScriptValue constructQWebElement(QScriptContext *context, QScriptEngine *engine);

class QWebElementProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebElementProto(QObject *parent);
    virtual ~QWebElementProto();

    Q_INVOKABLE void                    addClass(const QString & name);
    Q_INVOKABLE void                    appendInside(const QString & markup);
    Q_INVOKABLE void                    appendInside(const QWebElement & element);
    Q_INVOKABLE void                    appendOutside(const QString & markup);
    Q_INVOKABLE void                    appendOutside(const QWebElement & element);
    Q_INVOKABLE QString                 attribute(const QString & name, const QString & defaultValue = QString()) const;
    Q_INVOKABLE QString                 attributeNS(const QString & namespaceUri, const QString & name, const QString & defaultValue = QString()) const;
    Q_INVOKABLE QStringList             attributeNames(const QString & namespaceUri = QString()) const;
    Q_INVOKABLE QStringList             classes() const;
    Q_INVOKABLE QWebElement             clone() const;
    Q_INVOKABLE QWebElement             document() const;
    Q_INVOKABLE void                    encloseContentsWith(const QWebElement & element);
    Q_INVOKABLE void                    encloseContentsWith(const QString & markup);
    Q_INVOKABLE void                    encloseWith(const QString & markup);
    Q_INVOKABLE void                    encloseWith(const QWebElement & element);
    Q_INVOKABLE QVariant                evaluateJavaScript(const QString & scriptSource);
    Q_INVOKABLE QWebElementCollection   findAll(const QString & selectorQuery) const;
    Q_INVOKABLE QWebElement             findFirst(const QString & selectorQuery) const;
    Q_INVOKABLE QWebElement             firstChild() const;
    Q_INVOKABLE QRect                   geometry() const;
    Q_INVOKABLE bool                    hasAttribute(const QString & name) const;
    Q_INVOKABLE bool                    hasAttributeNS(const QString & namespaceUri, const QString & name) const;
    Q_INVOKABLE bool                    hasAttributes() const;
    Q_INVOKABLE bool                    hasClass(const QString & name) const;
    Q_INVOKABLE bool                    hasFocus() const;
    Q_INVOKABLE bool                    isNull() const;
    Q_INVOKABLE QWebElement             lastChild() const;
    Q_INVOKABLE QString                 localName() const;
    Q_INVOKABLE QString                 namespaceUri() const;
    Q_INVOKABLE QWebElement             nextSibling() const;
    Q_INVOKABLE QWebElement             parent() const;
    Q_INVOKABLE QString                 prefix() const;
    Q_INVOKABLE void                    prependInside(const QString & markup);
    Q_INVOKABLE void                    prependInside(const QWebElement & element);
    Q_INVOKABLE void                    prependOutside(const QString & markup);
    Q_INVOKABLE void                    prependOutside(const QWebElement & element);
    Q_INVOKABLE QWebElement             previousSibling() const;
    Q_INVOKABLE void                    removeAllChildren();
    Q_INVOKABLE void                    removeAttribute(const QString & name);
    Q_INVOKABLE void                    removeAttributeNS(const QString & namespaceUri, const QString & name);
    Q_INVOKABLE void                    removeClass(const QString & name);
    Q_INVOKABLE void                    removeFromDocument();
    Q_INVOKABLE void                    render(QPainter * painter);
    Q_INVOKABLE void                    render(QPainter * painter, const QRect & clip);
    Q_INVOKABLE void                    replace(const QString & markup);
    Q_INVOKABLE void                    replace(const QWebElement & element);
    Q_INVOKABLE void                    setAttribute(const QString & name, const QString & value);
    Q_INVOKABLE void                    setAttributeNS(const QString & namespaceUri, const QString & name, const QString & value);
    Q_INVOKABLE void                    setFocus();
    Q_INVOKABLE void                    setInnerXml(const QString & markup);
    Q_INVOKABLE void                    setOuterXml(const QString & markup);
    Q_INVOKABLE void                    setPlainText(const QString & text);
    Q_INVOKABLE void                    setStyleProperty(const QString & name, const QString & value);
    Q_INVOKABLE QString                 styleProperty(const QString & name, QWebElement::StyleResolveStrategy strategy) const;
    Q_INVOKABLE QString                 tagName() const;
    Q_INVOKABLE QWebElement &           takeFromDocument();
    Q_INVOKABLE QString                 toInnerXml() const;
    Q_INVOKABLE QString                 toOuterXml() const;
    Q_INVOKABLE QString                 toPlainText() const;
    Q_INVOKABLE void                    toggleClass(const QString & name);
    Q_INVOKABLE QWebFrame *             webFrame() const;

    Q_INVOKABLE QString                 toString() const;

};

#endif
#endif
