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

#ifndef SCREENCONTROL_H
#define SCREENCONTROL_H

#include "OpenMFGWidgets.h"
#include "xdatawidgetmapper.h"
#include "xsqltablemodel.h"

#include <QWidget>
#include <QSqlIndex>
#include <QList>

#include "ui_screenControl.h"

class OPENMFGWIDGETS_EXPORT ScreenControl : public QWidget, public Ui::screenControl
{
	Q_OBJECT
		
	Q_ENUMS(Modes)
	Q_ENUMS(SearchTypes)
	
	Q_PROPERTY (QString		schemaName	READ schemaName		WRITE setSchemaName)
	Q_PROPERTY (QString 		tableName 	READ tableName 		WRITE setTableName)
	Q_PROPERTY (QString		listReportName	READ listReportName	WRITE setListReportName)
	Q_PROPERTY (Modes		mode		READ mode		WRITE setMode)
	Q_PROPERTY (SearchTypes 	searchType	READ searchType		WRITE setSearchType)
	Q_PROPERTY (bool     		newVisible	READ newVisible		WRITE setNewVisible)
	Q_PROPERTY (bool     		saveVisible	READ saveVisible	WRITE setSaveVisible)
        Q_PROPERTY (QString             sortColumn      READ sortColumn         WRITE setSortColumn)

	public:
		ScreenControl(QWidget * = 0);
                ~ScreenControl();
		void showEvent ( QShowEvent * event );
	
		bool newVisible()	const   { return _new->isVisible();};
		bool saveVisible()	const   { return _save->isVisible();};
		bool viewVisible()	const   { return _view->isVisible();};
		
		enum Modes { New, Edit, View };
		enum SearchTypes { Query, List };
		Modes mode();
		SearchTypes searchType();
	
                bool  isDirty();
      //          QList<QString> primaryKey()    const { return _pklist; };
	        QString schemaName()      const { return _schemaName; };
	        QString sortColumn()      const { return _sortColumn; };
		QString tableName()   	  const { return _tableName; };
		QString listReportName()  const	{ return _listReportName;    }; 
                
        private slots:
                void enableSave();
       	
	public slots:
		void toNext();
		void toPrevious();
		void newRow();
		void print();
		void printList();
		void save();
		void search();
		void select();
                void setCurrentIndex(int p)             { _mapper.setCurrentIndex(p); };
		void setFilter(QString p);
		void setMode(Modes p);
		void setListReportName(QString p) 	{ _listReportName = p ; _print->setVisible(! p.isEmpty()); };
		void setNewVisible(bool p) 		{ _new->setVisible(p) ; };
		void setSaveVisible(bool p) 		{ _save->setVisible(p) ; };
		void setSearchType(SearchTypes p);
		void setSchemaName(QString p)  		{ _schemaName = p;};
		void setSortColumn(QString p);
 
  //              void setPrimaryKey(QList<QString> p)             { _pklist = p;};
		void setTableName(QString p)		{ _tableName = p;};
		void setTable(QString s, QString t);
		void setDataWidgetMapper(QSqlTableModel *p);
	
	protected slots:
		void languageChange();
	
	signals:
                void movedNext();
                void movedPrev();
                void movingNext();
                void movingPrev();
		void newClicked();
		void newDataWidgetMapper(XDataWidgetMapper *m);
                void newModel(XSqlTableModel *m);
		void printClicked();
		void saveClicked();
		void saved(bool);
                void saving();
		void searchTypeChanged(SearchTypes);

	private:
	
		bool			_autoSave;
		bool			_hasSearch;
		bool			_shown;
		enum  Modes		_mode;
		enum  SearchTypes	_searchType;
   //             QList<QString>          _pklist;
		QString 		_formReportName;
		QString           	_listReportName;
		QString			_schemaName;
                QString                 _sortColumn;
		QString			_tableName;
		XDataWidgetMapper	_mapper;
                XSqlTableModel*		_model;
};

#endif // SCREENNCONTROL_H
