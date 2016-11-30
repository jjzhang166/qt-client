/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QWEBSETTINGSPROTO_H__
#define __QWEBSETTINGSPROTO_H__

#include <QScriptEngine>

void setupQWebSettingsProto(QScriptEngine *engine);

#if QT_VERSION >= 0x050000
#include <QScriptable>
#include <QWebSettings>

Q_DECLARE_METATYPE(QWebSettings*)
//Q_DECLARE_METATYPE(QWebSettings) // Is private.
Q_DECLARE_METATYPE(enum QWebSettings::FontFamily)
Q_DECLARE_METATYPE(enum QWebSettings::FontSize)
Q_DECLARE_METATYPE(enum QWebSettings::ThirdPartyCookiePolicy)
Q_DECLARE_METATYPE(enum QWebSettings::WebAttribute)
Q_DECLARE_METATYPE(enum QWebSettings::WebGraphic)

QScriptValue constructQWebSettings(QScriptContext *context, QScriptEngine *engine);

class QWebSettingsProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QWebSettingsProto(QObject *parent);

    Q_INVOKABLE QString                               cssMediaType() const;
    Q_INVOKABLE QString                               defaultTextEncoding() const;
    Q_INVOKABLE QString                               fontFamily(QWebSettings::FontFamily which) const;
    Q_INVOKABLE int                                   fontSize(QWebSettings::FontSize type) const;
    Q_INVOKABLE QString                               localStoragePath() const;
    Q_INVOKABLE void                                  resetAttribute(QWebSettings::WebAttribute attribute);
    Q_INVOKABLE void                                  resetFontFamily(QWebSettings::FontFamily which);
    Q_INVOKABLE void                                  resetFontSize(QWebSettings::FontSize type);
    Q_INVOKABLE void                                  setAttribute(QWebSettings::WebAttribute attribute, bool on);
    Q_INVOKABLE void                                  setCSSMediaType(const QString & type);
    Q_INVOKABLE void                                  setDefaultTextEncoding(const QString & encoding);
    Q_INVOKABLE void                                  setFontFamily(QWebSettings::FontFamily which, const QString & family);
    Q_INVOKABLE void                                  setFontSize(QWebSettings::FontSize type, int size);
    Q_INVOKABLE void                                  setLocalStoragePath(const QString & path);
    Q_INVOKABLE void                                  setThirdPartyCookiePolicy(QWebSettings::ThirdPartyCookiePolicy policy);
    Q_INVOKABLE void                                  setUserStyleSheetUrl(const QUrl & location);
    Q_INVOKABLE bool                                  testAttribute(QWebSettings::WebAttribute attribute) const;
    Q_INVOKABLE QWebSettings::ThirdPartyCookiePolicy  thirdPartyCookiePolicy() const;
    // TODO: Why doesn't this work?
    //Q_INVOKABLE QUrl                                  userStyleSheetUrl() const;

};

#endif
#endif
