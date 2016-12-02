/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBPAGEPROTO_H__
#define __QWEBPAGEPROTO_H__

#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPalette>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QtScript>
#include <QUndoStack>
#include <QVariant>
#include <QWebFrame>
#include <QWebPage>
#include <QWebPluginFactory>
#include <QWebHistory>
#include <QWebSettings>
#include <QWidget>

Q_DECLARE_METATYPE(QWebPage*)
//Q_DECLARE_METATYPE(QWebPage) // Is private in qwebpage.h

Q_DECLARE_METATYPE(class QWebPage::ChooseMultipleFilesExtensionOption)
Q_DECLARE_METATYPE(class QWebPage::ChooseMultipleFilesExtensionReturn)
Q_DECLARE_METATYPE(class QWebPage::ErrorPageExtensionOption)
Q_DECLARE_METATYPE(class QWebPage::ErrorPageExtensionReturn)
Q_DECLARE_METATYPE(class QWebPage::ExtensionOption)
Q_DECLARE_METATYPE(class QWebPage::ExtensionReturn)
Q_DECLARE_METATYPE(class QWebPage::ViewportAttributes)

Q_DECLARE_METATYPE(enum QWebPage::ErrorDomain)
Q_DECLARE_METATYPE(enum QWebPage::Extension)
Q_DECLARE_METATYPE(enum QWebPage::Feature)
Q_DECLARE_METATYPE(enum QWebPage::FindFlag)
Q_DECLARE_METATYPE(enum QWebPage::LinkDelegationPolicy)
Q_DECLARE_METATYPE(enum QWebPage::NavigationType)
Q_DECLARE_METATYPE(enum QWebPage::PermissionPolicy)
#if QT_VERSION >= 0x050000
Q_DECLARE_METATYPE(enum QWebPage::VisibilityState)
#endif
Q_DECLARE_METATYPE(enum QWebPage::WebAction)
Q_DECLARE_METATYPE(enum QWebPage::WebWindowType)

void setupQWebPageProto(QScriptEngine *engine);
QScriptValue constructQWebPage(QScriptContext *context, QScriptEngine *engine);

class QWebPageProto : public QObject, public QScriptable
{
  Q_OBJECT

  Q_PROPERTY (bool contentEditable                                  READ isContentEditable          WRITE setContentEditable)
  Q_PROPERTY (bool forwardUnsupportedContent                        READ forwardUnsupportedContent  WRITE setForwardUnsupportedContent)
  Q_PROPERTY (const bool hasSelection                               READ hasSelection)
  Q_PROPERTY (QWebPage::LinkDelegationPolicy linkDelegationPolicy   READ linkDelegationPolicy       WRITE setLinkDelegationPolicy)
  Q_PROPERTY (const bool modified                                   READ isModified)
  Q_PROPERTY (QPalette palette                                      READ palette                    WRITE setPalette)
  Q_PROPERTY (QSize preferredContentsSize                           READ preferredContentsSize      WRITE setPreferredContentsSize)
  Q_PROPERTY (const QString selectedHtml                            READ selectedHtml)
  Q_PROPERTY (const QString selectedText                            READ selectedText)
  Q_PROPERTY (QSize viewportSize                                    READ viewportSize               WRITE setViewportSize)
#if QT_VERSION >= 0x050000
  Q_PROPERTY (QWebPage::VisibilityState visibilityState             READ visibilityState            WRITE setVisibilityState)
#endif

  public:
    QWebPageProto(QObject *parent);
    ~QWebPageProto();

