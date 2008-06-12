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

#ifndef POITEMTABLEMODEL_H
#define POITEMTABLEMODEL_H

#include <QDate>
#include <QHash>
#include <QObject>
#include <QPair>
#include <QSqlError>
#include <QSqlRelationalTableModel>
#include <QString>

/*
 Represent as a Table Model either the entire POITEM table, the POITEMS for
 a single POHEAD_ID, or a single POITEM row.

 This should probably be written as a an XAbstractTableModel that includes
 OpenMFG-generic concepts and PoitemTableModel as a subclass of that abstract
 class but we don't yet know what all of those abstractions should be.

 */

// these #defines have to match the order of fields in the select statement
#define POHEAD_NUMBER			 0
#define ITEM_ID_COL			 1
#define ITEM_NUMBER_COL			 2
#define WAREHOUS_ID_COL			 3
#define WAREHOUS_CODE_COL		 4
#define EXTPRICE_COL			 5
#define PRJ_NUMBER_COL			 6
#define EXPCAT_CODE_COL			 7
#define ITEMSRC_MINORDQTY_COL		 8
#define ITEMSRC_MULTORDQTY_COL		 9
#define ITEMSRC_INVVENDORUOMRATIO_COL	10
#define EARLIESTDATE_COL		11
#define POITEM_ID_COL			12
#define POITEM_STATUS_COL		13
#define POITEM_POHEAD_ID_COL		14
#define POITEM_LINENUMBER_COL		15
#define POITEM_DUEDATE_COL		16
#define POITEM_WOHEAD_ID		17
#define POITEM_ITEMSITE_ID_COL		18
#define POITEM_VEND_ID_COL		19
#define POITEM_VEND_ITEM_DESCRIP_COL	20
#define POITEM_VEND_UOM_COL		21
#define POITEM_INVVENDUOMRATIO_COL	22
#define POITEM_QTY_ORDERED_COL		23
#define POITEM_QTY_RECEIVED_COL		24
#define POITEM_QTY_RETURNED_COL		25
#define POITEM_QTY_VOUCHERED_COL	26
#define POITEM_UNITPRICE_COL		27
#define POITEM_NOCHARGE_COL		28
#define POITEM_DATE_PROMISE_COL		29
#define POITEM_DATE_LASTRECEIVE_COL	30
#define POITEM_DATE_LASTRETURN_COL	31
#define POITEM_DATE_CLOSED_COL		32
#define POITEM_VEND_ITEM_NUMBER_COL	33
#define POITEM_COMMENTS_COL		34
#define POITEM_QTY_TORECEIVE_COL	35
#define POITEM_EXPCAT_ID_COL		36
#define POITEM_ITEMSRC_ID_COL		37
#define POITEM_FREIGHT_COL		38
#define POITEM_FREIGHT_RECEIVED_COL	39
#define POITEM_FREIGHT_VOUCHERED_COL	40
#define POITEM_SOITEM_ID_COL		41
#define POITEM_PRJ_ID_COL		42
#define POITEM_STDCOST_COL		43

class PoitemTableModel : public QSqlRelationalTableModel
{
  Q_OBJECT

  public:
    PoitemTableModel(QObject * parent = 0, QSqlDatabase = QSqlDatabase());

    virtual Qt::ItemFlags flags (const QModelIndex &) const;
    inline virtual int	  currId() const {return _poheadcurrid; };
    inline virtual int	  headId() const { return _poheadid; };
    inline virtual int	  itemId() const { return _poitemid; };
    virtual bool	  isDirty() const;
    virtual bool	  removeRow(int, const QModelIndex & = QModelIndex());
    inline virtual QDate  transDate() const {return _poheaddate; };
    int		_vendid;
    bool	_vendrestrictpurch;

  public slots:
    virtual bool	select();
    virtual void	setCurrId(const int pId = -1);
    virtual void	setHeadId(const int pId = -1);
    virtual void	setItemId(const int pId = -1);
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
    int		_poheadcurrid;
    QDate	_poheaddate;
    int		_poheadid;
    int		_poitemid;
    QString	_postatus;
};

#endif // POITEMTABLEMODEL_H
