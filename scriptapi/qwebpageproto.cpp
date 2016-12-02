/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which(including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebpageproto.h"

#include <QAction>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QPalette>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QUndoStack>
#include <QVariant>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebPage>
#include <QWebPluginFactory>
#include <QWebSettings>
#include <QWidget>

QScriptValue ErrorDomainToScriptValue(QScriptEngine *engine, const QWebPage::ErrorDomain &item)
{
  return engine->newVariant(item);
}
void ErrorDomainFromScriptValue(const QScriptValue &obj, QWebPage::ErrorDomain &item)
{
  item = (QWebPage::ErrorDomain)obj.toInt32();
}

QScriptValue ExtensionToScriptValue(QScriptEngine *engine, const QWebPage::Extension &item)
{
  return engine->newVariant(item);
}
void ExtensionFromScriptValue(const QScriptValue &obj, QWebPage::Extension &item)
{
  item = (QWebPage::Extension)obj.toInt32();
}

QScriptValue FeatureToScriptValue(QScriptEngine *engine, const QWebPage::Feature &item)
{
  return engine->newVariant(item);
}
void FeatureFromScriptValue(const QScriptValue &obj, QWebPage::Feature &item)
{
  item = (QWebPage::Feature)obj.toInt32();
}

QScriptValue FindFlagToScriptValue(QScriptEngine *engine, const QWebPage::FindFlag &item)
{
  return engine->newVariant(item);
}
void FindFlagFromScriptValue(const QScriptValue &obj, QWebPage::FindFlag &item)
{
  item = (QWebPage::FindFlag)obj.toInt32();
}

QScriptValue LinkDelegationPolicyToScriptValue(QScriptEngine *engine, const QWebPage::LinkDelegationPolicy &item)
{
  return engine->newVariant(item);
}
void LinkDelegationPolicyFromScriptValue(const QScriptValue &obj, QWebPage::LinkDelegationPolicy &item)
{
  item = (QWebPage::LinkDelegationPolicy)obj.toInt32();
}

QScriptValue NavigationTypeToScriptValue(QScriptEngine *engine, const QWebPage::NavigationType &item)
{
  return engine->newVariant(item);
}
void NavigationTypeFromScriptValue(const QScriptValue &obj, QWebPage::NavigationType &item)
{
  item = (QWebPage::NavigationType)obj.toInt32();
}

QScriptValue PermissionPolicyToScriptValue(QScriptEngine *engine, const QWebPage::PermissionPolicy &item)
{
  return engine->newVariant(item);
}
void PermissionPolicyFromScriptValue(const QScriptValue &obj, QWebPage::PermissionPolicy &item)
{
  item = (QWebPage::PermissionPolicy)obj.toInt32();
}

#if QT_VERSION >= 0x050000
QScriptValue VisibilityStateToScriptValue(QScriptEngine *engine, const QWebPage::VisibilityState &item)
{
  return engine->newVariant(item);
}
void VisibilityStateFromScriptValue(const QScriptValue &obj, QWebPage::VisibilityState &item)
{
  item = (QWebPage::VisibilityState)obj.toInt32();
}
#endif

QScriptValue WebActionToScriptValue(QScriptEngine *engine, const QWebPage::WebAction &item)
{
  return engine->newVariant(item);
}
void WebActionFromScriptValue(const QScriptValue &obj, QWebPage::WebAction &item)
{
  item = (QWebPage::WebAction)obj.toInt32();
}

QScriptValue WebWindowTypeToScriptValue(QScriptEngine *engine, const QWebPage::WebWindowType &item)
{
  return engine->newVariant(item);
}
void WebWindowTypeFromScriptValue(const QScriptValue &obj, QWebPage::WebWindowType &item)
{
  item = (QWebPage::WebWindowType)obj.toInt32();
}

QScriptValue QWebPagetoScriptValue(QScriptEngine *engine, QWebPage* const &item)
{
  return engine->newQObject(item);
}
void QWebPagefromScriptValue(const QScriptValue &obj, QWebPage* &item)
{
  item = qobject_cast<QWebPage*>(obj.toQObject());
}

void setupQWebPageProto(QScriptEngine *engine)
{
  qScriptRegisterMetaType(engine, QWebPagetoScriptValue, QWebPagefromScriptValue);
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebPageProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebPage*>(), proto);
  // Not allowed. Is private in qwebpage.h
  //engine->setDefaultPrototype(qMetaTypeId<QWebPage>(), proto);

  QScriptValue constructor = engine->newFunction(constructQWebPage,
                                                 proto);
  engine->globalObject().setProperty("QWebPage",  constructor);

  qScriptRegisterMetaType(engine, ErrorDomainToScriptValue, ErrorDomainFromScriptValue);
  constructor.setProperty("QtNetwork", QScriptValue(engine, QWebPage::QtNetwork), permanent);
  constructor.setProperty("Http", QScriptValue(engine, QWebPage::Http), permanent);
  constructor.setProperty("WebKit", QScriptValue(engine, QWebPage::WebKit), permanent);

  qScriptRegisterMetaType(engine, ExtensionToScriptValue, ExtensionFromScriptValue);
  constructor.setProperty("ChooseMultipleFilesExtension", QScriptValue(engine, QWebPage::ChooseMultipleFilesExtension), permanent);
  constructor.setProperty("ErrorPageExtension", QScriptValue(engine, QWebPage::ErrorPageExtension), permanent);

  qScriptRegisterMetaType(engine, FeatureToScriptValue, FeatureFromScriptValue);
  constructor.setProperty("Notifications", QScriptValue(engine, QWebPage::Notifications), permanent);
  constructor.setProperty("Geolocation", QScriptValue(engine, QWebPage::Geolocation), permanent);

  qScriptRegisterMetaType(engine, FindFlagToScriptValue, FindFlagFromScriptValue);
  constructor.setProperty("FindBackward", QScriptValue(engine, QWebPage::FindBackward), permanent);
  constructor.setProperty("FindCaseSensitively", QScriptValue(engine, QWebPage::FindCaseSensitively), permanent);
  constructor.setProperty("FindWrapsAroundDocument", QScriptValue(engine, QWebPage::FindWrapsAroundDocument), permanent);
  constructor.setProperty("HighlightAllOccurrences", QScriptValue(engine, QWebPage::HighlightAllOccurrences), permanent);
#if QT_VERSION >= 0x050000
  constructor.setProperty("FindAtWordBeginningsOnly", QScriptValue(engine, QWebPage::FindAtWordBeginningsOnly), permanent);
  constructor.setProperty("TreatMedialCapitalAsWordBeginning", QScriptValue(engine, QWebPage::TreatMedialCapitalAsWordBeginning), permanent);
  constructor.setProperty("FindBeginsInSelection", QScriptValue(engine, QWebPage::FindBeginsInSelection), permanent);
#endif

  qScriptRegisterMetaType(engine, LinkDelegationPolicyToScriptValue, LinkDelegationPolicyFromScriptValue);
  constructor.setProperty("DontDelegateLinks", QScriptValue(engine, QWebPage::DontDelegateLinks), permanent);
  constructor.setProperty("DelegateExternalLinks", QScriptValue(engine, QWebPage::DelegateExternalLinks), permanent);
  constructor.setProperty("DelegateAllLinks", QScriptValue(engine, QWebPage::DelegateAllLinks), permanent);

  qScriptRegisterMetaType(engine, NavigationTypeToScriptValue, NavigationTypeFromScriptValue);
  constructor.setProperty("NavigationTypeLinkClicked", QScriptValue(engine, QWebPage::NavigationTypeLinkClicked), permanent);
  constructor.setProperty("NavigationTypeFormSubmitted", QScriptValue(engine, QWebPage::NavigationTypeFormSubmitted), permanent);
  constructor.setProperty("NavigationTypeBackOrForward", QScriptValue(engine, QWebPage::NavigationTypeBackOrForward), permanent);
  constructor.setProperty("NavigationTypeReload", QScriptValue(engine, QWebPage::NavigationTypeReload), permanent);
  constructor.setProperty("NavigationTypeFormResubmitted", QScriptValue(engine, QWebPage::NavigationTypeFormResubmitted), permanent);
  constructor.setProperty("NavigationTypeOther", QScriptValue(engine, QWebPage::NavigationTypeOther), permanent);

  qScriptRegisterMetaType(engine, PermissionPolicyToScriptValue, PermissionPolicyFromScriptValue);
  constructor.setProperty("PermissionUnknown", QScriptValue(engine, QWebPage::PermissionUnknown), permanent);
  constructor.setProperty("PermissionGrantedByUser", QScriptValue(engine, QWebPage::PermissionGrantedByUser), permanent);
  constructor.setProperty("PermissionDeniedByUser", QScriptValue(engine, QWebPage::PermissionDeniedByUser), permanent);

#if QT_VERSION >= 0x050000
  qScriptRegisterMetaType(engine, VisibilityStateToScriptValue, VisibilityStateFromScriptValue);
  constructor.setProperty("VisibilityStateVisible", QScriptValue(engine, QWebPage::VisibilityStateVisible), permanent);
  constructor.setProperty("VisibilityStateHidden", QScriptValue(engine, QWebPage::VisibilityStateHidden), permanent);
  constructor.setProperty("VisibilityStatePrerender", QScriptValue(engine, QWebPage::VisibilityStatePrerender), permanent);
  constructor.setProperty("VisibilityStateUnloaded", QScriptValue(engine, QWebPage::VisibilityStateUnloaded), permanent);
#endif

  qScriptRegisterMetaType(engine, WebActionToScriptValue, WebActionFromScriptValue);
  constructor.setProperty("NoWebAction", QScriptValue(engine, QWebPage::NoWebAction), permanent);
  constructor.setProperty("OpenLink", QScriptValue(engine, QWebPage::OpenLink), permanent);
  constructor.setProperty("OpenLinkInNewWindow", QScriptValue(engine, QWebPage::OpenLinkInNewWindow), permanent);
#if QT_VERSION >= 0x050000
  constructor.setProperty("OpenLinkInThisWindow", QScriptValue(engine, QWebPage::OpenLinkInThisWindow), permanent);
#endif
  constructor.setProperty("OpenFrameInNewWindow", QScriptValue(engine, QWebPage::OpenFrameInNewWindow), permanent);
  constructor.setProperty("DownloadLinkToDisk", QScriptValue(engine, QWebPage::DownloadLinkToDisk), permanent);
  constructor.setProperty("CopyLinkToClipboard", QScriptValue(engine, QWebPage::CopyLinkToClipboard), permanent);
  constructor.setProperty("OpenImageInNewWindow", QScriptValue(engine, QWebPage::OpenImageInNewWindow), permanent);
  constructor.setProperty("DownloadImageToDisk", QScriptValue(engine, QWebPage::DownloadImageToDisk), permanent);
  constructor.setProperty("CopyImageToClipboard", QScriptValue(engine, QWebPage::CopyImageToClipboard), permanent);
  constructor.setProperty("CopyImageUrlToClipboard", QScriptValue(engine, QWebPage::CopyImageUrlToClipboard), permanent);
  constructor.setProperty("Back", QScriptValue(engine, QWebPage::Back), permanent);
  constructor.setProperty("Forward", QScriptValue(engine, QWebPage::Forward), permanent);
  constructor.setProperty("Stop", QScriptValue(engine, QWebPage::Stop), permanent);
  constructor.setProperty("StopScheduledPageRefresh", QScriptValue(engine, QWebPage::StopScheduledPageRefresh), permanent);
  constructor.setProperty("Reload", QScriptValue(engine, QWebPage::Reload), permanent);
  constructor.setProperty("ReloadAndBypassCache", QScriptValue(engine, QWebPage::ReloadAndBypassCache), permanent);
  constructor.setProperty("Cut", QScriptValue(engine, QWebPage::Cut), permanent);
  constructor.setProperty("Copy", QScriptValue(engine, QWebPage::Copy), permanent);
  constructor.setProperty("Paste", QScriptValue(engine, QWebPage::Paste), permanent);
  constructor.setProperty("Undo", QScriptValue(engine, QWebPage::Undo), permanent);
  constructor.setProperty("Redo", QScriptValue(engine, QWebPage::Redo), permanent);
  constructor.setProperty("MoveToNextChar", QScriptValue(engine, QWebPage::MoveToNextChar), permanent);
  constructor.setProperty("MoveToPreviousChar", QScriptValue(engine, QWebPage::MoveToPreviousChar), permanent);
  constructor.setProperty("MoveToNextWord", QScriptValue(engine, QWebPage::MoveToNextWord), permanent);
  constructor.setProperty("MoveToPreviousWord", QScriptValue(engine, QWebPage::MoveToPreviousWord), permanent);
  constructor.setProperty("MoveToNextLine", QScriptValue(engine, QWebPage::MoveToNextLine), permanent);
  constructor.setProperty("MoveToPreviousLine", QScriptValue(engine, QWebPage::MoveToPreviousLine), permanent);
  constructor.setProperty("MoveToStartOfLine", QScriptValue(engine, QWebPage::MoveToStartOfLine), permanent);
  constructor.setProperty("MoveToEndOfLine", QScriptValue(engine, QWebPage::MoveToEndOfLine), permanent);
  constructor.setProperty("MoveToStartOfBlock", QScriptValue(engine, QWebPage::MoveToStartOfBlock), permanent);
  constructor.setProperty("MoveToEndOfBlock", QScriptValue(engine, QWebPage::MoveToEndOfBlock), permanent);
  constructor.setProperty("MoveToStartOfDocument", QScriptValue(engine, QWebPage::MoveToStartOfDocument), permanent);
  constructor.setProperty("MoveToEndOfDocument", QScriptValue(engine, QWebPage::MoveToEndOfDocument), permanent);
  constructor.setProperty("SelectNextChar", QScriptValue(engine, QWebPage::SelectNextChar), permanent);
  constructor.setProperty("SelectPreviousChar", QScriptValue(engine, QWebPage::SelectPreviousChar), permanent);
  constructor.setProperty("SelectNextWord", QScriptValue(engine, QWebPage::SelectNextWord), permanent);
  constructor.setProperty("SelectPreviousWord", QScriptValue(engine, QWebPage::SelectPreviousWord), permanent);
  constructor.setProperty("SelectNextLine", QScriptValue(engine, QWebPage::SelectNextLine), permanent);
  constructor.setProperty("SelectPreviousLine", QScriptValue(engine, QWebPage::SelectPreviousLine), permanent);
  constructor.setProperty("SelectStartOfLine", QScriptValue(engine, QWebPage::SelectStartOfLine), permanent);
  constructor.setProperty("SelectEndOfLine", QScriptValue(engine, QWebPage::SelectEndOfLine), permanent);
  constructor.setProperty("SelectStartOfBlock", QScriptValue(engine, QWebPage::SelectStartOfBlock), permanent);
  constructor.setProperty("SelectEndOfBlock", QScriptValue(engine, QWebPage::SelectEndOfBlock), permanent);
  constructor.setProperty("SelectStartOfDocument", QScriptValue(engine, QWebPage::SelectStartOfDocument), permanent);
  constructor.setProperty("SelectEndOfDocument", QScriptValue(engine, QWebPage::SelectEndOfDocument), permanent);
  constructor.setProperty("DeleteStartOfWord", QScriptValue(engine, QWebPage::DeleteStartOfWord), permanent);
  constructor.setProperty("DeleteEndOfWord", QScriptValue(engine, QWebPage::DeleteEndOfWord), permanent);
  constructor.setProperty("SetTextDirectionDefault", QScriptValue(engine, QWebPage::SetTextDirectionDefault), permanent);
  constructor.setProperty("SetTextDirectionLeftToRight", QScriptValue(engine, QWebPage::SetTextDirectionLeftToRight), permanent);
  constructor.setProperty("SetTextDirectionRightToLeft", QScriptValue(engine, QWebPage::SetTextDirectionRightToLeft), permanent);
  constructor.setProperty("ToggleBold", QScriptValue(engine, QWebPage::ToggleBold), permanent);
  constructor.setProperty("ToggleItalic", QScriptValue(engine, QWebPage::ToggleItalic), permanent);
  constructor.setProperty("ToggleUnderline", QScriptValue(engine, QWebPage::ToggleUnderline), permanent);
  constructor.setProperty("InspectElement", QScriptValue(engine, QWebPage::InspectElement), permanent);
  constructor.setProperty("InsertParagraphSeparator", QScriptValue(engine, QWebPage::InsertParagraphSeparator), permanent);
  constructor.setProperty("InsertLineSeparator", QScriptValue(engine, QWebPage::InsertLineSeparator), permanent);
  constructor.setProperty("SelectAll", QScriptValue(engine, QWebPage::SelectAll), permanent);
  constructor.setProperty("PasteAndMatchStyle", QScriptValue(engine, QWebPage::PasteAndMatchStyle), permanent);
  constructor.setProperty("RemoveFormat", QScriptValue(engine, QWebPage::RemoveFormat), permanent);
  constructor.setProperty("ToggleStrikethrough", QScriptValue(engine, QWebPage::ToggleStrikethrough), permanent);
  constructor.setProperty("ToggleSubscript", QScriptValue(engine, QWebPage::ToggleSubscript), permanent);
  constructor.setProperty("ToggleSuperscript", QScriptValue(engine, QWebPage::ToggleSuperscript), permanent);
  constructor.setProperty("InsertUnorderedList", QScriptValue(engine, QWebPage::InsertUnorderedList), permanent);
  constructor.setProperty("InsertOrderedList", QScriptValue(engine, QWebPage::InsertOrderedList), permanent);
  constructor.setProperty("Indent", QScriptValue(engine, QWebPage::Indent), permanent);
  constructor.setProperty("Outdent", QScriptValue(engine, QWebPage::Outdent), permanent);
  constructor.setProperty("AlignCenter", QScriptValue(engine, QWebPage::AlignCenter), permanent);
  constructor.setProperty("AlignJustified", QScriptValue(engine, QWebPage::AlignJustified), permanent);
  constructor.setProperty("AlignLeft", QScriptValue(engine, QWebPage::AlignLeft), permanent);
  constructor.setProperty("AlignRight", QScriptValue(engine, QWebPage::AlignRight), permanent);
#if QT_VERSION >= 0x050000
  constructor.setProperty("DownloadMediaToDisk", QScriptValue(engine, QWebPage::DownloadMediaToDisk), permanent);
  constructor.setProperty("CopyMediaUrlToClipboard", QScriptValue(engine, QWebPage::CopyMediaUrlToClipboard), permanent);
  constructor.setProperty("ToggleMediaControls", QScriptValue(engine, QWebPage::ToggleMediaControls), permanent);
  constructor.setProperty("ToggleMediaLoop", QScriptValue(engine, QWebPage::ToggleMediaLoop), permanent);
  constructor.setProperty("ToggleMediaPlayPause", QScriptValue(engine, QWebPage::ToggleMediaPlayPause), permanent);
  constructor.setProperty("ToggleMediaMute", QScriptValue(engine, QWebPage::ToggleMediaMute), permanent);
  constructor.setProperty("ToggleVideoFullscreen", QScriptValue(engine, QWebPage::ToggleVideoFullscreen), permanent);
#endif

  qScriptRegisterMetaType(engine, WebWindowTypeToScriptValue, WebWindowTypeFromScriptValue);
  constructor.setProperty("WebBrowserWindow", QScriptValue(engine, QWebPage::WebBrowserWindow), permanent);
  constructor.setProperty("WebModalDialog", QScriptValue(engine, QWebPage::WebModalDialog), permanent);
}

