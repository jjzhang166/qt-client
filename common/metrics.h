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


#ifndef metrics_h
#define metrics_h

#include <QObject>
#include <QString>
#include <QMap>

typedef QMap<QString, QString> MetricMap;

class Parameters : public QObject
{
  Q_OBJECT

  protected:
    MetricMap _values;
    QString   _readSql;
    QString   _setSql;
    QString   _username;
    bool      _dirty;

  public:
    Parameters(QObject * parent = 0);
    virtual ~Parameters() {};

    void load();

    QString value(const char *);
    bool    boolean(const char *);

    void set(const char *, bool);
    void set(const QString &, bool);
    void set(const char *, int);
    void set(const QString &, int);
    void set(const char *, const QString &);
    void set(const QString &, const QString &);

    QString parent(const QString &);

  public slots:
    QString value(const QString &);
    bool    boolean(const QString &);

  protected:
    void _set(const QString &, QVariant);

};

class Metrics : public Parameters
{
  Q_OBJECT

  public:
    Metrics();
};

class Preferences : public Parameters
{
  Q_OBJECT

  public:
    Preferences() {};
    Preferences(const QString &);

    void remove(const QString &);
};

class Privileges : public Parameters
{
  Q_OBJECT

  public:
    Privileges();

  public slots:
    bool check(const QString &);
};

#endif

