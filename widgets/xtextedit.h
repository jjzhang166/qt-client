/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTEXTEDIT_H__
#define __XTEXTEDIT_H__

#include <QTextEdit>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class XTUPLEWIDGETS_EXPORT XTextEdit : public QTextEdit
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName)
  Q_PROPERTY(QString defaultText  READ defaultText WRITE setDefaultText)

  public:
    XTextEdit(QWidget * = 0);
    
    virtual QString defaultText() const { return _default; };
    virtual QString fieldName()   const { return _fieldName; };

  public slots:
    virtual void setDataWidgetMap(XDataWidgetMapper* m);
    virtual void setDefaultText(QString p)  { _default = p; };
    virtual void setFieldName(QString p) { _fieldName = p; };
    virtual void updateMapperData();

  protected:
    XDataWidgetMapper *_mapper;

  private:
    QString _default;
    QString _fieldName;
};

#endif
