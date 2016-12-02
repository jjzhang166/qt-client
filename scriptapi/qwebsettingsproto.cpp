/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qwebsettingsproto.h"

#if QT_VERSION < 0x050000
void setupQWebSettingsProto(QScriptEngine *engine)
{
  Q_UNUSED(engine);
}
#else
QScriptValue FontFamilyToScriptValue(QScriptEngine *engine, const QWebSettings::FontFamily &item)
{
  return engine->newVariant(item);
}
void FontFamilyFromScriptValue(const QScriptValue &obj, QWebSettings::FontFamily &item)
{
  item = (QWebSettings::FontFamily)obj.toInt32();
}

QScriptValue FontSizeToScriptValue(QScriptEngine *engine, const QWebSettings::FontSize &item)
{
  return engine->newVariant(item);
}
void FontSizeFromScriptValue(const QScriptValue &obj, QWebSettings::FontSize &item)
{
  item = (QWebSettings::FontSize)obj.toInt32();
}

QScriptValue ThirdPartyCookiePolicyToScriptValue(QScriptEngine *engine, const QWebSettings::ThirdPartyCookiePolicy &item)
{
  return engine->newVariant(item);
}
void ThirdPartyCookiePolicyFromScriptValue(const QScriptValue &obj, QWebSettings::ThirdPartyCookiePolicy &item)
{
  item = (QWebSettings::ThirdPartyCookiePolicy)obj.toInt32();
}

QScriptValue WebAttributeToScriptValue(QScriptEngine *engine, const QWebSettings::WebAttribute &item)
{
  return engine->newVariant(item);
}
void WebAttributeFromScriptValue(const QScriptValue &obj, QWebSettings::WebAttribute &item)
{
  item = (QWebSettings::WebAttribute)obj.toInt32();
}

QScriptValue WebGraphicToScriptValue(QScriptEngine *engine, const QWebSettings::WebGraphic &item)
{
  return engine->newVariant(item);
}
void WebGraphicFromScriptValue(const QScriptValue &obj, QWebSettings::WebGraphic &item)
{
  item = (QWebSettings::WebGraphic)obj.toInt32();
}

QScriptValue clearIconDatabaseForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  QWebSettings::clearIconDatabase();
  return engine->undefinedValue();
}

QScriptValue clearMemoryCachesForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  QWebSettings::clearMemoryCaches();
  return engine->undefinedValue();
}

QScriptValue enablePersistentStorageForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString path = context->argument(0).toString();
    QWebSettings::enablePersistentStorage(path);
  } else {
    QWebSettings::enablePersistentStorage();
  }
  return engine->undefinedValue();
}

QScriptValue globalSettingsForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::globalSettings());
}

QScriptValue iconDatabasePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::iconDatabasePath());
}

// TODO: Can't seem to get this working. Something is wrong with how we expose QUrl.
/*
QScriptValue iconForUrlForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QUrl url = qscriptvalue_cast<QUrl>(context->argument(0));
    return engine->toScriptValue(QWebSettings::iconForUrl(url));
  } else {
    return engine->undefinedValue();
  }
}
*/

QScriptValue maximumPagesInCacheForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::maximumPagesInCache());
}

QScriptValue offlineStorageDefaultQuotaForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::offlineStorageDefaultQuota());
}

QScriptValue offlineStoragePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::offlineStoragePath());
}

QScriptValue offlineWebApplicationCachePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::offlineWebApplicationCachePath());
}

QScriptValue offlineWebApplicationCacheQuotaForJS(QScriptContext* context, QScriptEngine* engine)
{
  Q_UNUSED(context);
  return engine->toScriptValue(QWebSettings::offlineWebApplicationCacheQuota());
}

QScriptValue setIconDatabasePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString path = context->argument(0).toString();
    QWebSettings::setIconDatabasePath(path);
  }
  return engine->undefinedValue();
}

QScriptValue setMaximumPagesInCacheForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    int pages = context->argument(0).toInteger();
    QWebSettings::setMaximumPagesInCache(pages);
  }
  return engine->undefinedValue();
}

QScriptValue setObjectCacheCapacitiesForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 3) {
    int cacheMinDeadCapacity = context->argument(0).toInteger();
    int cacheMaxDead = context->argument(1).toInteger();
    int totalCapacity = context->argument(2).toInteger();
    QWebSettings::setObjectCacheCapacities(cacheMinDeadCapacity, cacheMaxDead, totalCapacity);
  }
  return engine->undefinedValue();
}

QScriptValue setOfflineStorageDefaultQuotaForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    qint64 maximumSize = context->argument(0).toInteger();
    QWebSettings::setOfflineStorageDefaultQuota(maximumSize);
  }
  return engine->undefinedValue();
}

QScriptValue setOfflineStoragePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString path = context->argument(0).toString();
    QWebSettings::setOfflineStoragePath(path);
  }
  return engine->undefinedValue();
}

QScriptValue setOfflineWebApplicationCachePathForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QString path = context->argument(0).toString();
    QWebSettings::setOfflineWebApplicationCachePath(path);
  }
  return engine->undefinedValue();
}

QScriptValue setOfflineWebApplicationCacheQuotaForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    qint64 maximumSize = context->argument(0).toInteger();
    QWebSettings::setOfflineWebApplicationCacheQuota(maximumSize);
  }
  return engine->undefinedValue();
}

QScriptValue setWebGraphicForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 2) {
    QWebSettings::WebGraphic type = (QWebSettings::WebGraphic)context->argument(0).toInt32();
    QPixmap graphic = qscriptvalue_cast<QPixmap>(context->argument(1));
    QWebSettings::setWebGraphic(type, graphic);
  }
  return engine->undefinedValue();
}

// TODO: Expose QPixmap for this to work.
/*
QScriptValue webGraphicForJS(QScriptContext* context, QScriptEngine* engine)
{
  if (context->argumentCount() == 1) {
    QWebSettings::WebGraphic type = (QWebSettings::WebGraphic)context->argument(0).toInt32();
    return engine->toScriptValue(QWebSettings::webGraphic(type, graphic));
  }
  return engine->undefinedValue();
}
*/

