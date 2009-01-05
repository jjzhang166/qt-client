/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#ifndef XTUPLEDESIGNER_H
#define XTUPLEDESIGNER_H

#include "guiclient.h"
#include "xmainwindow.h"

class QDesignerFormEditorInterface;
class QDesignerFormWindowInterface;
class QDesignerFormWindowManagerInterface;
class QDesignerIntegrationInterface;
class QDesignerObjectInspectorInterface;
class QDesignerPropertyEditorInterface;
class QDesignerWidgetBoxInterface;
class QIODevice;
class QMenu;
class WidgetBoxWindow;
class xTupleDesignerActions;

class xTupleDesigner : public XMainWindow
{
    Q_OBJECT

  public:
    xTupleDesigner(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~xTupleDesigner();
    QDesignerFormEditorInterface *formeditor()     { return _formeditor;  }
    virtual bool                  formEnabled()    { return _formEnabled; }
    virtual int                   formId()         { return _formId;      }
    QDesignerFormWindowInterface *formwindow()     { return _formwindow;  }
    virtual QString               name();
    virtual QString               notes() const    { return _notes;       }
    virtual int                   order()          { return _order;       }
    virtual XMainWindow          *objinspwindow()  { return _objinspwindow;  }
    virtual XMainWindow          *propinspwindow() { return _propinspwindow; }
    virtual XMainWindow          *slotedwindow()   { return _slotedwindow;   }
    virtual QString               source();
    virtual XMainWindow          *widgetwindow()   { return _widgetwindow;   }

  public slots:
    virtual void setFormEnabled(bool p);
    virtual void setFormId(int p);
    virtual void setNotes(QString p);
    virtual void setOrder(int p);
    virtual void setSource(QIODevice *, QString = QString());
    virtual void setSource(QString);
    virtual void sRevert()              { setSource(_source); }

  signals:
    void formEnabledChanged(bool);
    void formIdChanged(int);
    void nameChanged(QString);
    void notesChanged(QString);
    void orderChanged(int);
    void sourceChanged(QString);

  protected:
    virtual void closeEvent(QCloseEvent *);

  private:
    friend class WidgetBoxWindow;
    QWidget                             *_designer;
    QDesignerFormEditorInterface        *_formeditor;
    bool                                 _formEnabled;
    int                                  _formId;
    QDesignerFormWindowInterface        *_formwindow;
    QDesignerIntegrationInterface       *_integration;
    QString                              _notes;
    XMainWindow                         *_objinspwindow;
    int                                  _order;
    XMainWindow                         *_propinspwindow;
    XMainWindow                         *_slotedwindow;
    QIODevice                           *_source;
    XMainWindow                         *_widgetwindow;

    QMenuBar                            *_menubar;
    QMenu                               *_editmenu;
    QMenu                               *_filemenu;
    QMenu                               *_formmenu;
    QMenu                               *_toolmenu;

    xTupleDesignerActions               *_actions;
};

#endif // XTUPLEDESIGNER_H
