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

#ifndef __REVISIONCLUSTER_H__
#define __REVISIONCLUSTER_H__

#include "OpenMFGWidgets.h"
#include "xlineedit.h"

#include <QLabel>

#include <xsqlquery.h>

#include "virtualCluster.h"

class OPENMFGWIDGETS_EXPORT RevisionLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT;

  Q_ENUMS(Modes)
  Q_ENUMS(RevisionTypes)

  Q_PROPERTY(Modes     mode READ mode   WRITE setMode   )
  Q_PROPERTY(RevisionTypes   type READ type WRITE setType )

  public:
    RevisionLineEdit(QWidget *, const char * = 0);

    enum Modes { View, Use, Maintain };
    enum RevisionTypes { All, BOM, BOO };
    virtual Modes mode();
    virtual RevisionTypes type();
    virtual QString typeText();

  protected slots:
    virtual void setId(const int);
    virtual void sParse();

  public slots:
    void setActive();
      void setMode(QString);
      void setMode(Modes);
    void setTargetId(int pItem);
    void setType(QString);
    void setType(RevisionTypes);

  private:
    bool _allowNew;
    bool _isRevControl;
    enum Modes _mode;
    enum RevisionTypes _type;
    int _targetId;
    QString _cachenum;
    QString _typeText;

  signals:
    void canActivate(bool);
    void modeChanged();

};

class OPENMFGWIDGETS_EXPORT RevisionCluster : public VirtualCluster
{
  Q_OBJECT

  Q_ENUMS(RevisionLineEdit::Modes)
  Q_ENUMS(RevisionLineEdit::RevisionTypes)

  Q_PROPERTY(RevisionLineEdit::Modes     mode READ mode   WRITE setMode   )
  Q_PROPERTY(RevisionLineEdit::RevisionTypes   type READ type WRITE setType )

  public:
    RevisionCluster(QWidget *, const char * = 0);
    virtual RevisionLineEdit::Modes mode();
    virtual RevisionLineEdit::RevisionTypes type();

  private slots:
    void sModeChanged();
    void sCanActivate(bool p);
    void setActive();

  public slots:
    void activate();
    virtual void setMode(QString);
    virtual void setMode(RevisionLineEdit::Modes);
    virtual void setType(QString);
    virtual void setType(RevisionLineEdit::RevisionTypes);
    virtual void setTargetId(int pItem);

  signals:
    void canActivate(bool);
};

#endif
