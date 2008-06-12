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

#ifndef __XTREEWIDGET_H__
#define __XTREEWIDGET_H__

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVariant>
#include <QVector>

#include "OpenMFGWidgets.h"


//  Table Column Widths
#define _itemColumn        100
#define _whsColumn         35
#define _userColumn        80
#define _dateColumn        80
#define _timeDateColumn    160
#define _timeColumn        80
#define _qtyColumn         80
#define _priceColumn       60
#define _moneyColumn       60
#define _bigMoneyColumn    100
#define _costColumn        70
#define _prcntColumn       55
#define _transColumn       35
#define _uomColumn         45
#define _orderColumn       60
#define _statusColumn      45
#define _seqColumn         40
#define _ynColumn          45
#define _docTypeColumn     80
#define _currencyColumn    80


#include "xsqlquery.h"

class XTreeWidget;
class QMenu;
class QAction;

class OPENMFGWIDGETS_EXPORT XTreeWidgetItem : public QTreeWidgetItem
{
  friend class XTreeWidget;

  public:
    XTreeWidgetItem(XTreeWidgetItem *, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, int, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, int, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, XTreeWidgetItem *, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidget *, XTreeWidgetItem *, int, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, XTreeWidgetItem *, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );
    XTreeWidgetItem(XTreeWidgetItem *, XTreeWidgetItem *, int, int, QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant(),
                     QVariant = QVariant(), QVariant = QVariant() );

    void setText(int, const QVariant &);
    inline void setTextColor(int column, const QColor & color) { QTreeWidgetItem::setTextColor(column, color); }
    void setTextColor(const QColor &);

    inline int id() const         { return _id;    }
    inline int altId() const      { return _altId; }
    inline void setId(int pId)    { _id = pId;     }
    inline void setAltId(int pId) { _altId = pId;  }

    inline XTreeWidgetItem *child(int idx) const
    {
      QTreeWidgetItem * item = QTreeWidgetItem::child(idx);
      return ((XTreeWidgetItem*)item);
    }    

  private:
    void constructor( int, int, QVariant, QVariant, QVariant,
                      QVariant, QVariant, QVariant, QVariant,
                      QVariant, QVariant, QVariant, QVariant );

    int                _id;
    int                _altId;
};

class OPENMFGWIDGETS_EXPORT XTreeWidget : public QTreeWidget
{
  Q_OBJECT
  Q_PROPERTY(QString dragString READ dragString WRITE setDragString)
  Q_PROPERTY(QString altDragString READ altDragString WRITE setAltDragString)
  Q_PROPERTY(int id READ id WRITE setId DESIGNABLE false)
  Q_PROPERTY(int altId READ altId DESIGNABLE false)

  public:
    XTreeWidget(QWidget *);
    ~XTreeWidget();

    Q_INVOKABLE void populate(XSqlQuery, bool = FALSE);
    Q_INVOKABLE void populate(XSqlQuery, int, bool = FALSE);
    void populate(const QString &, bool = FALSE);
    void populate(const QString &, int, bool = FALSE);


    QString dragString() const;
    void setDragString(QString);
    QString altDragString() const;
    void setAltDragString(QString);

    void clear();

    int  id() const;
    int  altId() const;
    void setId(int);

    inline XTreeWidgetItem * topLevelItem(int idx) const
    {
      return (XTreeWidgetItem*)QTreeWidget::topLevelItem(idx);
    }

    Q_INVOKABLE void setColumnVisible(int, bool);
    Q_INVOKABLE void setColumnLocked(int, bool);
    XTreeWidgetItem *findXTreeWidgetItemWithId(const XTreeWidget *ptree, const int pid);
    XTreeWidgetItem *findXTreeWidgetItemWithId(const XTreeWidgetItem *ptreeitem, const int pid);

    virtual void sortItems(int, Qt::SortOrder);
    static  bool itemAsc(const QVariant &, const QVariant &);
    static  bool itemDesc(const QVariant &, const QVariant &);

  public slots:
    void addColumn(const QString &, int, int, bool = true, const QString = QString(), const QString = QString());
    inline void hideColumn(int colnum) { QTreeWidget::hideColumn(colnum); };
    void hideColumn(const QString &);

  signals:
    void  valid(bool);
    void  newId(int);
    void  itemSelected(int);
    void  populateMenu(QMenu *, QTreeWidgetItem *);
    void  populateMenu(QMenu *, QTreeWidgetItem *, int);

  protected slots:
    void sHeaderClicked(int);
    void populateCalculatedColumns();

  protected:
    QPoint dragStartPosition;

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void resizeEvent(QResizeEvent*);

  private:
    QString     _dragString;
    QString     _altDragString;
    QMenu       *_menu;
    QMap<int, int>      _defaultColumnWidths;
    QMap<int, int>      _savedColumnWidths;
    QMap<int, bool>     _savedVisibleColumns;
    QMap<int, QVariantMap*>	_roles;
    QList<int>          _lockedColumns;
    QVector<int>        _stretch;
    bool        _resizingInProcess;
    bool        _forgetful;
    bool        _settingsLoaded;
    QString     _settingsName;
    int         _resetWhichWidth;
    static void loadLocale();

  private slots:
    void sSelectionChanged();
    void sItemSelected(QTreeWidgetItem *, int);
    void sShowMenu(const QPoint &);
    void sShowHeaderMenu(const QPoint &);
    void sExport();
    //void sStartDrag(QTreeWidgetItem *, int);
    void sColumnSizeChanged(int, int, int);
    void sResetWidth();
    void sResetAllWidths();
    void sToggleForgetfulness();
    void popupMenuActionTriggered(QAction*);
};

#endif

