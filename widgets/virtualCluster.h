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

#ifndef virtCluster_h
#define virtCluster_h

#include "OpenMFGWidgets.h"
#include "parameter.h"
#include "xlineedit.h"
#include "xtreewidget.h"
#include "xcheckbox.h"
#include "xdatawidgetmapper.h"

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class QGridLayout;
class VirtualClusterLineEdit;

class OPENMFGWIDGETS_EXPORT VirtualList : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualList();
        VirtualList(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSearch(const QString&);
        virtual void sSelect();

    protected:
        virtual void init();

        VirtualClusterLineEdit* _parent;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        QLabel*      _titleLit;
        QPushButton* _close;
        QPushButton* _select;
        XTreeWidget* _listTab;
        int          _id;
};

class OPENMFGWIDGETS_EXPORT VirtualSearch : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSelect();

    protected:
        VirtualClusterLineEdit* _parent;
        int  _id;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        XCheckBox*   _searchNumber;
        XCheckBox*   _searchName;
        XCheckBox*   _searchDescrip;
        QLabel*      _titleLit;
        QPushButton* _close;
        QPushButton* _select;
        XTreeWidget* _listTab;
        QLayout*     dialogLyt;
        QLayout*     searchLyt;
        QLayout*     searchStrLyt;
        QGridLayout* selectorsLyt;
        QLayout*     tableLyt;
        QLayout*     buttonsLyt;
};

class OPENMFGWIDGETS_EXPORT VirtualInfo : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualInfo(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sPopulate();

    protected:
        VirtualClusterLineEdit* _parent;
        QLabel*         _titleLit;
        QLabel*         _numberLit;
        QLabel*         _number;
        QLabel*         _nameLit;
        QLabel*         _name;
        QLabel*         _descripLit;
        QLabel*         _descrip;
        QPushButton*    _close;

    private:
        int _id;
};

/*
    VirtualClusterLineEdit is an abstract class that encapsulates
    the basics of retrieving an ID given a NUMBER or a NUMBER given
    an ID.  This class and its subclasses are intended to be used
    by VirtualCluster objects, not directly by an application.

    Users of this class need to supply the names of the table, its
    id column, and its number column (numbers are user-readable
    strings such as customer abbreviations or purchase order numbers).

    Subclasses must supply implementations for the sInfo(), sList(),
    and sSearch() slots.  sInfo() is a distinct dialog that shows
    extended data from the record with table_id = id(). sList() and
    sSearch() are dialogs that let the user select a number/id pair
    value for the widget.
*/

class OPENMFGWIDGETS_EXPORT VirtualClusterLineEdit : public XLineEdit
{
    Q_OBJECT
    Q_PROPERTY(QString     number          READ text          WRITE setNumber);
    
    friend class VirtualCluster;
    friend class VirtualInfo;
    friend class VirtualList;
    friend class VirtualSearch;

    public:
        VirtualClusterLineEdit(QWidget*, const char*, const char*, const char*,
                               const char*, const char*, const char*,
                               const char* = 0);
        virtual int  id() const { return _id; }

    public slots:
        virtual void setId(const int);

    protected slots:
        virtual void clear();
        inline virtual void clearExtraClause() { _extraClause = ""; }

        virtual VirtualList*    listFactory();
        virtual VirtualSearch*  searchFactory();
        virtual VirtualInfo*    infoFactory();

        virtual void sList();
        virtual void sInfo();
        virtual void sSearch();
        virtual void sEllipses();
        virtual void sParse();

        virtual void setNumber(const QString&);
        virtual void setTitles(const QString&, const QString& = 0);
        inline virtual void setExtraClause(const QString& pExt) { _extraClause = pExt; };
        virtual void setTableAndColumnNames(const char* pTabName,
                                            const char* pIdColumn,
                                            const char* pNumberColumn,
                                            const char* pNameColumn,
                                            const char* pDescripColumn);

        void setStrict(bool);
        bool isStrict() const { return _strict; }

    signals:
        void newId(int);
        void parsed();
        void valid(bool);

