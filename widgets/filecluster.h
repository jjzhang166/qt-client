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

#ifndef FILECLUSTER_H
#define FILECLUSTER_H

#include "OpenMFGWidgets.h"
#include "parameter.h"
#include "xtreewidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QKeyEvent>
#include <QPushButton>
#include <QWidget>

class FileLineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    FileLineEdit(QWidget * parent = 0);

  protected:
    virtual void keyPressEvent(QKeyEvent *event);

  signals:
    void requestList();
    void requestSearch();
};

/*
  FileCluster allows the user to enter a string or browse the filesystem.
  The text() of the FileCluster should name a file or directory. The ellipsis
  button instantiates a QFileDialog, so the class has methods to control that
  QFileDialog.
 */

class OPENMFGWIDGETS_EXPORT FileCluster : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(QString		    caption READ caption     WRITE setCaption )
  Q_PROPERTY(bool	        listVisible READ listVisible WRITE setListVisible )
  Q_PROPERTY(QFileDialog::FileMode fileMode READ fileMode    WRITE setFileMode )
  Q_PROPERTY(bool	           readOnly READ isReadOnly  WRITE setReadOnly )
  Q_PROPERTY(QString		       text READ text	     WRITE setText )

  public:
    FileCluster(QWidget*, const char* = 0);
    ~FileCluster();

    virtual inline QString caption()	 const	{ return _caption; };
    virtual inline void    clear()		{ _file->clear(); };
    virtual inline QString dir()	 const	{ return _dir; };
    virtual inline QFileDialog::FileMode fileMode() const { return _fileMode; };
    virtual inline bool	   isReadOnly()	 const	{ return _file->isReadOnly(); };
    virtual inline bool	   listVisible() const	{ return _list->isVisible(); };
    virtual inline void    setListVisible(bool b) { _list->setVisible(b); };
    virtual inline void    setReadOnly(bool b)	  { _file->setReadOnly(b);
						    setListVisible(! b); };
    virtual inline QString text()	 const	  { return _file->text(); };

  public slots:
    virtual void sEllipses();
    virtual void setCaption(const QString &s) { _caption = s; } ;
    virtual void setDir(const QString &s)     { _dir = s; } ;
    virtual void setEnabled(const bool b)     { _file->setEnabled(b);
						_list->setEnabled(! isReadOnly()); } ;
    virtual void setFileMode(const QFileDialog::FileMode &m) { _fileMode = m; };
    virtual void setFilter(const QString &s)  { _filter = s; } ;
    virtual void setText(const QString &s)    { _file->setText(s); };

  signals:
    void textChanged(const QString&);

  protected:
    QString			_caption;
    QString			_dir;
    FileLineEdit		*_file;
    QString			_filter;
    QPushButton			*_list;
    QFileDialog::FileMode	_fileMode;
};

#endif