QScriptValue constructQWebPage(QScriptContext * context,
                                    QScriptEngine  *engine)
{
  QWebPage *obj = 0;
  if (context->argumentCount() == 1)
    obj = new QWebPage(qobject_cast<QWidget*>(context->argument(0).toQObject()));
  else
    obj = new QWebPage();
  return engine->toScriptValue(obj);
}

QWebPageProto::QWebPageProto(QObject *parent)
    : QObject(parent)
{
}
QWebPageProto::~QWebPageProto()
{
}

QAction* QWebPageProto::action(QWebPage::WebAction action) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->action(action);
  return 0;
}

quint64 QWebPageProto::bytesReceived() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->bytesReceived();
  return 0;
}

QMenu* QWebPageProto::createStandardContextMenu()
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->createStandardContextMenu();
  return 0;
}

QWebFrame* QWebPageProto::currentFrame() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->currentFrame();
  return 0;
}

bool QWebPageProto::extension(QWebPage::Extension extension, const QWebPage::ExtensionOption * option, QWebPage::ExtensionReturn * output)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->extension(extension, option, output);
  return false;
}

bool QWebPageProto::findText(const QString & subString, QWebPage::FindFlags options)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->findText(subString, options);
  return false;
}

bool QWebPageProto::focusNextPrevChild(bool next)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->focusNextPrevChild(next);
  return false;
}

