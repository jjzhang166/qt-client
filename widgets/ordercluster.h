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

#ifndef _orderCluster_h

#define _orderCluster_h

#include "virtualCluster.h"

class OPENMFGWIDGETS_EXPORT OrderLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public:
    OrderLineEdit(QWidget*, const char* = 0);

    enum OrderStatus
    {
      AnyStatus = 0x00,
      Unposted  = 0x01, Open = 0x02, Closed = 0x04
    };
    Q_DECLARE_FLAGS(OrderStatuses, OrderStatus)

    enum OrderType
    {
      AnyType = 0x00,
      Purchase = 0x01, Return = 0x02, Sales = 0x04, Transfer = 0x08
    };
    Q_DECLARE_FLAGS(OrderTypes, OrderType)

    virtual OrderStatuses allowedStatuses()	const;
    virtual OrderTypes	  allowedTypes()	const;
    virtual void	  clear();
    virtual QString	  from()		const;
    virtual bool	  isClosed()		const;
    virtual bool	  isOpen()		const;
    virtual bool	  isPO()		const;
    virtual bool	  isRA()		const;
    virtual bool	  isSO()		const;
    virtual bool	  isTO()		const;
    virtual bool	  isUnposted()		const;
    virtual void	  setExtraClause(const QString &, const QString &);
    virtual void	  setExtraClause(const OrderTypes, const QString &);
    virtual void	  sList();
    virtual void	  sSearch();
    virtual OrderStatus	  status();
    virtual QString	  to()			const;
    virtual QString	  type();

  public slots:
    virtual void	  setAllowedStatuses(const OrderStatuses);
    virtual void	  setAllowedType(const QString &);
    virtual void	  setAllowedTypes(const OrderTypes);
    virtual void	  setId(const int, const QString & = "");

  signals:
    void newId(const int, const QString &);
    void numberChanged(const QString &, const QString &);

  protected:
    OrderStatuses	_allowedStatuses;
    OrderTypes		_allowedTypes;
    QString		_from;
    QString		_to;

    virtual QString	buildExtraClause();
    virtual void	silentSetId(const int);


  protected slots:
    virtual void	sNewId(const int);
    virtual void	sParse();

  private:
    QString	_allClause;
    QString	_poClause;
    QString	_raClause;
    QString	_soClause;
    QString	_toClause;
    QString	_statusClause;
    QString	_typeClause;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(OrderLineEdit::OrderStatuses)
Q_DECLARE_OPERATORS_FOR_FLAGS(OrderLineEdit::OrderTypes)

class OPENMFGWIDGETS_EXPORT OrderCluster : public VirtualCluster
{
  Q_OBJECT

  public:
    OrderCluster(QWidget*, const char* = 0);

    virtual OrderLineEdit::OrderStatuses allowedStatuses()	const;
    virtual OrderLineEdit::OrderTypes	 allowedTypes()		const;
    virtual QString	from()		const;
    virtual bool	isClosed()	const;
    virtual bool	isOpen()	const;
    virtual bool	isPO()		const;
    virtual bool	isRA()		const;
    virtual bool	isSO()		const;
    virtual bool	isTO()		const;
    virtual bool	isUnposted()	const;
    virtual void	setExtraClause(const QString &, const QString &);
    virtual void	setExtraClause(const OrderLineEdit::OrderTypes,
				       const QString &);
    virtual OrderLineEdit::OrderStatus	 status()		const;
    virtual QString	to()		const;
    virtual QString	type()		const;

  public slots:
    virtual void	setAllowedStatuses(const OrderLineEdit::OrderStatuses);
    virtual void	setAllowedType(const QString &);
    virtual void	setAllowedTypes(const OrderLineEdit::OrderTypes);
    virtual void	setId(const int, const QString& = "");
    virtual void	sRefresh();

  signals:
    void newId(const int, const QString &);
    void numberChanged(const QString &, const QString &);

  protected:
    QLabel	*_fromLit;
    QLabel	*_from;
    QLabel	*_toLit;
    QLabel	*_to;
};

class OPENMFGWIDGETS_EXPORT OrderList : public VirtualList
{
  Q_OBJECT

  public:
    OrderList(QWidget*, Qt::WindowFlags = 0);

    QString	type() const;

  protected:
    QList<QTreeWidgetItem*> selectedAtDone;

  protected slots:
    virtual void	  done(int);
};

class OPENMFGWIDGETS_EXPORT OrderSearch : public VirtualSearch
{
  Q_OBJECT

  public:
    OrderSearch(QWidget*, Qt::WindowFlags = 0);

    QString	type() const;

  protected:
   QList<QTreeWidgetItem*> selectedAtDone;

  protected slots:
    virtual void	  done(int);
};

#endif