void setupQWebSettingsProto(QScriptEngine *engine)
{
  QScriptValue::PropertyFlags permanent = QScriptValue::ReadOnly | QScriptValue::Undeletable;

  QScriptValue proto = engine->newQObject(new QWebSettingsProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QWebSettings*>(), proto);
  // TODO: QWebSettings is private.
  //engine->setDefaultPrototype(qMetaTypeId<QWebSettings>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQWebSettings, proto);
  engine->globalObject().setProperty("QWebSettings", constructor);

  qScriptRegisterMetaType(engine, FontFamilyToScriptValue, FontFamilyFromScriptValue);
  constructor.setProperty("StandardFont", QScriptValue(engine, QWebSettings::StandardFont), permanent);
  constructor.setProperty("FixedFont", QScriptValue(engine, QWebSettings::FixedFont), permanent);
  constructor.setProperty("SerifFont", QScriptValue(engine, QWebSettings::SerifFont), permanent);
  constructor.setProperty("SansSerifFont", QScriptValue(engine, QWebSettings::SansSerifFont), permanent);
  constructor.setProperty("CursiveFont", QScriptValue(engine, QWebSettings::CursiveFont), permanent);
  constructor.setProperty("FantasyFont", QScriptValue(engine, QWebSettings::FantasyFont), permanent);

  qScriptRegisterMetaType(engine, FontSizeToScriptValue, FontSizeFromScriptValue);
  constructor.setProperty("MinimumFontSize", QScriptValue(engine, QWebSettings::MinimumFontSize), permanent);
  constructor.setProperty("MinimumLogicalFontSize", QScriptValue(engine, QWebSettings::MinimumLogicalFontSize), permanent);
  constructor.setProperty("DefaultFontSize", QScriptValue(engine, QWebSettings::DefaultFontSize), permanent);
  constructor.setProperty("DefaultFixedFontSize", QScriptValue(engine, QWebSettings::DefaultFixedFontSize), permanent);

  qScriptRegisterMetaType(engine, ThirdPartyCookiePolicyToScriptValue, ThirdPartyCookiePolicyFromScriptValue);
  constructor.setProperty("AlwaysAllowThirdPartyCookies", QScriptValue(engine, QWebSettings::AlwaysAllowThirdPartyCookies), permanent);
  constructor.setProperty("AlwaysBlockThirdPartyCookies", QScriptValue(engine, QWebSettings::AlwaysBlockThirdPartyCookies), permanent);
  constructor.setProperty("AllowThirdPartyWithExistingCookies", QScriptValue(engine, QWebSettings::AllowThirdPartyWithExistingCookies), permanent);

  qScriptRegisterMetaType(engine, WebAttributeToScriptValue, WebAttributeFromScriptValue);
  constructor.setProperty("AutoLoadImages", QScriptValue(engine, QWebSettings::AutoLoadImages), permanent);
  constructor.setProperty("DnsPrefetchEnabled", QScriptValue(engine, QWebSettings::DnsPrefetchEnabled), permanent);
  constructor.setProperty("JavascriptEnabled", QScriptValue(engine, QWebSettings::JavascriptEnabled), permanent);
  constructor.setProperty("JavaEnabled", QScriptValue(engine, QWebSettings::JavaEnabled), permanent);
  constructor.setProperty("PluginsEnabled", QScriptValue(engine, QWebSettings::PluginsEnabled), permanent);
  constructor.setProperty("PrivateBrowsingEnabled", QScriptValue(engine, QWebSettings::PrivateBrowsingEnabled), permanent);
  constructor.setProperty("JavascriptCanOpenWindows", QScriptValue(engine, QWebSettings::JavascriptCanOpenWindows), permanent);
  constructor.setProperty("JavascriptCanCloseWindows", QScriptValue(engine, QWebSettings::JavascriptCanCloseWindows), permanent);
  constructor.setProperty("JavascriptCanAccessClipboard", QScriptValue(engine, QWebSettings::JavascriptCanAccessClipboard), permanent);
  constructor.setProperty("DeveloperExtrasEnabled", QScriptValue(engine, QWebSettings::DeveloperExtrasEnabled), permanent);
  constructor.setProperty("SpatialNavigationEnabled", QScriptValue(engine, QWebSettings::SpatialNavigationEnabled), permanent);
  constructor.setProperty("LinksIncludedInFocusChain", QScriptValue(engine, QWebSettings::LinksIncludedInFocusChain), permanent);
  constructor.setProperty("ZoomTextOnly", QScriptValue(engine, QWebSettings::ZoomTextOnly), permanent);
  constructor.setProperty("PrintElementBackgrounds", QScriptValue(engine, QWebSettings::PrintElementBackgrounds), permanent);
  constructor.setProperty("OfflineStorageDatabaseEnabled", QScriptValue(engine, QWebSettings::OfflineStorageDatabaseEnabled), permanent);
  constructor.setProperty("OfflineWebApplicationCacheEnabled", QScriptValue(engine, QWebSettings::OfflineWebApplicationCacheEnabled), permanent);
  constructor.setProperty("LocalStorageEnabled", QScriptValue(engine, QWebSettings::LocalStorageEnabled), permanent);
  constructor.setProperty("LocalStorageDatabaseEnabled", QScriptValue(engine, QWebSettings::LocalStorageDatabaseEnabled), permanent);
  constructor.setProperty("LocalContentCanAccessRemoteUrls", QScriptValue(engine, QWebSettings::LocalContentCanAccessRemoteUrls), permanent);
  constructor.setProperty("LocalContentCanAccessFileUrls", QScriptValue(engine, QWebSettings::LocalContentCanAccessFileUrls), permanent);
  constructor.setProperty("XSSAuditingEnabled", QScriptValue(engine, QWebSettings::XSSAuditingEnabled), permanent);
  constructor.setProperty("AcceleratedCompositingEnabled", QScriptValue(engine, QWebSettings::AcceleratedCompositingEnabled), permanent);
  constructor.setProperty("TiledBackingStoreEnabled", QScriptValue(engine, QWebSettings::TiledBackingStoreEnabled), permanent);
  constructor.setProperty("FrameFlatteningEnabled", QScriptValue(engine, QWebSettings::FrameFlatteningEnabled), permanent);
  constructor.setProperty("SiteSpecificQuirksEnabled", QScriptValue(engine, QWebSettings::SiteSpecificQuirksEnabled), permanent);
  constructor.setProperty("CSSGridLayoutEnabled", QScriptValue(engine, QWebSettings::CSSGridLayoutEnabled), permanent);
  constructor.setProperty("CSSRegionsEnabled", QScriptValue(engine, QWebSettings::CSSRegionsEnabled), permanent);
  constructor.setProperty("ScrollAnimatorEnabled", QScriptValue(engine, QWebSettings::ScrollAnimatorEnabled), permanent);
  constructor.setProperty("CaretBrowsingEnabled", QScriptValue(engine, QWebSettings::CaretBrowsingEnabled), permanent);
  constructor.setProperty("NotificationsEnabled", QScriptValue(engine, QWebSettings::NotificationsEnabled), permanent);
  constructor.setProperty("Accelerated2dCanvasEnabled", QScriptValue(engine, QWebSettings::Accelerated2dCanvasEnabled), permanent);
  constructor.setProperty("WebGLEnabled", QScriptValue(engine, QWebSettings::WebGLEnabled), permanent);
  constructor.setProperty("HyperlinkAuditingEnabled", QScriptValue(engine, QWebSettings::HyperlinkAuditingEnabled), permanent);

  qScriptRegisterMetaType(engine, WebGraphicToScriptValue, WebGraphicFromScriptValue);
  constructor.setProperty("MissingImageGraphic", QScriptValue(engine, QWebSettings::MissingImageGraphic), permanent);
  constructor.setProperty("MissingPluginGraphic", QScriptValue(engine, QWebSettings::MissingPluginGraphic), permanent);
  constructor.setProperty("DefaultFrameIconGraphic", QScriptValue(engine, QWebSettings::DefaultFrameIconGraphic), permanent);
  constructor.setProperty("TextAreaSizeGripCornerGraphic", QScriptValue(engine, QWebSettings::TextAreaSizeGripCornerGraphic), permanent);
  constructor.setProperty("DeleteButtonGraphic", QScriptValue(engine, QWebSettings::DeleteButtonGraphic), permanent);
  constructor.setProperty("InputSpeechButtonGraphic", QScriptValue(engine, QWebSettings::InputSpeechButtonGraphic), permanent);
  constructor.setProperty("SearchCancelButtonGraphic", QScriptValue(engine, QWebSettings::SearchCancelButtonGraphic), permanent);
  constructor.setProperty("SearchCancelButtonPressedGraphic", QScriptValue(engine, QWebSettings::SearchCancelButtonPressedGraphic), permanent);

  QScriptValue clearIconDatabase = engine->newFunction(clearIconDatabaseForJS);
  constructor.setProperty("clearIconDatabase", clearIconDatabase);

  QScriptValue clearMemoryCaches = engine->newFunction(clearMemoryCachesForJS);
  constructor.setProperty("clearMemoryCaches", clearMemoryCaches);

  QScriptValue enablePersistentStorage = engine->newFunction(enablePersistentStorageForJS);
  constructor.setProperty("enablePersistentStorage", enablePersistentStorage);

  QScriptValue globalSettings = engine->newFunction(globalSettingsForJS);
  constructor.setProperty("globalSettings", globalSettings);

  QScriptValue iconDatabasePath = engine->newFunction(iconDatabasePathForJS);
  constructor.setProperty("iconDatabasePath", iconDatabasePath);

  // TODO: Can't seem to get this working. Something is wrong with how we expose QUrl.
  /*
  QScriptValue iconForUrl = engine->newFunction(iconForUrlForJS);
  constructor.setProperty("iconForUrl", iconForUrl);
  */

  QScriptValue maximumPagesInCache = engine->newFunction(maximumPagesInCacheForJS);
  constructor.setProperty("maximumPagesInCache", maximumPagesInCache);

  QScriptValue offlineStorageDefaultQuota = engine->newFunction(offlineStorageDefaultQuotaForJS);
  constructor.setProperty("offlineStorageDefaultQuota", offlineStorageDefaultQuota);

  QScriptValue offlineStoragePath = engine->newFunction(offlineStoragePathForJS);
  constructor.setProperty("offlineStoragePath", offlineStoragePath);

  QScriptValue offlineWebApplicationCachePath = engine->newFunction(offlineWebApplicationCachePathForJS);
  constructor.setProperty("offlineWebApplicationCachePath", offlineWebApplicationCachePath);

  QScriptValue offlineWebApplicationCacheQuota = engine->newFunction(offlineWebApplicationCacheQuotaForJS);
  constructor.setProperty("offlineWebApplicationCacheQuota", offlineWebApplicationCacheQuota);

  QScriptValue setIconDatabasePath = engine->newFunction(setIconDatabasePathForJS);
  constructor.setProperty("setIconDatabasePath", setIconDatabasePath);

  QScriptValue setMaximumPagesInCache = engine->newFunction(setMaximumPagesInCacheForJS);
  constructor.setProperty("setMaximumPagesInCache", setMaximumPagesInCache);

  QScriptValue setObjectCacheCapacities = engine->newFunction(setObjectCacheCapacitiesForJS);
  constructor.setProperty("setObjectCacheCapacities", setObjectCacheCapacities);

  QScriptValue setOfflineStorageDefaultQuota = engine->newFunction(setOfflineStorageDefaultQuotaForJS);
  constructor.setProperty("setOfflineStorageDefaultQuota", setOfflineStorageDefaultQuota);

  QScriptValue setOfflineStoragePath = engine->newFunction(setOfflineStoragePathForJS);
  constructor.setProperty("setOfflineStoragePath", setOfflineStoragePath);

  QScriptValue setOfflineWebApplicationCachePath = engine->newFunction(setOfflineWebApplicationCachePathForJS);
  constructor.setProperty("setOfflineWebApplicationCachePath", setOfflineWebApplicationCachePath);

  QScriptValue setOfflineWebApplicationCacheQuota = engine->newFunction(setOfflineWebApplicationCacheQuotaForJS);
  constructor.setProperty("setOfflineWebApplicationCacheQuota", setOfflineWebApplicationCacheQuota);

  QScriptValue setWebGraphic = engine->newFunction(setWebGraphicForJS);
  constructor.setProperty("setWebGraphic", setWebGraphic);

  // TODO: Expose QPixmap for this to work.
  /*
  QScriptValue webGraphic = engine->newFunction(webGraphicForJS);
  constructor.setProperty("webGraphic", webGraphic);
  */
}

