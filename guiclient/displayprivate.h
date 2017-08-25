/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __DISPLAYPRIVATE_H__
#define __DISPLAYPRIVATE_H__

#include "ui_display.h"

#include <parameter.h>

#include "parameterlistsetup.h"

class QToolButton;
class display;

class displayPrivate : public QObject, public Ui::display
{
  Q_OBJECT

  public:
    displayPrivate(::display *parent);

    bool setParams(ParameterList &params);
    void setupCharacteristics(QStringList uses);
    void print(ParameterList pParams, bool showPreview, bool forceSetParams);

    QString reportName;
    QString metasqlName;
    QString metasqlGroup;

    bool _useAltId;
    bool _queryOnStartEnabled;
    bool _autoUpdateEnabled;
    bool _filterChanged;

    QAction *_newAct;
    QAction *_closeAct;
    QAction *_sep1;
    QAction *_filterLitAct;
    QAction *_filterAct;
    QAction *_moreAct;
    QAction *_sep2;
    QAction *_expandAct;
    QAction *_collapseAct;
    QAction *_printAct;
    QAction *_previewAct;
    QAction *_sep3;
    QAction *_searchAct;
    QAction *_queryAct;
    QAction *_queryOnStartAct;
    QAction *_autoUpdateAct;

    QMenu *_queryMenu;

    QToolButton *_newBtn;
    QToolButton *_closeBtn;
    QToolButton *_moreBtn;
    QToolButton *_queryBtn;
    QToolButton *_previewBtn;
    QToolButton *_printBtn;
    QToolButton *_expandBtn;
    QToolButton *_collapseBtn;

    QList<QVariant> _charidstext;
    QList<QVariant> _charidslist;
    QList<QVariant> _charidsdate;

  public slots:
    void sFilterChanged();

  private:
    ::display *_parent;
};

#endif