    Q_INVOKABLE QAction                        *action(QWebPage::WebAction action) const;
    Q_INVOKABLE quint64                         bytesReceived() const;
    Q_INVOKABLE QMenu                          *createStandardContextMenu();
    Q_INVOKABLE QWebFrame                      *currentFrame() const;
    Q_INVOKABLE virtual bool                    extension(QWebPage::Extension extension, const QWebPage::ExtensionOption * option = 0, QWebPage::ExtensionReturn * output = 0);
    Q_INVOKABLE bool                            findText(const QString & subString, QWebPage::FindFlags options = 0);
    Q_INVOKABLE bool                            focusNextPrevChild(bool next);
    Q_INVOKABLE bool                            forwardUnsupportedContent() const;
    Q_INVOKABLE QWebFrame                      *frameAt(const QPoint & pos) const;
    Q_INVOKABLE bool                            hasSelection() const;
    Q_INVOKABLE QWebHistory                    *history() const;
    Q_INVOKABLE QVariant                        inputMethodQuery(Qt::InputMethodQuery property) const;
    Q_INVOKABLE bool                            isContentEditable() const;
    Q_INVOKABLE bool                            isModified() const;
    Q_INVOKABLE QWebPage::LinkDelegationPolicy  linkDelegationPolicy() const;
    Q_INVOKABLE QWebFrame                      *mainFrame() const;
    Q_INVOKABLE QNetworkAccessManager          *networkAccessManager() const;
    Q_INVOKABLE QPalette                        palette() const;
    Q_INVOKABLE QWebPluginFactory              *pluginFactory() const;
    Q_INVOKABLE QSize                           preferredContentsSize() const;
    Q_INVOKABLE QString                         selectedHtml() const;
    Q_INVOKABLE QString                         selectedText() const;
    Q_INVOKABLE void                            setActualVisibleContentRect(const QRect & rect) const;
    Q_INVOKABLE void                            setContentEditable(bool editable);
    Q_INVOKABLE void                            setFeaturePermission(QWebFrame * frame, QWebPage::Feature feature, QWebPage::PermissionPolicy policy);
    Q_INVOKABLE void                            setForwardUnsupportedContent(bool forward);
    Q_INVOKABLE void                            setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy);
    Q_INVOKABLE void                            setNetworkAccessManager(QNetworkAccessManager * manager);
    Q_INVOKABLE void                            setPalette(const QPalette & palette);
    Q_INVOKABLE void                            setPluginFactory(QWebPluginFactory * factory);
    Q_INVOKABLE void                            setPreferredContentsSize(const QSize & size) const;
    Q_INVOKABLE void                            setView(QWidget * view);
    Q_INVOKABLE void                            setViewportSize(const QSize & size) const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE void                            setVisibilityState(QWebPage::VisibilityState);
#endif
    Q_INVOKABLE QWebSettings                   *settings() const;
    Q_INVOKABLE virtual bool                    shouldInterruptJavaScript();
    Q_INVOKABLE QStringList                     supportedContentTypes() const;
    Q_INVOKABLE bool                            supportsContentType(const QString & mimeType) const;
    Q_INVOKABLE virtual bool                    supportsExtension(QWebPage::Extension extension) const;
    Q_INVOKABLE bool                            swallowContextMenuEvent(QContextMenuEvent * event);
    Q_INVOKABLE quint64                         totalBytes() const;
    Q_INVOKABLE virtual void                    triggerAction(QWebPage::WebAction action, bool checked = false);
    Q_INVOKABLE QUndoStack                     *undoStack() const;
    Q_INVOKABLE void                            updatePositionDependentActions(const QPoint & pos);
    Q_INVOKABLE QWidget                        *view() const;
    Q_INVOKABLE QWebPage::ViewportAttributes    viewportAttributesForSize(const QSize & availableSize) const;
    Q_INVOKABLE QSize                           viewportSize() const;
#if QT_VERSION >= 0x050000
    Q_INVOKABLE QWebPage::VisibilityState       visibilityState() const;
#endif

  // Reimplemented Public Functions
    Q_INVOKABLE bool                            event(QEvent * ev);

  signals:
    void    applicationCacheQuotaExceeded(QWebSecurityOrigin * origin, quint64 defaultOriginQuota, quint64 totalSpaceNeeded);
    void    contentsChanged();
    void    databaseQuotaExceeded(QWebFrame * frame, QString databaseName);
    void    downloadRequested(const QNetworkRequest & request);
    void    featurePermissionRequestCanceled(QWebFrame * frame, QWebPage::Feature feature);
    void    featurePermissionRequested(QWebFrame * frame, QWebPage::Feature feature);
    void    frameCreated(QWebFrame * frame);
    void    geometryChangeRequested(const QRect & geom);
    void    linkClicked(const QUrl & url);
    void    linkHovered(const QString & link, const QString & title, const QString & textContent);
    void    loadFinished(bool ok);
    void    loadProgress(int progress);
    void    loadStarted();
    void    menuBarVisibilityChangeRequested(bool visible);
    void    microFocusChanged();
    void    printRequested(QWebFrame * frame);
    void    repaintRequested(const QRect & dirtyRect);
    void    restoreFrameStateRequested(QWebFrame * frame);
    void    saveFrameStateRequested(QWebFrame * frame, QWebHistoryItem * item);
    void    scrollRequested(int dx, int dy, const QRect & rectToScroll);
    void    selectionChanged();
    void    statusBarMessage(const QString & text);
    void    statusBarVisibilityChangeRequested(bool visible);
    void    toolBarVisibilityChangeRequested(bool visible);
    void    unsupportedContent(QNetworkReply * reply);
    void    viewportChangeRequested();
    void    windowCloseRequested();

};

#endif