QScriptValue constructQWebSettings(QScriptContext * /*context*/, QScriptEngine  *engine)
{
  QWebSettings *obj = 0;
  // TODO: QWebSettings does not have a constructor.
  //obj = new QWebSettings();
  return engine->toScriptValue(obj);
}

QWebSettingsProto::QWebSettingsProto(QObject *parent)
    : QObject(parent)
{
}

QString QWebSettingsProto::cssMediaType() const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->cssMediaType();
  return QString();
}

QString QWebSettingsProto::defaultTextEncoding() const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->defaultTextEncoding();
  return QString();
}

QString QWebSettingsProto::fontFamily(QWebSettings::FontFamily which) const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->fontFamily(which);
  return QString();
}

int QWebSettingsProto::fontSize(QWebSettings::FontSize type) const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->fontSize(type);
  return 0;
}

QString QWebSettingsProto::localStoragePath() const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->localStoragePath();
  return QString();
}

void QWebSettingsProto::resetAttribute(QWebSettings::WebAttribute attribute)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->resetAttribute(attribute);
}

void QWebSettingsProto::resetFontFamily(QWebSettings::FontFamily which)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->resetFontFamily(which);
}

void QWebSettingsProto::resetFontSize(QWebSettings::FontSize type)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->resetFontSize(type);
}

