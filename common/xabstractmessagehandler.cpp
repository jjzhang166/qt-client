/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xabstractmessagehandler.h"

XAbstractMessageHandler::XAbstractMessageHandler(QObject *parent)
  : QAbstractMessageHandler(parent),
    _acceptDefaults(false)
{
}

XAbstractMessageHandler::~XAbstractMessageHandler()
{
}

bool XAbstractMessageHandler::acceptDefaults() const
{
  return _acceptDefaults;
}

/** Set whether or not the message handler should automatically accept
    the defaultButton when ::question() is called. This is initially
    set to @c false.

    @param accept @c true indicates yes, accept the default value
                  @c false indicates no, ask the question and let the user decide
    @return the previous setting
*/
bool XAbstractMessageHandler::setAcceptDefaults(const bool accept)
{
  bool result = _acceptDefaults;
  _acceptDefaults = accept;
  return result;
}
