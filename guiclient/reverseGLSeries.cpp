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

#include "reverseGLSeries.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a reverseGLSeries as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
reverseGLSeries::reverseGLSeries(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
reverseGLSeries::~reverseGLSeries()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void reverseGLSeries::languageChange()
{
    retranslateUi(this);
}

void reverseGLSeries::init()
{
  _glseries = -1;
}

enum SetResponse reverseGLSeries::set( const ParameterList & pParams )
{
  QVariant param;
  bool valid = false;
  
  param = pParams.value("glseries", &valid);
  if(valid)
  {
    _glseries = param.toInt();
    
    q.prepare("SELECT gltrans_journalnumber, gltrans_date "
              "FROM gltrans "
              "WHERE (gltrans_sequence=:glseries); " );
    q.bindValue(":glseries", _glseries);
    q.exec();
    if(q.first())
    {
      _journalNum->setText(q.value("gltrans_journalnumber").toString());
      _distDate->setDate(q.value("gltrans_date").toDate());
    }
    else
    {
      systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
      return UndefinedError;
    }
  }
  
  return NoError;
}

void reverseGLSeries::sPost()
{
  if(!_distDate->isValid())
  {
    QMessageBox::warning(this, tr("Cannot Reverse Series"),
                         tr("A valid distribution date must be entered before the G/L Series can be reversed.") );
    _distDate->setFocus();
    return;
  }

  if(_metrics->boolean("MandatoryGLEntryNotes") && _notes->text().stripWhiteSpace().isEmpty())
  {
    QMessageBox::information( this, tr("Cannot Post G/L Series"),
                                    tr("You must enter some Notes to describe this transaction.") );
    _notes->setFocus();
    return;
  }

  
  q.prepare("SELECT reverseGLSeries(:glseries, :distdate, :notes) AS result;");
  q.bindValue(":glseries", _glseries);
  q.bindValue(":distdate", _distDate->date());
  q.bindValue(":notes", _notes->text());
  q.exec();
  if(q.first())
  {
    int result = q.value("result").toInt();
    if(result < 0)
    {
      switch(result)
      {
      case -1:
      default:
        QMessageBox::warning( this, tr("Error Reversing G/L Series"),
                              tr("An Unknown Error was encountered while reversing the G/L Series.") );
      }
      return;
    }
  }
  else
    systemError( this, tr("A System Error occurred at reverseGLSeries::%1.")
                       .arg(__LINE__) ); 
  
  accept();
}

