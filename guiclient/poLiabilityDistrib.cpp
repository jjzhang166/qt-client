/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "poLiabilityDistrib.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include <QSqlError>
#include "errorReporter.h"

/*
 *  Constructs a poLiabilityDistrib as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
poLiabilityDistrib::poLiabilityDistrib(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _account->setType(GLCluster::cRevenue | GLCluster::cExpense |
                    GLCluster::cAsset | GLCluster::cLiability);

  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
poLiabilityDistrib::~poLiabilityDistrib()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void poLiabilityDistrib::languageChange()
{
  retranslateUi(this);
}

enum SetResponse poLiabilityDistrib::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("recv_id", &valid);
  if (valid)
  {
    _recvid = param.toInt();
    populate();
  }

  return NoError;
}

void poLiabilityDistrib::populate()
{
  XSqlQuery popopulate;
  popopulate.prepare( "SELECT recv_date, recv_value "
             "FROM recv "
             "WHERE (recv_id=:recv_id);" ) ;
  popopulate.bindValue(":recv_id", _recvid);
  popopulate.exec();
  if (popopulate.first())
  {
    _distDate->setDate(popopulate.value("recv_date").toDate());
    _amount->setLocalValue(popopulate.value("recv_value").toDouble());
  }
}

void poLiabilityDistrib::sPost()
{
  XSqlQuery poPost;
  if (!_account->isValid())
  {
    QMessageBox::warning( this, tr("Select Account"),
                          tr("You must select an Account to post the P/O Liability Distribution to.") );
    _account->setFocus();
    return;
  }
  if (!_distDate->isValid())
  {
    QMessageBox::warning( this, tr("Distribution Date"),
                          tr("You must select a valid Distribution Date to post the P/O Liability Distribution to.") );
    _distDate->setFocus();
    return;
  }

  poPost.prepare( "SELECT insertGLTransaction( 'G/L', 'PO', pohead_number, 'Qty. ' || formatqty(recv_qty) || ' for ' || COALESCE(item_number,poitem_vend_item_number) || ' marked as invoiced',"
             "                            :creditAccntid, COALESCE(costcat_liability_accnt_id,expcat_liability_accnt_id) ,-1, :amount, :distdate ) AS result"
             " FROM recv, pohead, poitem "
			 " LEFT OUTER JOIN expcat ON (poitem_expcat_id=expcat_id) "
			 " LEFT OUTER JOIN itemsite ON (poitem_itemsite_id=itemsite_id) "
			 " LEFT OUTER JOIN costcat ON (itemsite_costcat_id=costcat_id) "
			 " LEFT OUTER JOIN item ON (itemsite_item_id=item_id) "
             " WHERE ( (recv_id=:recv_id) "
             " AND (poitem_id=recv_orderitem_id) "
			 " AND (pohead_id=poitem_pohead_id) );" );
  poPost.bindValue(":creditAccntid", _account->id());
  poPost.bindValue(":distdate", _distDate->date());
  poPost.bindValue(":amount", _amount->baseValue());
  poPost.bindValue(":recv_id", _recvid);
  poPost.exec();
  if (!poPost.first())
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving GL Information"),
                         poPost, __FILE__, __LINE__);
    return;
  }

  done(_recvid);
}

