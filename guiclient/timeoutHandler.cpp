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

/*
 *     This class attaches itself to the qApp event filter
 * and listens for events to come across.  In addition it
 * listens for a timer that fires once a minute and no user
 * generated events have occured in an amount of time specified
 * by the user preferences value then a dialog will be popped up
 * notifing the user that they have 60 secs to cancel the shutdown
 * Sequence or their client will be terminated.
 */

#include "timeoutHandler.h"

#include <qapplication.h>
//Added by qt3to4:
#include <QEvent>

/*
 * This will initialize the TimeoutHandler which will be disabled
 * at first. You must connect the timeout signal to a slot in your
 * application then call setIdleMinutes() with a value greater than
 * zero to make the TimeoutHandler Active.
 */
TimeoutHandler::TimeoutHandler(QObject* parent, const char* name)
  : QObject(parent, name), _timer(this) {
    _timeoutValue = 0; // 0ms (disabled)
    // this will keep track of our elapsed time
    _lastEvent.start();

    // connect the timer to ourselves
    connect(&_timer, SIGNAL(timeout()), this, SLOT(sCheckTimeout()));

    // install this object into the applications
    // event filter so that we can see everything
    // that happens
    qApp->installEventFilter(this);
}

/*
 * Virtual Deconstructor
 */
TimeoutHandler::~TimeoutHandler() {
    if(_timer.isActive())
        _timer.stop();
}

/*
 * Returns true if this TimeoutHandler object is currently
 * active; otherwise, false will be returned
 */
bool TimeoutHandler::isActive() {
    return _timer.isActive();
}

/*
 * Returns the number of minutes a user can be idle
 * before the timeout() signal will be issued.
 */
int TimeoutHandler::idleMinutes() {
    if(_timeoutValue == 0)
        return 0;
    return ((_timeoutValue / 1000) / 60);
}

/*
 * Sets the number of minutes a user can idle before
 * the timeout() signal will be issued. If you pass
 * in a value of zero (0) or less it will disable the
 * TimeoutHandler class and no signals will be generated.
 */
void TimeoutHandler::setIdleMinutes(int minutes) {
    if(minutes < 1) {
        _timeoutValue = 0;
        if(_timer.isActive())
            _timer.stop();
    } else {
        _timeoutValue = minutes * 60000; // convert from minutes to milliseconds
        if(!_timer.isActive())
            _timer.start(60000);
    }
    _lastEvent.restart();
}

/*
 * This is the overrided method from QObject that receives the event
 * notifications that we setup to listen for (qApp) and on certain
 * events will reset the _lastEvent QTime object to the current time.
 */
bool TimeoutHandler::eventFilter(QObject* /*watched*/, QEvent* event) {
    QEvent::Type eType = event->type();
    if( eType == QEvent::MouseButtonPress || eType == QEvent::KeyPress ) {
        _lastEvent.restart(); // reset our elapsed timer
    }

    // ALWAYS RETURN FALSE so that the event gets
    // passed on to the next widget for processing
    return false;
}

/*
 * This slot is called by the QTimer object _timer each minute
 * and compares the elapsed time of from now and the last event
 * to the _timeoutValue.  If the elapsed time exceeds the _timeoutValue
 * the signal timeout() will be issued.
 *
 * NOTE: The elapsed time value is not reset here. If the TimeoutHandler
 *       is not manually reset or if no Events are received before the next
 *       time the QTimer object _timer calls this function then timeout()
 *       will be issued again..
 */
void TimeoutHandler::sCheckTimeout() {
    if(_lastEvent.elapsed() > _timeoutValue) {
        // the user is at or over the whatever idle time they
        // have been allowed so now we need to deal with it
        timeout();
    }
}

/*
 * Resets the QTime object value _lastEvent.
 */
void TimeoutHandler::reset() {
    _lastEvent.restart();
}