bool QWebPageProto::forwardUnsupportedContent() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->forwardUnsupportedContent();
  return false;
}

QWebFrame* QWebPageProto::frameAt(const QPoint & pos) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->frameAt(pos);
  return 0;
}

bool QWebPageProto::hasSelection() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->hasSelection();
  return false;
}

QWebHistory* QWebPageProto::history() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->history();
  return 0;
}

QVariant QWebPageProto::inputMethodQuery(Qt::InputMethodQuery property) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->inputMethodQuery(property);
  return QVariant();
}

bool QWebPageProto::isContentEditable() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->isContentEditable();
  return false;
}

bool QWebPageProto::isModified() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->isModified();
  return false;
}

QWebPage::LinkDelegationPolicy QWebPageProto::linkDelegationPolicy() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->linkDelegationPolicy();
  return QWebPage::LinkDelegationPolicy();
}

QWebFrame* QWebPageProto::mainFrame() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->mainFrame();
  return 0;
}

QNetworkAccessManager* QWebPageProto::networkAccessManager() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->networkAccessManager();
  return 0;
}

QPalette QWebPageProto::palette() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->palette();
  return QPalette();
}

QWebPluginFactory* QWebPageProto::pluginFactory() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->pluginFactory();
  return 0;
}

QSize QWebPageProto::preferredContentsSize() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->preferredContentsSize();
  return QSize();
}

