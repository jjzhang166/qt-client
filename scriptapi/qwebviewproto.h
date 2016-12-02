/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBVIEWPROTO_H__
#define __QWEBVIEWPROTO_H__

#include <QByteArray>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QPainter>
#include <QPrinter>
#include <QString>
#include <QtScript>
#include <QUrl>
#include <QWebFrame>
#include <QWebView>
#include <QWebPage>

Q_DECLARE_METATYPE(QWebView*)
//Q_DECLARE_METATYPE(QWebView) // Is private in qwebview.h

void setupQWebViewProto(QScriptEngine *engine);
QScriptValue constructQWebView(QScriptContext *context, QScriptEngine *engine);

class QWebViewProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY (const bool hasSelection               READ hasSelection)
  Q_PROPERTY (const QIcon icon                      READ icon)
  Q_PROPERTY (const bool modified                   READ isModified)
  Q_PROPERTY (QPainter::RenderHints renderHints     READ renderHints    WRITE setRenderHints)
  Q_PROPERTY (const QString selectedHtml            READ selectedHtml)
  Q_PROPERTY (const QString selectedText            READ selectedText)
  Q_PROPERTY (const QString title                   READ title)
  Q_PROPERTY (QUrl url                              READ url            WRITE setUrl)
  Q_PROPERTY (qreal zoomFactor                      READ zoomFactor     WRITE setZoomFactor)

  public:
    QWebViewProto(QObject *parent);
    virtual ~QWebViewProto();

    Q_INVOKABLE bool                    findText(const QString &subString, QWebPage::FindFlags options = 0);
    Q_INVOKABLE bool                    hasSelection() const;
    Q_INVOKABLE QWebHistory            *history() const;
    Q_INVOKABLE QIcon                   icon() const;
    Q_INVOKABLE bool                    isModified() const;
    Q_INVOKABLE void                    load(const QUrl &url);
    Q_INVOKABLE void                    load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation, const QByteArray &body = QByteArray());
    Q_INVOKABLE QWebPage               *page() const;
    Q_INVOKABLE QAction                *pageAction(QWebPage::WebAction action) const;
    Q_INVOKABLE QPainter::RenderHints   renderHints() const;
    Q_INVOKABLE QString                 selectedHtml() const;
    Q_INVOKABLE QString                 selectedText() const;
    Q_INVOKABLE void                    setContent(const QByteArray &data, const QString &mimeType = QString(), const QUrl &baseUrl = QUrl());
    Q_INVOKABLE void                    setHtml(const QString &html, const QUrl &baseUrl = QUrl());
    Q_INVOKABLE void                    setPage(QWebPage *page);
    Q_INVOKABLE void                    setRenderHint(QPainter::RenderHint hint, bool enabled = true);
    Q_INVOKABLE void                    setRenderHints(QPainter::RenderHints hints);
    Q_INVOKABLE void                    setTextSizeMultiplier(qreal factor);
    Q_INVOKABLE QWebSettings           *settings() const;
    Q_INVOKABLE void                    setUrl(const QUrl & url);
    Q_INVOKABLE void                    setZoomFactor(qreal factor);
    Q_INVOKABLE qreal                   textSizeMultiplier() const;
    Q_INVOKABLE QString                 title() const;
    Q_INVOKABLE void                    triggerPageAction(QWebPage::WebAction action, bool checked = false);
    Q_INVOKABLE QUrl                    url() const;
    Q_INVOKABLE qreal                   zoomFactor() const;

  // Reimplemented Public Functions.
    Q_INVOKABLE bool                    event(QEvent * e);
    Q_INVOKABLE QVariant                inputMethodQuery(Qt::InputMethodQuery property) const;
    Q_INVOKABLE QSize                   sizeHint() const;

  public Q_SLOTS:
    Q_INVOKABLE void                    back();
    Q_INVOKABLE void                    forward();
    Q_INVOKABLE void                    print(QPrinter * printer) const;
    Q_INVOKABLE void                    reload();
    Q_INVOKABLE void                    stop();

  signals:
    void    iconChanged();
    void    linkClicked(const QUrl & url);
    void    loadFinished(bool ok);
    void    loadProgress(int progress);
    void    loadStarted();
    void    selectionChanged();
    void    statusBarMessage(const QString & text);
    void    titleChanged(const QString & title);
    void    urlChanged(const QUrl & url);

};

#endif
