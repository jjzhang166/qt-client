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

#ifndef TOITEMTABLEMODEL_H
#define TOITEMTABLEMODEL_H

#include <QDate>
#include <QHash>
#include <QObject>
#include <QPair>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include <QString>

/*
 Represent as a Table Model either the entire TOITEM table, the TOITEMS for
 a single POHEAD_ID, or a single TOITEM row.

 This should probably be written as a an XAbstractTableModel that includes
 OpenMFG-generic concepts and PoitemTableModel as a subclass of that abstract
 class but we don't yet know what all of those abstractions should be.

 */

// these #defines have to match the order of fields in the select statement
#define TOHEAD_NUMBER			 0
#define ITEM_NUMBER_COL			 1
#define PRJ_NUMBER_COL			 2
#define EARLIESTDATE_COL		 3
#define TOITEM_ID_COL			 4
#define TOITEM_TOHEAD_ID_COL		 5
#define TOITEM_LINENUMBER_COL		 6
#define TOITEM_ITEM_ID_COL		 7
#define TOITEM_STATUS_COL		 8
#define TOITEM_DUEDATE_COL		 9
#define TOITEM_SCHEDSHIPDATE_COL	10
#define TOITEM_SCHEDRECVDATE_COL	11
#define TOITEM_QTY_ORDERED_COL		12
#define TOITEM_QTY_SHIPPED_COL		13
#define TOITEM_QTY_RECEIVED_COL		14
#define TOITEM_UOM_COL			15
#define TOITEM_STDCOST_COL		16
#define TOITEM_PRJ_ID_COL		17
#define TOITEM_FREIGHT_COL		18
#define TOITEM_FREIGHT_CURR_ID_COL	19
#define TOITEM_FREIGHTTAX_ID_COL	20
#define TOITEM_FREIGHTTAX_PCTA_COL	21
#define TOITEM_FREIGHTTAX_PCTB_COL	22
#define TOITEM_FREIGHTTAX_PCTC_COL	23
#define TOITEM_FREIGHTTAX_RATEA_COL	24
#define TOITEM_FREIGHTTAX_RATEB_COL	25
#define TOITEM_FREIGHTTAX_RATEC_COL	26
#define TOITEM_CLOSEDATE_COL		27
#define TOITEM_CLOSE_USERNAME_COL	28
#define TOITEM_LASTUPDATED_COL		29
#define TOITEM_CREATED_COL		30
#define TOITEM_CREATOR_COL		31

class ToitemTableModel : public QSqlRelationalTableModel
{
  Q_OBJECT

  public:
    ToitemTableModel(QObject * parent = 0, QSqlDatabase = QSqlDatabase());

    virtual Qt::ItemFlags flags (const QModelIndex &) const;
    inline virtual int	  currId()    const { return _toheadcurrid; };
    inline virtual int	  headId()    const { return _toheadid; };
    inline virtual int	  itemId()    const { return _toitemid; };
    virtual bool	  isDirty()   const;
    virtual bool	  removeRow(int, const QModelIndex & = QModelIndex());
    inline virtual QDate  shipDate()  const { return _toshipdate; };
    inline virtual int    srcWhsId()  const { return _toheadsrcwhsid; };
    inline virtual QDate  transDate() const { return _toheaddate; };

  public slots:
    virtual bool	select();
    virtual void	setCurrId(const int pId = -1);
    virtual void	setHeadId(const int pId = -1);
    virtual void	setItemId(const int pId = -1);
    virtual void	setShipDate(const QDate);
    virtual void	setSrcWhsId(const int pId = -1);
    virtual void	setTransDate(const QDate);
    virtual bool	submitAll();

  protected:
    virtual bool	insertRowIntoTable(const QSqlRecord&);
    virtual QString	selectStatement() const;
    virtual bool	updateRowInTable(int, const QSqlRecord&);
    virtual bool	validRow(QSqlRecord&);

    QString		_selectStatement;

  protected slots:
    virtual void	markDirty(QModelIndex, QModelIndex);

  private:
    void	findHeadData();
    bool	_dirty;
    int		_toheadcurrid;
    QDate	_toheaddate;
    int		_toheadid;
    int		_toitemid;
    QDate	_toshipdate;
    int		_toheadsrcwhsid;
    QString	_tostatus;
};

#endif // TOITEMTABLEMODEL_H
