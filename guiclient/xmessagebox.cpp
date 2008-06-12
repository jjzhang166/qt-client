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


#include "xmessagebox.h"

#include <qapplication.h>

int XMessageBox::message( QWidget * parent, QMessageBox::Icon severity,
                          const QString & caption, const QString & text,
                          const QString & button0Text,
                          const QString & button1Text,
                          bool snooze, int defaultButtonNumber, int escapeButtonNumber )
{
  int snoozeButtonNumber = -1;

  int b[3];
  b[0] = 1;
  b[1] = button1Text.isEmpty() ? 0 : 2;
  b[2] = 0;

  if(snooze)
  {
    if(b[1] == 0)
      snoozeButtonNumber = 1;
    else
      snoozeButtonNumber = 2;
    defaultButtonNumber = snoozeButtonNumber;
    escapeButtonNumber = snoozeButtonNumber;
    b[snoozeButtonNumber] = 3;
  }

  int i;
  for( i=0; i<3; i++ ) {
    if ( b[i] && defaultButtonNumber == i )
      b[i] += QMessageBox::Default;
    if ( b[i] && escapeButtonNumber == i )
      b[i] += QMessageBox::Escape;
  }

  QMessageBox *mb = new QMessageBox( caption, text, severity,
                                     b[0], b[1], b[2],
                                     parent, "openmfg_msgbox_snooze", TRUE);
  Q_CHECK_PTR( mb );
  if ( !button0Text.isEmpty() )
    mb->setButtonText( 1, button0Text );
  if ( !button1Text.isEmpty() )
    mb->setButtonText( 2, button1Text );
  if ( snooze )
    mb->setButtonText( 3, QObject::tr("Snooze") );

#ifndef QT_NO_CURSOR
  mb->setCursor( Qt::arrowCursor );
#endif
  int result = -1;
  do {
    QApplication::beep();
    result = mb->exec();
  } while(result == 3);

  delete mb;

  return result - 1;
}

