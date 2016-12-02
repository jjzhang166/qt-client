/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBFRAMEPROTO_H__
#define __QWEBFRAMEPROTO_H__

#include <QByteArray>
#include <QList>
#include <QMultiMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QPainter>
#include <QPoint>
#include <QPrinter>
#include <QRect>
#include <QRegion>
#include <QSize>
#include <QString>
#include <QtScript>
#include <QUrl>
#include <QVariant>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebHitTestResult>
#include <QWebFrame>
#include <QWebPage>
#include <QWebSecurityOrigin>

Q_DECLARE_METATYPE(QWebFrame*)
//Q_DECLARE_METATYPE(QWebFrame) // Is private in in qwebframe.h

Q_DECLARE_METATYPE(enum QWebFrame::RenderLayer)
#if QT_VERSION >= 0x050000
Q_DECLARE_METATYPE(enum QWebFrame::ValueOwnership)
#endif

void setupQWebFrameProto(QScriptEngine *engine);
QScriptValue constructQWebFrame(QScriptContext *context, QScriptEngine *engine);

class QWebFrameProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY (const QUrl baseUrl        READ baseUrl)
  Q_PROPERTY (const QSize contentsSize  READ contentsSize)
  Q_PROPERTY (const bool focus          READ hasFocus)
  Q_PROPERTY (const QIcon icon          READ icon)
  Q_PROPERTY (const QUrl requestedUrl   READ requestedUrl)
  Q_PROPERTY (QPoint scrollPosition     READ scrollPosition     WRITE setScrollPosition)
  Q_PROPERTY (const QString title       READ title)
  Q_PROPERTY (QUrl url                  READ url                WRITE setUrl)
  Q_PROPERTY (qreal zoomFactor          READ zoomFactor         WRITE setZoomFactor)

  public:
    QWebFrameProto(QObject *parent);

#if QT_VERSION >= 0x050000
    Q_INVOKABLE void                          addToJavaScriptWindowObject(const QString & name, QObject * object, QWebFrame::ValueOwnership own = QWebFrame::QtOwnership);
#endif
    Q_INVOKABLE QUrl                          baseUrl() const;
    Q_INVOKABLE QList<QWebFrame *>            childFrames() const;
    Q_INVOKABLE QSize                         contentsSize() const;
    Q_INVOKABLE QWebElement                   documentElement() const;
    Q_INVOKABLE QWebElementCollection         findAllElements(const QString & selectorQuery) const;
    Q_INVOKABLE QWebElement                   findFirstElement(const QString & selectorQuery) const;
    Q_INVOKABLE QString                       frameName() const;
    Q_INVOKABLE QRect                         geometry() const;
    Q_INVOKABLE bool                          hasFocus() const;
    Q_INVOKABLE QWebHitTestResult             hitTestContent(const QPoint & pos) const;
    Q_INVOKABLE QIcon                         icon() const;
    Q_INVOKABLE void                          load(const QUrl & url);
    Q_INVOKABLE void                          load(const QNetworkRequest & req, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation, const QByteArray & body = QByteArray());
    Q_INVOKABLE QMultiMap<QString, QString>   metaData() const;
    Q_INVOKABLE QWebPage                     *page() const;
    Q_INVOKABLE QWebFrame                    *parentFrame() const;
    Q_INVOKABLE QPoint                        pos() const;
    Q_INVOKABLE void                          render(QPainter * painter, const QRegion & clip = QRegion());
    Q_INVOKABLE void                          render(QPainter * painter, int layer, const QRegion & clip = QRegion());
    Q_INVOKABLE QUrl                          requestedUrl() const;
    Q_INVOKABLE void                          scroll(int dx, int dy);
    Q_INVOKABLE QRect                         scrollBarGeometry(Qt::Orientation orientation) const;
    Q_INVOKABLE int                           scrollBarMaximum(Qt::Orientation orientation) const;
    Q_INVOKABLE int                           scrollBarMinimum(Qt::Orientation orientation) const;
    Q_INVOKABLE Qt::ScrollBarPolicy           scrollBarPolicy(Qt::Orientation orientation) const;
    Q_INVOKABLE int                           scrollBarValue(Qt::Orientation orientation) const;
    Q_INVOKABLE QPoint                        scrollPosition() const;
    Q_INVOKABLE void                          scrollToAnchor(const QString & anchor);
    Q_INVOKABLE QWebSecurityOrigin            securityOrigin() const;
    Q_INVOKABLE void                          setContent(const QByteArray & data, const QString & mimeType = QString(), const QUrl & baseUrl = QUrl());
    Q_INVOKABLE void                          setFocus();
    Q_INVOKABLE void                          setHtml(const QString & html, const QUrl & baseUrl = QUrl());
    Q_INVOKABLE void                          setScrollBarPolicy(Qt::Orientation orientation, Qt::ScrollBarPolicy policy);
    Q_INVOKABLE void                          setScrollBarValue(Qt::Orientation orientation, int value);
    Q_INVOKABLE void                          setScrollPosition(const QPoint & pos);
    Q_INVOKABLE void                          setTextSizeMultiplier(qreal factor);
    Q_INVOKABLE void                          setUrl(const QUrl & url);
    Q_INVOKABLE void                          setZoomFactor(qreal factor);
    Q_INVOKABLE qreal                         textSizeMultiplier() const;
    Q_INVOKABLE QString                       title() const;
    Q_INVOKABLE QString                       toHtml() const;
    Q_INVOKABLE QString                       toPlainText() const;
    Q_INVOKABLE QUrl                          url() const;
    Q_INVOKABLE qreal                         zoomFactor() const;

  // Reimplemented Public Functions.
    Q_INVOKABLE bool                          event(QEvent * e);

  public Q_SLOTS:
    Q_INVOKABLE QVariant                      evaluateJavaScript(const QString& scriptSource);
    Q_INVOKABLE void                          print(QPrinter * printer) const;

  signals:
    void    contentsSizeChanged(const QSize & size);
    void    iconChanged();
    void    initialLayoutCompleted();
    void    javaScriptWindowObjectCleared();
    void    loadFinished(bool ok);
    void    loadStarted();
    void    pageChanged();
    void    titleChanged(const QString & title);
    void    urlChanged(const QUrl & url);

};

#endif
