/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#include <QMessageBox>

#include "externalShipping.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

bool externalShipping::userHasPriv(const int /*pMode*/)
{
  bool retval = _privileges->check("MaintainExternalShipping");
  return retval;
}

void externalShipping::setVisible(bool visible)
{
  if (! visible)
    XDialog::setVisible(false);
  else if (!userHasPriv())
  {
    systemError(this,
	        tr("You do not have sufficient privilege to view this window."),
		__FILE__, __LINE__);
    reject();
  }
  else
    XDialog::setVisible(true);
}

void externalShipping::showEvent(QShowEvent *event)
{
  if (event)
  {
    _screen->setModel(_model);
    _screen->setCurrentIndex(_idx);
    if (_mode == cNew)
      _screen->setMode(Screen::New);
    else if (_mode == cEdit)
      _screen->setMode(Screen::Edit);
    else if (_mode == cView)
      _screen->setMode(Screen::View);
  }
}

externalShipping::externalShipping(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _mode=-1;

  connect(_order, SIGNAL(newId(int)), this, SLOT(sHandleOrder()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(this, SIGNAL(rejected()), this, SLOT(sReject()));

  _order->setAllowedTypes(OrderLineEdit::Sales);
  _shipment->setType(ShipmentClusterLineEdit::SalesOrder);

  // Trolltech bug 150373, fixed in Qt 4.4: Numerics are returned as strings
  // TODO: retest in 4.4: are numerics displayed using the current locale?
#if QT_VERSION >= 0x040400
  _weight->setValidator(omfgThis->weightVal());
#endif
  
  _model = new XSqlTableModel(this);
}

externalShipping::~externalShipping()
{
    // no need to delete child widgets, Qt does it all for us
}

void externalShipping::languageChange()
{
    retranslateUi(this);
}

enum SetResponse externalShipping::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("currentIndex", &valid);
  if (valid)
    _idx=param.toInt();
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    _mode = param.toInt();
    _save->setHidden(_mode==cView);
  }

  return NoError;
}

enum SetResponse externalShipping::setModel(XSqlTableModel *model)
{
  _model=model;
  return NoError;
}

void externalShipping::sHandleOrder()
{
  if (_order->id() > 0)
  {
    _shipment->setType("SO");
    _shipment->limitToOrder(_order->id());
  }
  else
  {
    _shipment->removeOrderLimit();
    _shipment->setType("SO");
  }
}

void externalShipping::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_order->isValid(), _order,
                          tr("You must enter a valid Order Number for this External Shipping Record "
                             "before continuing"))
         << GuiErrorCheck(!_shipment->isValid(), _shipment,
                          tr("You must select a valid Shipment Number for this External Shipping Record "
                             "before continuing"))
         << GuiErrorCheck(_shipper->text().isEmpty(), _shipper,
                          tr("You must select a valid Shipper for this External Shipping Record "
                             "before continuing"))
         << GuiErrorCheck(_packnumTracknum->text().isEmpty(), _packnumTracknum,
                          tr("You must select a valid Package Tracking Number for this External Shipping Record "
                             "before continuing"))
         << GuiErrorCheck(_tracknum->text().isEmpty(), _tracknum,
                          tr("You must select a valid Tracking Number for this External Shipping Record "
                             "before continuing"))
     ;

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Shipping Record"), errors))
    return;
  else
  {
    _screen->save();
  }
  if (_screen->mode() != Screen::New)
  {
    accept();
  }
  else
  {
    _order->setFocus();
  }
}

void externalShipping::sClose()
{
  switch (_screen->check())
  {
    case Screen::Save:
      return;
    case Screen::Cancel:
      _screen->revertRow(_screen->currentIndex());
      break;
    default:
      break;
  }
  accept();
}

void externalShipping::sReject()
{
  _screen->revertRow(_screen->currentIndex());
}