QString QWebPageProto::selectedHtml() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->selectedHtml();
  return QString();
}

QString QWebPageProto::selectedText() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->selectedText();
  return QString();
}

void QWebPageProto::setActualVisibleContentRect(const QRect & rect) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setActualVisibleContentRect(rect);
}

void QWebPageProto::setContentEditable(bool editable)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setContentEditable(editable);
}

void QWebPageProto::setFeaturePermission(QWebFrame * frame, QWebPage::Feature feature, QWebPage::PermissionPolicy policy)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setFeaturePermission(frame, feature, policy);
}

void QWebPageProto::setForwardUnsupportedContent(bool forward)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setForwardUnsupportedContent(forward);
}

void QWebPageProto::setLinkDelegationPolicy(QWebPage::LinkDelegationPolicy policy)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setLinkDelegationPolicy(policy);
}

void QWebPageProto::setNetworkAccessManager(QNetworkAccessManager * manager)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setNetworkAccessManager(manager);
}

void QWebPageProto::setPalette(const QPalette & palette)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPalette(palette);
}

void QWebPageProto::setPluginFactory(QWebPluginFactory * factory)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPluginFactory(factory);
}

void QWebPageProto::setPreferredContentsSize(const QSize & size) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setPreferredContentsSize(size);
}

void QWebPageProto::setView(QWidget * view)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setView(view);
}

