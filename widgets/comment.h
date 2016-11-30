/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef COMMENT_H
#define COMMENT_H

#include <QDialog>

#include <xsqlquery.h>
#include "ui_comment.h"

class QPushButton;
class ParameterList;

class XTUPLEWIDGETS_EXPORT comment : public QDialog, public Ui::comment
{
    Q_OBJECT

    friend class Comments;

  public:
    comment(QWidget * = 0, const char * = 0, bool = false, Qt::WindowFlags = 0);

    QPushButton* _close;
    QPushButton* _save;
    QPushButton* _next;
    QPushButton* _prev;
    QPushButton* _more;

  public slots:
    virtual void set(const ParameterList & pParams);
    virtual void sSave();
    virtual void populate();
    virtual void sNextComment();
    virtual void sPrevComment();

  protected slots:
    virtual void saveSize();

  private:
    XSqlQuery _query;
    QList <QVariant> _commentIDList;
    int _commentLocation;
    int _commentid;
    int _targetId;
    int _mode;
    QString _sourcetype;
    enum Comments::CommentSources _source;
};

#endif 