    protected:
        QString _titleSingular;
        QString _titlePlural;
        QString _query;
        QString _idClause;
        QString _numClause;
        QString _extraClause;
        QString _name;
        QString _description;
        QString _idColName;
        QString _numColName;
        QString _nameColName;
        QString _descripColName;
        bool    _hasDescription;
        bool    _hasName;
        bool    _strict;
        virtual void silentSetId(const int);
        XDataWidgetMapper *_mapper;
};

/*

    VirtualCluster is a widget that contains a VirtualClusterLineEdit
    and surrounds it with a label and two buttons.  One button lets
    the user invoke VirtualClusterLineEdit's sInfo slot while another
    button invokes either sSearch() or sList(), depending on the
    user's preferences.
    
    VirtualCluster provides a copy of much of the VirtualClusterLineEdit
    API to hide from the caller the fact that it contains one of these
    objects.  It also lets the calling code set it as read-only.

*/

class OPENMFGWIDGETS_EXPORT VirtualCluster : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString label       READ label       WRITE setLabel);
    Q_PROPERTY(bool    infoVisible READ infoVisible WRITE setInfoVisible);
    Q_PROPERTY(bool    listVisible READ listVisible WRITE setListVisible);
    Q_PROPERTY(bool    nameVisible READ nameVisible WRITE setNameVisible);
    Q_PROPERTY(bool    readOnly    READ readOnly    WRITE setReadOnly);
    Q_PROPERTY(QString fieldName   READ fieldName   WRITE setFieldName);

    friend class VirtualClusterLineEdit;

    public:
        VirtualCluster(QWidget*, const char* = 0);
        VirtualCluster(QWidget*, VirtualClusterLineEdit* = 0, const char* = 0);

        inline virtual int     id()          const { return _number->id(); };
        inline virtual bool    infoVisible() const { return _info->isVisible(); };
        inline virtual bool    listVisible() const { return _list->isVisible(); };
        inline virtual QString label()       const { return _label->text(); };
        inline virtual bool    nameVisible() const { return _name->isVisible(); };
               virtual QString number()      const { return _number->text(); };
        inline virtual QString description() const { return _description->text(); };
        inline virtual bool    isValid()     const { return _number->isValid(); };
        inline virtual QString name()        const { return _name->text(); };
        inline virtual bool    isStrict()    const { return _number->isStrict(); };
        inline virtual bool    readOnly()    const { return _readOnly; };
        inline virtual QString fieldName()   const { return _fieldName; };

    public slots:
        // most of the heavy lifting is done by VirtualClusterLineEdit _number
        inline virtual void clearExtraClause() { _number->clearExtraClause(); };
        inline virtual void setDescription(const QString& p) { _description->setText(p); };
        inline virtual void setExtraClause(const QString& p) { _number->setExtraClause(p); };
        inline virtual void setFieldName(QString p) { _fieldName = p; };
        inline virtual void setId(const int p)           { _number->setId(p); };
        inline virtual void setInfoVisible(const bool p) { _info->setHidden(!p); };
        inline virtual void setListVisible(const bool p) { _list->setHidden(!p); };
        inline virtual void setLabel(const QString& p)  { _label->setText(p); };
        inline virtual void setName(const QString& p)   { _name->setText(p); };
        inline virtual void setNameVisible(const bool p) { _name->setHidden(!p); };
        inline virtual void setNumber(const int p)      { _number->setNumber(QString::number(p)); };
        inline virtual void setNumber(const QString& p) { _number->setNumber(p); };

        virtual void clear();
        virtual void setDataWidgetMap(XDataWidgetMapper* m);
        virtual void setEnabled(const bool p);
        virtual void setStrict(const bool b);
        virtual void setReadOnly(const bool b);
        virtual void sRefresh();

    signals:
        void newId(int);
        void valid(bool);

    protected:
        virtual void addNumberWidget(VirtualClusterLineEdit* pNumberWidget);

        QGridLayout*    _grid;
        QLabel*         _label;
        VirtualClusterLineEdit* _number;
        QPushButton*    _list;
        QPushButton*    _info;
        QLabel*         _description;
        QLabel*         _name;
        bool            _readOnly;
	QString         _fieldName;

    private:
        virtual void init();
};

#endif