void QWebPageProto::setViewportSize(const QSize & size) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setViewportSize(size);
}

#if QT_VERSION >= 0x050000
void QWebPageProto::setVisibilityState(QWebPage::VisibilityState state)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->setVisibilityState(state);
}
#endif

QWebSettings* QWebPageProto::settings() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->settings();
  return 0;
}

bool QWebPageProto::shouldInterruptJavaScript()
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->shouldInterruptJavaScript();
  return false;
}

QStringList QWebPageProto::supportedContentTypes() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportedContentTypes();
  return QStringList();
}

bool QWebPageProto::supportsContentType(const QString & mimeType) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportsContentType(mimeType);
  return false;
}

bool QWebPageProto::supportsExtension(QWebPage::Extension extension) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->supportsExtension(extension);
  return false;
}

bool QWebPageProto::swallowContextMenuEvent(QContextMenuEvent * event)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->swallowContextMenuEvent(event);
  return false;
}

quint64 QWebPageProto::totalBytes() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->totalBytes();
  return 0;
}

void QWebPageProto::triggerAction(QWebPage::WebAction action, bool checked)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->triggerAction(action, checked);
}

QUndoStack* QWebPageProto::undoStack() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->undoStack();
  return 0;
}

void QWebPageProto::updatePositionDependentActions(const QPoint & pos)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    item->updatePositionDependentActions(pos);
}

QWidget* QWebPageProto::view() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->view();
  return 0;
}

QWebPage::ViewportAttributes QWebPageProto::viewportAttributesForSize(const QSize & availableSize) const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->viewportAttributesForSize(availableSize);
  return QWebPage::ViewportAttributes();
}

QSize QWebPageProto::viewportSize() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->viewportSize();
  return QSize();
}

#if QT_VERSION >= 0x050000
QWebPage::VisibilityState QWebPageProto::visibilityState() const
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->visibilityState();
  return QWebPage::VisibilityStateVisible; // don't know the best default
}
#endif

// Reimplemented Public Functions
bool QWebPageProto::event(QEvent * ev)
{
  QWebPage *item = qscriptvalue_cast<QWebPage*>(thisObject());
  if (item)
    return item->event(ev);
  return false;
}