void QWebSettingsProto::setAttribute(QWebSettings::WebAttribute attribute, bool on)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setAttribute(attribute, on);
}

void QWebSettingsProto::setCSSMediaType(const QString & type)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setCSSMediaType(type);
}

void QWebSettingsProto::setDefaultTextEncoding(const QString & encoding)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setDefaultTextEncoding(encoding);
}

void QWebSettingsProto::setFontFamily(QWebSettings::FontFamily which, const QString & family)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setFontFamily(which, family);
}

void QWebSettingsProto::setFontSize(QWebSettings::FontSize type, int size)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setFontSize(type, size);
}

void QWebSettingsProto::setLocalStoragePath(const QString & path)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setLocalStoragePath(path);
}

void QWebSettingsProto::setThirdPartyCookiePolicy(QWebSettings::ThirdPartyCookiePolicy policy)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setThirdPartyCookiePolicy(policy);
}

void QWebSettingsProto::setUserStyleSheetUrl(const QUrl & location)
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    item->setUserStyleSheetUrl(location);
}

bool QWebSettingsProto::testAttribute(QWebSettings::WebAttribute attribute) const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->testAttribute(attribute);
  return false;
}

QWebSettings::ThirdPartyCookiePolicy QWebSettingsProto::thirdPartyCookiePolicy() const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->thirdPartyCookiePolicy();
  return QWebSettings::ThirdPartyCookiePolicy();
}

// TODO: Why doesn't this work?
/*
QUrl QWebSettingsProto::userStyleSheetUrl() const
{
  QWebSettings *item = qscriptvalue_cast<QWebSettings*>(thisObject());
  if (item)
    return item->userStyleSheetUrl();
  return QUrl();
}
*/

#endif
