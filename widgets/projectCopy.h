/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef projectCopy_h
#define projectCopy_h

#include <QVariant>
#include <QDialog>

#include "widgets.h"
#include "projectcluster.h"

class QLineEdit;
class QPushButton;
class DLineEdit;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT projectCopy : public QDialog
{
    Q_OBJECT

  public:
    projectCopy(QWidget * = 0, const char * = 0, bool = false, Qt::WindowFlags = 0 );

    virtual void set(const ParameterList & pParams);

  protected:
    XLineEdit* _number;
    XLineEdit* _name;
    XLineEdit* _newnumber;
    XLineEdit* _newname;
    DLineEdit* _due;

    QPushButton* _close;
    QPushButton* _copy;

  protected slots:
    virtual void sCopy();
    virtual void sHandleButtons();

  private:
    unsigned int _projectId;
    int _newProjectId;
};

#endif
