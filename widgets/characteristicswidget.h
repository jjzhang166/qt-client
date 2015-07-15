/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _CHARACTERISTICSWIDGET_H_
#define _CHARACTERISTICSWIDGET_H_

#include <QString>

#include "ui_characteristicswidget.h"

class CharacteristicsWidgetPrivate;
class QScriptEngine;

void setupCharacteristicsWidget(QScriptEngine *engine);

class CharacteristicsWidget : public QWidget, public Ui::CharacteristicsWidget
{
  Q_OBJECT

  public:
    CharacteristicsWidget(QWidget* parent = 0, const char* name = 0, QString type = QString(), int id = -1);
    ~CharacteristicsWidget();

    Q_INVOKABLE bool isValid() const;

  public slots:
    void sDelete();
    void sEdit();
    void sFillList();
    void sNew();
    void setId(int id);
    void setReadOnly(bool readOnly);
    void setType(QString doctype);

  signals:
    void newId(int);
    void newType(QString);
    void valid(bool);

  protected slots:
    void languageChange();

  private:
    CharacteristicsWidgetPrivate *_d;
};

Q_DECLARE_METATYPE(CharacteristicsWidget*)

#endif // _CHARACTERISTICSWIDGET_H_
